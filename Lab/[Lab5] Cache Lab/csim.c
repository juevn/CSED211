/*김주은 20210774 jueunk*/

#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

int miss_num = 0;
int hit_num = 0;
int evict_num = 0;

typedef struct {
    bool valid;
    int tag;
    unsigned long long int lru_val; //value for lru
} line;

typedef line* set;
typedef set* cache_t;

//global variables
cache_t cache = {0};
int h_flag = 0;
int v_flag = 0;
int s = 0;
int E = 0;
int b = 0;
char* trace = NULL;

int S; //num of sets
int B; //size of blocks

unsigned long long int lru_count = 1;

void init_cache();
void trace_input(char* trace_f);
void access_cache(unsigned long long int address);

int main(int argc, char* argv[])
{
    //command line input
    char op;
    while( (op = getopt(argc, argv, "hvs:E:b:t:")) != -1)
    {
        switch (op)
        {
            case 'h':
                h_flag = 1;
                break;
            case 'v':
                v_flag = 1;
                break;
            case 's':
                s = atoi(optarg);
                S = (unsigned int)pow(2,s);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                B = (unsigned int)pow(2,b);
                break;
            case 't':
                trace = optarg;
                break;
        }
    }

    if (h_flag == 1)
    {
        printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>");
        exit(0);
    }

    //initializing virtual cache
    init_cache();

    //getting trace input
    trace_input(trace);

    //printing result
    printSummary(hit_num, miss_num, evict_num);

    //free
    int i;
    for(i = 0; i < S; i++)
    {
        free(cache[i]);
    }
    free(cache);

    return 0;
}

void init_cache()
{
    cache = (set*)malloc(sizeof(set)* S);
    int i,j;
    for (i = 0; i < S; i++)
    {
        cache[i]= (line*)malloc(sizeof(line)* E);

        for (j = 0; j < E; j++)
        {
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru_val = 0;
        }
    }
}

void trace_input(char* trace_f)
{
    FILE* trace_file = fopen(trace_f, "r");
    char t_cmd;
    unsigned long long int addr;
    int size;

    while (fscanf(trace_file, " %c %llx,%d", &t_cmd, &addr, &size) == 3) 
    {
        if (v_flag == 1)
        {
            printf(" %c %llx,%d", t_cmd, addr, size);
        }
        switch(t_cmd)
        {
            case 'I':
                continue;
            case 'L': 
                access_cache(addr); 
                break;
            case 'S': 
                access_cache(addr); 
                break;
            case 'M': 
                access_cache(addr); 
                access_cache(addr); 
                break;
            default: 
                break;
        }
        printf("\n");
        
    }

    fclose(trace_file);
}

void access_cache(unsigned long long int address)
{
    unsigned long long int index_mask = S - 1;
    unsigned long long int set_index = (address >> b) & index_mask;
    unsigned long long int tag_bit = address >> (s + b);
    unsigned long long int min_lru = ULONG_MAX;
    unsigned int evict_line = 0;

    // hit check
    // when hit occur, increse the hit_num and return
    int i;
    for(i = 0; i < E; i++)
    {
        if(cache[set_index][i].valid)
        {
            if(cache[set_index][i].tag == tag_bit)
            {
                cache[set_index][i].lru_val = lru_count++;
                hit_num++;
                if (v_flag == 1)
                {
                    printf(" hit");
                }
                return;
            }
        }
    }

    // miss count
        miss_num++;

    // eviction check
    for (i = 0; i < E; i++)
    {
        //maximum lru checking
        if(min_lru > cache[set_index][i].lru_val)
        {
            evict_line = i;
            min_lru = cache[set_index][i].lru_val;
        }
    }

    if(cache[set_index][evict_line].valid)
    {
        evict_num++;
        if(v_flag == 1)
        {
            printf(" miss eviction");
        }
    }
    else
    {
        if (v_flag == 1)
        {
            printf(" miss");
        }
    }

    cache[set_index][evict_line].valid = 1;
    cache[set_index][evict_line].tag = tag_bit;
    cache[set_index][evict_line].lru_val = lru_count;

    return;
}