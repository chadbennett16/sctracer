# sctracer
Tracks system calls of another program when ran together
The tracer must be executed with another program. The tracer with print all system calls the other program makes
The tracer forks the other program to make it the child process. Ptrace is used to trace the system calls.
Once the child program has ended, the tracer exits as well
