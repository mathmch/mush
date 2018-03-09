#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "util.h"
#include "stage.h"
#include "parseline.h"

#define PROMPT "8-P "

void setup_env();
void sigint_handler(int signum);
void change_directory(char *path);
void launch_pipes(int total_stages, struct stage stages[]);
void telephone(int id);

int main(int argc, char *argv[]){
    struct stage stages[MAX_PIPES + 1];
    char command[MAX_COMMAND_LENGTH];
    int total_stages;
    //printf("%s", PROMPT);
    get_line(command);
    total_stages = parse_line(command, stages);
    launch_pipes(total_stages, stages);
    return 0;
}

/* set up the environment for the shell to run in */
void setup_env(){

}

void sigint_handler(int signum){
    /* do nothing, just keep mush from dying on SIGINT */
}

void change_directory(char *path){
    if (chdir(path) < 0)
	perror(path);
}

void telephone(int id){
    int c;
    while (EOF != (c = getchar()))
	putchar(c);
    printf("This is process %d\n", id);
}
void launch_pipes(int total_stages, struct stage stages[]){
    #define READ 0
    #define WRITE 1
    int old[2], next[2];
    pid_t child;
    int i;
    if (pipe(old) < 0) {
	perror("Pipe");
	exit(EXIT_FAILURE);
    }
    for (i = 0; i < total_stages; i++) {
	if ( i <  total_stages -1)
	    pipe(next);
	if (!(child = fork())) {
	    dup2(old[READ], STDIN_FILENO);
	    if (i != total_stages-1)
		dup2(next[WRITE], STDOUT_FILENO);
	    close(old[0]);
	    close(old[1]);
	    close(next[0]);
	    close(next[1]);
	    telephone(i);
	    exit(EXIT_FAILURE);  
	}
	else {
	    close(old[0]);
	    close(old[1]);
	    old[0] = next[0];
	    old[1] = next[1];
	}
    }
    while (total_stages--)
	wait(NULL);

}
