#ifndef __CACHE_H__
#define __CACHE_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define BLOCK_NUM 10

typedef struct Metadata
{
    char host[MAXLINE];
    char path[MAXLINE];
    char resp[MAXLINE];
    unsigned int lru;
    unsigned int respNum;
    bool valid;
} Meta;


void mallocCache();
void freeCache();
void storeObject(char* host, char* path, char* obj, char* resp, int respNum);      /* Store object in cache */
int loadObject(char* host, char* path, char* reObj, char* resp, int* respNum);       /* Load  object in cache */
int findEvictObject();  /* Find wether the object is in cache */
void updateLRU(int idx);



#endif /* __CACHE_H__ */