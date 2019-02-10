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
    "JW_SALANGBANG",
    /* First member's full name */
    "JUNHOO LEE",
    /* First member's email address */
    "ljunhoo98@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define FREE 0
#define WRITTEN 1
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define MINIMUM_BLOCK (SIZE_T_SIZE*3)
#define NEXTFREE(head) ((void*)*(size_t*)(head))
#define SIZE_WRITE(pointer) (*(size_t*)(pointer))
/* 
 * mm_init - initialize the malloc package.
 */
inline void mm_construct(int size, void*(location)){
	size = ((size<<1) + 1);
	SIZE_WRITE(location) = size;
}
inline void mm_free();

inline void mm_nextfree(void*(head),void*(nextfree)){
	*(size_t*)(head) = (size_t)(nextfree);
}

int mm_init(void)
{
	void *head = mem_heap_lo();
	mem_sbrk(SIZE_T_SIZE);
	mm_nextfree(head,mem_sbrk(0));
	return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{ 
	
//    int newsize = ALIGN(size + SIZE_T_SIZE);
//    void *p = mem_sbrk(newsize);
//    if (p == (void *)-1)
//	return NULL;
//    else {
//        *(size_t *)p = size;
//		printf("코드가 리턴하는 주소%p\n",((char *)p + SIZE_T_SIZE));
//		printf("만약 size_t 대신 int가 들어간다면 %p\n",((char *)p + sizeof(int))) ;
//        return (void *)((char *)p + SIZE_T_SIZE);
//	}
	void* next_free;
	int modified_size;
	
	next_free = *(size_t*)(mem_heap_lo());
	((size + SIZE_T_SIZE) > MINIMUM_BLOCK) ? (modified_size = (size + SIZE_T_SIZE)) : (modified_size = MINIMUM_BLOCK);
	//if next free is end of current heap
	if(next_free == mem_sbrk(0)){
		printf("next free %p\n",next_free);
		mm_construct(modified_size,next_free);
		mem_sbrk(modified_size);
		mm_nextfree(mem_heap_lo(), mem_sbrk(0));
		printf("return malloc %p\n",mem_sbrk(0));
		return mem_sbrk(0);
	}

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














