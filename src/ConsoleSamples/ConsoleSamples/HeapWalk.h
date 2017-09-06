// crt_heapwalk.c  

// This program "walks" the heap, starting  
// at the beginning (_pentry = NULL). It prints out each  
// heap entry's use, location, and size. It also prints  
// out information about the overall state of the heap as  
// soon as _heapwalk returns a value other than _HEAPOK  
// or if the loop has iterated 100 times.  

#include <stdio.h>  
#include <malloc.h>  

void heapdump(void);

int heap_walk_main(void)
{
	char *buffer;
	
	heapdump();
	printf("====================================\r\n");
	if ((buffer = (char *)malloc(1)) != NULL)
	{
		heapdump();
		free(buffer);
	}
	heapdump();

	return 0;
}

void heapdump(void)
{
	_HEAPINFO hinfo;
	int heapstatus;
	int numLoops;
	hinfo._pentry = NULL;
	numLoops = 0;
	while ((heapstatus = _heapwalk(&hinfo)) == _HEAPOK &&
		numLoops < 100)
	{
		printf("%6s block at %Fp of size %4.4X\n",
			(hinfo._useflag == _USEDENTRY ? "USED" : "FREE"),
			hinfo._pentry, hinfo._size);
		numLoops++;
	}

	switch (heapstatus)
	{
	case _HEAPEMPTY:
		printf("OK - empty heap\n");
		break;
	case _HEAPEND:
		printf("OK - end of heap\n");
		break;
	case _HEAPBADPTR:
		printf("ERROR - bad pointer to heap\n");
		break;
	case _HEAPBADBEGIN:
		printf("ERROR - bad start of heap\n");
		break;
	case _HEAPBADNODE:
		printf("ERROR - bad node in heap\n");
		break;
	}
}