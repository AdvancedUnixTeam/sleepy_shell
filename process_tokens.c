#include "shell.h"

process * create_process(   job * j, 
                            char ** tokens, int num_tokens, 
                            char ** infile, char ** outfile, char ** errfile, 
                            int position, int * process_count);
int is_io(char * s);
int is_pipe(char * s);
int is_input(char * s);
int is_output(char * s);
int is_err(char * s);


process * create_process(   job * j, 
                            char ** tokens, int num_tokens,
                            char ** infile, char ** outfile, char ** errfile, 
                            int position, int * process_count) {

    
    int buffsize = SHELL_RL_BUFFSIZE;
    int max_position = num_tokens;
    int cur_position = 0;
    int pipe_count = 0;
    int cur_argc = 0;
    struct process * cur_process = malloc(sizeof(struct process));
    char ** cur_args = malloc(sizeof(char * ) * buffsize);
    cur_process->argv = cur_args;
    cur_process->argc = cur_argc;
    
    if(!cur_args || !cur_process) {
        fprintf(stderr, "create_process: allocation error\n");
        exit(EXIT_FAILURE);
    }

    if(tokens[0] == NULL) {
        fprintf(stderr, "create_process: tokens are null\n");
        exit(EXIT_FAILURE);
    }

    if(max_position == 1) {
        (*process_count)++;
        cur_argc++;
        //printf("Process Count = %d\n", *process_count);
        cur_args[0] = tokens[0];
        cur_process->argv = cur_args;
        cur_process->argc = cur_argc;
        cur_process->next = NULL;
        return cur_process;
    }

    if(is_io(tokens[0])) {
        fprintf(stderr, "create_process: invalid input\n");
        exit(EXIT_FAILURE);
    }
    while(position < max_position) {
        
        if(is_io(tokens[position])) {
            //printf("IO found at position %d\n", position);
            if(is_pipe(tokens[position])) {
                //printf("Pipe found at position %d\n", position);
                cur_process->argv = cur_args;
                cur_process->argc = cur_argc;
                (*process_count)++;
                //printf("Process Count = %d\n", *process_count);
                cur_process->next = create_process(j, tokens, num_tokens, infile, outfile, errfile, ++position, process_count);
                return cur_process;
            }
            else if(is_input(tokens[position])) {
                *infile = tokens[++position];
                position++;
                continue;
            }
            else if(is_output(tokens[position])) {
                *outfile = tokens[++position];
                position++;
                continue;
            }
            else if(is_err(tokens[position])) {
                *errfile = tokens[++position];
                position++;
                continue;
            }
        }
        printf("Token %d: %s\nIncrementing argc\n",position, tokens[position]);
        cur_argc++;
        cur_process->argc = cur_argc;
        cur_args[cur_position] = tokens[position];
        cur_position++;
        position++;
        //printf("End of position %d\n", position);
    }
    (*process_count)++;
    //printf("Process Count = %d\n", *process_count);
    return cur_process;

}

int is_io(char * s) {

    if(is_pipe(s) || is_input(s) || is_output(s) || is_err(s)) {
        return 1;
    }
    return 0;
}

int is_pipe(char * s) {
    
    if(strcmp(s, "|") == 0) {
        return 1;
    }
    return 0;
}

int is_input(char * s) {
    if(strcmp(s, "<") == 0) {
        return 1;
    }
    return 0;
}

int is_output(char * s) {
    if(strcmp(s, ">") == 0) {
        return 1;
    }
    return 0;
}

int is_err(char * s) {
    if(strcmp(s, "2>") == 0) {
        return 1;
    }
    return 0;
}


int main() {
    /* Test Data */
    //printf("creating test data...\n");
    int num_tokens = 11;
    char * my_tokens[num_tokens];
    my_tokens[0] = "cmd1";
    my_tokens[1] = "-b";
    my_tokens[2] = "|";
    my_tokens[3] = "cmd2";
    my_tokens[4] = "-ce";
    my_tokens[5] = ">";
    my_tokens[6] = "out.txt";
    my_tokens[7] = "<";
    my_tokens[8] = "in.txt";
    my_tokens[9] = "2>";
    my_tokens[10] = "err.txt";


    

    int i, j;
    int cur_num_args = 0;
    int * process_count;
    char *infile = "";
    char *outfile = "";
    char *errfile = "";
    
    struct job *cur_job = malloc(sizeof(struct job));
    struct process *cur_process;

    if(!cur_job) {
        fprintf(stderr, "main: allocation error\n");
        exit(EXIT_FAILURE);
    }
    *process_count = 0;

    cur_job->first_process = create_process(cur_job, my_tokens, num_tokens, &infile, &outfile, &errfile, 0, process_count);

    cur_process = cur_job->first_process;
    
    i = 0;
    while(i < *process_count) {
        printf("# args for Process %d = %d\n", i, cur_process->argc);
        printf("Process %d: ", i);
        for(j=0; j < cur_process->argc; j++) {
            printf("%s ", cur_process->argv[j]);
        }
        cur_process = cur_process->next;
        printf("\n");
        i++;
    } 
    printf("Infile: %s\n", infile);
    printf("Outfile: %s\n", outfile);
    printf("Errfile: %s\n", errfile);
}

