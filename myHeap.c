
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "myHeap.h"
 
/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 * 
 * NOT COMPLETED BY ALEJANDRO
 */
typedef struct blockHeader {           

    int size_status;
    /*
     * Size of the block is always a multiple of 8.
     * Size is stored in all block headers and in free block footers.
     *
     * Status is stored only in headers using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit 
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     * Examples:
     * 
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     * 
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
} blockHeader;         

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */
blockHeader *heapStart = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int allocsize;

/*
 * Additional global variables may be added as needed below
 */
const int specSize = 8;
 
/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if not positive or if larger than heap space.
 * - Determine block size rounding up to a multiple of 8 
 *   and possibly adding padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to chose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 1. Update all heap blocks as needed for any affected blocks
 *   - 2. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split 
 *   - 1. SPLIT the free block into two valid heap blocks:
 *         1. an allocated block
 *         2. a free block
 *         NOTE: both blocks must meet heap block requirements 
 *       - Update all heap block header(s) and footer(s) 
 *              as needed for any affected blocks.
 *   - 2. Return the address of the allocated block payload
 *
 * - If a BEST-FIT block found is NOT found, return NULL
 *   Return NULL unable to find and allocate block for desired size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the 
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* myAlloc(int size) {     
    /* first, check the size value */
    if (size < 1 || size > allocsize) {
        return NULL;
    }
    
    //1 - take the given size, and add the header
    int trueSize = size + 4;
    
    //2 - create value to store the remainder
    int remVal = trueSize % specSize;
    
    //3 - if the remVal  = 0 do nothing, else calculate the true size
    if (remVal != 0) {
        trueSize = trueSize + (specSize - remVal);
    }

    //4 - check again if size is bigger than our alotted memory
    if (trueSize > allocsize) {
        return NULL;
    }
    
    //5 - create variables for iteration
    blockHeader* currentBlock = heapStart;
    blockHeader* endBlock = (blockHeader*) ((void*) currentBlock + allocsize);
    blockHeader* closestBlock = NULL;
    blockHeader* footerBlock = NULL;
    blockHeader* nextBlock = NULL;
    blockHeader* splitBlock = NULL;
    void* payLoadBlock = NULL;

    int currentVal = 0;
    int memoryVal = 0;
    int isFree = 0;
    int closeHead = 0;

    /* Begin While Loop */
    while (currentBlock->size_status != 1) {
        //reset used values each iteration
        memoryVal = 0;
        isFree = 0;

        //grab currentVal
        currentVal = currentBlock->size_status;

        /*Check remainder of currentVal to determine memoryval */
        //1 - block is free, and previous free
        if (currentVal % specSize == 0) {
            //set memoryVal
            memoryVal = currentVal;

            //set isFree to T (1)
            isFree = 1;
        }
        //2 - block is free, and previous alloc'd
        else if (currentVal % specSize == 2) {
            //set memoryVal
            memoryVal = currentVal - 2;
            
            //set isFree to T (1)
            isFree = 1;
        }
        //3 - block is allocated, determine which
        else {
            //block is allocated, and previous alloc'd
            if (currentVal % specSize == 3) {
                memoryVal = currentVal - 3;
            }
            //else the block is alloc'd, previous free
            else{
                memoryVal = currentVal - 1;
            }
        }

        //if isFree is T enter
        if (isFree == 1) {
            //base - if the memory val is exactly trueSize allocate
            if (memoryVal == trueSize) {
                //set payLoad
                payLoadBlock = (void*) ((void*) currentBlock + 4);

                //set the header of our block
                currentBlock->size_status += 1;

                //establish footer, next
                nextBlock = (blockHeader*) ((void*)currentBlock + memoryVal);
                footerBlock = (blockHeader*) ((void*)nextBlock - 4);

                //clear footer
                footerBlock->size_status = 0;
                footerBlock = NULL;

                //if the nextblock is not end, update nextblock
                if (nextBlock != endBlock) {
                    nextBlock->size_status += 2;
                }

                //immedately return payload
                return payLoadBlock;
            }
            //else check if memory is large enough to fit trueSize
            else if (memoryVal > trueSize) {
                //if closest is not null we check it
                if (closestBlock != NULL) {
                    if (currentVal < closestBlock->size_status) {
                        closestBlock = currentBlock;
                    }
                }
                else {
                    closestBlock = currentBlock;
                }
            }
        }

        //head to the next value
        currentBlock = (blockHeader*) ((void*) currentBlock + memoryVal);
    }
    
    //after we iterate if the closestBlock isn't empty we allocate
    if (closestBlock != NULL) {
        //check block sizestatus to set memoryVal
        if (closestBlock->size_status % specSize == 0) {
            memoryVal = closestBlock->size_status;
        }
        else {
            memoryVal = closestBlock->size_status - 2;
            closeHead = 2;
        }

        //create the split
        splitBlock = (blockHeader*) ((void*) closestBlock + trueSize);

        //set the new blocks sizestatus 
        splitBlock->size_status = (memoryVal - trueSize) + 2;

        //update the footer
        footerBlock = (blockHeader*) ((void*) splitBlock + ((memoryVal - trueSize) - 4));
        footerBlock->size_status = memoryVal - trueSize;

        //update the sizeStatus of the currentblock
        closestBlock->size_status = trueSize + (closeHead + 1);

        //set payload and return
        payLoadBlock = (void*) ((void*) closestBlock + 4);

        return payLoadBlock;
    }

    //if we don't have any allocation, return null
    return NULL; 
} 
 
/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 */                    
int myFree(void *ptr) {    
    //TODO: Your code goes in here.
    //if ptr is null return
    if (ptr == NULL) {
        return -1;
    }

    //if ptr % specSize != 8 return
    if (((int)ptr % specSize) != 0) {
        return -1;
    }
    
    //check if the ptr is out of our heap block
    blockHeader* endBlock = (blockHeader*) ((void*) heapStart + allocsize);

    if (ptr > (void*) endBlock || ptr < (void*) heapStart) {
        return -1;
    }
    
    //establish a block header given the address
    blockHeader* freeBlock = (blockHeader*) (ptr - 4);

    //create variables for function
    int blockRem = freeBlock->size_status % specSize;
    int memoryVal = 0;
    blockHeader* nextBlock = NULL;
    blockHeader* footerBlock = NULL;

    //if the block size mod 8 is not 0 or 2, begin free process
    if (blockRem != 0 && blockRem != 2) {
        //case 1 - block is alloc'd and previous is alloc'd
        if (blockRem == 3) {
            //assign memory value
            memoryVal = freeBlock->size_status - 3;
        }
        //case 2 - block is alloc'd and previous is free
        else if (blockRem == 1) {
            //assing memory value
            memoryVal = freeBlock->size_status - 1;
        }
        
        /* Memory Freeing */
        //1 - assign the nextBlock
        nextBlock = (blockHeader*) ((void*) freeBlock + memoryVal);

        //2 - set the free block's sizeStatus - 1
        freeBlock->size_status -= 1;

        //3 - add the footer
        footerBlock = (blockHeader*) ((void*) nextBlock - 4);
        footerBlock->size_status = memoryVal;

        //4 - update the header of the next block if next is not the end
        if (nextBlock != endBlock) {
            nextBlock->size_status -= 2;
        }

        //return 0 upon completion
        return 0;
    }
    
    //if the block is already freed don't do anything
    return -1;
} 

/*
 * Function for traversing heap block list and coalescing all adjacent 
 * free blocks.
 *
 * This function is used for delayed coalescing.
 * Updated header size_status and footer size_status as needed.
 */
int coalesce() {
    //TODO: Your code goes in here.
	//establish variables to use for coalesce
    blockHeader* currentBlock = heapStart;
    blockHeader* nextBlock = NULL;
    blockHeader* footerBlock = NULL;

    int currentVal = 0;
    int currentRem = 0;
    int nextVal = 0;
    int nextRem = 0;
    int newMemory = 0;
    int hasCoalesced = 0;

    /* Begin iteration */
    while (currentBlock->size_status != 1) {
        //before doing anything we need to check if currentBlock is free
        if (currentBlock->size_status % specSize == 2 || currentBlock->size_status % specSize == 0) {
            //first grab memory value of currentBlock
            if (currentBlock->size_status % specSize == 2) {
                currentVal = currentBlock->size_status - 2;
                currentRem = 2;
            }
            else {
                currentVal = currentBlock->size_status;
            }

            /* create nextBlock, check if free */
            nextBlock = (blockHeader*) ((void*)currentBlock + currentVal);

            if (nextBlock->size_status % specSize == 2 || nextBlock->size_status % specSize == 0) {
                //grab the nextBlock value first
                if (nextBlock->size_status % specSize == 2) {
                    nextVal = nextBlock->size_status - 2;
                    nextRem = 2;
                }
                else {
                    nextVal = nextBlock->size_status;
                }

                /* begin coalesce */
                //set new mem
                newMemory = currentVal + nextVal;

                //1 - grab the footer of current
                footerBlock = (blockHeader*) ((void*) currentBlock + (currentVal - 4));
                
                footerBlock->size_status = 0;
                footerBlock = NULL;

                //2 - update the size-status of the Current
                currentBlock->size_status = newMemory + currentRem + nextRem;

                //3 - update the nextBlocks info (header removal, footer update)
                footerBlock = (blockHeader*) ((void*) nextBlock + (nextVal - 4));
                footerBlock->size_status = currentVal + nextVal;

                nextBlock->size_status = 0;
                nextBlock = NULL;

                //4 - check new next value, update current accordingly
                nextBlock = (blockHeader*) ((void*)currentBlock + newMemory);

                //if the nextBlock is not free, current block becomes current + next 
                if (nextBlock->size_status % specSize == 1) {
                    nextVal = nextBlock->size_status - 1;
                    
                    currentBlock = (blockHeader*) ((void*) currentBlock + (newMemory + nextVal));
                    
                }
                
                //if we've coalesced at any point, we return one as the action has completed.
                hasCoalesced = 1;

            }
            //next block isn't free, we can skip over current and next (special case)
            else {
                //grab memory value of nextBlock
                if (nextBlock->size_status % specSize == 3) {
                    nextVal = nextBlock->size_status - 3;
                }
                else {
                    nextVal = nextBlock->size_status - 1;
                }

                //set current to the block of memory after next
                currentBlock = (blockHeader*) ((void*) currentBlock + (currentVal + nextVal));
            }
        }
        //block isn't free, go to the next
        else {
            //grab memory value
            if (currentBlock->size_status % specSize == 3) {
                currentVal = currentBlock->size_status - 3;
            }
            else {
                currentVal = currentBlock->size_status - 1;
            }

            //set current as the next block
            currentBlock = (blockHeader*) ((void*)currentBlock + currentVal);
        }
    }

    //return hasCoalesced value (0 if not done, 1 if done)
    return hasCoalesced;
}

 
/* 
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int myInit(int sizeOfRegion) {    
 
    static int allocated_once = 0; //prevent multiple myInit calls
 
    int pagesize;   // page size
    int padsize;    // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int fd;

    blockHeader* endMark;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    allocsize = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
    allocated_once = 1;

    // for double word alignment and end mark
    allocsize -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heapStart = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    endMark = (blockHeader*)((void*)heapStart + allocsize);
    endMark->size_status = 1;

    // Set size in header
    heapStart->size_status = allocsize;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heapStart->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heapStart + allocsize - 4);
    footer->size_status = allocsize;
  
    return 0;
} 
                  
/* 
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void dispMem() {     
 
    int counter;
    char status[6];
    char p_status[6];
    char *t_begin = NULL;
    char *t_end   = NULL;
    int t_size;

    blockHeader *current = heapStart;
    counter = 1;

    int used_size = 0;
    int free_size = 0;
    int is_used   = -1;

    fprintf(stdout, 
	"*********************************** Block List **********************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
    fprintf(stdout, 
	"*********************************************************************************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout, 
	"*********************************************************************************\n");
    fflush(stdout);

    return;  
} 


// end of myHeap.c (Spring 2022)                                         


