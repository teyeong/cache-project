/*
 * cache.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implementation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 16, 2022
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

#define MAX_BLOCK_ADDR 7 //digit in binary


/* DO NOT CHANGE THE FOLLOWING FUNCTION */
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
        
        if (i == 0 && j == i+gap)
            j = i + (++gap);
            
        printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
}   

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
    int i, j;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        for (j=0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            pEntry->valid = 0;
            pEntry->tag = -1;
            pEntry->timestamp = 0;
        }
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
    int i, j, k;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        printf("[Set %d] ", i);
        for (j=0; j <DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
            for (k=0; k<DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\t");
        }
        printf("\n");
    }
}

int access_data(void* addr, char data_type, cache_entry_t* pEntry) /* This function is to access data from cache */
{
    int byte_addr = *(int*)addr; /* Store "addr" to an integer variable "byte_adrr" to calculate easier */
    int block_addr = byte_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE; /* This is a block address */
    int block_index = block_addr * DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE; /* This is the index of the array to be taken from the main memory */
    int word_addr = byte_addr / WORD_SIZE_BYTE; /* This is a word address */
    int word_offset = byte_addr % WORD_SIZE_BYTE; /* This is a word offset */
    int byte = 0; /* This is an integer variable to store the size of the "data_type" */
    int index = 0; /* This is an integer variable to indicate the starting point of the data */
    int data = -1; /* This is an integer variable for storing the data which we want to find */

    if (word_addr > block_index) /* If "word_addr" is greater than "block_addr", the "index" of the data array(pEntry->data[]) must be at least 4 because the cache copys 2 words and it means the second one */
        index += 4;
    index += word_offset; /* Add "word_offset" to "index" */

    switch (data_type)
    {
    case 'b': /* byte */
        byte = 1; /* the size of one byte */
        data = pEntry->data[index] & 0x000000FF; /* Store one byte of value in "data" */
        break;
    case 'h': /* halfword */
        byte = 2; /* the size of halfword */
        if (index < 7) {
            data = (pEntry->data[index] & 0x000000FF) | ((pEntry->data[index + 1] << 8) & 0x0000FF00); /* Store two bytes of value in "data" */
        }
        break;
    case 'w': /* word */
        byte = 4; /* the size of word */
        if(index < 5)
            data = (pEntry->data[index] & 0x000000FF) | ((pEntry->data[index + 1] << 8) & 0x0000FF00) | ((pEntry->data[index + 2] << 16) & 0x00FF0000) | ((pEntry->data[index + 3] << 24 & 0xFF000000)); /* Store four bytes of value in "data" */
        break;
    }
    num_bytes += byte; /* Add "byte" to "num_bytes" */
    return data; /* Return the "data" obtained */
}

int get_set_number() { /* The function is to find the cache index bits */
    int temp = CACHE_SET_SIZE; /* "CACHE_SET_SIZE" decides the number of set index of block address */
    int index = 0; /* the value to return */
    while (temp > 1) { /* Divide "temp" by 2 and add 1 to "index" until "temp" is greater than 1 */
        temp /= 2;
        index++;
    }
    return index;
}

int get_set(int binary_addr[]) { /* This function is to get the number of set in decimal number */
    int set = 0, j = 1;
    int index = get_set_number();
    for (int i = 0; i < index; i++) { /* Depends on each value of "DEFAULT_CACHE_ASSOC" */
        j = (i == 0) ? 1 : 2 * j; /* If "i" is 0, "j" should be 2^0 so 1. Otherwise multiple 2 and "j" */
        set += binary_addr[i] * j; /* Add to the "set" */
    }
    return set;
}

int get_tag(int binary_addr[]) { /* This function is to get the number of tag */
    int tag = 0, j = 1;
    int index = get_set_number();
    for (int i = index; i < MAX_BLOCK_ADDR; i++) { /* As much as the binary number of block address minus the "set" part */
        j = (i == index) ? 1 : 2 * j; /* If "i" is "index", "j" should be 2^0, so 1. Otherwise multiple 2 and "j" */
        tag += binary_addr[i] * j; /* Add to the "tag" */
    }
    return tag;
}

int check_cache_data_hit(void *addr, char type) { /* This function is to check whether cache data is hit or miss */

    int byte_addr = *(int *)addr; /* Store "addr" to an integer variable "byte_adrr" to calculate easier */
    int block_addr = byte_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE; /* This is a block address */
    int binary_addr[MAX_BLOCK_ADDR] = { 0 }; /* This is a "binary_addr" array to store binary numbers for "block_address" */
    
    /* Store each number of binary numbers in array */
    for (int i = 0; block_addr > 0; i++) {
        binary_addr[i] = block_addr % 2;
        block_addr /= 2;
    }

    int set = get_set(binary_addr); /* Call get_set() to get the number of set */
    int tag = get_tag(binary_addr); /* Call get_tag() to get the number of tag */
    int temp = -1;


    /* Check all entries that have the same tag value as "addr" in cache */
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
        cache_entry_t* pEntry = &cache_array[set][i];
        if (tag == pEntry->tag) {
            temp = i;
            pEntry->timestamp = global_timestamp; /* Store the updated(recent) timestamp */
            break;
        }
    }

    /* Return the data */
    if (temp == -1) { /* miss (There is no entry that have the same tag as "addr") */
        return -1;
    }
    else { /* hit (There is a entry that have the same tag as "addr") */
        return access_data(&byte_addr, type, &cache_array[set][temp]); /* Call access_data() to return the data stored in cache */
    }
}

int find_entry_index_in_set(int cache_index) { /* This function is to find the entry and return it */
    int entry_index = -1; /* Entry index to find */
    int least_recent_used = 100; /* Initialize to any large number to find the least recently used timestamp */
    int index = -1; /* An integer variable to store index temporarily */

    /* Check if there exists any empty cache space by checking "valid" */
    for (int j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t* pEntry = &cache_array[cache_index][j];
            if (pEntry->valid == 0) { /* "valid" 0 means this cache space is empty */
                entry_index = j;
                break;
            }
    }

    /* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
    if (entry_index == -1) {
        for (int j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t* pEntry = &cache_array[cache_index][j];
            if (least_recent_used > pEntry->timestamp) { /* Find the smallest timestamp */
                least_recent_used = pEntry->timestamp;
                index = j;
            }
        }
        entry_index = index;
    }

    return entry_index; 
}

int access_memory(void *addr, char type) { /* This function is to access memory and copy 2 words in cache when the data can't be found in cache */
    
    /* Fetch the data from the main memory and copy them to the cache */
    /* void *addr: addr is byte address, whereas your main memory address is word address due to 'int memory_array[]' */

    /* You need to invoke find_entry_index_in_set() for copying to the cache */
    int byte_addr = *(int*)addr; /* Store "addr" to an integer variable "byte_adrr" to calculate easier */
    int block_addr = byte_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE; /* This is a block address */
    int block_index = block_addr * DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE; /* This is the index of the array to be taken from the main memory */
    int binary_addr[MAX_BLOCK_ADDR] = { 0 }; /* This is a "binary_addr" array to store binary numbers for "block_address" */
    
    /* Store each number of binary numbers in array */
    for (int i = 0; block_addr > 0; i++) {
        binary_addr[i] = block_addr % 2;
        block_addr /= 2;
    }
    int set = get_set(binary_addr); /* Call get_set() to get the number of set */
    int tag = get_tag(binary_addr); /* Call get_tag() to get the number of tag */

    int entry = find_entry_index_in_set(set); /* Call find_entry_index_in_set() to find the "entry" */
    cache_entry_t* pEntry = &cache_array[set][entry]; /* This is a cache entry to store values */
    
    /* Store the values(valid, tag, timestamp, data) */
    pEntry->valid = 1;
    pEntry->tag = tag;
    pEntry->timestamp = global_timestamp;
    
    int temp = memory_array[block_index]; /* Temporary variable to copy the first word */
    int index = 0; /* The index of "pEntry->data" array */


    /* This data is stored as 0xXX or 0xffffffXX */
    pEntry->data[index++] = (temp & 0x000000FF);
    pEntry->data[index++] = (temp & 0x0000FF00) >> 8;
    pEntry->data[index++] = (temp & 0x00FF0000) >> 16;
    pEntry->data[index++] = (temp & 0xFF000000) >> 24;


    temp = memory_array[block_index + 1]; /* Temporary variable to copy the second word */

    pEntry->data[index++] = (temp & 0x000000FF);
    pEntry->data[index++] = (temp & 0x0000FF00) >> 8;
    pEntry->data[index++] = (temp & 0x00FF0000) >> 16;
    pEntry->data[index++] = (temp & 0xFF000000) >> 24;
    

    /* Return the accessed data with a suitable type */    
    return access_data(&byte_addr, type, &cache_array[set][entry]); /* Call access_data() to return the data stored in cache */
}

