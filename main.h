
#include "stdafx.h"
#include "common.h"
#include "huffman.h"
#include "msg.h"

#define MAX_SNAPSHOTS		32
#define SNAPSHOT_MASK		(MAX_SNAPSHOTS-1)

#define MAX_SERVERCMDS		64
#define MAX_SERVERCMDS		64
#define SERVERCMD_MASK		(MAX_SERVERCMDS-1)


#define MAX_PARSE_ENTITIES	(MAX_GENTITIES*2)

#define MAX_ENTITIES_IN_SNAPSHOT	256

// for keeping all entityStates for delta encoding
#define MAX_PARSE_ENTITIES	(MAX_GENTITIES*2)
#define PARSE_ENTITIES_MASK	(MAX_PARSE_ENTITIES-1)

//svc_mapchange -- should be here ???  was in the sof2 exe file
enum svc_ops_e {
	svc_bad,
	svc_nop,
	svc_gamestate,
	svc_configstring,           // [short] [string] only in gamestate messages
	svc_baseline,               // only in gamestate messages
	svc_serverCommand,          // [string] to be executed by client game module
	svc_download,               // [short] size [size bytes]
	svc_snapshot,
#ifndef _Q68
	svc_mapchange,
#endif
	svc_EOF
};

typedef struct {
	qboolean		valid;

	int				seq;				// die seqeunc number des snapshots
	int				deltaSeq;
	int				snapFlags;			// SNAPFLAG_RATE_DELAYED, etc

	int				serverTime;		// server time the message is valid for (in msec)

	byte			areamask[MAX_MAP_AREA_BYTES];		// portalarea visibility bits

	playerState_t	ps;						// complete information about the current player at this time

	int				numEntities;			// all of the entities that need to be presented
	int				firstEntity;			// ab dieser Position sind im Ringbuffer die numEntities viele Entities des Snapshots
											// ersetzt entities[Max_entities_in snapshot]
} snapshot_t;

typedef struct {
	int		lastServerCommandNum;
	int		currentServerCommandNum;
	char	serverCommands[MAX_SERVERCMDS][MAX_STRING_CHARS];

	gameState_t		gameState;
	entityState_t	entityBaselines[MAX_GENTITIES];   // for delta compression when not in previous frame

	entityState_t	parseEntities[MAX_PARSE_ENTITIES];
	int				firstParseEntity;

	snapshot_t		snapshots[MAX_SNAPSHOTS];
	snapshot_t		*snap;



	int			checksumFeed;				// from the server for checksum calculations
	int			reliableSequence;
	//int			reliableAcknowledge;		// the last one the server has executed
	int			serverMessageSequence;
	//int			serverCommandSequence;
	int			clientNum;
	int			firstserverCommandSequence;
	int			serverMessageLen;

} demoState_t;

typedef struct {
	FILE		*demofile;
	FILE		*saveFile;
	int demoMessageSequence;

	int gameStatesParsed;
} demoFileState_t;

extern demoFileState_t	demo;
extern demoState_t		ds;

int		FS_Write( const void *buffer, int len, FILE	*f );

void	WriteMessage (  sizebuf_t *msg, int seq );
void	WriteConfig ( void );
void	GetTitleLine();
void    GetTagLine();
void	Parse_Snapshot( sizebuf_t *msg );

