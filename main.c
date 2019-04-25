/*
 * main.c
 *
 * 20493-02 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 13, 2017
 *
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "cache_impl.h"

int num_cache_hits = 0;
int num_cache_misses = 0;

int num_bytes = 0;
int num_access_cycles = 0;

int global_timestamp = 0;

int retrieve_data(void *addr, char data_type) {
    int value_returned = -1; /* accessed data */

    /* Invoke check_cache_data_hit() */
	/* In case of the cache miss event, access the main memory by invoking access_memory() */
	value_returned = check_cache_data_hit(addr, data_type);


	if (!(value_returned == -1)) { //cache hit 
		num_cache_hits++; 
		//increase num_access_cycles 
		num_access_cycles += 1;
	}
	else {  //cache miss 
		num_cache_misses++;
		num_access_cycles += 1;
		value_returned = access_memory(addr, data_type); //access_memory 

		//add 100 cycles if it is a miss
		num_access_cycles += 100; 
	}

    return value_returned;    
}

int main(void) {
    FILE *ifp = NULL, *ofp = NULL;
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */ 
	char* fileName;

    init_memory_content();
    init_cache_content();

	//read access_addr,access_type from the input file 
    ifp = fopen("access_input.txt", "r");
    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }

	//open an output file in write mode
    ofp = fopen("access_output.txt", "w");
    if (ofp == NULL) {
		printf("Can't open output file\n");
        return -1;
    }
    
	fprintf(ofp, "[Accessed Data]\n");
	/* Fill out here by invoking retrieve_data() */
	while (fscanf(ifp, "%d %c", &access_addr, &access_type)==2) {

			fprintf(ofp, "%d\t%c\t%#x\n",access_addr,access_type,retrieve_data(access_addr, access_type));
	
	}

	//write hit ratio and banwidth in the output file
	fprintf(ofp, "-------------------------------\n");
	if (DEFAULT_CACHE_ASSOC == 1) {
		fprintf(ofp, "[Direct mapped cache performance]\n");
	}
	else if (DEFAULT_CACHE_ASSOC == 2) {
		fprintf(ofp, "[2-way set associative cache performance]\n");
	}
	else {
		fprintf(ofp, "[Fully associative cache performance]\n");
	}
	
	//calculate hit ratio, bandwidth 
	fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n",(double)(num_cache_hits) /(num_cache_hits+num_cache_misses),num_cache_hits,num_cache_hits+num_cache_misses);
	fprintf(ofp, "Bandwidth = %.2f (%d/%d)\n",(double)(num_bytes)/num_access_cycles,num_bytes,num_access_cycles);

	//close the input and output file
    fclose(ifp);
    fclose(ofp);

	//print the cache entries
    print_cache_entries();

    return 0;
}
