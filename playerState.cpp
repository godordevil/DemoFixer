
#if 0

#ifdef _Q68

netField_t	playerStateFields[] = 
{
{ PSF(commandTime), 32 },				
{ PSF(origin[0]), 0 },
{ PSF(origin[1]), 0 },
{ PSF(bobCycle), 8 },
{ PSF(velocity[0]), 0 },
{ PSF(velocity[1]), 0 },
{ PSF(viewangles[1]), 0 },
{ PSF(viewangles[0]), 0 },
{ PSF(weaponTime), -16 },
{ PSF(origin[2]), 0 },
{ PSF(velocity[2]), 0 },
{ PSF(legsTimer), 8 },
{ PSF(pm_time), -16 },
{ PSF(eventSequence), 16 },
{ PSF(torsoAnim), 8 },
{ PSF(movementDir), 4 },
{ PSF(events[0]), 8 },
{ PSF(legsAnim), 8 },
{ PSF(events[1]), 8 },
{ PSF(pm_flags), 16 },
{ PSF(groundEntityNum), GENTITYNUM_BITS },
{ PSF(weaponstate), 4 },
{ PSF(eFlags), 16 },
{ PSF(externalEvent), 10 },
{ PSF(gravity), 16 },
{ PSF(speed), 16 },
{ PSF(delta_angles[1]), 16 },
{ PSF(externalEventParm), 8 },
{ PSF(viewheight), -8 },
{ PSF(damageEvent), 8 },
{ PSF(damageYaw), 8 },
{ PSF(damagePitch), 8 },
{ PSF(damageCount), 8 },
{ PSF(generic1), 8 },
{ PSF(pm_type), 8 },					
{ PSF(delta_angles[0]), 16 },
{ PSF(delta_angles[2]), 16 },
{ PSF(torsoTimer), 12 },
{ PSF(eventParms[0]), 8 },
{ PSF(eventParms[1]), 8 },
{ PSF(clientNum), 8 },
{ PSF(weapon), 5 },
{ PSF(viewangles[2]), 0 },
{ PSF(grapplePoint[0]), 0 },
{ PSF(grapplePoint[1]), 0 },
{ PSF(grapplePoint[2]), 0 },
{ PSF(jumppad_ent), 10 },
{ PSF(loopSound), 16 }
};


#else

//#ifndef _GOLD
#if 1
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
{ PSF(weaponAnimId), ANIM_BITS }, // 10 - ET ??
{ PSF(weaponAnimIdChoice), ANIM_BITS },
{ PSF(weaponCallbackTime), -16},
{ PSF(weaponCallbackStep), 8 },
// end

{ PSF(origin[2]), 0 },		// not found
{ PSF(velocity[2]), 0 },
{ PSF(pm_time), -16 },
{ PSF(eventSequence), 16 },
//{ PSF(eventSequence), 8 },
{ PSF(torsoAnim), ANIM_BITS },		// not found

// wasnt there, but might be due to duplicates not showing
{ PSF(torsoTimer),  16 }, // + [check bits]
{ PSF(movementDir), 8 },	//8  in ET
{ PSF(events[0]), 8 },
{ PSF(events[1]), 8 },
{ PSF(events[2]), 8 },		 // + [check bits]
{ PSF(events[3]), 8 },		 // + [check bits]
{ PSF(legsAnim), ANIM_BITS },		// not found

{ PSF(pm_flags), 16 },
{ PSF(pm_debounce), 16 },	// + [check bits]
{ PSF(groundEntityNum), GENTITYNUM_BITS },  // not found

{ PSF(weaponstate), 4 },
{ PSF(eFlags), 22 },		// not found  -  should be changed 22/23/19???
{ PSF(externalEvent), 10 },
{ PSF(speed), 8 },
{ PSF(delta_angles[1]), 16 },

{ PSF(externalEventParm), 8 },
{ PSF(viewheight), -8 },
{ PSF(damageEvent), 8 },
{ PSF(damageYaw), 8 },
{ PSF(damagePitch), 8 },

{ PSF(damageCount), 8 },
{ PSF(generic1), 8 },	// not found
//  +  // all added [check bits] (also unknown if generic1 is above or below.. put it below)
{ PSF(inaccuracy), 8 },			// + [check bits]
{ PSF(inaccuracyTime), -16 },	// + [check bits]
{ PSF(kickPitch), 4 },			// + [check bits]
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
{ PSF(weapon), 4 },			// not found

{ PSF(viewangles[2]), 0 },

//  +  // all added [check bits]
{ PSF(zoomTime), 8 },
{ PSF(zoomFov), 8 }, 
{ PSF(leanTime), 8 },
{ PSF(grenadeTimer), -16 },	
{ PSF(respawnTimer), 16},	
//end

{ PSF(loopSound), 8 },		// not found

};
#else
#define INC 0

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
{ PSF(weaponAnimId), 8 },
{ PSF(weaponAnimIdChoice), 8 },
{ PSF(weaponCallbackTime), -16},
{ PSF(weaponCallbackStep), 8 },
// end

{ PSF(origin[2]), 0 },		// not found
{ PSF(velocity[2]), 0 },
{ PSF(pm_time), -16 },
{ PSF(eventSequence), 16 },
{ PSF(torsoAnim), 8 },		// not found

// wasnt there, but might be due to duplicates not showing
{ PSF(torsoTimer),  -16 }, // + [check bits]
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
{ PSF(generic1), 8 },	// not found
//  +  // all added [check bits] (also unknown if generic1 is above or below.. put it below)
{ PSF(inaccuracy), -8 },			// + [check bits]
{ PSF(inaccuracyTime), 16 },	// + [check bits]
{ PSF(kickPitch), 8 },			// + [check bits]
// end

{ PSF(generic1), 8 },		// not found
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
{ PSF(zoomTime), 8 },
{ PSF(zoomFov), 8 }, 
{ PSF(leanTime), 8 },
{ PSF(grenadeTimer), 8 },	
{ PSF(respawnTimer), 8},	
//end

{ PSF(loopSound), 16 },		// not found

};

#endif
#endif

static const int psTableSize = sizeof( playerStateFields ) / sizeof( playerStateFields[0] );

static int psCount = 0;
#define MAX_PS psTableSize
usedArray_t		psUsed[MAX_ES];



void Print_ps(void)
{
#if DEBUG_PS

	int i;
	netField_t	*field;
	
	if(!psParsed)
		return;
	DCom_Printf( "\n\n========================================================\n Player State Fields USED\n========================================================\n" );

	for (i = 0, field=playerStateFields; i <= MAX_PS; i++, field++)
	{
		if(!psUsed[i].used)
			continue;
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
		psUsed[i].name = field->name;
		psParsed = 1;

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

		psUsed[i].used = 1;

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

#endif



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
			Parse_DeltaEntity( msg, newframe, newnum, &ds.baselines[newnum], qfalse );
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

void Parse_Snapshot( sizebuf_t *msg )
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


