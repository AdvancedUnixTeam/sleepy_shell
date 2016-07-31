#include "shell.h"

int main(void)
{
    shell_terminal = STDIN_FILENO;
    shell_is_interactive = isatty (shell_terminal); // See if we are running interactively.
    char *line;
    char **args;
    int status=1;
    struct job *j = malloc(sizeof(struct job));
    job_count = 0;
    first_job = malloc(sizeof(struct job));


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

            int num_tokens;
            int foreground;
            args = shell_split_line(line, &num_tokens);

            struct job *cur_job = malloc(sizeof(struct job));
            if(!cur_job){
                fprintf(stderr, "main: allocation error\n");
                        exit(EXIT_FAILURE);
            }

            foreground = 1;

            int process_count = 0;
            cur_job->stdin = STDIN_FILENO;
            cur_job->stdout = STDOUT_FILENO;
            cur_job->stderr = STDERR_FILENO;

            cur_job->first_process = create_process(cur_job, args, num_tokens, 0, &process_count, &foreground);
            cur_job->next = NULL;

            
            // TO DO: Build Job Linked List, currently just keeping track of 1  job

            status = launch_job(cur_job, foreground); //change so it returns int status

            if(job_count == 0) {
                
                first_job = cur_job;
                first_job->next = NULL;
                job_count++;
            }
            else {
                if(first_job->next == NULL) {
                    first_job->next = cur_job;
                    job_count++;
                }
                else {
                    j = first_job;
                    while(j->next != NULL) {
                        j = j->next;
                    }

                    j->next = cur_job;
                    job_count++;
                }
    
            }
            do_job_notification();


            free(line);
            free(args);
        } while (status);

/***************Finish Shell Loop****************/

    // Shut Down / Clean Up
    return EXIT_SUCCESS;
}
