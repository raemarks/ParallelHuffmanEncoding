#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <stdint.h>
#include <mpi.h>

extern int p;
extern int mpirank;
/* Number of divisions of compressed data which can be distributed across
 * processors. */
extern uint32_t ndivisions;
/* Array storing the chunk offsets */
extern uint64_t *offsets;
/* Amount of the beginning of the compressed file reserved for the huffman map
 * and the division offsets. */
extern uint32_t metadataOffset;
/* Length of each chunk, with chunks divided up with the devisions */
extern uint64_t *chunkLengths;

#endif
