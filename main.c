#include "shell.h"

int main(void)
{
    shell_terminal = STDIN_FILENO;
    shell_is_interactive = isatty (shell_terminal); // See if we are running interactively.
    char *line;
    char **args;
    int status=1;

    // Config Files
    // Command Line Loop

/*********** Initialize shell******************/

/* Make sure the shell is running interactively as the foreground job
before proceeding. */
    if (shell_is_interactive) {
        /* Loop until we are in the foreground.  */
        while (tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp ()))
            kill (- shell_pgid, SIGTTIN);

        /* Ignore interactive and job-control signals.  */
        signal (SIGINT, SIG_IGN);
        signal (SIGQUIT, SIG_IGN);
        signal (SIGTSTP, SIG_IGN);
        signal (SIGTTIN, SIG_IGN);
        signal (SIGTTOU, SIG_IGN);
        signal (SIGCHLD, SIG_IGN);

        /* Put ourselves in our own process group.  */
        shell_pgid = getpid ();
        if (setpgid (shell_pgid, shell_pgid) < 0) {
            perror ("Couldn't put the shell in its own process group");
            exit (1);
        }

        /* Grab control of the terminal.  */
        tcsetpgrp (shell_terminal, shell_pgid);

        /* Save default terminal attributes for shell.  */
        tcgetattr (shell_terminal, &shell_tmodes);
    }

/***************Finished Initializing Shell******/

/***************Start Shell Loop*****************/
        do
        {
            printf("【ツ】 ");
            line = shell_read_line();
            //printf("line entered: %s\n", line);
            int num_tokens;
            args = shell_split_line(line, &num_tokens);
            int i;
            // for(i=0;args[i] != NULL;i++)
            //     printf("token #%d: %s\n", i+1, args[i]);
            struct job *cur_job = malloc(sizeof(struct job));
            if(!cur_job){
                fprintf(stderr, "main: allocation error\n");
                        exit(EXIT_FAILURE);
            }
     //     shell_process_tokens(theJob, args);
            struct process *cur_process;
            char *infile = "";
            char *outfile = "";
            char *errfile = "";
            int process_count = 0;
            int cur_num_args = 0;

            cur_job->first_process = create_process(cur_job, args, num_tokens, &infile, &outfile, &errfile, 0, &process_count);
            cur_process = cur_job->first_process;

            i = 0;
            while(i < process_count) {

                cur_num_args = cur_process->argc;
                printf("Process %d: ", i);

                int j;
                for(j=0; j < cur_num_args; j++) {
                    printf("%s ", cur_process->argv[j]);
                }

                cur_process = cur_process->next;
                printf("\n");

                i++;
            }
            printf("Infile: %s\n", infile);
            printf("Outfile: %s\n", outfile);
            printf("Errfile: %s\n", errfile);

            status = launch_job(cur_job, 0); //change so it returns int status
            free(line);
            free(args);
        } while (status);

/***************Finish Shell Loop****************/

    // Shut Down / Clean Up
    return EXIT_SUCCESS;
}
