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
    "I've, wrote almost 5,000 chars in here, and it printed all chars what i've written. I think CMU's TA should watch attack lab videos.",
	/* First member's full name */
    "JunHoo Lee",
    /* First member's email address */
    "wlkw",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};


/* MY MACROS  */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
//write size masked with allocated boolean
#define PACK(size, alloc) ((size) | (alloc))
//get adress of written in this block
#define GET(p)			(*(unsigned int *)(p))
//write adress, or useful numbers in defined block
#define PUT(p, val) 	(*(unsigned int *)(p) = (val))
//get size by unmasking allocation boolean
#define GET_SIZE(p)		(GET(p) & ~0x7)
//get allocation boolean
#define GET_ALLOC(p)	(GET(p) & 0x1)
//i allocated size block just before handling pointer
#define HDRP(bp)		((char *)(bp) - WSIZE)
//i allocated size block both side of block
#define FTRP(bp)		((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
//adding size of current block size into current pointer, by doing this, we can get next adress pointer
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
//vice versa
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

//since we've defined next,previous block's pointer, we can get its size
#define GET_NEXT_SIZE(bp) (GET_SIZE(HDRP(NEXT_BLKP(bp))))       
#define GET_PREV_SIZE(bp) (GET_SIZE(HDRP(PREV_BLKP(bp))))       
//i defined writing prev free node in given pointer, and next free node pointer by next block.
#define PREV_FREE_NODE(bp)	((char*)bp)
#define NEXT_FREE_NODE(bp)	((char*)bp + WSIZE)
//should wipe out allocated pointer when deleting block.
#define MASK_PREV_NODE(bp)  (PUT(PREV_FREE_NODE(bp), NULL))
#define MASK_NEXT_NODE(bp)  (PUT(NEXT_FREE_NODE(bp), NULL))


/* MY MACROS  */

/* MY GLOBAL VARIABLES */

static char* heap_listp = 0; //to handle stack position easily
static char* free_listp = 0; //indicate free list's header
static int ll_size = 0;		//indicate free list's size

/* MY GLOBAL VARIABLES */


/* MY FUNCTIONS  */
void check_heap(void);
int mm_init(void);
void *mm_malloc(size_t size);
void  mm_free(void* ptr);
void *mm_realloc(void *ptr, size_t size);
static void *extend_heap(size_t words);
static inline void *coalesce(void *bp);
static inline void *first_fit(size_t asize);
static inline void place(char *bp, size_t asize);
static inline void insert_node(char *bp);
static inline void delete_node(char *bp);


/* 
 *  heap checker, it checks prologue, epilogue, coalesce
 * to see whether it is allighed optimally.
 * i commented it line to line.
 */


void check_heap(){
	char* ref_pointer = heap_listp;
	int free_flag = 0;
	//check prologue
	if((GET_SIZE(ref_pointer) == DSIZE) && GET_ALLOC(ref_pointer)){
		printf("prologue alligned properly\n");
	}
	else{
		printf("invalid prologue!!!\n");
	}
	
	//check memory allign
	while(GET_SIZE(HDRP(ref_pointer)) != 0){
		if(GET_ALLOC(ref_pointer) == 1){
			if(free_flag == 1) printf("memory optimization error\n");
			else free_flag = 1;
			
		}
		ref_pointer = NEXT_BLKP(ref_pointer);
	}
	//check epilogue
	if(GET_ALLOC(HDRP(ref_pointer) != 1)){
		printf("epilogue error!");
	}
	else 
		printf("it alligned properly");
}

/* MY FUNCTIONS  */
//will discuss below
/* 
 * mm_init - initialize the malloc package.
 * i allocated free_list printer in the beginning of heap, 
 *and gave prologue and epilogue in order to alligh 8 byte.
 *also, i extended heap a lot to reduce function calls.
 * 
 */
int mm_init(void)
{	
//	if(init_flag != 0) return 0;
//	printf("init\n");
	ll_size = 0;
	if((heap_listp = mem_sbrk(4*WSIZE)) == (void *) -1)
		return -1;
	PUT(heap_listp, 0);							/*free linked list*/
	PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));/* Prologue header  */
	PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));/* Prologue footer  */
	PUT(heap_listp + (3*WSIZE), PACK(0,	1));	/* Epilogue header  */
	free_listp = heap_listp;
	heap_listp += (2*WSIZE);


	if ((extend_heap(CHUNKSIZE/WSIZE)) == NULL) {
		return -1;
	}
	


    return 0;
}
/*
 *
 *
 *
 *
 */

static void *extend_heap(size_t words)
{

	char* bp;
	size_t size;
	
	/* Allocate even number to size  */

	size = (words % 2)  ? (words + 1) * WSIZE : words *WSIZE;

	if((long)(bp = mem_sbrk(size)) == -1)
		return NULL;
	/* init free block header/footer and the epilogue header  */
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
	
	if (free_listp == 0){

	}

	return coalesce(bp);

}

/*
* what i've first wrote is best_fit, but it took so long time,
* and it couldn't pass re_malloc trace scenario. so i changed
* best_fit to first_fit into tears. it is just first fit, 
* when it found free_node which size is bigger than required
* size, it returns, or it retss NULL
*/


static inline void *first_fit(size_t asize){

	if(ll_size <= 0) return NULL;

	int i;
	char* ref_pointer = GET(free_listp);
	
	for(i = 0; i < ll_size; ++i){
		if(GET_SIZE(HDRP(ref_pointer)) >= asize) return ref_pointer;
		ref_pointer = GET(NEXT_FREE_NODE(ref_pointer));
	}

	return NULL;
	
}

/* coalesce handles free node in order to optimize space.
* when we free new  blocks, continous free block allign case
* can occur. it handles this case.
*
*i separated this case into 3 tiny case, and i handled it
*
*
 */


static inline void *coalesce(void *bp){

	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));

	if(prev_alloc && next_alloc){

	}
	else if(prev_alloc && !next_alloc){
		delete_node(NEXT_BLKP(bp));
		size += GET_NEXT_SIZE(bp);
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	else if(!prev_alloc && next_alloc){
		delete_node(PREV_BLKP(bp));
		size += GET_PREV_SIZE(bp);
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}
	else{

		delete_node(NEXT_BLKP(bp));
		delete_node(PREV_BLKP(bp));
		size += (GET_PREV_SIZE(bp) + GET_NEXT_SIZE(bp));
		bp = PREV_BLKP(bp);
		PUT(HDRP(bp) , PACK(size, 0));
		PUT(FTRP(bp) , PACK(size, 0));

	}

	insert_node(bp);

	return bp;

}
/*
*	simple function, it adds new node into free linkedlist
*	when first case, we should mask first bock
*
 */

static inline void insert_node(char* bp){
	


	if(ll_size ==0){

		PUT(free_listp, bp);
		PUT(PREV_FREE_NODE(bp), free_listp);
		MASK_NEXT_NODE(bp);
	}
	else{
		char* next_node = GET(free_listp);
		PUT(PREV_FREE_NODE(next_node), bp);
		PUT(NEXT_FREE_NODE(bp), next_node);
		PUT(PREV_FREE_NODE(bp), free_listp);
		PUT(free_listp, bp);
	}
		++ll_size;

}

/*
 * simple deleting free node. when deleting, we should
 * mask previous_free block.
*/
static inline void delete_node(char* bp){
	


	ll_size--;
	
	int  tail = 0;

	char* prev_node;
	char* next_node;

	if(ll_size < 0) printf("ll_size is minus");
	if(GET(NEXT_FREE_NODE(bp)) == NULL){
		tail = 1;
	}
	else{
		next_node = GET(NEXT_FREE_NODE(bp));
	}
	
	prev_node = GET(PREV_FREE_NODE(bp));	

	
	if(prev_node == free_listp){
		if(tail == 1){
		PUT(free_listp, NULL);
		}
		else{
		PUT(free_listp, next_node);
		PUT(PREV_FREE_NODE(next_node),free_listp);
		}
	}

	else{
		if(tail == 1){
			MASK_NEXT_NODE(prev_node);
		}
		else{
			PUT(NEXT_FREE_NODE(prev_node),next_node);
			PUT(PREV_FREE_NODE(next_node),prev_node);
		}
	}
	MASK_PREV_NODE(bp);
	MASK_NEXT_NODE(bp);

}

/*
 * allocate size block into assigned pointer with assigned
 * size, when it is too small than free block, divide block
 * in order to optimize space
*/
static inline void place(char* bp, size_t asize){


	size_t node_room = GET_SIZE(HDRP(bp));

	if (node_room > (asize + DSIZE * 2)){
		delete_node(bp);
		size_t spare_size = node_room - asize;
		PUT(FTRP(bp), PACK(spare_size, 0));
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		PUT(HDRP(NEXT_BLKP(bp)), PACK(spare_size, 0));
		
		insert_node(NEXT_BLKP(bp));
		return;
	}
	delete_node(bp);	
	PUT(HDRP(bp), PACK(node_room, 1));
	PUT(FTRP(bp), PACK(node_room, 1));
	
}

/*main function. first, adjust size to required size, round up
 * to 8, then add block size padding, then find location by
 * using first_fit function. then place it by using place function
 * heap has no enough space, extend it!
*/


void *mm_malloc(size_t size)
{
	size_t asize;
	size_t extendsize;
	char* bp;

	if(size == 0)
		return NULL;

	if(size <= DSIZE)
		asize = DSIZE * 2;

	else
		asize = DSIZE * ((size + (DSIZE) + (DSIZE -1)) / DSIZE);
	
	if((bp = first_fit(asize)) != NULL){
		place(bp, asize);
		return bp;
	}

	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
		return NULL;
	place(bp, asize);
	return bp;

}

/*
 * get size and mask it. and call coalesce to optimize space
 */
void mm_free(void *ptr)
{
	size_t size = GET_SIZE(HDRP(ptr));
	PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * just simply free previous block, and malloc it to required size. easy!
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














