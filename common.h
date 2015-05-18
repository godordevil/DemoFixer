#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include "bswap.h"

#ifndef _COMMON
#define _COMMON
typedef struct {
	int		used;		// number of bits written to or read from the buffer
	char	*name;
} usedArray_t;


typedef enum {qfalse, qtrue}	qboolean;
typedef unsigned char			byte;
typedef float					vec3_t[3];

#ifndef NULL
#define NULL ((void *)0)
#endif

#define Com_Memset memset
#define Com_Memcpy memcpy
#define LittleLong le2me_32
#define LittleShort le2me_16


#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		  1024
#define	MAX_INFO_VALUE		1024


#define MAXPRINTMSG				4096
#define	MAX_STRING_CHARS		1024	// max length of a string passed to Cmd_TokenizeString
#define	BIG_INFO_STRING			8192  // used for system info key only
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility
#define	BIG_INFO_KEY		  8192
#define	BIG_INFO_VALUE		8192

#define	GENTITYNUM_BITS		10		// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)
#define	ENTITYNUM_NONE		(MAX_GENTITIES-1)
#define	MAX_CONFIGSTRINGS	1400
#define	MAX_GAMESTATE_CHARS	16000

// bit field limits
#define	MAX_STATS				16
#define	MAX_PERSISTANT			16
#define	MAX_AMMO				16
#define	MAX_WEAPONS				32
#define MAX_GAMETYPE_ITEMS		5

#define	MAX_PS_EVENTS			4

#define	RESERVED_CONFIGSTRINGS	2	// game can't modify below this, only the system can

#define	MAX_CLIENTS			64		// absolute limit
#define MAX_LOCATIONS		64

#define MAX_TERRAINS		32
#define MAX_LADDERS			64

#define MAX_INSTANCE_TYPES		16
#define	MAX_TOKEN_CHARS		1024
#define	MAX_STRING_TOKENS	1024

#define	CS_SERVERINFO		0		// an info string with all the serverinfo cvars
#define	CS_SYSTEMINFO		1		// an info string for server system to client system configuration (timescale, etc)
#define	RESERVED_CONFIGSTRINGS	2	// game can't modify below this, only the system can

#define	CS_SERVERINFO		0		// an info string with all the serverinfo cvars
#define	CS_SYSTEMINFO		1		// an info string for server system to client system configuration (timescale, etc)
#define CS_PLAYERS			2		// info string for player user info
#define CS_CUSTOM			(CS_PLAYERS + MAX_CLIENTS )

#define	MAX_MODELS				256		// these are sent over the net as 8 bits
#define	MAX_SOUNDS				256		// so they cannot be blindly increased
#define MAX_AMBIENT_SOUNDSETS	64
#define MAX_FX					64		// max effects strings, I'm hoping that 64 will be plenty
#define MAX_SUB_BSP				32
#define MAX_ICONS				32
#define	MAX_CHARSKINS			64		// character skins

#define LS_STYLES_START			0
#define LS_NUM_STYLES			32
#define	LS_SWITCH_START			(LS_STYLES_START+LS_NUM_STYLES)
#define LS_NUM_SWITCH			32
#define MAX_LIGHT_STYLES		64

#define	MAX_CONFIGSTRINGS	1400

#define	CS_PLAYERS				2

typedef enum 
{
	TEAM_FREE,
	TEAM_RED,
	TEAM_BLUE,
	TEAM_SPECTATOR,

	TEAM_NUM_TEAMS

} team_t;
enum
{
//	CS_SERVERINFO,
//	CS_SYSTEMINFO,
//  CS_PLAYERS,

	CS_MUSIC = CS_CUSTOM,

	CS_MESSAGE,
	CS_MOTD,
	CS_WARMUP,

	CS_VOTE_TIME,
	CS_VOTE_STRING,
	CS_VOTE_YES,
	CS_VOTE_NO,
	CS_VOTE_NEEDED,

	CS_GAME_VERSION,
	CS_GAME_ID,
	CS_LEVEL_START_TIME,
	CS_INTERMISSION,
	CS_SHADERSTATE,
	CS_BOTINFO,
	
	CS_GAMETYPE_TIMER,
	CS_GAMETYPE_MESSAGE,
	CS_GAMETYPE_REDTEAM,
	CS_GAMETYPE_BLUETEAM,

	CS_ITEMS,

	CS_PICKUPSDISABLED,

	// Config string ranges
	CS_MODELS,
	CS_SOUNDS				= CS_MODELS + MAX_MODELS,
	CS_LOCATIONS			= CS_SOUNDS + MAX_SOUNDS,
	CS_LADDERS				= CS_LOCATIONS + MAX_LOCATIONS,
	CS_BSP_MODELS			= CS_LADDERS + MAX_LADDERS,
	CS_TERRAINS				= CS_BSP_MODELS + MAX_SUB_BSP,
	CS_EFFECTS				= CS_TERRAINS + MAX_TERRAINS,
	CS_LIGHT_STYLES			= CS_EFFECTS + MAX_FX,
	CS_ICONS				= CS_LIGHT_STYLES + (MAX_LIGHT_STYLES*3),
	CS_TEAM_INFO			= CS_ICONS + MAX_ICONS,
	CS_AMBIENT_SOUNDSETS	= CS_TEAM_INFO + TEAM_NUM_TEAMS,

	CS_MAX					= CS_AMBIENT_SOUNDSETS + MAX_AMBIENT_SOUNDSETS
};

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

#define	EV_EVENT_BIT1		0x00000100
#define	EV_EVENT_BIT2		0x00000200
#define	EV_EVENT_BITS		(EV_EVENT_BIT1|EV_EVENT_BIT2)
#define	EVENT_VALID_MSEC	300
typedef enum 
{
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER,
	ET_BEAM,
	ET_PORTAL,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORT_TRIGGER,
	ET_INVISIBLE,
	ET_GRAPPLE,				// grapple hooked on wall
	ET_BODY,
	ET_DAMAGEAREA,
	ET_TERRAIN,

	ET_DEBUG_CYLINDER,

	ET_EVENTS				// any of the EV_* events can be added freestanding
							// by setting eType to ET_EVENTS + eventNum
							// this avoids having to set eFlags and eventNum
	///RxCxW
	//	,ET_GAMETYPE_TRIGGER			// any of the EV_* events can be added freestanding
	///End
} entityType_t;

// parameters to the main Error routine
typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
	ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;

typedef struct {
	int			stringOffsets[MAX_CONFIGSTRINGS];
	char		stringData[MAX_GAMESTATE_CHARS];
	int			dataCount;
} gameState_t;

typedef enum
{
	ATTACK_NORMAL,
	ATTACK_ALTERNATE,
	ATTACK_MAX

} attackType_t;
typedef enum {
	TR_STATIONARY,
	TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	TR_SINE,					// value = base + sin( time / duration ) * delta
	TR_GRAVITY
} trType_t;

typedef struct {
	trType_t	trType;
	int		trTime;
	int		trDuration;			// if non 0, trTime + trDuration = stop time
	vec3_t	trBase;
	vec3_t	trDelta;			// velocity, etc
} trajectory_t;

typedef struct playerState_s 
{
	int			commandTime;	// cmd->serverTime of last executed command
	int			pm_type;
	int			bobCycle;		// for view bobbing and footstep generation
	int			pm_flags;		// ducked, etc
	int			pm_debounce;	// debounce buttons
	int			pm_time;

	vec3_t		origin;
	vec3_t		velocity;

	int			weaponTime;
	int			weaponFireBurstCount;
	int			weaponAnimId;
	int			weaponAnimIdChoice;
	int			weaponAnimTime;
	int			weaponCallbackTime;
	int			weaponCallbackStep;

	int			gravity;
	int			speed;
	int			delta_angles[3];				// add to command angles to get view direction
												// changed by spawns, rotating objects, and teleporters
	int			groundEntityNum;				// ENTITYNUM_NONE = in air
												
	int			legsAnim;						// mask off ANIM_TOGGLEBIT
												
	int			torsoTimer;						// don't change low priority animations until this runs out
	int			torsoAnim;						// mask off ANIM_TOGGLEBIT
												
	int			movementDir;					// a number 0 to 7 that represents the reletive angle
												// of movement to the view angle (axial and diagonals)
												// when at rest, the value will remain unchanged
												// used to twist the legs during strafing
												
	int			eFlags;							// copied to entityState_t->eFlags
												
	int			eventSequence;					// pmove generated events
	int			events[MAX_PS_EVENTS];			
	int			eventParms[MAX_PS_EVENTS];		
												
	int			externalEvent;					// events set on player from another source
	int			externalEventParm;				
	int			externalEventTime;				
												
	int			clientNum;						// ranges from 0 to MAX_CLIENTS-1
	int			weapon;							// copied to entityState_t->weapon
	int			weaponstate;					
												
	vec3_t		viewangles;						// for fixed views
	int			viewheight;						
												
	// damage feedback							
	int			damageEvent;					// when it changes, latch the other parms
	int			damageYaw;						
	int			damagePitch;					
	int			damageCount;					
												
	int			painTime;						// used for both game and client side to process the pain twitch - NOT sent across the network
	int			painDirection;					// NOT sent across the network
										
	int			stats[MAX_STATS];				
	int			persistant[MAX_PERSISTANT];		// stats that aren't cleared on death
	int			ammo[MAX_AMMO];
	int			clip[ATTACK_MAX][MAX_WEAPONS];
	int			firemode[MAX_WEAPONS];

	int			generic1;
	int			loopSound;

	// Incaccuracy values for firing
	int			inaccuracy;
	int			inaccuracyTime;
	int			kickPitch;

	// not communicated over the net at all
	int			ping;							// server to game info for scoreboard
	int			pmove_framecount;				// FIXME: don't transmit over the network
	int			jumppad_frame;
	int			entityEventSequence;
	vec3_t		pvsOrigin;						// view origin used to calculate PVS (also the lean origin)
												// THIS VARIABLE MUST AT LEAST BE THE PLAYERS ORIGIN ALL OF THE 
												// TIME OR THE PVS CALCULATIONS WILL NOT WORK.

	// Zooming
	int			zoomTime;
	int			zoomFov;

	// LAdders
	int			ladder;
	int			leanTime;

	// Timers 
	int			grenadeTimer;
	int			respawnTimer;

} playerState_t;


typedef struct entityState_s 
{
	int				number;			// entity index
	int				eType;			// entityType_t
	int				eFlags;

	trajectory_t	pos;			// for calculating position
	trajectory_t	apos;			// for calculating angles

	int				time;
	int				time2;
					
	vec3_t			origin;
	vec3_t			origin2;
					
	vec3_t			angles;
	vec3_t			angles2;
					
	int				otherEntityNum;	// shotgun sources, etc
	int				otherEntityNum2;
					
	int				groundEntityNum;	// -1 = in air
					
	int				loopSound;		// constantly loop this sound
	int				mSoundSet;
										
	int				modelindex;
	int				modelindex2;
	int				clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses
	int				frame;
					
	int				solid;			// for client side prediction, trap_linkentity sets this properly
					
	int				event;			// impulse events -- muzzle flashes, footsteps, etc
	int				eventParm;

	int				generic1;

	// for players
	// these fields are only transmitted for client entities!!!!!
	int				gametypeitems;	// bit flags indicating which items are carried
	int				weapon;			// determines weapon and flash model, etc
	int				legsAnim;		// mask off ANIM_TOGGLEBIT
	int				torsoAnim;		// mask off ANIM_TOGGLEBIT
	int				torsoTimer;		// time the animation will play for
	int				leanOffset;		// Lean direction
} entityState_t;

typedef enum 
{
	EV_NONE,

	EV_FOOTSTEP,
	EV_FOOTWADE,
	EV_SWIM,

	EV_STEP_4,
	EV_STEP_8,
	EV_STEP_12,
	EV_STEP_16,

	EV_FALL_SHORT,
	EV_FALL_MEDIUM,
	EV_FALL_FAR,

	EV_JUMP,
	EV_WATER_FOOTSTEP,
	EV_WATER_TOUCH,	// foot touches
	EV_WATER_LAND,  // landed in water
	EV_WATER_CLEAR,

	EV_ITEM_PICKUP,			// normal item pickups are predictable

	EV_NOAMMO,
	EV_CHANGE_WEAPON,
	EV_CHANGE_WEAPON_CANCELLED,
	EV_READY_WEAPON,
	EV_FIRE_WEAPON,
	EV_ALT_FIRE,

	EV_USE,			// +Use key

	EV_ITEM_RESPAWN,
	EV_ITEM_POP,
	EV_PLAYER_TELEPORT_IN,
	EV_PLAYER_TELEPORT_OUT,

	EV_GRENADE_BOUNCE,		// eventParm will be the soundindex

	EV_PLAY_EFFECT,

	EV_GENERAL_SOUND,
	EV_GLOBAL_SOUND,		// no attenuation
	EV_ENTITY_SOUND,

	EV_GLASS_SHATTER,

	EV_MISSILE_HIT,
	EV_MISSILE_MISS,

	EV_BULLET_HIT_WALL,
	EV_BULLET_HIT_FLESH,
	EV_BULLET,				// otherEntity is the shooter

	EV_EXPLOSION_HIT_FLESH,

	EV_PAIN,
	EV_PAIN_WATER,
	EV_OBITUARY,

	EV_DESTROY_GHOUL2_INSTANCE,

	EV_WEAPON_CHARGE,
	EV_WEAPON_CHARGE_ALT,

	EV_DEBUG_LINE,
	EV_TESTLINE,
	EV_STOPLOOPINGSOUND,

	EV_BODY_QUEUE_COPY,
	EV_BOTWAYPOINT,

	// Procedural gore event.
	EV_PROC_GORE,
	
	EV_GAMETYPE_RESTART,			// gametype restarting
	EV_GAME_OVER,					// game is over

	EV_GOGGLES,						// goggles turning on/off

	EV_WEAPON_CALLBACK,

} entity_event_t;			// There is a maximum of 256 events (8 bits transmission, 2 high bits for uniqueness)


int		FS_Write( const void *buffer, int len, FILE	*f );
char	*strrep (char *str, const char *orig, const char *rep);



void Com_Printf( const char *text, ... );
void Com_Error( errorParm_t level, const char *error, ... ) ;
void Q_strncpyz( char *dest, const char *src, int destsize );

void	Com_sprintf (char *dest, int size, const char *fmt, ...);
char	*va(char *format, ...);
char *CopyString( const char *in );

// buffer size safe library replacements
void	Q_strncpyz( char *dest, const char *src, int destsize );
void	Q_strcat( char *dest, int size, const char *src );

int		Q_stricmpn (const char *s1, const char *s2, int n);
int		Q_strncmp (const char *s1, const char *s2, int n);
int		Q_stricmp (const char *s1, const char *s2);
void	Q_strcat( char *dest, int size, const char *src );

//
// key / value info strings
//
char *Info_ValueForKey( const char *s, const char *key );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_big( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
qboolean Info_Validate( const char *s );
void Info_NextPair( const char **s, char *key, char *value );
void Info_Print( const char *s );
void Cmd_TokenizeString (char *text);



int		Cmd_Argc( void );
char *	Cmd_Argv( int arg );
char *	Cmd_Args( void );
void	Cmd_TokenizeString( char *text );

#endif // COMMON
