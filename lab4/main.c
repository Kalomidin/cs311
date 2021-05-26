#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define BYTES_PER_WORD 4
char **INSTR;
static int INSTR_COUNT = 0;
static int WORDS_PER_BLOCK = 0; 
static int CAPACITY = 256;
static int WAY = 4; // associativity
static int BLOCKSIZE = 8;

struct block_info {
	uint32_t addr;

	// Bool to check if block is empty
	bool is_empty;


	// Boolean to track whether data is dirty
	bool is_dirty;

	// Used to pick the victim
	uint32_t access_count;
};

/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */   
/*                                                             */
/***************************************************************/
void cdump(int capacity, int assoc, int blocksize){

	printf("Cache Configuration:\n");
    	printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", assoc);
	printf("Block Size: %dB\n", blocksize);
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : sdump                                           */
/*                                                             */
/* Purpose   : Dump cache stat		                       */   
/*                                                             */
/***************************************************************/
void sdump(int total_reads, int total_writes, int write_backs,
	int reads_hits, int write_hits, int reads_misses, int write_misses) {
	printf("Cache Stat:\n");
    	printf("-------------------------------------\n");
	printf("Total reads: %d\n", total_reads);
	printf("Total writes: %d\n", total_writes);
	printf("Write-backs: %d\n", write_backs);
	printf("Read hits: %d\n", reads_hits);
	printf("Write hits: %d\n", write_hits);
	printf("Read misses: %d\n", reads_misses);
	printf("Write misses: %d\n", write_misses);
	printf("\n");
}


/***************************************************************/
/*                                                             */
/* Procedure : xdump                                           */
/*                                                             */
/* Purpose   : Dump current cache state                        */ 
/* 							       */
/* Cache Design						       */
/*  							       */
/* 	    cache[set][assoc][word per block]		       */
/*      						       */
/*      						       */
/*       ----------------------------------------	       */
/*       I        I  way0  I  way1  I  way2  I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set0  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set1  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*      						       */
/*                                                             */
/***************************************************************/
void xdump(int set, int way, struct block_info** cache)
{
	int i,j,k = 0;

	printf("Cache Content:\n");
    	printf("-------------------------------------\n");
	for(i = 0; i < way;i++)
	{
		if(i == 0)
		{
			printf("    ");
		}
		printf("      WAY[%d]",i);
	}
	printf("\n");

	for(i = 0 ; i < set;i++)
	{
		printf("SET[%d]:   ",i);
		for(j = 0; j < way;j++)
		{
			if(k != 0 && j == 0)
			{
				printf("          ");
			}
			printf("0x%08x  ", cache[i][j].addr);
		}
		printf("\n");
	}
	printf("\n");
}



/***************************************************************/
/*                                                             */
/* Procedure: str_split                                        */
/*                                                             */
/* Purpose: To parse main function argument                    */
/*                                                             */
/***************************************************************/
char** str_split(char *a_str, const char a_delim){
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp){
	if (a_delim == *tmp){
	    count++;
	    last_comma = tmp;
	}
	tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
     *        knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result){
	size_t idx  = 0;
	char* token = strtok(a_str, delim);

	while (token){
	    assert(idx < count);
	    *(result + idx++) = strdup(token);
	    token = strtok(0, delim);
	}
	assert(idx == count - 1);
	*(result + idx) = 0;
    }

    return result;
}

void load_program(char *program_filename) {
	char buffer[13];

	FILE *prog = fopen(program_filename, "r");
	if (prog == NULL) {
		printf("Error: Can't open program file %s\n", program_filename);
		exit(-1);
	}

	int ii = 0;
	// Read  all over to know the exact amount of instructions
	while (fgets(buffer, 13, prog) != NULL) {
		assert(buffer[12] == '\0');
		if(strncmp(buffer, "\n", 1) == 0) {
			continue;
		}
		ii++;
	}

	// Allocate memory for the exact size
	INSTR = malloc(12 * ii);
	INSTR_COUNT = ii;

	fclose(prog);
	ii = 0;

	// Set the instructions
	// Note: It might a problem since size is not always 13??(R 0x01 is not 12)
	prog = fopen(program_filename, "r");
	while (fgets(buffer, 13, prog) != NULL) {
		if(strncmp(buffer, "\n", 1) == 0) {
			continue;
		}
		// Make sure the last read is `\0`
		assert(buffer[12] == '\0');
		// printf("Buffer is: %s, %d\n", buffer, strncmp(buffer, "\n", 1));
		INSTR[ii] = malloc(12);
		memcpy(INSTR[ii], buffer, 12);
		ii++;
	}
}

bool is_in_block(uint32_t block_addr, uint32_t addr) {
	// printf("Is in block\n");
	uint32_t to_block_size = block_addr - (block_addr % BLOCKSIZE);
	uint32_t addr_block_size = addr - (addr % BLOCKSIZE);
	return to_block_size == addr_block_size;
}

void evict_block(struct block_info *set, uint32_t addr, int *write_back) {
	// printf("Evicting the block\n");
	for(int i = 0; i < WAY; i++) {
		// printf("i is: %d, %d\n", i, set == NULL);
		if (set[i].is_empty || is_in_block(set[i].addr, addr)) {
			return;
		}
	}
	// printf("Evicted the block\n");
	int victim_idx = 0;
	int victim_access_count = set[0].access_count;
	for(int i = 0; i < WAY; i++) {
		if(set[i].access_count < victim_access_count) {
			victim_idx = i;
			victim_access_count = set[i].access_count;
		}
	}
	if(write_back != NULL && set[victim_idx].is_dirty) {
		*write_back += 1;
	}
	set[victim_idx].access_count = 0;
	set[victim_idx].addr = 0;
	set[victim_idx].is_dirty = false;
	set[victim_idx].is_empty = true;
}

void process_read(uint32_t addr,struct block_info *set, int *read_hits, int *read_misses, int *write_back, uint32_t access_count) {
	// printf("Read instruction \n");
	
	// Check if empty block exists in any way in the set
	evict_block(set, addr, write_back);
	// printf("Evicted the block\n");
	for(int i =0; i < WAY; i++) {
		if (set[i].is_empty || is_in_block(set[i].addr, addr)) {
			if(!set[i].is_empty && is_in_block(set[i].addr, addr)) {
				set[i].access_count = access_count;
				*read_hits += 1;
				return;
			}
			// printf("Increasing the read\n");
			*read_misses += 1;
			// printf("Increasing the read2\n");
			set[i].addr = addr - (addr % BLOCKSIZE);
			set[i].access_count = access_count;
			set[i].is_empty = false;
			set[i].is_dirty = false;
			return;
		}
	}
}

void process_write(uint32_t addr, struct block_info *set, int *write_hits, int *write_misses, int *write_back, uint32_t access_count) {
	// printf("Write instruction\n");
	evict_block(set, addr, write_back);
	for(int i =0; i < WAY; i++) {
		if (set[i].is_empty || is_in_block(set[i].addr, addr)) {
			if(!set[i].is_empty && is_in_block(set[i].addr, addr)) {
				set[i].is_dirty = true;
				set[i].access_count = access_count;
				*write_hits += 1;
				return;
			}
			*write_misses += 1;
			set[i].addr = addr - (addr % BLOCKSIZE);
			set[i].access_count = access_count;
			set[i].is_empty = false;
			set[i].is_dirty = true;
			return;
		}
	}
}

void process_instructions(char *program_filename, struct block_info **cache) {
	load_program(program_filename);

	int num_words = BLOCKSIZE / BYTES_PER_WORD;
	int sets = CAPACITY / WAY / BLOCKSIZE;

	int per_set = BLOCKSIZE * sets;

	// sdump values
	int read_misses = 0;
	int write_misses = 0;
	int read_hits = 0;
	int write_hits = 0;
	int total_reads = 0;
	int total_writes = 0;
	int write_backs = 0;

	for(int i = 0; i < INSTR_COUNT; i++) {
		// printf("Instr at %d is: %s, %d\n", i, INSTR[i], INSTR[i] != NULL);
		char **tokens = str_split(INSTR[i], ' ');
		// printf("Instruction at 1: %s\n", tokens[1]);
		uint32_t addr = (uint32_t) strtol(tokens[1], NULL, 16);
		// printf("Address is: %x and instruction is: %s\n", addr, tokens[0]);
		int cache_addr = addr % CAPACITY;
		int set_num = (cache_addr / BLOCKSIZE) % (sets);
		// printf("Set num: %d\n", set_num);
		if (strcmp(tokens[0], "R") == 0) {
			// If it is read from cache
			process_read(addr, cache[set_num], &read_hits, &read_misses, &write_backs, i);
			total_reads++;
			// printf("Read increased by 1\n");
		} else {
			// printf("Write\n");
			// If it is write to cache
			process_write(addr, cache[set_num], &write_hits, &write_misses, &write_backs, i);
			total_writes++;
		}
	}

	cdump(CAPACITY, WAY, BLOCKSIZE);
	sdump(total_reads, total_writes, write_backs, read_hits, write_hits, read_misses, write_misses); 
	xdump(sets, WAY, cache);
};

int main(int argc, char *argv[]) {                              

	struct block_info** cache;
	int i, j, k;	
	bool xdump_print = false;
	
	int count = 1;
    while(count != argc-1){
	if(strcmp(argv[count], "-c") == 0){
		char **tokens;
		tokens = str_split(argv[++count], ':');
		CAPACITY = (int) strtol(*(tokens), NULL, 10);
		WAY = (int) strtol(*(tokens + 1), NULL, 10);
		BLOCKSIZE = (int) strtol(*(tokens + 2), NULL, 10);  
		continue; 
	}
	else if(strcmp(argv[count], "-x") == 0) {
		xdump_print = true;
	}
	count++;
    }

	// printf("Capacity: %d, way: %d, blocksize %d\n", capacity, way, blocksize);

	int set = CAPACITY/WAY/BLOCKSIZE;

	int words = BLOCKSIZE / BYTES_PER_WORD;	

	// allocate
	cache = (struct block_info **) malloc (sizeof(struct block_info *) * set);
	for(i = 0; i < set; i++) {
		cache[i] = (struct block_info *) malloc(sizeof(struct block_info) * WAY);
	}
	for(i = 0; i < set; i++) {
		for(j = 0; j < WAY; j ++) {
			cache[i][j].access_count = 0;
			cache[i][j].addr = 0x0;
			cache[i][j].is_empty = true;
			cache[i][j].is_dirty = false;
			// printf("Cache is: 0x%08x  , i: %d, j: %d\n", cache[i][j].addr, i, j);
		}
	}

	process_instructions(argv[argc-1], cache);


	// test example

    return 0;
}
