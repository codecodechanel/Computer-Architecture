/*
 * cache.c
 *
 * 20493-02 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 13, 2017
 *
 */


#include <stdio.h>
#include <string.h>
#include "cache_impl.h"

extern int num_cache_hits;
extern int num_cache_misses;

extern int num_bytes;
extern int num_access_cycles;

extern int global_timestamp;

cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC];
int memory_array[DEFAULT_MEMORY_SIZE_WORD];


/* DO NOT CHANGE THE FOLLOWING FUNCTION */
//This function initializes the contents in the main memory
void init_memory_content() {
    unsigned char sample_upward[16] = {0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab, 0x0bc, 0x0cd, 0x0de, 0x0ef};
    unsigned char sample_downward[16] = {0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054, 0x043, 0x032, 0x021, 0x010};
    int index, i=0, j=1, gap = 1;
    
    for (index=0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
        memory_array[index] = (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
        if (++i >= 16)
            i = 0;
        if (++j >= 16)
            j = 0;
        
        if (i == 0 && j == i+gap)	//difference of i and j == gap
            j = i + (++gap);		//increases 1 gap and new j for each cycle
            
        printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
}   

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
//This function initializes the contents in the cache memory
void init_cache_content() {
    int i, j;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        for (j=0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            pEntry->valid = 0;		//invalid
            pEntry->tag = -1;		//no tag	
            pEntry->timestamp = 0;	//no access trial
        }
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
    int i, j, k;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        printf("[Set %d] ", i);
		//for each entry in a set
        for (j=0; j <DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
			//for each block in a entry
            for (k=0; k<DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\t");
        }
        printf("\n");
    }
}
//This function is to return the data in cache
int check_cache_data_hit(void *addr, char type) {
	
	//make variables for later use in this function
	int i;
	int block_address;
	int cache_index;
	int data;
	int byte_address;
	int tag;
	int word_address;
	int word_offset;

	//give values to each variables
	block_address = ((int)(addr) / DEFAULT_CACHE_BLOCK_SIZE_BYTE);
	cache_index = (block_address % CACHE_SET_SIZE);
	byte_address = (int)(addr);
	tag = block_address / CACHE_SET_SIZE;
	word_address = (byte_address / sizeof(int));
	word_offset = byte_address % sizeof(int);

    /* Fill out here */
	//check vaild and tag in the for loop
	for (i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
		//if valid is 1(if a data is in the cache)
		if (cache_array[cache_index][i].valid == 1) {
			//if tag is the same
			if (cache_array[cache_index][i].tag == tag) {
				//update the timestamp
				cache_array[cache_index][i].timestamp = global_timestamp++;



				//depending on the type('b', 'h', 'w'), return the data from the cache
				if (type == 'b') {  // 'b' type
					char b = 0;
					num_bytes += 1;

					//b -> char(reads 1 byte) , h -> short(reads 2 bytes) , w -> int(reads 4 bytes)

					if (word_address % 2 == 1)	//among the two memories that is saved in the cache, bring the second one
						memcpy(&b, cache_array[cache_index][i].data + word_offset + 4, sizeof(char));
					else	//among the two memories that is saved in the cache, bring the first one
						memcpy(&b, cache_array[cache_index][i].data + word_offset, sizeof(char));
					return b;
				}
				else if (type == 'h') { // 'h' type
					short h = 0;
					num_bytes += 2;
					if (word_address % 2 == 1)
						memcpy(&h, cache_array[cache_index][i].data + word_offset + 4, sizeof(short));
					else
						memcpy(&h, cache_array[cache_index][i].data + word_offset, sizeof(short));
					return h;
				}
				else {  // 'w' byte
					int w = 0;
					num_bytes += 4;
					if (word_address % 2 == 1)
						memcpy(&w, cache_array[cache_index][i].data + word_offset + 4, sizeof(int));
					else
						memcpy(&w, cache_array[cache_index][i].data + word_offset, sizeof(int));
					return w;
					//return the hit data
				}
			}
		}
	}

    /* miss */
    return -1;    
}

//This function finds the entry index in set for copying to cache
int find_entry_index_in_set(int cache_index){
	int entry_index=0;
	int j;
	int min = cache_array[cache_index][0].timestamp;	//initiialize the timestamp
	/* Check if there exists any empty cache space by checking 'valid' */
	for (j = 0; j < DEFAULT_CACHE_ASSOC; j++)
	{
		//if valid is 0
		if (cache_array[cache_index][j].valid == 0) {
			entry_index = j;	//entry_index gets the value of the entry that does not have data
			break;	//escapes the for loop
		}
		//if valid is 1
		else {
			//gets the index of the entry that has the smallest timestamp and gives it to entry_index
			if (cache_array[cache_index][j].timestamp < min) {
				min = cache_array[cache_index][j].timestamp;
				entry_index = j;
			}	
		}
	}
	return entry_index;
	/* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
}
//This function returns the data in main memory
int access_memory(void *addr, char type) {
	//make variables for later use in this function
	int i, j;
	int block_address;
	int cache_index;
	int byte_address;
	int word_offset;
	int tag;
	int word_address;
	int set_index =0;

	//give values to each variables
	block_address = ((int)(addr) / DEFAULT_CACHE_BLOCK_SIZE_BYTE);
	cache_index = (block_address % CACHE_SET_SIZE);
	byte_address = (int)(addr);
	tag = block_address / CACHE_SET_SIZE;
	word_address = (byte_address /sizeof(int));
	word_offset = byte_address % sizeof(int);

    /* Fetch the data from the main memory and copy them to the cache */
    /* void *addr: addr is byte address, whereas your main memory address is word address due to 'int memory_array[]' */
	

	/* You need to invoke find_entry_index_in_set() for copying to the cache */
	set_index = find_entry_index_in_set(cache_index);

	//set the values that will be saved in cache
	cache_array[cache_index][set_index].valid = 1;
	cache_array[cache_index][set_index].tag = tag;
	cache_array[cache_index][set_index].timestamp = global_timestamp++;

	//copy the block into cache
	 memcpy(&(cache_array[cache_index][set_index].data), memory_array + (block_address*8/4)
			, sizeof(char)*DEFAULT_CACHE_BLOCK_SIZE_BYTE);

	
    /* Return the accessed data with a suitable type */   
	//depending on the type('b', 'h', 'w'), return the data from the cache
	//b -> char(reads 1 byte) , h -> short(reads 2 bytes) , w -> int(reads 4 bytes)
	if (type == 'b') {  // 'b' type
		char b=0;
		num_bytes += 1;
		if(word_address%2==1)	//among the two memories that is saved in the cache, bring the second one
			memcpy(&b, cache_array[cache_index][set_index].data + word_offset+ 4, sizeof(char));
		else     //among the two memories that is saved in the cache, bring the first one
			memcpy(&b, cache_array[cache_index][set_index].data + word_offset , sizeof(char));
		return b;
	}
	else if (type == 'h') { // 'h' type
		short h = 0;
		num_bytes += 2;
		if (word_address % 2 == 1)
			memcpy(&h, cache_array[cache_index][set_index].data + word_offset+4, sizeof(short));
		else
			memcpy(&h, cache_array[cache_index][set_index].data + word_offset, sizeof(short));
		return h;
	}
	else {  // 'w' byte
		int w = 0;
		num_bytes += 4;
		if(word_address%2==1)
			memcpy(&w, cache_array[cache_index][set_index].data + word_offset+4, sizeof(int));
		else
			memcpy(&w, cache_array[cache_index][set_index].data + word_offset, sizeof(int));
		return w;
	}

}
