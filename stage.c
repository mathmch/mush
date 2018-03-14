#include <stdio.h>
#include <string.h>
#include "stage.h"

/* builds the stage, containing the command name, its location in the pipe,
 * its inputs and outputs, its arguments, and its number of arguments */
void setup_stage(struct stage *stage, int number, char *command, char *input,
                 char *output, int argc, char *argv[], int total_stages) {
    int i;

    stage->number = number;
    strcpy(stage->command, command);

    if (*input != '\0') {
	/* stage has input redirect */
        strcpy(stage->input, input);
        stage->has_input_redirection = 1;
    } else if (number == 0) {
	/* stage is 0th stage with no input redirect */
        strcpy(stage->input, "original stdin");
        stage->has_input_redirection = 0;
    } else {
	/* stage is non 0th stage with no input redirect */
        char formatted[INPUT_MAX];
        sprintf(formatted, "pipe from stage %d", number - 1);
        strcpy(stage->input, formatted);
        stage->has_input_redirection = 0;
    }

    if (*output != '\0') {
	/* stage has input redirect */
        strcpy(stage->output, output);
        stage->has_output_redirection = 1;
    } else if (number == total_stages - 1) {
	/* stage is last stage with output redirect */
        strcpy(stage->output, "original stdout");
        stage->has_output_redirection = 0;
    } else {
	/* stage is non last stage with no output redirect */
        char formatted[OUTPUT_MAX];
        sprintf(formatted, "pipe to stage %d", number + 1);
        strcpy(stage->output, formatted);
        stage->has_output_redirection = 0;
    }

    stage->argc = argc;
    for (i = 0; i < argc; i++) {
        stage->argv[i] = argv[i];
    }
    /* NULL terminate argv array */
    stage->argv[i] = NULL;
}
/* prints the information about the stage in an easily readable format
 * useful for debugging */
void print_stage(struct stage *stage) {
    int i;
    printf("--------\n");
    printf("Stage %d: \"%s\"\n", stage->number, stage->command);
    printf("--------\n");
    printf("     input: %s\n", stage->input);
    printf("    output: %s\n", stage->output);
    printf("      argc: %d\n", stage->argc);
    printf("      argv: ");
    for (i = 0; i < stage->argc; i++) {
        printf("\"%s\"", stage->argv[i]);
        if (stage->argc != 1 && i < stage->argc - 1)
            printf(",");
    }

    printf("\n\n");
}
