#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "util.h"
#include "stage.h"
#include "parseline.h"

#define PROMPT "8-P "

void setup_env(void);
void sigint_handler(int signum);
void change_directory(char *path);
void launch_pipes(int total_stages, struct stage stages[]);

/* global variable for whether the prompt should be printed */
int should_print_prompt = 1;

int main(int argc, char *argv[]) {
    struct stage stages[MAX_PIPES + 1];
    /* command length: +1 for null, +1 for checking if too long */
    char command[MAX_COMMAND_LENGTH + 2];
    FILE *file;
    int total_stages;

    if (argc == 2) {
        /* batch processing */
        if ((file = fopen(argv[1], "r")) == NULL) {
            perror(argv[1]);
            exit(EXIT_FAILURE);
        }
    } else {
        /* interactive */
        file = stdin;
    }

    /* only print the prompt if input and output are both ttys */
    should_print_prompt = isatty(fileno(file)) && isatty(fileno(stdout));
    setup_env();

    while (1) {
        int line_length;
        if (should_print_prompt)
            printf("%s", PROMPT);
        line_length = get_line(command, file);
        switch (line_length) {
            case -1:
                /* invalid command */
                continue;
            case 0:
                /* ^D */
                if (should_print_prompt)
                    putchar('\n');
                exit(EXIT_SUCCESS);
        }
        total_stages = parse_line(command, stages);
        if (total_stages == -1)
            continue;
        else
            launch_pipes(total_stages, stages);
    }

    return 0;
}

/* set up the environment for the shell to run in,
 * i.e. handle SIGINT appropriately */
void setup_env(){
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

/* SIGINT should be caught while the shell is in action */
void sigint_handler(int signum) {
    if (should_print_prompt)
        putchar('\n');
}

/* util for changing directory and catching error */
void change_directory(char *path){
    if (chdir(path) < 0)
        perror(path);
}

void launch_pipes(int total_stages, struct stage stages[]) {
    #define READ 0
    #define WRITE 1
    sigset_t new_set, old_set;
    int old[2], next[2];
    pid_t child;
    int i;
    int children = 0;

    /* block SIGINT while setting up plumbing */
    sigemptyset(&new_set);
    sigaddset(&new_set, SIGINT);
    if (sigprocmask(SIG_BLOCK, &new_set, &old_set) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    if (total_stages > 1) {
        /* only create the pipe if more than one stage */
        if (pipe(old) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < total_stages; i++) {
        /* handle the special cd case */
        if (strcmp(stages[i].argv[0], "cd") == 0) {
            if (stages[i].argc == 1) {
                printf("cd: missing argument.\n");
                continue;
            } else if (stages[i].argc > 2) {
                printf("cd: too many arguments.\n");
                continue;
            }
            change_directory(stages[i].argv[1]);
            continue;
        }

        if (i < total_stages - 1)
            pipe(next); /* pipe for the next stage */

        if (!(child = fork())) {
            /* child */
            if (stages[i].has_input_redirection) {
                /* open the file for input redirection */
                int fd = open(stages[i].input, O_RDONLY);
                if (fd < 0) {
                    perror(stages[i].input);
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            } else if (total_stages > 1) {
                dup2(old[READ], STDIN_FILENO);
            }

            if (total_stages > 1) {
                close(old[WRITE]);
                close(old[READ]);
            }

            if (stages[i].has_output_redirection) {
                /* create the file for output redirection */
                int fd = open(stages[i].output,
                              O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror(stages[i].output);
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            } else if (i < total_stages-1) {
                dup2(next[WRITE], STDOUT_FILENO);
            }

            if (i < total_stages -1) {
                close(next[READ]);
                close(next[WRITE]);
            }

            /* unblock SIGINT in child */
            if (sigprocmask(SIG_SETMASK, &old_set, NULL)) {
                perror("sigprocmask");
                exit(EXIT_FAILURE);
            }

            execvp(stages[i].argv[0], stages[i].argv);
            perror(stages[i].argv[0]);
            exit(EXIT_FAILURE);
        }

        /* parent */
        children++;
        if (total_stages > 1) {
            close(old[READ]);
            close(old[WRITE]);
            if (i < total_stages - 1) {
                old[READ] = next[READ];
                old[WRITE] = next[WRITE];
            }
        }
    }

    /* unblock SIGINT in parent */
    if (sigprocmask(SIG_SETMASK, &old_set, NULL)) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    /* wait for children to die (sorry!) */
    while (children--)
        wait(NULL);
}

