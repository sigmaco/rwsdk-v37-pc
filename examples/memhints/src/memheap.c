
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

#include <string.h>

#include <rwcore.h>

#include "memheap.h"

static void
SplitBlock(MemHeapBlockHeader * block, RwUInt32 size)
{
    MemHeapBlockHeader *newBlock;

    /* This if the size is valid. */
    if ((size == 0) || ((size + sizeof(MemHeapBlockHeader)) >= block->size))
    {
        return;
    }

    newBlock = (MemHeapBlockHeader *)
        (((RwUInt8 *) block) + size + sizeof(MemHeapBlockHeader));

    /* Is next block free? */
    if (block->next && (~block->next->flags & MEMHEAP_BLOCKUSED))
    {
        /* Make the rest part of the next block */
        newBlock->next = block->next->next;
        newBlock->size = (block->size - size) + block->next->size;
    }
    else
    {
        /* Create new block */
        newBlock->next = block->next;
        newBlock->size =
            block->size - size - sizeof(MemHeapBlockHeader);
    }

    /* Miscellaneous stuff */
    block->next = newBlock;
    newBlock->flags = (MemHeapBlockFlags) 0;

    /* Link in previous and next of new block */
    newBlock->prev = block;
    if (newBlock->next)
    {
        newBlock->next->prev = newBlock;
    }

    /* Adjust sizes */
    block->size = size;

    /* Copy link to the heap header */
    newBlock->heap = block->heap;
}

RwUInt32
MemHeapCreate(size_t initialSize)
{
    void                *heapMem;
    MemHeapHeader       *heapInfo;
    MemHeapBlockHeader  *firstBlock;
    RwUInt32            start, end;
    RwInt32             blockSize;

    /* Exit if zero. */
    if (!initialSize)
    {
        return 0;
    }
    
    /* Allocate the memory */
    heapMem = OrgMemFuncs.rwmalloc(initialSize, rwMEMHINTDUR_GLOBAL);
    if (!heapMem)
    {
        return 0;
    }
    
    heapInfo = (MemHeapHeader *) heapMem;

    /* Get 32 byte aligned start and end of the heap block */
    start = ((RwUInt32) heapMem + sizeof(MemHeapHeader) + 31) & 0xFFFFFFE0;
    end = ((RwUInt32) heapMem + initialSize) & 0xFFFFFFE0;

    /* Get size of available storage */
    blockSize = end - start - sizeof(MemHeapBlockHeader);
    if (blockSize < 32)
    {
        return 0;
    }

    /* Initialize the first block header */
    firstBlock = (MemHeapBlockHeader *) start;
    firstBlock->heap = heapInfo; /* Link back to heap header */
    firstBlock->next = (MemHeapBlockHeader *) NULL;
    firstBlock->prev = (MemHeapBlockHeader *) NULL;
    firstBlock->flags = (MemHeapBlockFlags) 0;
    firstBlock->size = blockSize;

    /* Initialize heap info */
    heapInfo->firstBlock = firstBlock;
    heapInfo->firstFreeBlock = firstBlock;

    return ((RwUInt32) heapMem);
}

void
MemHeapDestroy(RwUInt32 heapId)
{
    OrgMemFuncs.rwfree((void*) heapId);
}

void *
MemHeapAlloc(RwUInt32 heapId, size_t size)
{
    MemHeapHeader       *heapInfo = (MemHeapHeader *) heapId;
    MemHeapBlockHeader  *targetBlock, *curBlock;

    if (!heapId || !size || !heapInfo->firstBlock)
    {
        return NULL;
    }
    
    /* Round allocation size up to next 32 byte multiple */
    size += 31;
    size &= 0xFFFFFFE0;

    /* Walk the heap...
     * Find the first block that satisfies our needs and
     * return that (splitting off excess if necessary)
     */
    targetBlock = (MemHeapBlockHeader *) NULL;
    curBlock = heapInfo->firstFreeBlock;
    while (curBlock && !targetBlock)
    {
        /* If it's big enough and is not used, use it */
        if ((~curBlock->flags & MEMHEAP_BLOCKUSED)
            && curBlock->size >= size)
        {
            /* This is better than where we are now */
            targetBlock = curBlock;
        }

        curBlock = curBlock->next;
    }

    /* Did memory allocation fail? */
    if (!targetBlock)
    {
        return NULL;
    }

    /* Split off excess into new block (if it's worth it) */
    if (targetBlock->size > (size + 64))
    {
        SplitBlock(targetBlock, size);
    }

    /* May need to find the new first free block */
    if (targetBlock == heapInfo->firstFreeBlock)
    {
        do
        {
            heapInfo->firstFreeBlock = heapInfo->firstFreeBlock->next;
        }
        while (heapInfo->firstFreeBlock &&
               (heapInfo->firstFreeBlock->flags & MEMHEAP_BLOCKUSED));
    }

    /* Mark the block as allocated and return it */
    targetBlock->flags = MEMHEAP_BLOCKUSED;

    return ((void *) (targetBlock + 1));
}

void *
MemHeapRealloc(RwUInt32 heapId, void *mem, size_t newSize)
{
    MemHeapBlockHeader    *block = (((MemHeapBlockHeader *) mem) - 1);

    if (!heapId)
    {
        return NULL;
    }

    /* Call heap alloc if mem is null */
    if (!mem)
    {
        return MemHeapAlloc(heapId, newSize);
    }

    /* Check if the memory belongs to the given heap */
    if (block->heap != ((MemHeapHeader *) heapId))
    {
        return NULL;
    }
    
    /* Call heap free if the newSize is zero */
    if (!newSize)
    {
        MemHeapFree(mem);
        return NULL;
    }

    /* Round allocation size up to next 32 byte multiple */
    newSize += 31;
    newSize &= 0xFFFFFFE0;

    /* Ideally, we'd have enough space to just use this block */
    if (newSize <= block->size)
    {
        /* We're shrinking it, so chop off the end of the block to create
         * another - if it's worth it.
         */
        if (block->size > (newSize + 64))
        {
            MemHeapHeader    *heap = block->heap;

            SplitBlock(block, newSize);

            /* New block may have become the first free one */
            if (!heap->firstFreeBlock
                || (block->next < heap->firstFreeBlock))
            {
                heap->firstFreeBlock = block->next;
            }
        }

        return mem;
    }
    else
    {
        RwUInt32            sizeAfter;
        RwUInt32            sizeNeeded;

        /* We're growing the block. Is the next block unused and
         * does it have enough space? We should, at most, only have 
         * one free block next.
         */
        sizeNeeded = newSize - block->size;
        sizeAfter = 0;
        if (block->next && (~block->next->flags & MEMHEAP_BLOCKUSED))
        {
            /* We have a block we can grow into */
            sizeAfter =
                block->next->size + sizeof(MemHeapBlockHeader);
        }

        if (sizeAfter >= sizeNeeded)
        {
            MemHeapHeader    *heap = block->heap;

            /* We can extend the block in situ */
            if (sizeNeeded + 64 >= sizeAfter)
            {
                /* Don't bother with the wast - just merge the blocks */

                /* May need to find the new first free block */
                if (block->next == heap->firstFreeBlock)
                {
                    do
                    {
                        heap->firstFreeBlock =
                            heap->firstFreeBlock->next;
                    }
                    while (heap->firstFreeBlock &&
                           (heap->firstFreeBlock->
                            flags & MEMHEAP_BLOCKUSED));
                }

                /* Merge the blocks */
                block->next = block->next->next;
                if (block->next)
                {
                    block->next->prev = block;
                }
                block->size += sizeAfter;
            }
            else
            {
                /* Chop off the waste as a separate block */
                MemHeapBlockHeader *newBlock =
                    (MemHeapBlockHeader *) (((RwUInt8 *) mem) + newSize);

                newBlock->next = block->next->next;
                newBlock->prev = block;
                newBlock->size = sizeAfter - sizeNeeded
                    - sizeof(MemHeapBlockHeader);
                newBlock->heap = block->heap;
                newBlock->flags = (MemHeapBlockFlags) 0;

                /* Has new block become the first free block? */
                if (block->next == heap->firstFreeBlock)
                {
                    heap->firstFreeBlock = newBlock;
                }

                /* Links to new block */
                block->next = newBlock;
                if (newBlock->next)
                {
                    newBlock->next->prev = newBlock;
                }

                block->size = newSize;
            }

            /* All sorted now */
            return mem;
        }
        else
        {
            void               *newMem;

            /* We must allocate a new block and copy it */
            newMem = MemHeapAlloc((RwUInt32) block->heap, newSize);

            if (newMem)
            {
                /* We are growing the block, so copy all the old size */
                RwUInt32            numWords = block->size >> 2;
                RwUInt32           *src = (RwUInt32 *) mem;
                RwUInt32           *dst = (RwUInt32 *) newMem;

                while (numWords--)
                {
                    *dst++ = *src++;
                }

                MemHeapFree(mem);
            }

            return newMem;
        }
    }
}

void *
MemHeapCalloc(RwUInt32 heapId,size_t numObj, size_t sizeObj)
{
    void    *mem;

    /* Pass to alloc */
    mem = MemHeapAlloc(heapId, numObj * sizeObj);

    /* Zero memory */
    if (mem)
    {
        memset(mem, 0, numObj * sizeObj);
    }

    return mem;
}

RwBool
MemHeapFree(void *mem)
{
    MemHeapBlockHeader  *block = (((MemHeapBlockHeader *) mem) - 1);
    MemHeapBlockHeader  *prevBlock;
    MemHeapBlockHeader  *nextBlock;

    /* Exit if null. */
    if (!mem)
    {
        return FALSE;
    }    

    /* This block has been freed */
    block->flags = (MemHeapBlockFlags) 0;

    /* Find previous and next (for merging blocks) */
    prevBlock = block->prev;
    nextBlock = block->next;

    /* Check if this is now the first free block */
    if (!block->heap->firstFreeBlock
        || (block < block->heap->firstFreeBlock))
    {
        block->heap->firstFreeBlock = block;
    }

    /* Try and merge with previous block (if there is one) */
    if (prevBlock && (~prevBlock->flags & MEMHEAP_BLOCKUSED))
    {
        /* Merge it */
        prevBlock->next = nextBlock;
        if (nextBlock)
        {
            nextBlock->prev = prevBlock;
        }

        /* Adjust the size */
        prevBlock->size += (block->size + sizeof(MemHeapBlockHeader));

        /* The current block now becomes the previous block */
        block = prevBlock;
    }

    /* try and merge with next block (if there is one) */
    if (nextBlock && (~nextBlock->flags & MEMHEAP_BLOCKUSED))
    {
        /* Merge it */
        block->next = nextBlock->next;
        if (nextBlock->next)
        {
            nextBlock->next->prev = block;
        }

        /* Adjust the size */
        block->size += (nextBlock->size + sizeof(MemHeapBlockHeader));
    }

    return TRUE;
}