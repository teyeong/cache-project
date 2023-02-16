/*
 * main.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implementation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 16, 2022
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

int retrieve_data(void *addr, char data_type) { /* This function is to get the data from cache or memory */
    int value_returned = -1; /* accessed data */
    int byte_addr = *(int*)addr; /* Store "addr" to an integer variable "byte_adrr" to calculate easier */

    /* Invoke check_cache_data_hit() */
    value_returned = check_cache_data_hit(&byte_addr, data_type);
    
    /* In case of the cache miss event, access the main memory by invoking access_memory(), increase "num_cache_misses" by 1, and "num_access_cycles" by 101 */
    if (value_returned == -1) {
        value_returned = access_memory(&byte_addr, data_type);
        num_cache_misses++;
        num_access_cycles += 101;
    }
    else { /* In case of the cache hit event, increase "num_cache_hits" and "num_access_cycless" by 1 */
        num_cache_hits++;
        num_access_cycles++;
    }

    return value_returned; /* Return the "value_returned" */
}

int main(void) {
    FILE *ifp = NULL, *ofp = NULL;
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */ 
    
    init_memory_content(); /* Initiate the memory content */
    init_cache_content(); /* Initiate the cache content */
    
    ifp = fopen("access_input.txt", "r"); /*/ text file to read */
    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }
    /* text file to write */
    ofp = fopen("access_output.txt", "w");
    
    if (ofp == NULL) {
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }
    
    fputs("[Accessed Data]\n", ofp); 
    /* Fill out here by invoking retrieve_data() */
    char buf[255]; /* This is a buffer to read and store one line from the "access_input.txt" */
    while (fgets(buf, sizeof(buf), ifp) != NULL) { /* Read one line until there is nothing more to read */
        int k = sscanf(buf, "%d %c", &access_addr, &access_type); /* Read "access_addr" and "access_type" from "buf" (buffer)*/
        accessed_data = retrieve_data(&access_addr, access_type); /* Invoke retrieve_data() and store the return value in "accessed_data" */
        fprintf(ofp, "%d\t%c\t%#x\n", access_addr, access_type, accessed_data); /* Put "access_data", "access_type" and "accessed_data" into output file */
        global_timestamp++; /* Increment "global_timestamp" by 1 */
    }

    double hit_ratio = (double)(num_cache_hits) / ((double)num_cache_hits + (double)num_cache_misses); /* Calculate the "hit_ratio" */
    double bandwidth = (double)(num_bytes) / (double)(num_access_cycles); /* Calculate the "bandwidth" */

    fputs("----------------------------------------------\n", ofp);
    

    if (DEFAULT_CACHE_ASSOC == 1)
        fputs("[Direct mapped cache performance]\n", ofp);
    else if (DEFAULT_CACHE_ASSOC == 2)
        fputs("[2-way set associative cache performance]\n", ofp);
    else if (DEFAULT_CACHE_ASSOC == 4)
        fputs("[Fully associative cache performance]\n", ofp);

    
    fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n", hit_ratio, num_cache_hits, (num_cache_hits + num_cache_misses)); /* Put "hit_ratio" into output file */
    fprintf(ofp, "Bandwidth = %.2f (%d/%d)", bandwidth, num_bytes, num_access_cycles); /* "Put "bandwidth" into output file */


    fclose(ifp); /* Close the input file */
    fclose(ofp); /* Close the output file */
    
    print_cache_entries(); /* Print the cache entries */

    return 0;
}
