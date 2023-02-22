/* 
 * csim.c
 * Author: HaveAnApplePie
 */ 
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int sNum = -1;
int eNum = -1;
int bNum = -1;

int hitNum = 0;
int missNum = 0;
int evictNum = 0;

bool vFlag = false;


typedef struct Line{
    bool valid;
    unsigned long tag;
} Line;

Line** buildCache();
void destoryCache(Line** cache);
void dealwithCommand(char* cmd, Line** cache);
void findInCache(unsigned long addr,Line** cache);
void cacheLoad(unsigned long addr,Line** cache);
void cacheStor(unsigned long addr,Line** cache);
void updateLRU(unsigned long sIdx, unsigned long lIdx, Line** cache);


int main(int argc,char * argv[])
{
    int arg;
    char* fileName;
    Line** cache;
    //deal with the arguments
    while((arg = getopt(argc,argv,"vs:E:b:t:")) != -1){
        switch(arg){
            case 'v':
                vFlag = true;
                printf("You choose V!\n");
                break;
            case 's':
                sNum = atoi(optarg);
                printf("The argument of -s is %d \n", sNum);
                break;
            case 'E':
                eNum = atoi(optarg);
                printf("The argument of -E is %d \n", eNum);                
                break;
            case 'b':
                bNum = atoi(optarg);
                printf("The argument of -b is %d \n", bNum);
                break;
            case 't':
                fileName = optarg;
                printf("The argument of -t is %s \n", fileName);
                break;
            default:
                printf("Error Argument!\n");
                exit(-1);
                break;
        }
    }

    if(sNum == -1 || bNum == -1 || eNum == -1 || fileName == NULL){
        printf("Not Enough Arguments!\n");
        exit(-1);
    }

    //open file
    FILE *fp = fopen(fileName,"r"); //read only
    if(fp == NULL){
        printf("Error opening file.\n");
        exit(-1);
    }

    //buildCache
    cache = buildCache();
    
    //main loop
    char command[100];
    while(fgets(command,100,fp) != NULL){
        dealwithCommand(command,cache);
    }


    //destory cache
    for(int i = 0; i < (1 << sNum) ; i++){
        free(cache[i]);
    }
    //destorycache(cache);

    free(cache);

    //close file
    fclose(fp);

    //output value
    printSummary(hitNum, missNum, evictNum);
    return 0;
}

Line** buildCache(){
    
    Line ** cache = (Line**) malloc((1 << sNum)  * sizeof(Line *));
    for(int i = 0; i < (1 << sNum); i++){
        cache[i] = (Line*) malloc(eNum * sizeof(Line));
    }

    for(int i = 0; i < (1 << sNum); i++){
        for(int j = 0; j < eNum; j++){
            cache[i][j].valid = false;
            cache[i][j].tag = 0;
        }
    }
    
    return cache;
}

void destoryCache(Line** cache){
    for(int i = 0; i < (1 << sNum); i++){
        free(cache[i]);
    }

    free(cache);
}

/*
 * dealwithCommand -- dealwith each command:1.parse cmd 2.S/L data in cache
 */
void dealwithCommand(char* cmd, Line** cache){
    if(cmd[0] == 'I')
        return;
    char blank,op;
    unsigned long addr;
    unsigned size;
    
    
    sscanf(cmd,"%c%c %lx,%d",&blank,&op,&addr,&size); //parse string

    if(vFlag){
        printf("%s ",cmd);
        fflush(stdout);
    }

    switch(op){
        case 'L':
            cacheLoad(addr,cache);
            break;
        case 'S':
            cacheStor(addr,cache);
            break;
        case 'M':
            cacheLoad(addr,cache);
            cacheStor(addr,cache);
            break;
        default:
            printf("Op Error!\n");
            break;
    }

    if(vFlag){
        printf("\n");
    }
}

void cacheLoad(unsigned long addr,Line** cache){
    findInCache(addr,cache);
}

void cacheStor(unsigned long addr,Line** cache){
    findInCache(addr,cache);
}

/*
 * findInCache -- the main function that find the data in cache,
 *                and update the LRU rule in cache
 */
void findInCache(unsigned long addr,Line** cache){
    unsigned long cmdS;
    unsigned long cmdT;
    
    unsigned long sMark;
    sMark = (0x1 << sNum) - 1;
    cmdS = (addr >> bNum) & sMark;

    cmdT = ((addr >> bNum) >> sNum);

    //printf("Set:%ld Tag:%lx\n",cmdS,cmdT);

    int i;
    for(i = 0; i < eNum && cache[cmdS][i].valid; i++){  
        if(cache[cmdS][i].tag == cmdT){ // cache hit
            updateLRU(cmdS,i,cache); 
            //hit
            hitNum++;
            if(vFlag){             
                printf("hit ");
                fflush(stdout);
            }
          
            return;
        }
    }
    
    //cache miss
    if(i < eNum){   //there is enough spaces in cache
        cache[cmdS][i].valid = true;
        cache[cmdS][i].tag = cmdT;
        updateLRU(cmdS,i,cache);  
        //miss
        missNum++;
        if(vFlag){
            printf("miss ");
            fflush(stdout);
        } 
        return;        
    }
    else{       //eviction happened
        cache[cmdS][eNum-1].valid = true;
        cache[cmdS][eNum-1].tag = cmdT;
        updateLRU(cmdS,eNum-1,cache);
        //miss & eviction
        missNum++;
        evictNum++;
        if(vFlag){
            printf("miss eviction ");
            fflush(stdout);
        }
  
        return;
    }
}

/*
 *  updateLRU -- update cache with rule : the least reccently used is in 
 *               the end of the array.(the speed of this implementation is
 *               slow ,but it is convenient )
 */
void updateLRU(unsigned long sIdx, unsigned long lIdx, Line** cache){
    Line tmp = cache[sIdx][lIdx];

    for(int i = lIdx; i > 0; i--){
        cache[sIdx][i] = cache[sIdx][i-1];
    }

    cache[sIdx][0] = tmp;
}