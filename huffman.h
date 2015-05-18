#ifndef HUFFMAN_H
#define HUFFMAN_H
/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
 * Compression book.  The ranks are not actually stored, but implicitly defined
 * by the location of a node within a doubly-linked list */

#define NYT HMAX                /* NYT = Not Yet Transmitted */
#define INTERNAL_NODE (HMAX+1)

typedef struct nodetype
{
  struct nodetype *left, *right, *parent;       /* tree structure */
  struct nodetype *next, *prev; /* doubly-linked list */
  struct nodetype **head;       /* highest ranked node in block */
  int weight;
  int symbol;
} node_t;

#define HMAX 256                /* Maximum symbol */

typedef struct
{
  int blocNode;
  int blocPtrs;

  node_t *tree;
  node_t *lhead;
  node_t *ltail;
  node_t *loc[HMAX + 1];
  node_t **freelist;

  node_t nodeList[768];
  node_t *nodePtrs[768];
} huff_t;

typedef struct
{
  huff_t compressor;
  huff_t decompressor;
} huffman_t;

#if 0
typedef struct
{
  qboolean allowoverflow;       // if false, do a Com_Error
  qboolean overflowed;          // set to true if the buffer size failed (with allowoverflow set)
  qboolean oob;                 // set to true if the buffer size failed (with allowoverflow set)
  byte *data;
  int maxsize;
  int cursize;
  int readcount;
  int bit;                      // for bitwise reads and writes
} msg_t;

void Huff_Compress (msg_t * buf, int offset);
void Huff_Decompress (msg_t * buf, int offset);
#endif
void Huff_Init (huffman_t * huff);
void Huff_addRef (huff_t * huff, byte ch);
int Huff_Receive (node_t * node, int *ch, byte * fin);
void Huff_transmit (huff_t * huff, int ch, byte * fout);
void Huff_offsetReceive (node_t * node, int *ch, byte * fin, int *offset);
void Huff_offsetTransmit (huff_t * huff, int ch, byte * fout, int *offset);
void Huff_putBit (int bit, byte * fout, int *offset);
int Huff_getBit (byte * fout, int *offset);

extern huffman_t clientHuffTables;

#define	SV_ENCODE_START		4
#define SV_DECODE_START		12
#define	CL_ENCODE_START		12
#define CL_DECODE_START		4

#endif  // HUFFMAN_H
