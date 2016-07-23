#include "shell.h"

int main(int argc, char **argv)
{
    shell_terminal = STDIN_FILENO;
    shell_is_interactive = isatty (shell_terminal); // See if we are running interactively.
    char *line;
    char **args;
    int status =1;

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
            args = shell_split_line(line);
            job *theJob = malloc(sizeof(job));
            shell_process_tokens(theJob, args);
            printf("before stderr: %d\n", theJob->stderr);
            printf("stdin: %d\n", theJob->stdin);
            printf("stdout: %d\n", theJob->stdout);
            printf("pgid: %d\n", theJob->pgid);
            struct process *proc = theJob->first_process;
            printf("first process: %s\n", proc->argv[0]);
            launch_job(theJob, 1); //change so it returns int status
            printf("after stderr: %d\n", theJob->stderr);
            printf("stdin: %d\n", theJob->stdin);
            printf("stdout: %d\n", theJob->stdout);
            printf("pgid: %d\n", theJob->pgid);
            printf("first process: %s\n", theJob->first_process->argv[0]);
            //status = shell_execute(args);
            free(line);
            free(args);
        } while (status);

/***************Finish Shell Loop****************/

    // Shut Down / Clean Up
    return EXIT_SUCCESS;
}
