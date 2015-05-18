/*
===========================================================================
Copyright (C) 2011 GodOrDevil

This file is part of DemoFixer source code.

DemoFixer source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

DemoFixer source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "stdafx.h"
#include "main.h"
#include "version.h"

#include <iostream>
#include <windows.h>
#include <iomanip>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

//using namespace System;
using namespace std;

demoFileState_t	demo;
demoState_t		ds;

int serverMessageSequence = 0;
int serverMessageLen = 0;
int reliableSequence = 0;
static int PATHLEN = 0;
HANDLE  hConsole =  GetStdHandle(STD_OUTPUT_HANDLE); 


typedef struct {
	entityState_t	currentState;	
	qboolean		currentValid;	// true if cg.frame holds this entity
	int				previousEvent;
} centity_t;

static char clientNames[MAX_CLIENTS][36];
static centity_t	centities[MAX_GENTITIES];


char *stripPath( char* file)
{
   static char buffer[255] = "\0";
   char *ptr = strrchr(file,'\\');
   strcpy(buffer,ptr+1);
   PATHLEN = strlen(buffer);
   if(ptr != 0)
      *ptr = 0;

   return buffer;
}
int moveFiles ( char* original)
{
	char fileOut[255];
	char filename[128];
	char path[255];	
	
	strcpy(path, original);
	strcpy(filename, stripPath(path));
	sprintf(path, "%s\\originals", path);

	_mkdir(path);
	sprintf(fileOut, "%s\\%s", path, filename);
	rename(original, fileOut);
	return 1;
}

static void Parse_DeltaEntity( sizebuf_t *msg, snapshot_t *frame, int newnum, entityState_t *old, qboolean unchanged ) 
{
	entityState_t *state;

	state = &ds.parseEntities[ds.firstParseEntity & PARSE_ENTITIES_MASK];

	if( unchanged ) {
		memcpy( state, old, sizeof( *state ) ); // don't read any bits
	} else {
		MSG_ReadDeltaEntity( msg, old, state, newnum );
		if( state->number == ENTITYNUM_NONE ) {
			// the entity present in oldframe is not in the current frame
			return;
		}
	}

	ds.firstParseEntity++;
	frame->numEntities++;
}
static void Parse_PacketEntities( sizebuf_t *msg, snapshot_t *oldframe, snapshot_t *newframe )
{
	int			newnum;
	entityState_t	*oldstate;
	int			oldindex, oldnum;

	newframe->firstEntity = ds.firstParseEntity;
	newframe->numEntities = 0;

	// delta from the entities present in oldframe
	oldindex = 0;
	if( !oldframe ) {
		oldnum = 99999;
	} else {
		if( oldindex >= oldframe->numEntities ) {
			oldnum = 99999;
		} else {
			oldstate = &ds.parseEntities[(oldframe->firstEntity + oldindex) & PARSE_ENTITIES_MASK];
			oldnum = oldstate->number;
		} 
	}

	while( 1 ) {
		newnum = MSG_ReadBits( msg, GENTITYNUM_BITS );
		if( newnum < 0 || newnum >= MAX_GENTITIES ) {
			Com_Error( ERR_DROP, "Parse_PacketEntities: bad number %i", newnum );
		}

		if( msg->readcount > msg->cursize ) {
			Com_Error( ERR_DROP, "Parse_PacketEntities: end of message" );
		}

		if( newnum == ENTITYNUM_NONE ) {
			break; // end of packetentities
		}

		while( oldnum < newnum ) {
			// one or more entities from the old packet are unchanged
			Parse_DeltaEntity( msg, newframe, oldnum, oldstate, qtrue );

			oldindex++;

			if( oldindex >= oldframe->numEntities ) {
				oldnum = 99999;
			} else {
				oldstate = &ds.parseEntities[(oldframe->firstEntity + oldindex) & PARSE_ENTITIES_MASK];
				oldnum = oldstate->number;
			} 
		}	

		if( oldnum == newnum ) {
			// delta from previous state
			Parse_DeltaEntity( msg, newframe, newnum, oldstate, qfalse );

			oldindex++;

			if( oldindex >= oldframe->numEntities ) {
				oldnum = 99999;
			} else {
				oldstate = &ds.parseEntities[(oldframe->firstEntity + oldindex) & PARSE_ENTITIES_MASK];
				oldnum = oldstate->number;
			}
			continue;
		}

		if( oldnum > newnum ) {
			// delta from baseline
			Parse_DeltaEntity( msg, newframe, newnum, &ds.entityBaselines[newnum], qfalse );
		}
	}

	// any remaining entities in the old frame are copied over
	while( oldnum != 99999 ) {
		// one or more entities from the old packet are unchanged
		Parse_DeltaEntity( msg, newframe, oldnum, oldstate, qtrue );

		oldindex++;

		if( oldindex >= oldframe->numEntities ) {
			oldnum = 99999;
		} else {
			oldstate = &ds.parseEntities[(oldframe->firstEntity + oldindex) & PARSE_ENTITIES_MASK];
			oldnum = oldstate->number;
		}
	}
}
static void CG_CheckEvents( centity_t *cent )
{
	entityState_t *es = &cent->currentState;
	int event;
	int	mod;
	int	target, attacker;
	char *targetName, *attackerName;
	char			*message;
	char			*message2 = "";

	// check for event-only entities
	if( es->eType > ET_EVENTS ) {
		if( cent->previousEvent ) {
			return;	// already fired
		}
		cent->previousEvent = 1;

		es->event = es->eType - ET_EVENTS;
	} else {
		// check for events riding with another entity
		if ( es->event == cent->previousEvent ) {
			return;
		}
		cent->previousEvent = es->event;
		if( ( es->event & ~EV_EVENT_BITS ) == 0 ) {
			return;
		}
	}

	event = es->event & ~EV_EVENT_BITS;
	if( event != EV_OBITUARY ) {
		return;
	}

	target = es->otherEntityNum;
	attacker = es->otherEntityNum2;
	mod = es->eventParm;

	if( target < 0 || target >= MAX_CLIENTS ) {
		return;
	}

	if( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = target;
	}

	targetName = clientNames[target];
	attackerName = clientNames[attacker];

	if (attacker == target)	// selbstmord oder "unfall"
	{
		switch (mod) 
		{
		case 1:
		default:
			message = "Died"; break;
		}
		Com_Printf( "%s %s.\n", targetName, message );
		return;
	}
	
	switch (mod) 
	{	
		case 1:
		default:				message = "Was killed by"; break;

	}
	Com_Printf( "%s %s %s%s.\n", targetName, message, attackerName, message2);
	return;
	

	// we don't know what it was
	Com_Printf( "%s died.\n", targetName );
}
static void Com_AppendToGameState( gameState_t *gameState, int index, const char *configString ) {
	int len;

	if( !configString || !(len=strlen( configString )) ) {
		return;
	}

	if( gameState->dataCount + len + 2 >= MAX_GAMESTATE_CHARS ) {
		Com_Error( ERR_DROP, "Com_AppendToGameState: MAX_GAMESTATE_CHARS" );
	}

	gameState->stringOffsets[index] = gameState->dataCount + 1;
	strcpy( &gameState->stringData[gameState->dataCount + 1], configString );
	gameState->dataCount += len + 1;
}
void Com_InsertIntoGameState( gameState_t *gameState, int index, const char *configString ) {
	char *strings[MAX_CONFIGSTRINGS];
	int ofs;
	int i;

	if( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error( ERR_DROP, "Com_InsertIntoGameState: bad index %i", index );
	}

	if( !gameState->stringOffsets[index] ) {
		// just append to the end of gameState
		Com_AppendToGameState( gameState, index, configString );
		return;
	}

	//
	// resort gameState
	//
	for( i=0 ; i<MAX_CONFIGSTRINGS ; i++ ) {
		ofs = gameState->stringOffsets[i];
		if( !ofs ) {
			strings[i] = NULL;
		} else if( i == index ) {
			strings[i] = CopyString( configString );
		} else {
			strings[i] = CopyString( &gameState->stringData[ofs] );
		}
	}

	memset( gameState, 0, sizeof( *gameState ) );

	for( i=0 ; i<MAX_CONFIGSTRINGS ; i++ ) {
		if( strings[i] ) {
			Com_AppendToGameState( gameState, i, strings[i] );
			free( strings[i] );
		}
	}
}
static void Parse_Snapshot( sizebuf_t *msg )
{
	snapshot_t	*oldsnap;
	int				delta;
	int				len;

	// save the frame off in the backup array for later delta comparisons
	ds.snap = &ds.snapshots[demo.demoMessageSequence & SNAPSHOT_MASK];
	memset( ds.snap, 0, sizeof( *ds.snap ) );

	ds.snap->seq = demo.demoMessageSequence;
	ds.snap->serverTime = MSG_ReadLong( msg );

	// If the frame is delta compressed from data that we
	// no longer have available, we must suck up the rest of
	// the frame, but not use it, then ask for a non-compressed
	// message 
	delta = MSG_ReadByte( msg );
	if( delta ) {
		ds.snap->deltaSeq = demo.demoMessageSequence - delta;
		oldsnap = &ds.snapshots[ds.snap->deltaSeq & SNAPSHOT_MASK];

		if( !oldsnap->valid ) {
			// should never happen
			Com_Printf( "Delta from invalid frame (not supposed to happen!).\n" );
		} else if( oldsnap->seq != ds.snap->deltaSeq ) {
			// The frame that the server did the delta from
			// is too old, so we can't reconstruct it properly.
			Com_Printf( "Delta frame too old.\n" );
		} else if( ds.firstParseEntity - oldsnap->firstEntity >
			MAX_PARSE_ENTITIES - MAX_ENTITIES_IN_SNAPSHOT )
		{
			Com_Printf( "Delta parse_entities too old.\n" );
		} else {
			ds.snap->valid = qtrue; // valid delta parse
		}
	} else {
		oldsnap = NULL;
		ds.snap->deltaSeq = -1;
		ds.snap->valid = qtrue; // uncompressed frame
	}

	// read snapFlags
	ds.snap->snapFlags = MSG_ReadByte( msg );

	// read areabits
	len = MSG_ReadByte( msg );
	MSG_ReadData( msg, ds.snap->areamask, len );

	// read playerinfo
	MSG_ReadDeltaPlayerstate( msg, oldsnap ? &oldsnap->ps : NULL, &ds.snap->ps );

	// read packet entities
	Parse_PacketEntities( msg, oldsnap, ds.snap );
}
static void Parse_ServerCommand( sizebuf_t *msg ) {
	int		number;
	char	*string;

	number = MSG_ReadLong( msg );
	string = MSG_ReadString( msg );

	if( number < ds.lastServerCommandNum ) {
		return; // we have already received this command
	}
	ds.lastServerCommandNum = number;

	// archive the command to be processed later
	Q_strncpyz( ds.serverCommands[number & SERVERCMD_MASK], string, sizeof( ds.serverCommands[0] ) );
}
static void ParseGameState( sizebuf_t *msg ) {
	int		c;
	char	*s;
	static int clear = 0;
	memset( &ds, 0, sizeof( ds ) );

	ds.lastServerCommandNum = MSG_ReadLong( msg );	
	ds.currentServerCommandNum = ds.lastServerCommandNum;

	// Set only the first one for cfg setup
	if(!ds.firstserverCommandSequence)
		ds.firstserverCommandSequence = ds.lastServerCommandNum;
		
	ds.gameState.dataCount = 1;		
	while( 1 ) {
		c = MSG_ReadByte( msg );

		if( c == -1 ) {	
			SetConsoleTextAttribute(hConsole, 12);
			cout << "ParseGameState: read past end of demo message" << endl;
			system("pause");
			exit( EXIT_FAILURE );
		}

		if( c == svc_EOF) {			
			break;
		}

		switch( c ) {
		default:	
			SetConsoleTextAttribute(hConsole, 12);
			cout << "Parse_GameState: bad command byte" << endl;
			system("pause");
			exit( EXIT_FAILURE );
			break;

		case svc_configstring: // 3
			{
				int index, len;	
				char	*configString;

				index = MSG_ReadShort( msg );
				if( index < 0 || index >= MAX_CONFIGSTRINGS ) {
					SetConsoleTextAttribute(hConsole, 12);
					cout << "ParseGameState: configString index " << index << " out of range" << endl;
					system("pause");
					exit( EXIT_FAILURE );
				}

				s = MSG_ReadBigString( msg );
				len = strlen(s);

				ds.gameState.stringOffsets[ index ] = ds.gameState.dataCount;
				memcpy( ds.gameState.stringData + ds.gameState.dataCount, s, len + 1 );
				ds.gameState.dataCount += len + 1;
				
				configString = MSG_ReadBigString( msg );
				Com_InsertIntoGameState( &ds.gameState, index, configString );
			}
			break;			

		case svc_baseline:
			{
				int		index;

				index = MSG_ReadBits( msg, GENTITYNUM_BITS );
				if( index < 0 || index >= MAX_GENTITIES ) {
					SetConsoleTextAttribute(hConsole, 12);
					cout << "ParseGameState: baseline index " << index << " out of range" << endl;
					system("pause");
					exit( EXIT_FAILURE );
				}
				MSG_ReadDeltaEntity( msg, NULL, &ds.entityBaselines[index], index );
			}
			break;		
		}
	}
	
	ds.clientNum = MSG_ReadLong( msg ); //1  // need 7 bits (9)
	ds.checksumFeed = MSG_ReadLong( msg );  // need 6 bits  (7)
		
#if _GOLD  // GOLD seems to have extra info written here.. no clue what! 1.00 should have something AFTER a save (LONG)
	MSG_ReadShort( msg );
	MSG_ReadLong( msg );
#endif
	//MSG_ReadLong( msg );  // Needed if parsing demos already FIXED in 1.00

	demo.gameStatesParsed++;	
}
static void ParseMessage( sizebuf_t *msg ) {
	int			cmd;
	int			i; 

	// remaining data is Huffman compressed
	MSG_Bitstream( msg );

	// the first one gets saved to the cfg write, so skip any others
	i = MSG_ReadLong( msg ); 

	if(!reliableSequence) 
		reliableSequence = i;

	while( 1 ) {
		if( msg->readcount > msg->cursize ) {
			SetConsoleTextAttribute(hConsole, 12);
			cout << "ParseMessage: read past end of demo message" << endl;
			system("pause");
			exit( EXIT_FAILURE );
		}

		cmd = MSG_ReadByte( msg );

		if( cmd == svc_EOF ) {
			break;
		}
	
	// other commands
		switch( cmd ) 
		{
		case svc_gamestate:		//2
			ParseGameState( msg );	
			break;
			
		case svc_mapchange:		//8
		case svc_nop:			//1	
			break;
		
		case svc_serverCommand:	//5
			Parse_ServerCommand( msg );	
			break;
			
		case svc_snapshot:		// 7		
			Parse_Snapshot( msg );		
			break;

		default:
		case svc_baseline:		//4
		case svc_configstring:  //3
		case svc_download:		//6
			SetConsoleTextAttribute(hConsole, 12);
			cout << "ParseMessage: illegible demo message" << endl;
			system("pause");
			exit( EXIT_FAILURE );
			break;
		}
	}
}
qboolean ParseNextMessage( void ) {
	sizebuf_t	msg;
	byte		buffer[MAX_MSGLEN];
	int			len;
	int			seq;

	if( fread( &seq, 1, 4, demo.demofile ) != 4 ) {
		SetConsoleTextAttribute(hConsole, 12);
		cout << "Demo file was truncated"<< endl;	
		return qfalse;
	}

	if( fread( &len, 1, 4, demo.demofile ) != 4 ) {
		SetConsoleTextAttribute(hConsole, 12);
		cout << "Demo file was truncated"<< endl;	
		return qfalse;
	}

	if( seq == -1 || len == -1 ) {
		return qfalse; // demo EOF reached
	}

	MSG_Init( &msg, buffer, sizeof( buffer ) );

	msg.cursize = LittleLong( len );

	// Save seq and len
	serverMessageSequence = LittleLong( seq );
	serverMessageLen = LittleLong( len );
	
	if( msg.cursize <= 0 || msg.cursize >= msg.maxsize ) {
		SetConsoleTextAttribute(hConsole, 12);
		cout << "Illegal demo message length" << endl;
		system("pause");
		//exit( EXIT_FAILURE );
		return qfalse;
	}

	if( fread( msg.data, 1, msg.cursize, demo.demofile ) != msg.cursize ) {
		SetConsoleTextAttribute(hConsole, 12);
		cout << "Demo file was truncated"<< endl;
		return qfalse;
	}

	if(demo.gameStatesParsed)
	{
		//if(serverMessageSequence > 73633)  // this will cut the demo off at 2000 sequence (working)
		//	return qtrue;
		WriteMessage(&msg, serverMessageSequence );
		return qtrue;
	}

	ParseMessage( &msg ); // parse the message
	WriteConfig ();		
	return qtrue;
}
static void UpdateClientInfo( int clientNum, const char *info ) {
	if( !info || !info[0] ) {
		Q_strncpyz( clientNames[clientNum], "BADNAME", sizeof( clientNames[0] ) );
	} else {
		Q_strncpyz( clientNames[clientNum], Info_ValueForKey( info, "n" ), sizeof( clientNames[0] ) );
	}
}
static void UpdateConfigString( int index, char *string ) {
	Com_InsertIntoGameState( &ds.gameState, index, string );

	if( index > CS_PLAYERS && index < CS_PLAYERS + MAX_CLIENTS ) {
		UpdateClientInfo( index, string );
	}
}
char *Com_GetStringFromGameState( gameState_t *gameState, int index ) {
	int ofs;

	if( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error( ERR_DROP, "Com_GetStringFromGameState: bad index  %i", index  );
	}

	ofs = gameState->stringOffsets[index ];

	if( !ofs ) {
		return "";
	}

	return &gameState->stringData[ofs];
}
void GameStateParsed( void ) {
	int i;
	char *configString;

	for( i=0 ; i<MAX_CONFIGSTRINGS ; i++ ) {
		configString = Com_GetStringFromGameState( &ds.gameState, i );
		if( configString[0] ) {
			if( i < RESERVED_CONFIGSTRINGS ) {
				Com_Printf( "%s Info:\n", (i == CS_SERVERINFO) ? "Server" : "System" );
				Info_Print( configString );
				Com_Printf( "\n" );
			}/* else {
				Com_Printf( "configString %i: \"%s\"\n", i, configString );
			}*/
		}
	}

	for( i=0 ; i<MAX_CLIENTS ; i++ ) {
		configString = Com_GetStringFromGameState( &ds.gameState, CS_PLAYERS + i );
		UpdateClientInfo( i, configString );
	}
}

void NewFrameParsed( void ) {
	int i;
	entityState_t *es;
	char *serverCommand;

	for( i=ds.currentServerCommandNum+1 ; i<=ds.lastServerCommandNum ; i++ ) {
		serverCommand = ds.serverCommands[i & SERVERCMD_MASK];
		Cmd_TokenizeString( serverCommand );

		if( !Q_stricmp( Cmd_Argv( 0 ), "print" ) || !Q_stricmp( Cmd_Argv( 0 ), "cp" ) ) {
			Com_Printf( "%s", Cmd_Args() );
		} else if( !Q_stricmp( Cmd_Argv( 0 ), "cs" ) ) {
			UpdateConfigString( atoi( Cmd_Argv( 1 ) ), Cmd_Argv( 2 ) );
		}

	//	Com_Printf( "serverCommand: %i\n", i );		
	}
	ds.currentServerCommandNum = ds.lastServerCommandNum;

	if( !ds.snap || !ds.snap->valid ) {
		return;
	}

	for( i=0 ; i<MAX_GENTITIES ; i++ ) {
		centities[i].currentValid = qfalse;
	}

	for( i=0 ; i<ds.snap->numEntities ; i++ ) {
		es = &ds.parseEntities[(ds.snap->firstEntity+i) & PARSE_ENTITIES_MASK];
		memcpy( &centities[es->number].currentState, es, sizeof( entityState_t ) );
		centities[es->number].currentValid = qtrue;
		//CG_CheckEvents( &centities[es->number] );
	}

	for( i=0 ; i<MAX_GENTITIES ; i++ ) {
		if( !centities[i].currentValid ) {
			centities[i].previousEvent = 0;
		}
	}
}


int main(array<System::String ^> ^args)
{  
	char DEMOFILE[256];
	char DEMOSAVEFILE[256];
	//char DEMONEWNAME[256];
	int len = 0;

	SetConsoleTitle(L"DEMOFIXER - Soldier of Fortune 2 v1.00"); 
	GetTitleLine();
	
	SetConsoleTextAttribute(hConsole, 14);
	cout << "Usage: ";
	SetConsoleTextAttribute(hConsole, 15);
	cout << "Drop a demo into the window and press ENTER\n" << endl;	

	SetConsoleTextAttribute(hConsole, 15);  
	cout << "Demo to fix: ";

start:

	SetConsoleTextAttribute(hConsole, 14);  

	string s;
	getline( cin, s );
	// Remove all double-quote characters
	s.erase(
		remove( s.begin(), s.end(), '\"' ),
		s.end()
	);
	sprintf(DEMOFILE,"%s", s.c_str());

	cout << "" << endl;
	
	// Check full path
	demo.demofile = fopen ( DEMOFILE , "rb" );
	if (!demo.demofile)
	{
		// name only	
		sprintf(DEMOSAVEFILE,"%s_df%s", DEMOFILE, APP_EXT);
		sprintf(DEMOFILE, "%s%s", DEMOFILE, APP_EXT);
		demo.demofile = fopen ( DEMOFILE, "rb" );
		
		// Aint gonna happen
		if (!demo.demofile)
		{
			//system("Color 1A");
			SetConsoleTextAttribute(hConsole, 14);
			cout << "ERROR: ";
			SetConsoleTextAttribute(hConsole, 15);
			cout << DEMOFILE;
			SetConsoleTextAttribute(hConsole, 12);
			cout << " not found!\n" << endl;
			SetConsoleTextAttribute(hConsole, 15); 
			goto start;
		}
	}
	else
	{
		// Full path
		strncpy(DEMOSAVEFILE, DEMOFILE, strlen(DEMOFILE));
		DEMOSAVEFILE[strlen(DEMOFILE)-8]='\0';
		sprintf(DEMOSAVEFILE,"%s_df%s", DEMOSAVEFILE, APP_EXT );
	}

	demo.saveFile = fopen ( DEMOSAVEFILE , "wb" );
	if (!demo.saveFile)
	{
		SetConsoleTextAttribute(hConsole, 14);
		cout << "ERROR: ";
		SetConsoleTextAttribute(hConsole, 12);
		cout << "Could not create the new demo file: " << DEMOSAVEFILE << endl;
		SetConsoleTextAttribute(hConsole, 15); 
		goto start;
	}

	MSG_initHuffman();

#ifdef _PS  // This way does a little more, so we can try and edit player states, etc..
	while( !demo.gameStatesParsed ) {
		if( !ParseNextMessage() ) {
			break;
		}
	}

	GameStateParsed();

	while( 1 ) {
		if( !ParseNextMessage() ) {
			break;
		}
		NewFrameParsed();
	}

#else  // this is the simple fix way, doing only what is required for the fix.
	while( 1 ) 
	{
		if( !ParseNextMessage() ) 
			break;
	}

#endif		

	FS_Write (&len, 4, demo.saveFile);	
	FS_Write (&len, 4, demo.saveFile);

	fclose( demo.saveFile );
	fclose( demo.demofile );		

	SetConsoleTextAttribute(hConsole, 11);
	cout << "Save successful!" << endl;	

	SetConsoleTextAttribute(hConsole, 15);
	cout << "Output file: ";
	
	// THIS WILL RENAME THE DEMO TO THE ORIGINAL.  
	// ... Cool, but confusing to know what demos has and hasnt been fixed
	if(moveFiles(DEMOSAVEFILE))
	{
	//	sprintf(DEMONEWNAME,"%s", s.c_str() );
	//	rename(DEMOSAVEFILE, DEMONEWNAME);
	//	SetConsoleTextAttribute(hConsole, 14);
	//	cout << DEMONEWNAME << endl << endl;
	//}
	//else
	//{
		SetConsoleTextAttribute(hConsole, 14);
		cout << DEMOSAVEFILE << endl << endl;
	}

	SetConsoleTextAttribute(hConsole, 15);  
	cout << "Next Demo to fix: ";
	demo.gameStatesParsed = 0;
	goto start;
	
	return EXIT_SUCCESS;
}


