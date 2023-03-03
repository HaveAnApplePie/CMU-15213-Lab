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

/* Given free block ptr fbp, compute address of next and previous free blocks */
#define NEXT_FBLKP(fbp)  (*(unsigned int *)(fbp))
#define PREV_FBLKP(fbp)  (*(unsigned int *)((void *)fbp + WSIZE))

/* Given free block ptr fbp, compute address of the current block to store next and previous free blocks */
#define NEXT_FBLKP_STORE(fbp)  ((void *)(fbp))
#define PREV_FBLKP_STORE(fbp)  ((void *)(fbp) + WSIZE)

// /* Given the header/footer, compute address of its block ptr bp */
// #define HEAD2BP(p)  ((void *)(p) + WSIZE)
// #define FOOT2BP(P)  ((void *)(p) - GET_SIZE(p) + DSIZE)

void* heap_listp;   /* heap pointer point to the next block of prologue block*/
void* free_listp;   /* heap free list pointer point to the first free block address */
/* helper functions */
static void* extend_heap(size_t words);
static void* coalesce(void* bp);
static void* find_fit(size_t asize);
static void  place(void *bp, size_t asize);
static void  LIFO(void* bp);
//int mm_check(void);

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
    heap_listp += 3*WSIZE;
    free_listp = NULL;

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
        //mm_check();
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    //printf("Ne size : %p\n",bp);
    //mm_check();
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
    //mm_check();
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
        LIFO(bp);
        return bp;
    }
    else if (prev_alloc & !next_alloc)      /* Case 2 */
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        void * next_bp = NEXT_BLKP(bp);
        
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        if(PREV_FBLKP(next_bp)){
            PUT(NEXT_FBLKP_STORE(PREV_FBLKP(next_bp)), NEXT_FBLKP(next_bp));
        }
        else{   /* root */
            free_listp = (void *)NEXT_FBLKP(next_bp);
        }

        if(NEXT_FBLKP(next_bp)){
            PUT(PREV_FBLKP_STORE(NEXT_FBLKP(next_bp)), PREV_FBLKP(next_bp));
        }

        LIFO(bp);
    }
    else if ((!prev_alloc) & next_alloc)    /* Case 3 */
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));

        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));        
        
        if(PREV_FBLKP(bp)){
            PUT(NEXT_FBLKP_STORE(PREV_FBLKP(bp)), NEXT_FBLKP(bp));
        }
        else{   /* root */
            free_listp = (void *)NEXT_FBLKP(bp);
        }
        if(NEXT_FBLKP(bp)){
            PUT(PREV_FBLKP_STORE(NEXT_FBLKP(bp)), PREV_FBLKP(bp));
        }

        LIFO(bp);        
    }
    else{                                   /* Case 4 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp)));

        void * next_bp = NEXT_BLKP(bp);
        
        if(PREV_FBLKP(next_bp)){
            PUT(NEXT_FBLKP_STORE(PREV_FBLKP(next_bp)), NEXT_FBLKP(next_bp));
        }
        else{   /* root */
            free_listp = (void *)NEXT_FBLKP(next_bp);
        }
        if(NEXT_FBLKP(next_bp)){
            PUT(PREV_FBLKP_STORE(NEXT_FBLKP(next_bp)), PREV_FBLKP(next_bp));
        }

        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));    
        
        if(PREV_FBLKP(bp)){
            PUT(NEXT_FBLKP_STORE(PREV_FBLKP(bp)), NEXT_FBLKP(bp));
        }
        else{   /* root */
            free_listp = (void *)NEXT_FBLKP(bp);
        }
        if(NEXT_FBLKP(bp)){
            PUT(PREV_FBLKP_STORE(NEXT_FBLKP(bp)), PREV_FBLKP(bp));
        }

        LIFO(bp);        

    }
    
    return bp;
}

/*
 * find_fit - First-fit search of the implicit free list.
 */
static void* find_fit(size_t asize){
    void* fbp = free_listp;

    while (fbp != NULL)
    {
        if(GET_SIZE(HDRP(fbp)) >= asize){
            return fbp;
        }

        fbp = (void *)NEXT_FBLKP(fbp);
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

        if(PREV_FBLKP(bp)){
            PUT(NEXT_FBLKP_STORE(PREV_FBLKP(bp)), NEXT_FBLKP(bp));
        }
        else{   /* root */
            free_listp = (void *)NEXT_FBLKP(bp);
        }
        if(NEXT_FBLKP(bp)){
            PUT(PREV_FBLKP_STORE(NEXT_FBLKP(bp)), PREV_FBLKP(bp));
        }

        LIFO(NEXT_BLKP(bp));

    }
    else{   /* No split */
        PUT(HDRP(bp), PACK(block_size, 1));   /* Header */
        PUT(FTRP(bp), PACK(block_size, 1));  /* Footer */
        
        if(PREV_FBLKP(bp)){
            PUT(NEXT_FBLKP_STORE(PREV_FBLKP(bp)), NEXT_FBLKP(bp));
        }
        else{   /* root */
            free_listp = (void *)NEXT_FBLKP(bp);
        }
        if(NEXT_FBLKP(bp)){
            PUT(PREV_FBLKP_STORE(NEXT_FBLKP(bp)), PREV_FBLKP(bp));
        }

    }
}


static void LIFO(void *bp){
    if(free_listp == NULL){
        PUT(NEXT_FBLKP_STORE(bp), 0);
    }
    else{
        PUT(NEXT_FBLKP_STORE(bp), (unsigned int)free_listp); /* set pointer to next free block */
    }

    free_listp = bp;
    
    PUT(PREV_FBLKP_STORE(bp), 0);                        /* set pointer to prev free block*/
    
    if(NEXT_FBLKP(bp))  /* not equal to NULL */
    {
        PUT(PREV_FBLKP_STORE(NEXT_FBLKP(bp)), (unsigned int)bp);  /* Update next free block's prev ptr */
    }
}


// int mm_check(void){
//     void* checkPtr;
//     int freeblk_num = 0;

//     unsigned int heap_first = (unsigned int) mem_heap_lo();
//     unsigned int heap_last  = (unsigned int) mem_heap_hi();

//     // static int num = 0;
//     // num++;
//     // printf("CHECKER NUMBER[%d]\n",num);


//     /* 1. Is every block in the free list marked as free? */
//     /* 2. Are there any contiguous free blocks that somehow escaped coalescing? */
//     /* 4. Do the pointers in a heap block point to valid heap addresses? */
//     checkPtr = free_listp;
//     while(checkPtr != NULL){
//         // printf("Free Pointer:%p Size:%d\n", checkPtr, GET_SIZE(HDRP(checkPtr)));

//         if(GET_ALLOC(HDRP(checkPtr)) != 0){
//             printf("CHECKER ERROR: [1] not marked as free at %p\n", checkPtr);
//             return 0;
//         }

//         if(GET_ALLOC(HDRP(PREV_BLKP(checkPtr))) == 0 || GET_ALLOC(HDRP(NEXT_BLKP(checkPtr))) == 0 ){
//             printf("CHECKER ERROR: [2] free blocks escaped coalescing at %p\n", checkPtr);
//             return 0;
//         }

//         if(NEXT_FBLKP(checkPtr) != 0){
//             if(NEXT_FBLKP(checkPtr) < heap_first ||
//                NEXT_FBLKP(checkPtr) > heap_last){
//                     printf("CHECKER ERROR: [4-1] point to invalid heap address at %p\n", checkPtr);
//                     return 0;                
//                }
//         }

//         if(PREV_FBLKP(checkPtr) != 0){
//             if(PREV_FBLKP(checkPtr) < heap_first ||
//                PREV_FBLKP(checkPtr) > heap_last){
//                     printf("CHECKER ERROR: [4-2] point to invalid heap address at %p\n", checkPtr);
//                     return 0;                
//                }
//         }


//         freeblk_num++;
//         checkPtr = (void *)NEXT_FBLKP(checkPtr);
//     }


//     /* 3.  Is every free block actually in the free list? */
//     checkPtr = heap_listp;
//     int freeblk_count = 0;

//     while(GET_ALLOC(checkPtr) != 1 || GET_SIZE(checkPtr) != 0){
//         if (GET_ALLOC(checkPtr) == 0)
//         {
//             freeblk_count++;
//         }
//         checkPtr += GET_SIZE(checkPtr);
//     }

//     if(freeblk_count != freeblk_num){
//             printf("CHECKER ERROR: [3] not every free block in the free list\n");
//             return 0;       
//     }


    

//     return 1;
// }


