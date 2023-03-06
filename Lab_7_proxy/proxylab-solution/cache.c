#include "cache.h"

char** cache;
Meta* meta;
unsigned int seed;


/*
 * mallocCache - Initialize the cache
 */
void mallocCache(){
    
    cache = (char**)malloc(BLOCK_NUM * sizeof(char *));
    
    for(int i = 0; i < BLOCK_NUM; i++){
        cache[i] = (char *)malloc(MAX_OBJECT_SIZE * sizeof(char));
        memset(cache[i],0,MAX_OBJECT_SIZE);
    }

    meta = (Meta*)malloc(BLOCK_NUM * sizeof(Meta));
    
    for(int i = 0; i < BLOCK_NUM; i++){
        memset(&meta[i],0,sizeof(Meta));
    }

    seed = 0;
}

/*
 * mallocCache - free the cache
 */
void freeCache(){
    for(int i = 0; i < BLOCK_NUM; i++){
        free(cache[i]);
    }
    free(cache);

    free(meta);
}


/*
 * loadObject - load object with aruguments "host" and "path".
 *      If object is already in cache, load it to "reObj" and
 *      return 1. Else return 0.
 */
int loadObject(char* host, char* path, char* reObj, char* resp, int* respNum){
    int idx = -1;
    for(int i = 0; i < BLOCK_NUM; i++){
        if( meta[i].valid &&
            strcmp(meta[i].host, host) == 0 &&
            strcmp(meta[i].path, path) == 0 ){
            idx = i;
            updateLRU(idx);
            break;
        }
    }

    if(idx == -1){
        printf("[Cache] Can't find cache: %s %s\n",host,path);
        return 0;
    }
    else{
        memcpy(reObj,cache[idx],MAX_OBJECT_SIZE);
        memcpy(resp, meta[idx].resp, meta[idx].respNum);
        *respNum = meta[idx].respNum;
        printf("[Cache] Find cache: %s %s\n",host,path);
        printf("[Cache] Respones Num: %d\n",*respNum);
        return 1;
    }
}

/*
 * storeObject - Store object in cache with aruguments "host" and "path".
 */
void storeObject(char* host, char* path, char* obj, char* resp, int respNum){
    int idx = findEvictObject();
    
    strcpy(meta[idx].host, host);
    strcpy(meta[idx].path, path);
    memcpy(meta[idx].resp, resp, respNum);
    meta[idx].respNum = respNum;
    meta[idx].valid = true;

    updateLRU(idx);

    memcpy(cache[idx], obj, MAX_OBJECT_SIZE);

    printf("[Cache] Store: %s %s\n",host,path);
}


/*
 * findEvictObject - find which objects to evict
 */
int findEvictObject(){
    unsigned int idx,min;
    idx = 0;
    min = 0;

    for (int i = 0; i < BLOCK_NUM; i++)
    {
        if(meta[i].valid == false){
            printf("[Cache] Find free cache block:%d\n",i);
            return i;
        }
            
        if(meta[i].lru < min){
            idx = i;
            min = meta[i].lru;
        }
    }

    return idx;
}

/*
 * updateLRU - Update lru number in the meta[idx] object. If
 *      the seed is going to overflow, then every meta objects
 *      minus minimum lru number.
 */
void updateLRU(int idx){
    meta[idx].lru = ++seed;

    //clear seed
    if(seed == 0xffffffff){
        unsigned int min = 0;
        
        for (int i = 0; i < BLOCK_NUM; i++)
        {
            if(meta[i].valid && min > meta[i].lru)
                min = meta[i].lru;
        }
        
        seed -= min;
        for (int i = 0; i < BLOCK_NUM; i++)
        {
            if(meta[i].valid)
                meta[i].lru -= min;
        }        
    }
}