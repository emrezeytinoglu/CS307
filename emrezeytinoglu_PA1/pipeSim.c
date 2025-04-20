#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
   
    
    int fd[2];
    pipe(fd);
    printf("I'm the parent process with PID: %d\n", (int) getpid());
    int man_pid = fork();
    if (man_pid < 0) {
        perror("Forking 'man' process failed");
        exit(1);
    }

    else if (man_pid == 0) {
       
        printf("I'm the first child (man) process with PID: %d\n", (int) getpid());
        int grep_pid = fork();
        if (grep_pid < 0) {
            perror("Forking 'grep' process failed");
            exit(1);
        }
        else if(grep_pid == 0){
        printf("I'm the second child (grep) process with PID: %d\n", (int) getpid());
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(STDOUT_FILENO);
        open("./output.txt", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
        char *myargs[4];
        myargs[0] = strdup("grep");
        myargs[1] = strdup("--");
        myargs[2] = strdup("-l");
        myargs[3] = NULL;
        execvp(myargs[0], myargs);
        }
     
      
        else {
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            char *myargs[3];
	    myargs[0] = strdup("man");
            myargs[1] = strdup("ls");
	    myargs[2] = NULL;
	    execvp(myargs[0], myargs);
            } 
        }
       
      else {
      	    close(fd[1]);
            close(fd[0]); 
            wait(NULL); 
            wait(NULL);
            printf("Parent process is done.\n");
        }
    

    return 0;
}

  
