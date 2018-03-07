#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "util.h"
#include "stage.h"
#include "parseline.h"

void change_directory(char *path);


int main(int argc, char *argv[]){
    
    return 0;
}


void change_directory(char *path){
    if (chdir(path) < 0)
	perror(path);
}
