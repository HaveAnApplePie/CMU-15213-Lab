## Introduction
The original code is placed in the `bomb.tar`, and my solution is in the file `bomb-solution`.  
In solving this lab, I found `tmux` is very useful, which allows you to split screen in a terminal. By the way, I named the solution to this lab **"Holy Light"** because it can solve this evil bomb.(laughs)
## Problem Analysis
### Phase_1
Phase_1 use the function `strings_not_equal` to cmpare the string you input with the saved string to see if they are the same.
### Phase_2
Phase_2 requirs you to enter 6 numbers, and test the first number. And it requirs all subsequent numbers are somehow related to the previous number.
### Phase_3
Phase_3 requirs you to enter 2 numbers. The first number is to find th jump address which is similar to `switch`. And depending on the different jump address, the second number needs to be set to a different value.  
So, this phase may have different solutions. I'm not sure.
### Phase_4
Phase_4 needs you to enter 2 numbers. And this two numebr will be treated as the parameter of the function `func4`, which need to make sure the return value `eax` be 0.
### Phase_5
Phase_5 needs you to enter a string of length 6. And the lowest 4 bits of each character will  be taken as positions in the given dictionary. You should use the 6 positions to rearrange a specific word.  
The solution of this phase is only require specific values for the lowest 4 bits. So, this phase have different solutions.
### Phase_6
Phase_6 is very hard due to the more than 200 lines assembly code, which includes multiple loops. I first look at the end of the code to locate the correct return point of the code. After that, analyze the corresponding assembly code page by page.  
In my opinion, the whole assembly code can be divided into 3 parts. In the beginning, Phase_6 requirs you to enter 6 numbers.  
The first part make sure the 6 numbers are all from 1-6, and they are not equal to each other. In the other words, the input is some permutation of 1 to 6.  
The second part use 7 to sub each number. Number 1 will be 6 (7-1) after this part.  
The third part is the hardest part. But just to slove this problem, we can find out that it use 6 numbers to build a table  which contains 6 constant values, and determin the read order of the table. What we need to do is make sure the 6 constant values be read from small to large.

