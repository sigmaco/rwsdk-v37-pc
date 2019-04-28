
/****************************************************************************
 *
 * This file is a product of Criterion Software Ltd.
 *
 * This file is provided as is with no warranties of any kind and is
 * provided without any obligation on Criterion Software Ltd.
 * or Canon Inc. to assist in its use or modification.
 *
 * Criterion Software Ltd. and Canon Inc. will not, under any
 * circumstances, be liable for any lost revenue or other damages
 * arising from the use of this file.
 *
 * Copyright (c) 2001 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * main.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the usage of memory hints.
 *
 ****************************************************************************/

#ifndef MEMHEAP_H
#define MEMHEAP_H

extern RwMemoryFunctions OrgMemFuncs;

enum MemHeapBlockFlags
{
    MEMHEAP_BLOCKUSED = 1
};
typedef enum MemHeapBlockFlags MemHeapBlockFlags;

typedef struct MemHeapHeader MemHeapHeader;
typedef struct MemHeapBlockHeader MemHeapBlockHeader;

struct MemHeapHeader
{
    MemHeapBlockHeader *firstBlock;       /* First block in heap list */
    MemHeapBlockHeader *firstFreeBlock;   /* First free block */
};

struct MemHeapBlockHeader
{
    MemHeapHeader           *heap;  /* Pointer to heap header */
    MemHeapBlockHeader      *next;  /* Pointer to next block in heap list */
    MemHeapBlockHeader      *prev;  /* Pointer to prev block in heap list */
    RwUInt32                size;   /* Size of this heap block */
    MemHeapBlockFlags       flags;  /* Heap block flags */
    RwUInt32                pad[3]; /* Align this structure to 32 bytes */
};

#if defined(__cplusplus)
extern "C"
{
#endif /* defined(__cplusplus) */

extern RwUInt32     MemHeapCreate(size_t initialSize);
extern void         MemHeapDestroy(RwUInt32 heapId);
extern void         *MemHeapAlloc(RwUInt32 heapId, size_t size);
extern void         *MemHeapRealloc(RwUInt32 heapId, void *mem,
                                    size_t newSize);
extern void         *MemHeapCalloc(RwUInt32 heapId,size_t numObj,
                                size_t sizeObj);
extern RwBool       MemHeapFree(void *mem);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* MEMHEAP_H */
