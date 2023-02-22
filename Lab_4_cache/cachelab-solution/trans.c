/* 
 * trans.c - Matrix transpose B = A^T
 * Author: HaveAnApplePie
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"
#include <stdbool.h>

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans(int M, int N, int A[N][M], int B[M][N]);
void transpose_submit_64(int M, int N, int A[N][M], int B[M][N]);
void transpose_submit_block(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M == 32 && N == 32){
        transpose_submit_block(M,N,A,B);
    }
    else if(M == 64 && N == 64){
        transpose_submit_64(M,N,A,B);
    }
    else if(M == 61 && N == 67){
        transpose_submit_block(M,N,A,B);
    }
    else{
        trans(M,N,A,B);
    }
}

/* 
 * transpose_submit_64 - This function is optimized specifically
 *      for the 64 x 64 case. The blocks on the diagonal are changed
 *      manually by using 4 int variables to minimize the miss number
 *      of a 8x8 block to 16.
 *      The other part is the same with transpose_submit_block with 
 *      different blcok size.
 */ 

char transpose_submit_64_desc[] = "Transpose submission 64";
void transpose_submit_64(int M, int N, int A[N][M], int B[M][N])
{
	int i, j;
    int bSize = 4; 
    int x,y;
    int offset;
    int xx,yy;

    for(y = 0; y < M; y += (bSize*2)){
        for(x = 0; x < N; x += (bSize*2)){
            if(x == y){ //deal with blocks on the diagnoal with 4x4 
                for(yy = y; yy < y+bSize*2; yy += bSize){
                    for(xx = x; xx < x+bSize*2; xx += bSize){
                        for(i = xx; i < xx+ bSize; i++){
                            offset = i - xx + 1;
                            for(j = yy + offset; j < yy + bSize; j++){
                                B[j][i] = A[i][j];
                            }
                            for(j = yy; j < yy + offset; j++){
                                B[j][i] = A[i][j];
                            }
                        }
                    }
                }
            }
            else{   //change manually
                int a0,a1,a2,a3;
                for(j = y; j < y + bSize; j++){
                    for(i = x; i < x + bSize; i++){
                        B[j][i] = A[i][j];
                    }
                    a0 = A[x][j+bSize];
                    a1 = A[x+1][j+bSize];
                    a2 = A[x+2][j+bSize];
                    a3 = A[x+3][j+bSize];

                    B[y+(j-y)][x+bSize] = a0;
                    B[y+(j-y)][x+bSize+1] = a1;
                    B[y+(j-y)][x+bSize+2] = a2;
                    B[y+(j-y)][x+bSize+3] = a3;
                }

                for(j = y; j < y + bSize; j++){
                    a0 = B[y+(j-y)][x+bSize];
                    a1 = B[y+(j-y)][x+bSize+1];
                    a2 = B[y+(j-y)][x+bSize+2];
                    a3 = B[y+(j-y)][x+bSize+3];

                    for(i = x + bSize; i < x + bSize*2; i++){
                        B[j][i] = A[i][j];
                    }

                    B[y+bSize+(j-y)][x] = a0;
                    B[y+bSize+(j-y)][x+1] = a1;
                    B[y+bSize+(j-y)][x+2] = a2; 
                    B[y+bSize+(j-y)][x+3] = a3;              
                }

                for(j = y + bSize; j < y + bSize*2; j++){
                    for(i = x + bSize; i < x + bSize*2; i++){
                        B[j][i] = A[i][j];
                    }
                }
            }

        }
    }
}

/* 
 * transpose_submit_block - This function is optimized specifically
 *      for the 32 x 32 case and 61 x 67 case. 
 *      Here are two trick:
 *      1. Blcok
 *      2. For each line in the blocks on the diagonal(A[][]), read 
 *      the number on the diagonal last, which can readuce the miss.  
 */ 

char transpose_submit_block_desc[] = "Transpose submission normal block";
void transpose_submit_block(int M, int N, int A[N][M], int B[M][N])
{
	int i, j;
    int bSize;
    int x,y;
    int offset;


    if(M == 32 && M == 32){
        bSize = 8;
    }
    else if(M == 61 && N == 67){
        bSize = 12;
    }
    else{
        return;
    }

    int bM = bSize * (M/bSize);
    int bN = bSize * (N/bSize);

    //block
    for(y = 0; y < bM; y += bSize){
        for(x = 0; x < bN; x += bSize){
            for(i = x; i < x+ bSize; i++){
                offset = i - x + 1;
                for(j = y + offset; j < y + bSize; j++){    //read the number on diganoal last
                    B[j][i] = A[i][j];
                }
                for(j = y; j < y + offset; j++){
                    B[j][i] = A[i][j];
                }
            }
        }
    }

    //deal with the remaining items
    for(j = bM; j < M; j++){
        for(i = 0; i < bN; i++){
            B[j][i] = A[i][j];
        }
    }

    for(j = 0; j < M; j++){
        for(i = bN; i < N; i++){
            B[j][i] = A[i][j];
        }
    } 

}


/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    
}


/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    
    //registerTransFunction(trans, trans_desc); 
    //registerTransFunction(transpose_submit_64,transpose_submit_64_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}