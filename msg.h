
#define	MAX_MSGLEN		0x4000	// max length of demo message

typedef struct sizebuf_s {
	qboolean	allowoverflow;	// if false, do a Com_Error
	qboolean	overflowed;	 	// set to true if the buffer size failed
	qboolean	oob;	// don't do Huffman encoding, write raw bytes
	byte		*data;	// pointer to message buffer, set by MSG_Init
	int		maxsize;	// size in bytes of message buffer, set by MSG_Init
	int		cursize;	// number of bytes written to the buffer, set by MSG_WriteBits
	int		readcount;	// number of bytes read from the buffer, set by MSG_ReadBits
	int		bit;		// number of bits written to or read from the buffer
} sizebuf_t;

void MSG_Init (sizebuf_t * buf, byte * data, int length);
void MSG_Bitstream (sizebuf_t * buf);
void MSG_ReadData( sizebuf_t *msg, void *data, int len );

int MSG_ReadBits (sizebuf_t * msg, int bits);
int MSG_ReadByte (sizebuf_t * sb);
int MSG_ReadShort (sizebuf_t * sb);
int MSG_ReadSignedShort (sizebuf_t * sb);
int MSG_ReadLong (sizebuf_t * sb);
float MSG_ReadFloat (sizebuf_t * sb);
char *MSG_ReadString (sizebuf_t * sb);
char *MSG_ReadBigString (sizebuf_t * sb);
char *MSG_ReadStringLine (sizebuf_t * sb);

void MSG_WriteBits( sizebuf_t *msg, int value, int bits );
void MSG_WriteChar( sizebuf_t *sb, int c );
void MSG_WriteByte( sizebuf_t *sb, int c );
void MSG_WriteData( sizebuf_t *buf, const void *data, int length );
void MSG_WriteShort( sizebuf_t *sb, int c );
void MSG_WriteLong( sizebuf_t *sb, int c );
void MSG_WriteFloat( sizebuf_t *sb, float f );
void MSG_WriteString( sizebuf_t *sb, const char *s );
void MSG_WriteBigString( sizebuf_t *sb, const char *s );
void MSG_WriteAngle( sizebuf_t *sb, float f );
void MSG_WriteAngle16( sizebuf_t *sb, float f );
void MSG_Copy(sizebuf_t *buf, byte *data, int length, sizebuf_t *src);

void MSG_WriteDeltaEntity( sizebuf_t *msg, struct entityState_s *from, struct entityState_s *to, qboolean force );
void MSG_WriteDeltaPlayerstate( sizebuf_t *msg, const playerState_t *from, const playerState_t *to );
void MSG_ReadDeltaEntity( sizebuf_t *msg, const entityState_t *from, entityState_t *to, int number );
void MSG_ReadDeltaEntityCFG( sizebuf_t *msg, sizebuf_t *cfg, const entityState_t *from, entityState_t *to, int number );
void MSG_ReadDeltaPlayerstate( sizebuf_t *msg, const playerState_t *from, playerState_t *to );
void MSG_initHuffman ();



