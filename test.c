#include <stdio.h>
#include <stdlib.h>

static char *mem_brk;

void *mem_heap_lo()
{
	return (void *) mem_brk;
}


int main(){
	int* a, *b;
	a = (int*)malloc(4);
	b = (int*)malloc(4);
	free(a);
	free(b);
/*char* mem_brk = (char*)malloc(sizeof(char));
	*mem_brk = 3;
	char* test;
	test = (char*)mem_heap_lo();
	*test = 4;
//	*test = 3;
	printf("%p %p %d\n",test, &mem_brk, *mem_brk);
*/

	return 0;
}
