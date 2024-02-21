
#ifndef __myHeap_h
#define __myHeap_h

int   myInit(int sizeOfRegion);
void* myAlloc(int size);
int   myFree(void *ptr);
int   coalesce();
void  dispMem();

void* malloc(size_t size) {
    return NULL;
}

#endif // __myHeap_h__

