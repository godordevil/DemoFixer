/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2011 GodOrDevil

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "stdafx.h"
//#include "common.h"
//#include "huffman.h"
//#include "msg.h"
#include <stdio.h>
#include <assert.h>

#include "main.h"

#include <iostream>
#include <windows.h>
#include <iomanip>

using namespace std;

extern HANDLE hConsole; 

// snapped vectors are packed in 13 bits instead of 32
#define SNAPPED_BITS		13
#define MAX_SNAPPED			(1<<SNAPPED_BITS)

int	overflows;
int oldsize			= 0;
static huffman_t	msgHuff;
static qboolean		msgInit = qfalse;


// field data accessing
#define FIELD_INTEGER(s)	(*(int   *)((byte *)(s)+field->offset))
#define FIELD_FLOAT(s)		(*(float *)((byte *)(s)+field->offset))


#define	FLOAT_INT_BITS	13
#define	FLOAT_INT_BIAS	(1<<(FLOAT_INT_BITS-1))

static const entityState_t	nullEntityState;

typedef struct {
	char	*name;
	int		offset;
	int		bits;		// 0 = float
} netField_t;

#define	NETF(x) #x,(int)&((entityState_t*)0)->x
#define	PSF(x) #x,(int)&((playerState_t*)0)->x

#define ANIM_BITS 10
#define ETYPE_BITS 8
#define EFLAGS_BITS 23  // was 21; updated to 23 in v1.4, fixing errors in mp_pra1 dom maps. 
#define MSOUND_BITS 6
#define SOUND_BITS 8
#define EVENPARMS_BITS 0

netField_t	entityStateFields[] = 
{
{ NETF(pos.trTime), 32 },
{ NETF(pos.trBase[0]), 0 },
{ NETF(pos.trBase[1]), 0 },
{ NETF(pos.trDelta[0]), 0 },
{ NETF(pos.trDelta[1]), 0 },

{ NETF(pos.trBase[2]), 0 },
{ NETF(apos.trBase[1]), 0 },
{ NETF(pos.trDelta[2]), 0 },
{ NETF(apos.trBase[0]), 0 },
{ NETF(event), 10},

{ NETF(angles2[1]), 0 },
{ NETF(eType), ETYPE_BITS },
{ NETF(torsoAnim), ANIM_BITS},
{ NETF(torsoTimer), -16},
{ NETF(eventParm), EVENPARMS_BITS},

{ NETF(legsAnim), ANIM_BITS },
{ NETF(groundEntityNum), GENTITYNUM_BITS },
{ NETF(pos.trType), 8 },
{ NETF(eFlags), EFLAGS_BITS},
{ NETF(otherEntityNum), GENTITYNUM_BITS },

{ NETF(weapon), 8 },
{ NETF(clientNum), 8 },
{ NETF(angles[1]), 0 },
{ NETF(pos.trDuration), 32}, 
{ NETF(apos.trType), 8 },

{ NETF(origin[0]), 0 },
{ NETF(origin[1]), 0 },
{ NETF(origin[2]), 0 },
{ NETF(solid), 24 }, 
{ NETF(gametypeitems), 6},

{ NETF(modelindex), 8 },
{ NETF(otherEntityNum2), GENTITYNUM_BITS },
{ NETF(loopSound), SOUND_BITS},
{ NETF(generic1), 8 },
{ NETF(mSoundSet), MSOUND_BITS},

{ NETF(origin2[2]), 0 },
{ NETF(origin2[0]), 0 },
{ NETF(origin2[1]), 0 },
{ NETF(modelindex2), 8 },
{ NETF(angles[0]), 0 },

{ NETF(time), 32 },
{ NETF(apos.trTime), 32 },
{ NETF(apos.trDuration), 32 },
{ NETF(apos.trBase[2]), 0 },
{ NETF(apos.trDelta[0]), 0 },

{ NETF(apos.trDelta[1]), 0 },
{ NETF(apos.trDelta[2]), 0 },
{ NETF(time2), 32 },
{ NETF(angles[2]), 0 },
{ NETF(angles2[0]), 0 },

{ NETF(angles2[2]), 0 },
{ NETF(frame), 16 },
{ NETF(leanOffset), 2 },
};


static const int esTableSize = sizeof( entityStateFields ) / sizeof( entityStateFields[0] );
static int esCount = 0;
#define MAX_ES (sizeof( entityStateFields ) / sizeof( entityStateFields[0] ))

void MSG_ReadDeltaEntity( sizebuf_t *msg, const entityState_t *from, entityState_t *to, int number ) {
	netField_t	*field;
	int		maxFieldNum;
	int		i;
	int		print = 0;

#ifdef _DEBUG	
	char s[1024] = "\0";
	print = 1;
#endif

	if( number < 0 || number >= MAX_GENTITIES ) {
		SetConsoleTextAttribute(hConsole, 12);
		cout << "MSG_ReadDeltaEntity: Bad delta entity number: "<<  number << endl;
		system("pause");
		exit( EXIT_FAILURE );
	}
	if( !to ) {
		return;
	}

	if( MSG_ReadBits( msg, 1 ) ) { 
		memset( to, 0, sizeof( *to ) );
		to->number = ENTITYNUM_NONE;
		return;	// removed	
	}

	if( !from ) {
		memset( to, 0, sizeof( *to ) ); // nodelta update
	} else {
		memcpy( to, from, sizeof( *to ) );
	}
	to->number = number;

	if( !MSG_ReadBits( msg, 1 ) ) {
		return; // unchanged
	}

	maxFieldNum = MSG_ReadByte( msg );

	if( maxFieldNum > esTableSize ) {
		SetConsoleTextAttribute(hConsole, 12);
		cout << "MSG_ReadDeltaEntity: maxFieldNum > esTableSize" << endl;
		system("pause");
		exit( EXIT_FAILURE );
	}

	for( i=0, field=entityStateFields ; i<maxFieldNum ; i++, field++ ) 
	{		
		if(i == 0 && print)
		{			
			SetConsoleTextAttribute(hConsole, 15);
			cout << "\n====================================\n ENTITY:  " << number << "\n====================================" << endl;
			SetConsoleTextAttribute(hConsole, 7);
			esCount++;
		}
		if( !MSG_ReadBits( msg, 1 ) ) {
			continue; // field unchanged
		}

		if( !MSG_ReadBits( msg, 1 ) ) {
			FIELD_INTEGER( to ) = 0;
			continue; // field set to zero
		}

		if( field->bits ) {
			FIELD_INTEGER( to ) = MSG_ReadBits( msg, field->bits );	
#ifdef _DEBUG	
			if(print)
			{	
				//sprintf( s, "[%-2i] %-18s: %-14i", i, field->name, FIELD_INTEGER( to ) );
				sprintf( s, " %-18s: %-14i", field->name, FIELD_INTEGER( to ) );
				cout << s << endl;
			}
#endif
			continue;	// integer value
		}

		// read packed float value
		if( !MSG_ReadBits( msg, 1 ) ) {
			FIELD_FLOAT( to ) = (float)(MSG_ReadBits( msg, SNAPPED_BITS ) - MAX_SNAPPED/2);				
#ifdef _DEBUG	
			if(print)
			{
				//sprintf( s, "[%-2i] %-18s: %-14.1f", i, field->name, FIELD_FLOAT( to ) );
				sprintf( s, " %-18s: %-14.1f", field->name, FIELD_FLOAT( to ) );
				cout << s << endl;
			}
#endif
		} else {
			FIELD_INTEGER( to ) = MSG_ReadLong( msg );
#ifdef _DEBUG	
			if(print)
			{		
				//sprintf( s, "[%-2i] %-18s: %-14i", i, field->name, FIELD_INTEGER( to ) );
				sprintf( s, " %-18s: %-14i", field->name, FIELD_INTEGER( to ) );				
				cout << s << endl;	
			}
#endif
		}
	}	
	
	if(print)	
	{		
		SetConsoleTextAttribute(hConsole, 15);
		cout << "------------------------------------" << endl;	
	}
}


netField_t	playerStateFields[] = 
{
{ PSF(commandTime), 32 },				
{ PSF(origin[0]), 0 },		// not found
{ PSF(origin[1]), 0 },		// not found
{ PSF(bobCycle), 8 },
{ PSF(velocity[0]), 0 },
{ PSF(velocity[1]), 0 },
{ PSF(viewangles[1]), 0 },
{ PSF(viewangles[0]), 0 },
{ PSF(weaponTime), -16 },

//  +  // all added [check bits] (also unknown if origin is above or below.. put it below)
{ PSF(weaponAnimTime), -16 },
{ PSF(weaponFireBurstCount), 4 },
{ PSF(weaponAnimId), 10 },
{ PSF(weaponAnimIdChoice), 8 },
{ PSF(weaponCallbackTime), -16 },
{ PSF(weaponCallbackStep), 8 },
// end

{ PSF(origin[2]), 0 },		// not found
{ PSF(velocity[2]), 0 },
{ PSF(pm_time), -16 },
{ PSF(eventSequence), 16 },
{ PSF(torsoAnim), 8 },		// not found

// wasnt there, but might be due to duplicates not showing
{ PSF(torsoTimer),  16 }, // + [check bits]
//

{ PSF(movementDir), 4 },
{ PSF(events[0]), 8 },
{ PSF(legsAnim), 8 },		// not found
{ PSF(events[1]), 8 },
{ PSF(events[2]), 8 },		 // + [check bits]
{ PSF(events[3]), 8 },		 // + [check bits]
{ PSF(pm_flags), 16 },
{ PSF(pm_debounce), 16 },	// + [check bits]
{ PSF(groundEntityNum), GENTITYNUM_BITS },  // not found
{ PSF(weaponstate), 4 },
{ PSF(eFlags), 16 },		// not found
{ PSF(externalEvent), 10 },
{ PSF(speed), 16 },
{ PSF(delta_angles[1]), 16 },
{ PSF(externalEventParm), 8 },
{ PSF(viewheight), -8 },
{ PSF(damageEvent), 8 },
{ PSF(damageYaw), 8 },
{ PSF(damagePitch), 8 },
{ PSF(damageCount), 8 },
{ PSF(generic1), 8 },		// not found

//  +  // all added [check bits] (also unknown if generic1 is above or below.. put it below)
{ PSF(inaccuracy), 32 },			// + [check bits]
{ PSF(inaccuracyTime), 32 },	// + [check bits]
{ PSF(kickPitch), 8 },			// + [check bits]
// end

//{ PSF(generic1), 8 },		// not found
{ PSF(pm_type), 8 },					
{ PSF(delta_angles[0]), 16 },
{ PSF(delta_angles[2]), 16 },
{ PSF(eventParms[0]), 8 },
{ PSF(eventParms[1]), 8 },

{ PSF(eventParms[2]), 8 },	// + [check bits]
{ PSF(eventParms[3]), 8 },	// + [check bits]

{ PSF(clientNum), 8 },		// not found
{ PSF(weapon), 5 },			// not found
{ PSF(viewangles[2]), 0 },

//  +  // all added [check bits]
{ PSF(zoomTime), 32 },
{ PSF(zoomFov), 8 }, 
{ PSF(leanTime), 32 },
{ PSF(grenadeTimer), 32 },	
{ PSF(respawnTimer), 32 },	
{ PSF(loopSound), 16 }		// not found
//end
};

static const int psTableSize = sizeof( playerStateFields ) / sizeof( playerStateFields[0] );
static const playerState_t	nullPlayerState;
static int psCount = 0;
#define MAX_PS psTableSize

#define _DEBUGPRINT
#define MAX_GAMETYPE_ITEMS		5
//#define _SAVEPRINT
extern FILE *outPut;

void DCom_Printf( const char *text, ... ) {
#ifdef _DEBUGPRINT

	va_list		argptr;
	char		buffer[MAXPRINTMSG];

	va_start( argptr, text );
	vsprintf( buffer, text, argptr );
	va_end( argptr );

	printf( "%s", buffer );
	
#ifdef _SAVEPRINT
		fputs (buffer,outPut);
#endif

#endif
}
void Print_ps(void)
{
#ifdef DEBUG_PS

	int i;
	netField_t	*field;
	
	//if(!psParsed)
	//	return;
	DCom_Printf( "\n\n========================================================\n Player State Fields USED\n========================================================\n" );

	for (i = 0, field=playerStateFields; i <= MAX_PS; i++, field++)
	{
		//if(!psUsed[i].used)
		//	continue;
		if(!field->bits)
			continue;
		//esUsed[i].name = field->name;
		DCom_Printf( "[%-2i] %-20s %-6i  %i\n", i, field->name, field->bits, field->offset );
	}
#endif
}

void MSG_ReadDeltaPlayerstate( sizebuf_t *msg, const playerState_t *from, playerState_t *to ) {
	//field_t	*field;
	netField_t	*field;
	int		maxFieldNum;
	int		bitmask;
	int		i;

	if( !to ) {
		return;
	}
	DCom_Printf( "\n <===== START playerStateFields =====>\n" );

	if( !from ) {
		memset( to, 0, sizeof( *to ) ); // nodelta update
	} else {
		memcpy( to, from, sizeof( *to ) );
	}

	maxFieldNum = MSG_ReadByte( msg );
	if( maxFieldNum > psTableSize ) {
		Com_Error( ERR_DROP, "MSG_ReadDeltaPlayerstate: maxFieldNum > psTableSize" );
	}

	//
	// read all modified fields
	//
	//for( i=0, field=psTable ; i<maxFieldNum ; i++, field++ ) {	
	for( i=0, field=playerStateFields ; i<maxFieldNum ; i++, field++ ) {
		if( !MSG_ReadBits( msg, 1 ) ) {
		//	DCom_Printf( "%-18s: %-12i  <== UNCHANGED\n", field->name, FIELD_INTEGER( to ) );
			continue; // field unchanged
		}

		if( field->bits ) {
			FIELD_INTEGER( to ) = MSG_ReadBits( msg, field->bits );
			DCom_Printf( "%-18s: %-12i  <== int\n", field->name, FIELD_INTEGER( to ) );
			continue;	// integer value
		}

		//
		// read packed float value
		//
		if( !MSG_ReadBits( msg, 1 ) ) {
			FIELD_FLOAT( to ) = (float)(MSG_ReadBits( msg, SNAPPED_BITS ) - MAX_SNAPPED/2);		
			DCom_Printf( "%-18s: %-12i  <== float\n", field->name, FIELD_FLOAT( to ) );
		} else {
			FIELD_INTEGER( to ) = MSG_ReadLong( msg );		
			DCom_Printf( "%-18s: %-12i  <== float/int\n", field->name, FIELD_INTEGER( to ) );
		}
	}	


	//
	// read all modified arrays
	//
	if( !MSG_ReadBits( msg, 1 ) ) {
		return; // no arrays modified
	}

	// PS_STATS
	if( MSG_ReadBits( msg, 1 ) ) { 
		bitmask = MSG_ReadShort( msg );
		for( i=0 ; i<MAX_STATS ; i++ ) {
			if( bitmask & (1 << i) ) {
				to->stats[i] = MSG_ReadSignedShort( msg ); // PS_STATS can be negative
			}
		}
	}

	// PS_PERSISTANT
	if( MSG_ReadBits( msg, 1 ) ) {
		bitmask = MSG_ReadShort( msg );
		for( i=0 ; i<MAX_PERSISTANT ; i++ ) {
			if( bitmask & (1 << i) ) {
				to->persistant[i] = MSG_ReadSignedShort( msg ); // PS_PERSISTANT can be negative
			}
		}
	}

	// PS_AMMO
	if( MSG_ReadBits( msg, 1 ) ) {
		bitmask = MSG_ReadShort( msg );
		for( i=0 ; i<MAX_WEAPONS ; i++ ) {
			if( bitmask & (1 << i) ) {
				to->ammo[i] = MSG_ReadShort( msg );
			}
		}
	}

	// PS_POWERUPS
	if( MSG_ReadBits( msg, 1 ) ) {
		bitmask = MSG_ReadShort( msg );
		for( i=0 ; i<MAX_AMMO ; i++ ) {
			if( bitmask & (1 << i) ) {
				to->ammo[i] = MSG_ReadLong( msg ); // WARNING: powerups use 32 bits, not 16
			}
		}
	}
	DCom_Printf( "\n <===== START MAX_GAMETYPE_ITEMS =====>\n");

	if( MSG_ReadBits( msg, 1 ) ) {
		bitmask = MSG_ReadShort( msg );
		for( i=0 ; i<MAX_GAMETYPE_ITEMS ; i++ ) {
			if( bitmask & (1 << i) ) {
				to->ammo[i] = MSG_ReadLong( msg ); // WARNING: powerups use 32 bits, not 16
				DCom_Printf( "\n\nMAX_GAMETYPE_ITEMS %i\n\n", to->ammo[i] );
			}
		}
	}
	DCom_Printf( " <===== END MAX_GAMETYPE_ITEMS =====>\n\n");

	DCom_Printf( " <===== END playerStateFields =====>\n\n");

}


void MSG_ReadDeltaPlayerstate2( sizebuf_t *msg, const playerState_t *from, playerState_t *to ) {
	//field_t	*field;
	netField_t	*field;
	int		maxFieldNum;
	int		bitmask;
	int		i;
	int		found = 0;
	int		print = 0;
	int			startBit, endBit;
	playerState_t	dummy;

#if 1
	print = 1;
#endif

	if ( !from ) {
		from = &dummy;
		Com_Memset( &dummy, 0, sizeof( dummy ) );
	}
	*to = *from;

	if ( msg->bit == 0 ) {
		startBit = msg->readcount * 8 - GENTITYNUM_BITS;
	} else {
		startBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
	}

	maxFieldNum = MSG_ReadByte( msg );
	if( maxFieldNum > psTableSize ) {
		Com_Error( ERR_DROP, "MSG_ReadDeltaPlayerstate: maxFieldNum > psTableSize" );
	}

	//
	// read all modified fields
	//
	//for( i=0, field=psTable ; i<maxFieldNum ; i++, field++ ) {	
	for( i=0, field=playerStateFields ; i<maxFieldNum ; i++, field++ ) 
	{
		//psUsed[i].name = field->name;
//		psParsed = 1;

		if(i == 0 && print)
		{
			DCom_Printf( "\n========================================================\n  START playerStateFields = SIZE: %i [%i] ===  COUNT: %i  \n========================================================\n", msg->cursize, msg->readcount, psCount++ );
			DCom_Printf( "maxFieldNum  %i\n-----------------\n", maxFieldNum, entityStateFields[maxFieldNum].name );	
		}
			
		//DCom_Printf( "%-18s: <== FIELD\n", field->name );
		found = 1;
		if( !MSG_ReadBits( msg, 1 ) ) {
			//if(print)
			//	DCom_Printf( "%-20s: %-14i  <== UNCHANGED\n", field->name, FIELD_INTEGER( to ) );
			continue; // field unchanged
		}

		//psUsed[i].used = 1;

		if( field->bits ) {
			FIELD_INTEGER( to ) = MSG_ReadBits( msg, field->bits );
			if(print)
				DCom_Printf( "%-20s: %-14i  <== int\n", field->name, FIELD_INTEGER( to ) );
			continue;	// integer value
		}

		//
		// read packed float value
		//
		if( !MSG_ReadBits( msg, 1 ) ) {
			FIELD_FLOAT( to ) = (float)(MSG_ReadBits( msg, SNAPPED_BITS ) - MAX_SNAPPED/2);		
			if(print)
				DCom_Printf( "%-20s: %-14i  <== float\n", field->name, FIELD_FLOAT( to ) );
		} else {
			FIELD_INTEGER( to ) = MSG_ReadLong( msg );		
			if(print)
				DCom_Printf( "%-20s: %-14i  <== float/int\n", field->name, FIELD_INTEGER( to ) );
		}
	}	
	if(found && print)
		DCom_Printf( "========================================================\n  End playerStateFields\n\n", msg->cursize, msg->readcount );


	// read the arrays
	if ((i = MSG_ReadBits( msg, 1 )) ) 
	{
		// parse stats
		if ((i = MSG_ReadBits( msg, 1 )) ) 
		{
			bitmask = MSG_ReadShort (msg);
			for (i=0 ; i<16 ; i++) {
				if(print)
					DCom_Printf( "    %-18i\n", to->stats[i]);
				to->stats[i] = MSG_ReadShort(msg);
				}
			}
		}

		// parse persistant stats
		if ((i = MSG_ReadBits( msg, 1 )) ) 
		{
			bitmask = MSG_ReadShort (msg);
			for (i=0 ; i<16 ; i++) 
			{
				if(print)
					DCom_Printf( "    %-18i\n", to->stats[i]);
				to->persistant[i] = MSG_ReadShort(msg);
				
			}
		}

		// parse ammo
		if ((i = MSG_ReadBits( msg, 1 )) ) 
		{
			bitmask = MSG_ReadShort (msg);
			for (i=0 ; i<16 ; i++) {
			if( bitmask & (1 << i) ) {
				if(print)
					DCom_Printf( "    %-18i\n", to->stats[i]);
				to->ammo[i] = MSG_ReadShort(msg);
				}
			}
		}
#ifndef _Q68
		// parse powerups
		if ((i = MSG_ReadBits( msg, 1 )) ) 
		{
			bitmask = MSG_ReadShort (msg);
			for (i=0 ; i<MAX_GAMETYPE_ITEMS ; i++) 
			{
				if( bitmask & (1 << i) ) 
				{
					if(print)
						DCom_Printf( "    %-18i\n", to->stats[i]);
					to->ammo[i] = MSG_ReadLong(msg);
				}
			}
		}
#else
		// parse powerups
		if ((i = MSG_ReadBits( msg, 1 )) ) 
		{
			bitmask = MSG_ReadShort (msg);
			for (i=0 ; i<16 ; i++) {
			if( bitmask & (1 << i) ) {
				if(print)
					DCom_Printf( "    %-18i\n", to->stats[i]);
				to->powerups[i] = MSG_ReadLong(msg);
				}
			}
		}
#endif

	if ( print ) {
		if ( msg->bit == 0 ) {
			endBit = msg->readcount * 8 - GENTITYNUM_BITS;
		} else {
			endBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
		}
		Com_Printf( " (%i bits)\n", endBit - startBit  );
	}

}




void MSG_WriteDeltaEntity( sizebuf_t *msg, struct entityState_s *from, struct entityState_s *to, qboolean force ) {
	int			i, lc;
	int			numFields;
	netField_t	*field;
	int			trunc;
	float		fullFloat;
	int			*fromF, *toF;

	numFields = sizeof(entityStateFields)/sizeof(entityStateFields[0]);

	// all fields should be 32 bits to avoid any compiler packing issues
	// the "number" field is not part of the field list
	// if this assert fails, someone added a field to the entityState_t
	// struct without updating the message fields
	assert( numFields + 1 == sizeof( *from )/4 );

	// a NULL to is a delta remove message
	if ( to == NULL ) {
		if ( from == NULL ) {
			return;
		}
		MSG_WriteBits( msg, from->number, GENTITYNUM_BITS );
		MSG_WriteBits( msg, 1, 1 );
		return;
	}

	if ( to->number < 0 || to->number >= MAX_GENTITIES ) {
		SetConsoleTextAttribute(hConsole, 12);
		cout << "MSG_WriteDeltaEntity: Bad entity number: " << to->number << endl;
		system("pause");
		exit( EXIT_FAILURE );
	}

	lc = 0;
	// build the change vector as bytes so it is endien independent
	for ( i = 0, field = entityStateFields ; i < numFields ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		if ( *fromF != *toF ) {
			lc = i+1;
		}
	}

	if ( lc == 0 ) {
		// nothing at all changed
		if ( !force ) {
			return;		// nothing at all
		}
		// write two bits for no change
		MSG_WriteBits( msg, to->number, GENTITYNUM_BITS );
		MSG_WriteBits( msg, 0, 1 );		// not removed
		MSG_WriteBits( msg, 0, 1 );		// no delta
		return;
	}

	MSG_WriteBits( msg, to->number, GENTITYNUM_BITS );
	MSG_WriteBits( msg, 0, 1 );			// not removed
	MSG_WriteBits( msg, 1, 1 );			// we have a delta

	MSG_WriteByte( msg, lc );	// # of changes

	oldsize += numFields;

	for ( i = 0, field = entityStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if ( *fromF == *toF ) {
			MSG_WriteBits( msg, 0, 1 );	// no change
			continue;
		}

		MSG_WriteBits( msg, 1, 1 );	// changed

		if ( field->bits == 0 ) {
			// float
			fullFloat = *(float *)toF;
			trunc = (int)fullFloat;

			if (fullFloat == 0.0f) {
					MSG_WriteBits( msg, 0, 1 );
					oldsize += FLOAT_INT_BITS;
			} else {
				MSG_WriteBits( msg, 1, 1 );
				if ( trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 && 
					trunc + FLOAT_INT_BIAS < ( 1 << FLOAT_INT_BITS ) ) {
					// send as small integer
					MSG_WriteBits( msg, 0, 1 );
					MSG_WriteBits( msg, trunc + FLOAT_INT_BIAS, FLOAT_INT_BITS );
				} else {
					// send as full floating point value
					MSG_WriteBits( msg, 1, 1 );
					MSG_WriteBits( msg, *toF, 32 );
				}
			}
		} else {
			if (*toF == 0) {
				MSG_WriteBits( msg, 0, 1 );
			} else {
				MSG_WriteBits( msg, 1, 1 );
				// integer
				MSG_WriteBits( msg, *toF, field->bits );
			}
		}
	}
}
void MSG_Init (sizebuf_t * buf, byte * data, int length)
{
  if (!msgInit)
    {
      MSG_initHuffman ();
    }
  Com_Memset (buf, 0, sizeof (*buf));
  buf->data = data;
  buf->maxsize = length;
  buf->oob = qtrue;
}
void MSG_Bitstream (sizebuf_t * buf)
{
  buf->oob = qfalse;
}
int MSG_ReadBits (sizebuf_t * msg, int bits)
{
  int value;
  int get;
  qboolean sgn;
  int i, nbits;
   //FILE*   fp;

  value = 0;

  if (bits < 0)
    {
      bits = -bits;
      sgn = qtrue;
    }
  else
    {
      sgn = qfalse;
    }

  if (msg->oob)
    {
      if (bits == 8)
        {
          value = msg->data[msg->readcount];
          msg->readcount += 1;
          msg->bit += 8;
        }
      else if (bits == 16)
        {
          unsigned short *sp = (unsigned short *) &msg->data[msg->readcount];
          value = LittleShort (*sp);
          msg->readcount += 2;
          msg->bit += 16;
        }
      else if (bits == 32)
        {
          unsigned int *ip = (unsigned int *) &msg->data[msg->readcount];
          value = LittleLong (*ip);
          msg->readcount += 4;
          msg->bit += 32;
        }
      else
        {
			SetConsoleTextAttribute(hConsole, 12);
			cout << "can't read bits: " << bits;
			system("pause");
			exit( EXIT_FAILURE );
        }
    }
  else
    {
      nbits = 0;
      if (bits & 7)
        {
          nbits = bits & 7;
          for (i = 0; i < nbits; i++)
            {
              value |= (Huff_getBit (msg->data, &msg->bit) << i);
            }
          bits = bits - nbits;
        }
      if (bits)
        {
          //            fp = fopen("netchan.bin", "a");
          for (i = 0; i < bits; i += 8)
            {
              Huff_offsetReceive (msgHuff.decompressor.tree, &get, msg->data,
                                  &msg->bit);
            //                  fwrite(&get, 1, 1, fp);
              value |= (get << (i + nbits));
            }
            //          fclose(fp);
        }
      msg->readcount = (msg->bit >> 3) + 1;
    }
  if (sgn)
    {
      if (value & (1 << (bits - 1)))
        {
          value |= -1 ^ ((1 << bits) - 1);
        }
    }

  return value;
}
int MSG_ReadByte (sizebuf_t * msg)
{
  int c;

  c = (unsigned char) MSG_ReadBits (msg, 8);
  if (msg->readcount > msg->cursize)
    {
      c = -1;
    }
  return c;
}
int MSG_ReadShort (sizebuf_t * msg)
{
  int c;

  c = (short) MSG_ReadBits (msg, 16);
  if (msg->readcount > msg->cursize)
    {
      c = -1;
    }

  return c;
}
int MSG_ReadSignedShort( sizebuf_t *msg ) {
	int c = MSG_ReadBits( msg, -16 );

	if( msg->readcount > msg->cursize ) {
		return -1;
	}

	return c;
}
int MSG_ReadLong (sizebuf_t * msg)
{
  int c;

  c = MSG_ReadBits (msg, 32);
  if (msg->readcount > msg->cursize)
    {
      c = -1;
    }

  return c;
}
float MSG_ReadFloat (sizebuf_t * msg)
{
  union
  {
    byte b[4];
    float f;
    int l;
  } dat;

  dat.l = MSG_ReadBits (msg, 32);
  if (msg->readcount > msg->cursize)
    {
      dat.f = -1;
    }

  return dat.f;
}
char *MSG_ReadString (sizebuf_t * msg)
{
  static char string[MAX_STRING_CHARS];
  int l, c;

  l = 0;
  do
    {
      c = MSG_ReadByte (msg);   // use ReadByte so -1 is out of bounds
      if (c == -1 || c == 0)
        {
          break;
        }
      // translate all fmt spec to avoid crash bugs
      if (c == '%')
        {
          c = '.';
        }
      // don't allow higher ascii values
      if (c > 127)
        {
          c = '.';
        }

      string[l] = c;
      l++;
    }
  while ((unsigned) l < sizeof (string) - 1);

  string[l] = 0;

  return string;
}
char *MSG_ReadBigString (sizebuf_t * msg)
{
  static char string[BIG_INFO_STRING];
  int l, c;

  l = 0;
  do
    {
      c = MSG_ReadByte (msg);   // use ReadByte so -1 is out of bounds
      if (c == -1 || c == 0)
        {
          break;
        }
      // translate all fmt spec to avoid crash bugs
      if (c == '%')
        {
          c = '.';
        }

      string[l] = c;
      l++;
    }
  while ((unsigned) l < sizeof (string) - 1);

  string[l] = 0;

  return string;
}
char *MSG_ReadStringLine (sizebuf_t * msg)
{
  static char string[MAX_STRING_CHARS];
  int l, c;

  l = 0;
  do
    {
      c = MSG_ReadByte (msg);   // use ReadByte so -1 is out of bounds
      if (c == -1 || c == 0 || c == '\n')
        {
          break;
        }
      // translate all fmt spec to avoid crash bugs
      if (c == '%')
        {
          c = '.';
        }
      string[l] = c;
      l++;
    }
  while ((unsigned) l < sizeof (string) - 1);

  string[l] = 0;

  return string;
}
void MSG_ReadData( sizebuf_t *msg, void *data, int len ) {
	int		i;
	int		c;

	for( i=0 ; i<len ; i++ ) {
		c = MSG_ReadByte( msg );
		if( c == -1 ) {
			break;
		} 
		if( data ) {
			((byte *)data)[i] = c;
		}
	}
}
void MSG_WriteBits( sizebuf_t *msg, int value, int bits ) {
	int	i;
	//FILE*	fp;

	oldsize += bits;

	// this isn't an exact overflow check, but close enough
	if ( msg->maxsize - msg->cursize < 4 ) {
		msg->overflowed = qtrue;
		return;
	}

	if ( bits == 0 || bits < -31 || bits > 32 ) {
		SetConsoleTextAttribute(hConsole, 12);
		cout << "MSG_WriteBits: bad bits: " << bits;
		system("pause");
		exit( EXIT_FAILURE );
	}

	// check for overflows
	if ( bits != 32 ) {
		if ( bits > 0 ) {
			if ( value > ( ( 1 << bits ) - 1 ) || value < 0 ) {
				overflows++;
			}
		} else {
			int	r;

			r = 1 << (bits-1);

			if ( value >  r - 1 || value < -r ) {
				overflows++;
			}
		}
	}
	if ( bits < 0 ) {
		bits = -bits;
	}
	if (msg->oob) {
		if (bits==8) {
			msg->data[msg->cursize] = value;
			msg->cursize += 1;
			msg->bit += 8;
		} else if (bits==16) {
			unsigned short *sp = (unsigned short *)&msg->data[msg->cursize];
			*sp = LittleShort(value);
			msg->cursize += 2;
			msg->bit += 16;
		} else if (bits==32) {
			unsigned int *ip = (unsigned int *)&msg->data[msg->cursize];
			*ip = LittleLong(value);
			msg->cursize += 4;
			msg->bit += 8;
		} else {
			SetConsoleTextAttribute(hConsole, 12);
			cout << "can't read bits: " << bits;
			system("pause");
			exit( EXIT_FAILURE );
		}
	} else {
		value &= (0xffffffff>>(32-bits));
		if (bits&7) {
			int nbits;
			nbits = bits&7;
			for(i=0;i<nbits;i++) {
				Huff_putBit((value&1), msg->data, &msg->bit);
				value = (value>>1);
			}
			bits = bits - nbits;
		}
		if (bits) {
			for(i=0;i<bits;i+=8) {
				Huff_offsetTransmit (&msgHuff.compressor, (value&0xff), msg->data, &msg->bit);
				value = (value>>8);
			}
		}
		msg->cursize = (msg->bit>>3)+1;
	}
}
void MSG_WriteChar( sizebuf_t *sb, int c ) {

	MSG_WriteBits( sb, c, 8 );
}
void MSG_WriteByte( sizebuf_t *sb, int c ) {

	MSG_WriteBits( sb, c, 8 );
}
void MSG_WriteData( sizebuf_t *buf, const void *data, int length ) {
	int i;
	for(i=0;i<length;i++) {
		MSG_WriteByte(buf, ((byte *)data)[i]);
	}
}
void MSG_WriteShort( sizebuf_t *sb, int c ) {
	MSG_WriteBits( sb, c, 16 );
}
void MSG_WriteLong( sizebuf_t *sb, int c ) {
	MSG_WriteBits( sb, c, 32 );
}
void MSG_WriteFloat( sizebuf_t *sb, float f ) {
	union {
		float	f;
		int	l;
	} dat;
	
	dat.f = f;
	MSG_WriteBits( sb, dat.l, 32 );
}
void MSG_WriteString( sizebuf_t *sb, const char *s ) {
	if ( !s ) {
		MSG_WriteData (sb, "", 1);
	} else {
		int		l,i;
		char	string[MAX_STRING_CHARS];

		l = strlen( s );
		if ( l >= MAX_STRING_CHARS ) {
			SetConsoleTextAttribute(hConsole, 12);
			cout << "MSG_WriteString: MAX_STRING_CHARS" << endl;
			MSG_WriteData (sb, "", 1);
			return;
		}
		strncpy( string, s, sizeof( string ) );

		// get rid of 0xff chars, because old clients don't like them
		for ( i = 0 ; i < l ; i++ ) {
			if ( ((byte *)string)[i] > 127 ) {
				string[i] = '.';
			}
		}

		MSG_WriteData (sb, string, l+1);
	}
}
void MSG_WriteBigString( sizebuf_t *sb, const char *s ) {
	if ( !s ) {
		MSG_WriteData (sb, "", 1);
	} else {
		int		l,i;
		char	string[BIG_INFO_STRING];

		l = strlen( s );
		if ( l >= BIG_INFO_STRING ) {
			SetConsoleTextAttribute(hConsole, 12);
			cout << "MSG_WriteString: BIG_INFO_STRING" << endl;
			MSG_WriteData (sb, "", 1);
			return;
		}
		strncpy( string, s, sizeof( string ) );

		// get rid of 0xff chars, because old clients don't like them
		for ( i = 0 ; i < l ; i++ ) {
			if ( ((byte *)string)[i] > 127 ) {
				string[i] = '.';
			}
		}

		MSG_WriteData (sb, string, l+1);
	}
}
void MSG_WriteAngle( sizebuf_t *sb, float f ) {
	MSG_WriteByte (sb, (int)(f*256/360) & 255);
}
void MSG_WriteAngle16( sizebuf_t *sb, float f ) {
	MSG_WriteShort (sb, ANGLE2SHORT(f));
}
void MSG_Copy(sizebuf_t *buf, byte *data, int length, sizebuf_t *src)
{
	if (length  < src->cursize) {
		SetConsoleTextAttribute(hConsole, 12);
		cout << "MSG_Copy: can't copy into a smaller msg_t buffer" << endl;
		system("pause");
		exit( EXIT_FAILURE );
	}
	Com_Memcpy(buf, src, sizeof(sizebuf_t));
	buf->data = data;
	Com_Memcpy(buf->data, src->data, src->cursize);
}
int msg_hData[256] = {
  250315,                       // 0
  41193,                        // 1
  6292,                         // 2
  7106,                         // 3
  3730,                         // 4
  3750,                         // 5
  6110,                         // 6
  23283,                        // 7
  33317,                        // 8
  6950,                         // 9
  7838,                         // 10
  9714,                         // 11
  9257,                         // 12
  17259,                        // 13
  3949,                         // 14
  1778,                         // 15
  8288,                         // 16
  1604,                         // 17
  1590,                         // 18
  1663,                         // 19
  1100,                         // 20
  1213,                         // 21
  1238,                         // 22
  1134,                         // 23
  1749,                         // 24
  1059,                         // 25
  1246,                         // 26
  1149,                         // 27
  1273,                         // 28
  4486,                         // 29
  2805,                         // 30
  3472,                         // 31
  21819,                        // 32
  1159,                         // 33
  1670,                         // 34
  1066,                         // 35
  1043,                         // 36
  1012,                         // 37
  1053,                         // 38
  1070,                         // 39
  1726,                         // 40
  888,                          // 41
  1180,                         // 42
  850,                          // 43
  960,                          // 44
  780,                          // 45
  1752,                         // 46
  3296,                         // 47
  10630,                        // 48
  4514,                         // 49
  5881,                         // 50
  2685,                         // 51
  4650,                         // 52
  3837,                         // 53
  2093,                         // 54
  1867,                         // 55
  2584,                         // 56
  1949,                         // 57
  1972,                         // 58
  940,                          // 59
  1134,                         // 60
  1788,                         // 61
  1670,                         // 62
  1206,                         // 63
  5719,                         // 64
  6128,                         // 65
  7222,                         // 66
  6654,                         // 67
  3710,                         // 68
  3795,                         // 69
  1492,                         // 70
  1524,                         // 71
  2215,                         // 72
  1140,                         // 73
  1355,                         // 74
  971,                          // 75
  2180,                         // 76
  1248,                         // 77
  1328,                         // 78
  1195,                         // 79
  1770,                         // 80
  1078,                         // 81
  1264,                         // 82
  1266,                         // 83
  1168,                         // 84
  965,                          // 85
  1155,                         // 86
  1186,                         // 87
  1347,                         // 88
  1228,                         // 89
  1529,                         // 90
  1600,                         // 91
  2617,                         // 92
  2048,                         // 93
  2546,                         // 94
  3275,                         // 95
  2410,                         // 96
  3585,                         // 97
  2504,                         // 98
  2800,                         // 99
  2675,                         // 100
  6146,                         // 101
  3663,                         // 102
  2840,                         // 103
  14253,                        // 104
  3164,                         // 105
  2221,                         // 106
  1687,                         // 107
  3208,                         // 108
  2739,                         // 109
  3512,                         // 110
  4796,                         // 111
  4091,                         // 112
  3515,                         // 113
  5288,                         // 114
  4016,                         // 115
  7937,                         // 116
  6031,                         // 117
  5360,                         // 118
  3924,                         // 119
  4892,                         // 120
  3743,                         // 121
  4566,                         // 122
  4807,                         // 123
  5852,                         // 124
  6400,                         // 125
  6225,                         // 126
  8291,                         // 127
  23243,                        // 128
  7838,                         // 129
  7073,                         // 130
  8935,                         // 131
  5437,                         // 132
  4483,                         // 133
  3641,                         // 134
  5256,                         // 135
  5312,                         // 136
  5328,                         // 137
  5370,                         // 138
  3492,                         // 139
  2458,                         // 140
  1694,                         // 141
  1821,                         // 142
  2121,                         // 143
  1916,                         // 144
  1149,                         // 145
  1516,                         // 146
  1367,                         // 147
  1236,                         // 148
  1029,                         // 149
  1258,                         // 150
  1104,                         // 151
  1245,                         // 152
  1006,                         // 153
  1149,                         // 154
  1025,                         // 155
  1241,                         // 156
  952,                          // 157
  1287,                         // 158
  997,                          // 159
  1713,                         // 160
  1009,                         // 161
  1187,                         // 162
  879,                          // 163
  1099,                         // 164
  929,                          // 165
  1078,                         // 166
  951,                          // 167
  1656,                         // 168
  930,                          // 169
  1153,                         // 170
  1030,                         // 171
  1262,                         // 172
  1062,                         // 173
  1214,                         // 174
  1060,                         // 175
  1621,                         // 176
  930,                          // 177
  1106,                         // 178
  912,                          // 179
  1034,                         // 180
  892,                          // 181
  1158,                         // 182
  990,                          // 183
  1175,                         // 184
  850,                          // 185
  1121,                         // 186
  903,                          // 187
  1087,                         // 188
  920,                          // 189
  1144,                         // 190
  1056,                         // 191
  3462,                         // 192
  2240,                         // 193
  4397,                         // 194
  12136,                        // 195
  7758,                         // 196
  1345,                         // 197
  1307,                         // 198
  3278,                         // 199
  1950,                         // 200
  886,                          // 201
  1023,                         // 202
  1112,                         // 203
  1077,                         // 204
  1042,                         // 205
  1061,                         // 206
  1071,                         // 207
  1484,                         // 208
  1001,                         // 209
  1096,                         // 210
  915,                          // 211
  1052,                         // 212
  995,                          // 213
  1070,                         // 214
  876,                          // 215
  1111,                         // 216
  851,                          // 217
  1059,                         // 218
  805,                          // 219
  1112,                         // 220
  923,                          // 221
  1103,                         // 222
  817,                          // 223
  1899,                         // 224
  1872,                         // 225
  976,                          // 226
  841,                          // 227
  1127,                         // 228
  956,                          // 229
  1159,                         // 230
  950,                          // 231
  7791,                         // 232
  954,                          // 233
  1289,                         // 234
  933,                          // 235
  1127,                         // 236
  3207,                         // 237
  1020,                         // 238
  927,                          // 239
  1355,                         // 240
  768,                          // 241
  1040,                         // 242
  745,                          // 243
  952,                          // 244
  805,                          // 245
  1073,                         // 246
  740,                          // 247
  1013,                         // 248
  805,                          // 249
  1008,                         // 250
  796,                          // 251
  996,                          // 252
  1057,                         // 253
  11457,                        // 254
  13504,                        // 255
};



void MSG_initHuffman ()
{
  int i, j;

  msgInit = qtrue;
  Huff_Init (&msgHuff);
  for (i = 0; i < 256; i++)
    {
      for (j = 0; j < msg_hData[i]; j++)
        {
          Huff_addRef (&msgHuff.compressor, (byte) i);  // Do update
          Huff_addRef (&msgHuff.decompressor, (byte) i);        // Do update
        }
    }
}





