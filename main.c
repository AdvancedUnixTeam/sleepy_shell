#include "shell.h"

int main(int argc, char **argv)
{
    // Config Files
    // Command Line Loop
    init_shell();
    shell_loop();
    // Shut Down / Clean Up
    return EXIT_SUCCESS;
}

/* Make sure the shell is running interactively as the foreground job
   before proceeding. */
void init_shell (void) {
    /* See if we are running interactively.  */
    shell_terminal = STDIN_FILENO;
    shell_is_interactive = isatty (shell_terminal);

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
}

void shell_loop(void)
{
    char *line;
    char **args;
    int status=1;
    do
    {
        printf("【ツ】 ");
        //line = shell_readLine();
        //args = shell_split_line(line);
        job *theJob = malloc(sizeof(job));
        //shell_processTokens(theJob, args);
        //launch_job(theJob);
        //status = shell_execute(args);
       // free(line);
       // free(args);
    } while (status);
}
