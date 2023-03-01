## Introduction
The original code is placed in the `shlab-handout.tar`, and my solution is in the file `shlab-solution`.A detailed introduction about this lab is in `writeup`.  
## Problem Analysis
This shell lab needs us to complete the remaining empty functions in `tsh.c`, and build a shell which has the same behavior as `tshref` at last.  
In this lab, after read the `writeup`, if you are confused with how to start, here are some advice.  
1. Read the sample code in the ppt `14-ecf-procs` and `# 15-ecf-signals`. 
2. Add and test your code step by step according to `trace1-16.txt`, and use `-v` to see how the reference program `tshref` work in the txt in detail . 
3. You can use the `Rio_writen()` in `csapp.c`  to write a string to stdout in signal handler, which is a wrapper of `write()`. Unlike `printf()`, it is a `async-signal-safe`.  
4. The second and third arugments in `waitpid()` are very important. It can help us realize reap a zombie but doesn't wait for any other currently running children to terminate. And can help us deal with the process who is stopped because of a SIGSTOP or SIGTSTP signal.