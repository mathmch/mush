#ifndef parseline_h
#define parseline_h



int parse_line(char command[], struct stage stages[]);
int get_line(char command[], FILE *file);
int parse_stage(char *command, struct stage *stage,
                 int current_stage, int total_stages);
int handle_invalid_redirection(int argc, char *argv[], int is_input);
int handle_ambiguous_input(char *argv[], int is_input);

#endif
