#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include "stage.h"
#include "util.h"
#include "parseline.h"

typedef enum { none, expecting, received } redirect_status;

int parse_line(char command[], struct stage stages[]) {
    char *token;
    int total_stages;
    int i;
    char *end;

    total_stages = count_occurrences(command, '|') + 1;

    if (total_stages > MAX_PIPES + 1) {
        fprintf(stderr, "pipeline too deep\n");
        return -1;
    }

    token = command;
    for (i = 0; i < total_stages; i++) {
        end = strpbrk(token, "|\n\0");
        *end = '\0';
        if (strcmp(token, " ") == 0 || *token == '\0') {
            fprintf(stderr, "invalid null command\n");
            return -1;
        }
        parse_stage(token, &stages[i], i, total_stages);
        token = end + 1;
    }
    return total_stages;
}

void get_line(char command[], FILE *file) {
    #define SUCCESS 0
    errno = SUCCESS;
    while (fgets(command, MAX_COMMAND_LENGTH*2, file) == NULL) {
        if (errno == SUCCESS)
            exit(EXIT_SUCCESS);
        if (errno != EINTR) {
            perror("fgets");
            exit(EXIT_FAILURE);
        }	
    }
    if (strlen(command) > MAX_COMMAND_LENGTH) {
        fprintf(stderr, "command too long\n");
        exit(EXIT_FAILURE);
    }
}

void parse_stage(char *command, struct stage *stage,
                 int current_stage, int total_stages) {
    char command_copy[MAX_COMMAND_LENGTH];
    char input[INPUT_MAX];
    char output[OUTPUT_MAX];
    int argc = 0;
    char *argv[MAX_ARGUMENTS];
    redirect_status input_status = none;
    redirect_status output_status = none;
    char *token;
    const char *delim = " \n";

    strcpy(command_copy, command);
    input[0] = '\0';
    output[0] = '\0';

    for (token = strtok(command, delim);
         token != NULL;
         token = strtok(NULL, delim)) {
        if (strcmp(token, "<") == 0) {
            /* input redirection */
            if (input_status != none || output_status == expecting)
                handle_invalid_redirection(argc, argv,
                                           input_status != none);
            input_status = expecting;
        } else if (strcmp(token, ">") == 0) {
            /* output redirection */
            if (input_status == expecting || output_status != none)
                handle_invalid_redirection(argc, argv,
                                           input_status == expecting);
            output_status = expecting;
        } else if (input_status == expecting) {
            /* record input redirection source */
            strcpy(input, token);
            input_status = received;
        } else if (output_status == expecting) {
            /* record output redirection source */
            strcpy(output, token);
            output_status = received;
        } else {
            /* command name or argument */
            argv[argc] = token;
            argc++;

            if (argc > MAX_ARGUMENTS) {
                fprintf(stderr, "%s: too many arguments", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

    if (input_status == expecting)
        handle_invalid_redirection(argc, argv, 1);
    else if (output_status == expecting)
        handle_invalid_redirection(argc, argv, 0);

    if (*input != '\0' && current_stage != 0)
        handle_ambiguous_input(argv, 1);
    else if (*output != '\0' && current_stage != total_stages - 1)
        handle_ambiguous_input(argv, 0);

    setup_stage(stage, current_stage, command_copy, input,
                output, argc, argv, total_stages);
}

void handle_invalid_redirection(int argc, char *argv[], int is_input) {
    char *source = (argc != 0) ? argv[0] : is_input ? "<" : ">";
    char *redirect_type = is_input ? "input" : "output";
    fprintf(stderr, "%s: bad %s redirection\n", source, redirect_type);
    exit(EXIT_FAILURE);
}

void handle_ambiguous_input(char *argv[], int is_input) {
    char *redirect_type = is_input ? "input" : "output";
    fprintf(stderr, "%s: ambiguous %s\n", argv[0], redirect_type);
    exit(EXIT_FAILURE);
}
