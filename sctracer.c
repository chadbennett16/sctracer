#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

//Global array of system calls, each index is an ID
//Different systems have different number of system calls, allocate more than I need
int syscall_array[1000] = {0};

//When the child exits, print out all the system calls made in an output file
//Output is specified on command line
void print_syscalls(int * array, char * filename) {
	FILE *fp;
	fp = fopen(filename, "w");
	int i = 0;
	for (i = 0; i < 1000; i++) {
		if (syscall_array[i] > 0 && syscall_array[i] == 1) {
			fprintf(fp, "%d\t%d\n", i, syscall_array[i]);
		}
		else if (syscall_array[i] > 0) fprintf(fp, "%d\t%d\n", i, syscall_array[i]/2);
	}
}

int main (int argc, char **argv) {
	
	int i = 0;
	char *token;
	char *args[strlen(argv[1])];

	//Use strtok to get the name of the child program and its dependants.
	//Separate the spaces from the second command line argument

	token = strtok(argv[1], " ");

	while (token != NULL) {
		args[i] = token;
		i++;
		token = strtok(NULL, " ");
	}
	args[i] = NULL;

	pid_t child = fork();
	if (child == 0) {	//Child process
		
		ptrace(PTRACE_TRACEME); //indicate this child wants to be traced
		
		kill(getpid(), SIGSTOP); //Send signal to parent indicating child stopped

		int i = execvp(args[0], args);
		if (i < 0) perror("exec failed");

	}

	else {	//Parent process

		int status, syscall_num;

		//wait for the child to stop itself
		waitpid(child, &status, 0);

		//distinguish between normal traps and system calls
		ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);

		while (1) {	
			//Trace system calls until the child exits
			do{

            	ptrace(PTRACE_SYSCALL, child, 0, 0);

            	waitpid(child, &status, 0);

            	if (WIFEXITED(status)) {
					print_syscalls(syscall_array, argv[argc-1]);
					exit(1);

				}

        	} while (!(WIFSTOPPED(status) && WSTOPSIG(status) & 0x80));
			//Get the system call ID and save its count in the array
        	syscall_num = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*ORIG_RAX, NULL);
			syscall_array[syscall_num]++;

		}
		
	
	waitpid(child, NULL, 0); //waiting for end of child

	}
	
	return 0;

}
