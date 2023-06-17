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

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* define macros */
#define WSIZE 4 /* word and header/footer size */
#define DSIZE 8 /* double word size */
#define CHUNKSIZE (1<<12) /* extend heap by this amount */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define PUT_PTR(p,ptr) (*(unsigned int*)(p) = (unsigned int)(ptr))
/* Read the size and allocated fields from address p */
/* size is in the front of address and allocated bit is at the lsb */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block pointer bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block pointer bp, compute address of next and previous block */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* pointer computing for segregation free list */
#define PRED_PTR(bp) ((char*)bp)
#define SUCC_PTR(bp) ((char*)(bp)+WSIZE)

#define SEG_PREV(bp) (*(char**)bp)
#define SEG_NEXT(bp) (*(char**)(SUCC_PTR(bp)))


/* number of lists in segregation free lists */
#define NUM_LIST 20
#define SEG_INDEX_LIST(i)  (*((char **)seg_free_list + i))
static char *heap_list;
static void *seg_free_list;

static void *extend_heap(size_t size);
static void *coalesce(void * bp);
static void *find_fit(size_t asize);
static void *place(void* bp, size_t asize);
static void block_add(void *block, size_t size);
static void block_delete(void* block);

/*
int mm_check(void);

// functions for mm_check()
static int ch_mark_free(void);
static int ch_coalesce(void);
static int ch_fb_inlist(void);
static int ch_valid_fb(void);

*/


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //Initialize segregation free list
    int i;

    if((seg_free_list = mem_sbrk(NUM_LIST*WSIZE)) == (void*) - 1)
    {
        return -1;
    }

    for (i = 0; i < NUM_LIST; i++)
    {
        SEG_INDEX_LIST(i) = NULL;
    }

    /* Create the initial empty heap */
    if ((heap_list = mem_sbrk(4*WSIZE)) == (void*) - 1)
    {
        return -1;
    }
    
    PUT(heap_list, 0);  /* alignment paddiing */
    PUT(heap_list + (1*WSIZE), PACK(DSIZE, 1)); /* prologue header */
    PUT(heap_list + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_list + (3*WSIZE), PACK(0, 1)); /* Epilogue header */
    heap_list += (2*WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if(extend_heap(1<<7) == NULL)
    {
        return -1;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    void* bp;
    void* ptr;

    /* ignore spurious requests */
    if(size == 0)
    {
        return NULL;
    }

    asize = ALIGN(size + DSIZE);

    /* search the free list for a fit (first fit) */
    if((bp = find_fit(asize)) != NULL) /* free block exists */
    {
        ptr = place(bp, asize);
        return ptr;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize)) == NULL)
    {
        return NULL;
    }
    ptr = place(bp, asize);
    return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    block_add(ptr,size);
    coalesce(ptr);

    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void* new_block = ptr;
    // int rest;
    int final_size = 0;
    int available_size = 0;

    if(ptr == NULL)
    {
        return mm_malloc(size);
    }
    if(size == 0)
    {
        mm_free(ptr);
        return NULL;
    }

    // alignment 
    size = ALIGN(size + DSIZE);

    // old block size is bigger than input size
    if(GET_SIZE(HDRP(ptr)) > size) 
    {
        return ptr;
    }
    // input size is bigger than old block size
    else if(!GET_ALLOC(HDRP(NEXT_BLKP(ptr)))) // and next block is not allocated
    {
        available_size = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        final_size = available_size;

        if(available_size < size)
        {
            if(extend_heap(MAX(size - available_size, CHUNKSIZE)) == NULL)
            {
                return NULL;
            }
            final_size += MAX(size - available_size, CHUNKSIZE);
        }
        block_delete(NEXT_BLKP(ptr));
        PUT(HDRP(ptr), PACK(final_size, 1));
        PUT(FTRP(ptr), PACK(final_size, 1));
    }
    else // input size > old block size, and next block is allocated. So I can't use the next block
    {
        new_block = mm_malloc(size);
        memcpy(new_block, ptr, GET_SIZE(HDRP(ptr)));
        mm_free(ptr);
    }
    return new_block;
 
}

static void *extend_heap(size_t size)
{
    void *bp;

    /* allocate an even number of words to maintain alignment */
    size = ALIGN(size);

    if((bp = mem_sbrk(size)) == (void*) -1)
    {
        return NULL;
    }

    /* initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));   /* free block header */
    PUT(FTRP(bp), PACK(size, 0));   /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1)); /* new epilogue header */
    block_add(bp, size);

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    /* case 1 : both prev_block, next_block are allocated */
    if(prev_alloc && next_alloc)
    {
        return bp;
    }
    /* case 2 : prev_block is allocated, next_block is free */
    else if(prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))); /* compute size */
        block_delete(bp);
        block_delete(NEXT_BLKP(bp));
        
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    /* case 3 : prev_block is free, next_block is allocated */
    else if(!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        block_delete(PREV_BLKP(bp));
        block_delete(bp);
        
        
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    /* case 4 : both blocks are free */
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        block_delete(PREV_BLKP(bp));
        block_delete(bp);
        block_delete(NEXT_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    block_add(bp, size);
    
    return bp;
}

static void *find_fit(size_t asize)
{
    int i;
    size_t target_size = asize; /* size that I want to search for */
    void *bp = NULL;

    for(i = 0; i < NUM_LIST; i++)
    {
        if((i == NUM_LIST-1) || ((target_size <= 1) && (SEG_INDEX_LIST(i) != NULL)))
        {
            // search in the seg_index_list
            bp = SEG_INDEX_LIST(i);
            while(bp)
            {
                if(asize <= GET_SIZE(HDRP(bp)))
                {
                    break;
                }
                bp = SEG_NEXT(bp);
            }

            // when find the appropriate block, return the block pointer
            if(bp != NULL)
            {
                return bp;
            }
        }
        
        target_size >>= 1;
    }

    
    return NULL;
}

static void *place(void* bp, size_t asize)
{
    size_t blksize = GET_SIZE(HDRP(bp));
    size_t rest = blksize - asize;

    if(rest > 2*DSIZE)
    {
        block_delete(bp);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(rest, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(rest, 0));
        block_add(NEXT_BLKP(bp),rest);
    }
    else
    {
        PUT(HDRP(bp), PACK(blksize, 1));
        PUT(FTRP(bp), PACK(blksize, 1));
        block_delete(bp);
    }

    return bp;
}

static void block_add(void *block, size_t size)
{
    int i=0;
    void *list_ptr = block;
    //void *target_ptr = NULL; // location to insert

    for(; (i < (NUM_LIST - 1))&&(size > 1); i++)
    {
        size >>= 1;
    }
    
    list_ptr = SEG_INDEX_LIST(i);

    if(list_ptr)
    {
        /*
                                    <------
            NULL    <----   block             list_ptr
                                    ------>
        */
        // block -> list_ptr
        PUT_PTR(SUCC_PTR(block), list_ptr);
        // block <- list_ptr
        PUT_PTR(PRED_PTR(list_ptr), block);   
    }
        
    else
    {
        /*
                                    
            NULL    <----   block
                                    
        */
        // block -> NULL
        PUT_PTR(SUCC_PTR(block), NULL);
    }
    // NULL <- block
    PUT_PTR(PRED_PTR(block), NULL);
    // the head of the seg_index_list is current block
    SEG_INDEX_LIST(i) = block;
    return;
}

static void block_delete(void* block)
{
    int i = 0;
    size_t size = GET_SIZE(HDRP(block));
    for(; i < (NUM_LIST - 1)&&(size > 1); i++)
    {
        size >>= 1;
    }

    if(SEG_NEXT(block) && SEG_PREV(block))
    {
        /*
                    ---->          
         prev_block          next_block
                    <----
        */
        // prev_block -> next_block
        PUT_PTR(SUCC_PTR(SEG_PREV(block)), SEG_NEXT(block));
        // prev_block <- next_block
        PUT_PTR(PRED_PTR(SEG_NEXT(block)), SEG_PREV(block));
    }
    else if(SEG_NEXT(block))
    {
        /*
                          
        NULL    <-----     next_block
     
        */
        // NULL <- next_block 
        PUT_PTR(PRED_PTR(SEG_NEXT(block)), NULL);
        // the head of the seg_index_list is next block
        SEG_INDEX_LIST(i) = SEG_NEXT(block);
    }
    else if(SEG_PREV(block))
    {
        /*
                          
        prev_block  ----->    NULL
     
        */
        // prev_block -> NULL
        PUT_PTR(SUCC_PTR(SEG_PREV(block)), NULL);
    }
    else
    {
        // just delete the seg_index_list
        SEG_INDEX_LIST(i) = NULL;
    }
    return;
}

/*

int mm_check(void)
{
    // is every block in the free list marked as free
    if(ch_mark_free() == 0)
    {
        return 0;
    }
    // are there any contiguous free blocks that somehow escape coalescing
    if(ch_coalesce() == 0)
    {
        return 0;
    }
    // is every free block actually in the free list
    if(ch_fb_inlist() == 0)
    {
        return 0;
    }
    // do the pointers in the free list to valid free blocks
    if(ch_valid_fb() == 0)
    {
        return 0;
    }
    return 1;
}

// is every block in the free list marked as free
static int ch_mark_free(void)
{
    void* ptr = NULL;
    int i = 0;
    for(;i<NUM_LIST;i++)
    {
        ptr = SEG_INDEX_LIST(i);
        while(ptr != NULL)
        {
            // header and footer has '0' for free block
            // so, if there is any '1', error must be occured.
            if(!GET_ALLOC(HDRP(ptr)) && !GET_ALLOC(FTRP(ptr)))
            {
                return 0;
            }

            ptr = SEG_NEXT(ptr);
        }
        
    }
    return 1;
}

// are there any contiguous free blocks that somehow escape coalescing
static int ch_coalesce(void)
{
    void* ptr = NULL;
    int i = 0;
    for(;i<NUM_LIST;i++)
    {
        ptr = SEG_INDEX_LIST(i);
        while(ptr != NULL)
        {
            if(!GET_ALLOC(HDRP(PREV_BLKP(ptr))))
            {
                return 0;
            }
            else if(!GET_ALLOC(HDRP(NEXT_BLKP(ptr))))
            {
                return 0;
            }

            ptr = SEG_NEXT(ptr);
        }
        
    }
    return 1;
}

// is every free block actually in the free list
static int ch_fb_inlist(void)
{
    void *ptr = heap_list;
    void *p_list = NULL;
    int i = 0;
    int check = 0;
    while(!ptr)
    {
        // for a given ptr, check in the free list.
        if(!GET_ALLOC(HDRP(ptr))) // free block
        {
            // loop for the free list
            p_list = NULL;
            i = 0;
            check = 0;
            for(;i<NUM_LIST;i++)
            {
                p_list = SEG_INDEX_LIST(i);
                while(p_list != NULL)
                {
                    if(p_list == ptr)
                    {
                        check = 1;
                        break;
                    }
                ptr = SEG_NEXT(ptr);
            }

            if(check == 0)
            {
                return 0;
            }
        
            }
        }
        ptr = NEXT_BLKP(ptr);
    }
    return 1;
}

// do the pointers in the free list to valid free blocks
static int ch_valid_fb(void)
{
    void* ptr = NULL;
    int i = 0;
    int check = 0;
    void* p_heap = NULL;


    for(;i<NUM_LIST;i++)
    {
        ptr = SEG_INDEX_LIST(i);
        while(ptr != NULL)
        {
            p_heap = heap_list;
            while(!p_heap)
            {
                if(p_heap == ptr)
                {
                    check = 1;
                }
                p_heap = NEXT_BLKP(p_heap);
            }
            if(check == 0)
            {
                return 0;
            }
            ptr = SEG_NEXT(ptr);
        }
        
    }
    return 1;
}

*/