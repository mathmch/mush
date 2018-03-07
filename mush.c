#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "util.h"
#include "stage.h"
#include "parseline.h"

void setup_env();
void sigint_handler(int signum);
void change_directory(char *path);


int main(int argc, char *argv[]){
    
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
