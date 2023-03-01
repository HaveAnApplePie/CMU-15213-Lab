## Introduction
The original code is placed in the `cachelab-handout.tar`, and my solution is in the file `cachelab-solution`.A detailed introduction about this lab is in `writeup`. 
## Problem Analysis
### Part A: Writing a Cache Simulator
Part A needs us to build a cache, which is relatively simple because it just have requirements for the correctness of LRU, not performance. 
In this part, `getopt()` is very useful to deal with the input arguments. You should call `fflush(stdout)` after `printf()`，if there is no '\\n' in your printf. Otherwise, it may cause segment fault. And cache can be defined as a 2D array pointer. 
### Part B: Optimizing Matrix Transpose
#### 32 x 32
Because the block size is 32 bytes, it can read 8 items of int at a time. And the total matrix will fill the cache 4 times.(32 x 32 / 8 / 32).So 8 is the best block size.
Here are some tricks I use:
1. block, and the block size is 8.
2. For each line in the blocks on the diagonal(Matix A), readthe number on the diagonal last, which can readuce the miss.
#### 64 x 64
This case is the most difficult. I write my detailed ideas in my blog page -- [cmu15213-cachelab](https://haveanapplepie.github.io/posts/tech/cmu15213-cachelab/).  
I found if I use the block size of 4 , it will have 1651 miss which can not meet the requirements.
Here is a brief description of the tricks I use:
1. block, and the big block size is 8. And it divides the matrix into 64 blocks(8 diagonal blocks, 56 off-diagonal blocks).
2. For each line in the blocks on the diagonal(Matix A), readthe number on the diagonal last, which can readuce the miss.
3. In diagonal blocks, use small block size 4 to deal with just  like 32 x 32. It will cause 38 miss in every block with size of 8.
4. In off-diagonal blocks, use 4 temporary variable to reduce the miss number to the theoretical minimum number(16, 8 in A, 8 in B).
The theoretical result is 38x8 + 56 x16 = 1200, which is very close to the test result 1203. 
#### 61 x 67
In fact, even though I ended up satisfying the miss requirement(1965miss), I don't know much about the details. 
The idea is to block first, and then transpose the rest of the matrix. In experiments, I found that it is easier to get lower miss counts with block sizes that have a common factor of 8. And 12 is the best block size during my test. 

