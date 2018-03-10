#ifndef parseline_h
#define parseline_h



int parse_line(char command[], struct stage stages[]);
int get_line(char command[], FILE *file);
void parse_stage(char *command, struct stage *stage,
                 int current_stage, int total_stages);
void handle_invalid_redirection(int argc, char *argv[], int is_input);
void handle_ambiguous_input(char *argv[], int is_input);

#endif
