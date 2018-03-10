#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "util.h"
#include "stage.h"
#include "parseline.h"

#define PROMPT "8-P "
/* TODO: Deal with ^D ^D */
void setup_env();
void sigint_handler(int signum);
void change_directory(char *path);
void launch_pipes(int total_stages, struct stage stages[]);
void telephone(int id);

int main(int argc, char *argv[]){
    struct stage stages[MAX_PIPES + 1];
    char command[MAX_COMMAND_LENGTH];
    int total_stages;
    setup_env();
    while (1) {
	printf("%s", PROMPT);
	get_line(command);
	if (command[0] == '\n')
	    continue;
	total_stages = parse_line(command, stages);
	
	if (total_stages == -1)
	    continue;
	else
	    launch_pipes(total_stages, stages);
    }
    return 0;
}

/* set up the environment for the shell to run in */
void setup_env(){
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
}

void sigint_handler(int signum){
    printf("\n%s", PROMPT);
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
    sigset_t new_set, old_set;
    int old[2], next[2];
    pid_t child;
    int i;
    int children = 0;
    sigemptyset(&new_set);
    sigaddset(&new_set, SIGINT);
    if (sigprocmask(SIG_BLOCK, &new_set, &old_set) < 0)
	perror("Sigmask");
    if (pipe(old) < 0) {
	perror("Pipe");
	exit(EXIT_FAILURE);
    }
    for (i = 0; i < total_stages; i++) {
	if (!strcmp(stages[i].argv[0], "cd")){
	    
	        if (stages[i].argc == 1){
		    printf("cd: missing argument.\n");
		    continue;
		}
		else if (stages[i].argc > 2) {
		    printf("cd: too many arguments.\n");
		    continue;
		}   
	        change_directory(stages[i].argv[1]);
		continue;
	    }
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
	    if (sigprocmask(SIG_SETMASK, &old_set, NULL))
		perror("Sigunset");
	    execvp(stages[i].argv[0], stages[i].argv);
	    perror(stages[i].argv[0]);
	    exit(EXIT_FAILURE);  
	}
	else {
	    children++;
	    close(old[0]);
	    close(old[1]);
	    old[0] = next[0];
	    old[1] = next[1];
	}
    }
    if (sigprocmask(SIG_SETMASK, &old_set, NULL))
	perror("Sigunset");
    while (children--)
	wait(NULL);
}
