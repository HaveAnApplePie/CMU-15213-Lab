/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Linker",
    /* First member's full name */
    "HaveAnApplePie",
    /* First member's email address */
    "https://github.com/HaveAnApplePie",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE   (1<<12) /* Extend heap by this amount (bytes) at least */

#define MAX(x,y)    ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size,alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)      (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((void *)(bp) - WSIZE)
#define FTRP(bp)        ((void *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)   ((void *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)   ((void *)(bp) - GET_SIZE(((void *)(bp) - DSIZE)))

// /* Given the header/footer, compute address of its block ptr bp */
// #define HEAD2BP(p)  ((void *)(p) + WSIZE)
// #define FOOT2BP(P)  ((void *)(p) - GET_SIZE(p) + DSIZE)

void* heap_listp;   /* heap pointer point to the next block of prologue block*/

/* helper functions */
static void* extend_heap(size_t words);
static void* coalesce(void* bp);
static void* find_fit(size_t asize);
static void place(void *bp, size_t asize);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    
    PUT(heap_listp, 0);                             /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));        /* Epilogue header */
    heap_listp += (3*WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;       /* Adjusted block size */
    size_t extendsize;  /* Amount to extend heap if not fit */
    void* bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;
    
    /* Adjust block size to include overhead and alignment reqs */
    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE -1)) / DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        //printf("In size : %p\n",bp);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    //printf("Ne size : %p\n",bp);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *bp, size_t size)
{

    if(bp == NULL)
        return mm_malloc(size);
    
    if(size == 0){
        mm_free(bp);
        return NULL;
    }
        

    void *oldbp = bp;
    void *newbp;
    size_t copySize;    

    newbp = mm_malloc(size);
    if (newbp == NULL)
      return NULL;
    //copySize = *(size_t *)((void *)oldptr - SIZE_T_SIZE);

    copySize = GET_SIZE(HDRP(bp)) < size ? GET_SIZE(HDRP(bp)) : size;

    memcpy(newbp, oldbp, copySize);
    mm_free(oldbp);
    
    return newbp;
}


/* helper functions */
/*
 * extend_heap - Extend the heap with size, and then coalesce
 */
static void* extend_heap(size_t words){
    void *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));           /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));           /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * coalesce - Divided into four cases to merge the prev and 
 *            next blocks of the free block
 */
static void* coalesce(void* bp){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc & next_alloc)            /* Case 1 */
    {
        return bp;
    }
    else if (prev_alloc & !next_alloc)      /* Case 2 */
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if ((!prev_alloc) & next_alloc)      /* Case 3 */
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        
    }
    else{                                   /* Case 4 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));        
    }
    
    return bp;
}

/*
 * find_fit - First-fit search of the implicit free list.
 */
static void* find_fit(size_t asize){
    void* ptr = heap_listp;

    while (GET_SIZE(ptr) != 0 || (!GET_ALLOC(ptr)))
    {
        if(!GET_ALLOC(ptr) && GET_SIZE(ptr) >= asize){
            return (ptr+WSIZE);
        }

        ptr += GET_SIZE(ptr);
    }
    
    return NULL;   
}

/*
 * place - place the requested block and split if it is necessary
 */
static void place(void *bp, size_t asize){
    int block_size = GET_SIZE(HDRP(bp));
    if(block_size >= asize + 2*DSIZE){  /* Split */
        PUT(HDRP(bp), PACK(asize, 1));  /* Header */
        PUT(FTRP(bp), PACK(asize, 1));  /* Footer */

        PUT(HDRP(NEXT_BLKP(bp)), PACK(block_size-asize, 0));    /* Header */
        PUT(FTRP(NEXT_BLKP(bp)), PACK(block_size-asize, 0));    /* Footer */

    }
    else{   /* No split */
        PUT(HDRP(bp), PACK(block_size, 1));   /* Header */
        PUT(FTRP(bp), PACK(block_size, 1));  /* Footer */
    }
}







