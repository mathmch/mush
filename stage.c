#include <stdio.h>
#include <string.h>
#include "stage.h"

void setup_stage(struct stage *stage, int number, char *command, char *input,
                 char *output, int argc, char *argv[], int total_stages) {
    int i;

    stage->number = number;
    strcpy(stage->command, command);

    if (*input != '\0') {
        strcpy(stage->input, input);
    } else if (number == 0) {
        strcpy(stage->input, "original stdin");
    } else {
        char formatted[INPUT_MAX];
        sprintf(formatted, "pipe from stage %d", number - 1);
        strcpy(stage->input, formatted);
    }

    if (*output != '\0') {
        strcpy(stage->output, output);
    } else if (number == total_stages - 1) {
        strcpy(stage->output, "original stdout");
    } else {
        char formatted[OUTPUT_MAX];
        sprintf(formatted, "pipe to stage %d", number + 1);
        strcpy(stage->output, formatted);
    }

    stage ->argc = argc;
    for (i = 0; i < argc; i++) {
        stage->argv[i] = argv[i];
    }
    stage->argv[i] = NULL;
}

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
