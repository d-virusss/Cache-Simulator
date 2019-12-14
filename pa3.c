/**********************************************************************
 * Copyright (c) 2019
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

 // 2019.12.12 02:00 지금부터 과제 시작합니다 ^^
 // 2019.12.12 23:00 과제 이해중
// ㅁㄴㅇㄹ

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

enum cache_simulator_constants {
	CACHE_HIT = 0,
	CACHE_MISS,

	CB_INVALID = 0,	/* Cache block is invalid */
	CB_VALID = 1,	/* Cache block is valid */

	CB_CLEAN = 0,	/* Cache block is clean */
	CB_DIRTY = 1,	/* Cache block is dirty */

	BYTES_PER_WORD = 4,	/* This is 32 bit machine (1 word is 4 bytes) */
	MAX_NR_WORDS_PER_BLOCK = 32,	/* Maximum cache block size */
};


typedef unsigned char bool;
#define true  1
#define false 0

/* 8 KB Main memory */
static unsigned char memory[8 << 10] = {
	0xde, 0xad, 0xbe, 0xef, 0xba, 0xda, 0xca, 0xfe,
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
	'h',  'e',  'l',  'l',  'o',  ' ' , 'w' , 'o',
	'r',  'l',  'd',  '!',  0x89, 0xab, 0xcd, 0xef,
	0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e,
	0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e,
	0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e,
	0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e,
};

/* Cache block */
struct cache_block {
	bool valid;				/* Whether the block is valid or invalid.
							   Use CB_INVALID or CB_VALID macro above  */
	bool dirty;				/* Whether te block is updated or not.
							   Use CB_CLEAN or CB_DIRTY macro above */
	unsigned int tag;		/* Tag */
	unsigned int timestamp;	/* Timestamp or clock cycles to implement LRU */
	unsigned char data[BYTES_PER_WORD * MAX_NR_WORDS_PER_BLOCK];
	/* Each block holds 4 words */
};

/* An 1-D array for cache blocks. */
static struct cache_block *cache = NULL;

/* The size of cache block. The value is set during the initialization */
static int nr_words_per_block = 4;

/* Number of cache blocks. The value is set during the initialization */
static int nr_blocks = 16;

/* Number of ways for the cache. Note @nr_ways == 1 means direct mapped cache
 * and @nr_ways == nr_blocks implies fully associative cache */
static int nr_ways = 2;

/* Number of @nr_ways-way sets in the cache. This value will be set according to
 * @nr_blocks and @nr_ways values */
static int nr_sets = 8;

/* Clock cycles */
const int cycles_hit = 1;
const int cycles_miss = 100;

/* Clock cycles so far */
static unsigned int cycles = 0;

int now_index = 0;

#define MAX_INTEGER 100000000;


/**
 * strmatch()
 *
 * DESCRIPTION
 *   Compare strings @str and @expect and return 1 if they are the same.
 *   You may use this function to simplify string matching :)
 *
 * RETURN
 *   1 if @str and @expect are the same
 *   0 otherwise
 */
static inline bool strmatch(char * const str, const char *expect)
{
	return (strlen(str) == strlen(expect)) && (strncmp(str, expect, strlen(expect)) == 0);
}

/**
 * log2_discrete
 *
 * DESCRIPTION
 *   Return the integer part of log_2(@n). FREE TO USE IN YOUR IMPLEMENTATION.
 *   Will be useful for calculating the length of tag for a given address.
 */
static int log2_discrete(int n)
{
	int result = -1;
	do {
		n = n >> 1;
		result++;
	} while (n);

	return result;
}
/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/


/**************************************************************************
 * load_word
 *
 * DESCRIPTION
 *   Simulate the case when the processor is handling a lw instruction for @addr.
 *   To that end, you should look up the cache blocks to find the block
 *   containing the target address @addr. If exists, it's cache hit; return
 *   CACHE_HIT after updating the cache block's timestamp with @cycles.
 *   If not, replace the LRU cache block in the set. Should handle dirty blocks
 *   properly according to the write-back semantic.
 *
 * PARAMAMETERS
 *   @addr: Target address to load
 *
 * RETURN
 *   CACHE_HIT on cache hit, CACHE_MISS otherwise
 *
 */
int load_word(unsigned int addr)
{
	/* TODO: Implement your load_word function */
	int bit_tag, bit_index, bit_offset;
	bit_offset = log2_discrete(nr_words_per_block * 4);
	bit_index = log2_discrete(nr_sets);
	bit_tag = 32 - bit_index - bit_offset;

	int addr_index;
	int addr_set;
	int start_word;


	addr_index = (addr / (nr_words_per_block * 4)) % nr_blocks; // 받은 주소에 해당하는 block 넘버 계산
	addr_set = (addr_index % nr_sets); // 받은 주소에 해당하는 set 넘버 계산
	start_word = (addr / 16) * 16;
	int old_block_timestamp = MAX_INTEGER;
	int old_block_addr = 0;


	int count = 0; // word 수 카운트

	for (int i = (addr_set * nr_ways); i < (addr_set * nr_ways) + nr_ways; i++) // set에서 맨 처음 block부터 돌면서 check
	{

		if (cache[i].timestamp < old_block_timestamp)
		{
			old_block_timestamp = cache[i].timestamp;
			old_block_addr = i;
		}
		if (cache[i].valid == CB_INVALID) // block이 invalid 였을 때
		{
			cache[i].timestamp = cycles;
			cache[i].valid = CB_VALID;
			cache[i].tag = addr >> (bit_offset + bit_index);
			for (int j = start_word; j < start_word + (nr_words_per_block * 4); j++) // cache.data에 memory에 있는 data 입력
			{
				cache[i].data[count++] = memory[j];
			}
			now_index = i;
			break;
		}
		else if (cache[i].valid == CB_VALID) // data 들어있을 때
		{
			/*for (int k = addr_set * nr_ways; k < (addr_set * nr_ways) + nr_ways; k++)
			{*/
			//printf("k값 : %d, cache[k].tag 값 : %d \n", k, cache[k].tag);
			if (cache[i].tag == (addr >> (bit_offset + bit_index))) // tag 같으면 cache_hit남
			{
				if (cache[i].dirty == CB_CLEAN)
				{
					printf("태그 같고 CLEAN 일때\n");
					cache[i].timestamp = cycles;
					now_index = i;
					return CACHE_HIT;
				}
				else if (cache[i].dirty == CB_DIRTY)
				{
					printf("태그 같고 DIRTY 일때\n");
					cache[i].timestamp = cycles;
					cache[i].dirty = CB_CLEAN;
					for (int j = (((cache[i].tag << (bit_offset + bit_index)) / 16) * 16); j < start_word + (nr_words_per_block * 4); j++)
					{
						memory[j] = cache[i].data[count++];
					}
					count = 0;
					for (int j = start_word; j < start_word + (nr_words_per_block * 4); j++) // cache.data에 memory에 있는 data 입력
					{
						cache[i].data[count++] = memory[j];
					}
					now_index = i;
					return CACHE_HIT;
				}
				//printf("k값 : %d, cache[k].tag 값 : %d \n", k, cache[k].tag);
				cache[i].timestamp = cycles;
				/*for (int j = start_word; j < start_word + (nr_words_per_block * 4); j++) // cache.data에 memory에 있는 data 입력
				{
					cache[i].data[count++] = memory[j];
				}*/
				return CACHE_HIT;
			}
			else // tag가 다를때
			{
				if (i == ((addr_set * nr_ways) + nr_ways - 1)) // 다 차있을때
				{
					if (cache[old_block_addr].dirty == CB_CLEAN)
					{
						printf("태그 다르고 CLEAN 일때\n");
						cache[old_block_addr].dirty = CB_CLEAN;
						cache[old_block_addr].timestamp = cycles;
						cache[old_block_addr].tag = addr >> (bit_offset + bit_index);
						for (int j = start_word; j < start_word + (nr_words_per_block * 4); j++) // cache.data에 memory에 있는 data 입력
						{
							cache[old_block_addr].data[count++] = memory[j];
						}
					}
					else // old_block이 더티일 때
					{
						printf("태그 다르고 DIRTY 일때\n");
						printf("현재 i 값은 : %d\n", i);
						printf("현재 old_block_addr 값은 : %d\n", old_block_addr);
						cache[old_block_addr].dirty = CB_CLEAN;
						cache[old_block_addr].timestamp = cycles;
						cache[old_block_addr].tag = addr >> (bit_offset + bit_index);
						/*for (int j = start_word; j < start_word + (nr_words_per_block * 4); j++) // cache.data에 memory에 있는 data 입력
						{
							cache[i].data[count++] = memory[j];
						}*/
						/*for (int j = (((cache[old_block_addr].tag << (bit_offset + bit_index)) / 16) * 16); j < start_word + (nr_words_per_block * 4); j++)
						{
							printf("%d ", j);
							printf("%d ", count);
							memory[j] = cache[old_block_addr].data[count++];
						} // write - back , cache에 있는 data를 memory로 옮김*/
						count = 0;
						for (int j = start_word; j < start_word + (nr_words_per_block * 4); j++) // cache.data에 memory에 있는 data 입력
						{
							cache[old_block_addr].data[count++] = memory[j];
						}
					}
				}
				now_index = i;
				continue;
			}
			//}

			continue;
		}
	}


	return CACHE_MISS;
}


/**************************************************************************
 * store_word
 *
 * DESCRIPTION
 *   Simulate the case when the processor is handling sw instruction.
 *   Cache should be write-back and write-allocate. Note that the least
 *   recently used (LRU) block should be replaced in case of eviction.
 *
 * PARAMETERS
 *   @addr: Starting address for @data
 *   @data: New value for @addr. @data is 1-word in size
 *
 * RETURN
 *   CACHE_HIT on cache hit, CACHE_MISS otherwise
 *
 */
int store_word(unsigned int addr, unsigned int data)
{
	/* TODO: Implement your store_word function */
	int bit_tag, bit_index, bit_offset;
	bit_offset = log2_discrete(nr_words_per_block * 4);
	bit_index = log2_discrete(nr_sets);
	bit_tag = 32 - bit_index - bit_offset;

	int addr_index;
	int addr_set;
	int start_word;

	int count = 0;
	int lw_hit;
	int start_word_addr;
	start_word_addr = addr % 16;
	addr_set = (((addr / (nr_words_per_block * 4)) % nr_blocks) % nr_sets); // 받은 주소에 해당하는 set 넘버 계산
	start_word = (addr / 16) * 16;

	for (int i = (addr_set * nr_ways); i < (addr_set * nr_ways) + nr_ways; i++) // set에서 맨 처음 block부터 돌면서 check
	{
		if (cache[i].valid == CB_INVALID)
		{
			printf("swA\n");
			lw_hit = load_word(addr);
			printf("11now_index는 %d \n", now_index);
			for (int j = 3; j >= 0; j--) // cache.data에 memory에 있는 data 입력
			{
				cache[i].data[start_word_addr++] = data >> 8 * j;
			}
			cache[i].dirty = CB_DIRTY;
			break;
		}
		else if (cache[i].valid == CB_VALID && cache[i].dirty == CB_CLEAN)
		{
			printf("swB\n");
			lw_hit = load_word(addr);
			printf("22now_index는 %d \n", now_index);
			for (int j = 3; j >= 0; j--) // cache.data에 memory에 있는 data 입력
			{
				cache[now_index].data[start_word_addr++] = data >> 8 * j;
			}
			cache[now_index].dirty = CB_DIRTY;
			break;

		}
		else if (cache[i].valid == CB_VALID && cache[i].dirty == CB_DIRTY)
		{
			printf("swC\n");

			lw_hit = load_word(addr);
			printf("현재 i값 : %d\n", i);
			for (int j = 3; j >= 0; j--) // cache.data에 parameter로 받은 data 입력
			{
				cache[now_index].data[start_word_addr++] = data >> 8 * j;
			}
			cache[now_index].dirty = CB_DIRTY;
			printf("현재 i값 : %d\n", i);
			printf("now_index값 : %d \n", now_index);
			break;
		}

	}
	if (lw_hit == CACHE_HIT)
		return CACHE_HIT;
	else
		return CACHE_MISS;
}


/**************************************************************************
 * init_simulator
 *
 * DESCRIPTION
 *   This function is called before starting the simulation. This is the
 *   perfect place to put your initialization code.
 */
void init_simulator(void)
{
	/* TODO: You may place your initialization code here */
}



/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
static void __show_cache(void)
{
	for (int i = 0; i < nr_blocks; i++) {
		fprintf(stderr, "[%3d] %c%c %8x %8u | ", i,
			cache[i].valid == CB_VALID ? 'v' : ' ',
			cache[i].dirty == CB_DIRTY ? 'd' : ' ',
			cache[i].tag, cache[i].timestamp);
		for (int j = 0; j < BYTES_PER_WORD * nr_words_per_block; j++) {
			fprintf(stderr, "%02x", cache[i].data[j]);
			if ((j + 1) % 4 == 0) fprintf(stderr, " ");
		}
		fprintf(stderr, "\n");
		if (nr_ways > 1 && ((i + 1) % nr_ways == 0)) printf("\n");
	}
}

static void __dump_memory(unsigned int start)
{
	for (int i = start; i < start + 64; i++) {
		if (i % 16 == 0) {
			fprintf(stderr, "[0x%08x] ", i);
		}
		fprintf(stderr, "%02x", memory[i]);
		if ((i + 1) % 4 == 0) fprintf(stderr, " ");
		if ((i + 1) % 16 == 0) fprintf(stderr, "\n");
	}
}

static void __init_cache(void)
{
	cache = (struct cache_block *)malloc(sizeof(struct cache_block) * nr_blocks);

	for (int i = 0; i < nr_blocks; i++) {
		struct cache_block *c = cache + i;

		c->valid = CB_INVALID;
		c->dirty = CB_CLEAN;
		c->tag = 0;
		c->timestamp = 0;
		memset(c->data, 0x00, sizeof(c->data));
	}
}

static void __fini_cache(void)
{
	free(cache);
}

static int __parse_command(char *command, int *nr_tokens, char *tokens[])
{
	char *curr = command;
	int token_started = false;
	*nr_tokens = 0;

	while (*curr != '\0') {
		if (isspace(*curr)) {
			*curr = '\0';
			token_started = false;
		}
		else {
			if (!token_started) {
				tokens[*nr_tokens] = curr;
				*nr_tokens += 1;
				token_started = true;
			}
		}
		curr++;
	}

	/* Exclude comments from tokens */
	for (int i = 0; i < *nr_tokens; i++) {
		if (strmatch(tokens[i], "//") || strmatch(tokens[i], "#")) {
			*nr_tokens = i;
			tokens[i] = NULL;
		}
	}

	return 0;
}

static void __simulate_cache(FILE *input)
{
	int argc;
	char *argv[10];
	char command[80];

	unsigned int hits = 0, misses = 0;

	__init_cache();
	if (input == stdin) printf(">> ");

	while (fgets(command, sizeof(command), input)) {
		unsigned int addr;
		int hit;

		__parse_command(command, &argc, argv);

		if (argc == 0) continue;

		if (strmatch(argv[0], "show")) {
			__show_cache();
			goto next;
		}
		else if (strmatch(argv[0], "dump")) {
			addr = argc == 1 ? 0 : strtoimax(argv[1], NULL, 0) & 0xfffffffc;
			__dump_memory(addr);
			goto next;
		}
		else if (strmatch(argv[0], "cycles")) {
			fprintf(stderr, "%3u %3u   %u\n", hits, misses, cycles);
			goto next;
		}
		else if (strmatch(argv[0], "quit")) {
			break;
		} if (strmatch(argv[0], "lw")) {
			if (argc == 1) {
				printf("Wrong input for lw\n");
				printf("Usage: lw <address to load>\n");
				goto next;
			}
			addr = strtoimax(argv[1], NULL, 0);
			hit = load_word(addr);
		}
		else if (strmatch(argv[0], "sw")) {
			if (argc != 3) {
				printf("Wrong input for sw\n");
				printf("Usage: sw <address to store> <word-size value to store>\n");
				goto next;
			}
			addr = strtoimax(argv[1], NULL, 0);
			hit = store_word(addr, strtoimax(argv[2], NULL, 0));
		}
		else {
			goto next;
		}

		if (hit == CACHE_HIT) {
			hits++;
			cycles += cycles_hit;
		}
		else {
			misses++;
			cycles += cycles_miss;
		}
	next:
		if (input == stdin) printf(">> ");
	}

	__fini_cache();
}

int main(int argc, const char *argv[])
{
	FILE *input = stdin;

	if (argc > 1) {
		input = fopen(argv[1], "r");
		if (!input) {
			perror("Input file error");
			return EXIT_FAILURE;
		}
	}


	if (input == stdin) {
		printf("*****************************************************\n");
		printf("*                    _                              *\n");
		printf("*      ___ __ _  ___| |__   ___                     *\n");
		printf("*     / __/ _` |/ __| '_ \\ / _ \\                    *\n");
		printf("*    | (_| (_| | (__| | | |  __/                    *\n");
		printf("*     \\___\\__,_|\\___|_| |_|\\___|                    *\n");
		printf("*    	 _                 _       _                *\n");
		printf("*     ___(_)_ __ ___  _   _| | __ _| |_ ___  _ __   *\n");
		printf("*    / __| | '_ ` _ \\| | | | |/ _` | __/ _ \\| '__|  *\n");
		printf("*    \\__ \\ | | | | | | |_| | | (_| | || (_) | |     *\n");
		printf("*    |___/_|_| |_| |_|\\__,_|_|\\__,_|\\__\\___/|_|     *\n");
		printf("*                                                   *\n");
		printf("*                                   2019.12         *\n");
		printf("*****************************************************\n\n");
	}

#ifndef _USE_DEFAULT
	if (input == stdin) printf("- words per block:  ");
	fscanf(input, "%d", &nr_words_per_block);
	if (input == stdin) printf("- number of blocks: ");
	fscanf(input, "%d", &nr_blocks);
	if (input == stdin) printf("- number of ways:   ");
	fscanf(input, "%d", &nr_ways);

	nr_sets = nr_blocks / nr_ways;
#endif

	init_simulator();
	__simulate_cache(input);

	if (input != stdin) fclose(input);

	return EXIT_SUCCESS;
}
