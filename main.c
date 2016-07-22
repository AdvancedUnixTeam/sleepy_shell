#include "shell.h"

#define SHELL_TOK_BUFFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
#define SHELL_RL_BUFFSIZE 1024
#define SHELL_TOK_BUFFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

/* Check for processes that have status information available,
   without blocking.  */
void update_status (void) {
    int status;
    pid_t pid;

    do
        pid = waitpid (WAIT_ANY, &status, WUNTRACED|WNOHANG);
    while (!mark_process_status (pid, status));
}

/* Return true if all processes in the job have completed.  */
int job_is_completed (job *j) {

    process *p;

    for (p = j->first_process; p; p = p->next)
    if (!p->completed)
      return 0;

    return 1;
}

/* Return true if all processes in the job have stopped or completed.  */
int job_is_stopped (job *j) {

    process *p;

    for (p = j->first_process; p; p = p->next)
    if (!p->completed && !p->stopped)
      return 0;

    return 1;
}

/* Notify the user about stopped or terminated jobs.
   Delete terminated jobs from the active job list.  */
void do_job_notification (void) {
    job *j, *jlast, *jnext;
    process *p;

    /* Update status information for child processes.  */
    update_status ();

    jlast = NULL;
    for (j = first_job; j; j = jnext) {
        jnext = j->next;

        /* If all processes have completed, tell the user the job has
         completed and delete it from the list of active jobs.  */
        if (job_is_completed (j)) {
            format_job_info (j, "completed");

            if (jlast)
                jlast->next = jnext;
            else
                first_job = jnext;

            //free_job (j);  ************ NOTE: Not yet implemented *****************
        }
        /* Notify the user about stopped jobs,
         marking them so that we won’t do this more than once.  */
        else if (job_is_stopped (j) && !j->notified) {
            format_job_info (j, "stopped");
            j->notified = 1;
            jlast = j;
        }
        /* Don’t say anything about jobs that are still running.  */
        else
            jlast = j;
    }
}

/* Store the status of the process pid that was returned by waitpid.
   Return 0 if all went well, nonzero otherwise.  */
int mark_process_status (pid_t pid, int status) {
    job *j;
    process *p;
    if (pid > 0) {
        /* Update the record for the process.  */
        for (j = first_job; j; j = j->next)
        for (p = j->first_process; p; p = p->next)
            if (p->pid == pid) {
                p->status = status;
                if (WIFSTOPPED (status))
                    p->stopped = 1;
                else {
                    p->completed = 1;
                    if (WIFSIGNALED (status))
                        fprintf (stderr, "%d: Terminated by signal %d.\n", (int) pid, WTERMSIG (p->status));
                }
                return 0;
             }
        fprintf (stderr, "No child process %d.\n", pid);
        return -1;
    }
    else if (pid == 0 || errno == ECHILD)
        /* No processes ready to report.  */
        return -1;
    else {
        /* Other weird errors.  */
        perror ("waitpid");
        return -1;
    }
}

/* Format information about job status for the user to look at.  */
void format_job_info (job *j, const char *status) {
  fprintf (stderr, "%ld (%s): %s\n", (long)j->pgid, status, j->command);
}

/* Check for processes that have status information available,
   blocking until all processes in the given job have reported.  */
void wait_for_job (job *j) {
    int status;
    pid_t pid;

    do
        pid = waitpid (WAIT_ANY, &status, WUNTRACED);
    while (!mark_process_status (pid, status) && !job_is_stopped (j) && !job_is_completed (j));
}

void launch_process (process *p,
                        pid_t pgid,
                        int infile, int outfile, int errfile,
                        int foreground) {
    pid_t pid;

    if (shell_is_interactive) {
        /* Put the process into the process group and give the process group
         the terminal, if appropriate.
         This has to be done both by the shell and in the individual
         child processes because of potential race conditions.  */
        pid = getpid ();

        if (pgid == 0)
            pgid = pid;

        setpgid (pid, pgid);

        if (foreground)
            tcsetpgrp (shell_terminal, pgid);

        /* Set the handling for job control signals back to the default.  */
        signal (SIGINT, SIG_DFL);
        signal (SIGQUIT, SIG_DFL);
        signal (SIGTSTP, SIG_DFL);
        signal (SIGTTIN, SIG_DFL);
        signal (SIGTTOU, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
    }

    /* Set the standard input/output channels of the new process.  */
    if (infile != STDIN_FILENO) {
        dup2 (infile, STDIN_FILENO);
        close (infile);
    }
    if (outfile != STDOUT_FILENO) {
        dup2 (outfile, STDOUT_FILENO);
        close (outfile);
    }
    if (errfile != STDERR_FILENO) {
        dup2 (errfile, STDERR_FILENO);
        close (errfile);
    }

    /* Exec the new process.  Make sure we exit.  */
    execvp (p->argv[0], p->argv);
    perror ("execvp");
    exit (1);
}

void launch_job (job *j, int foreground) {
    struct process *p;
    pid_t pid;
    int mypipe[2], infile, outfile;
    infile = j->stdin;
    for (p = j->first_process; p; p = p->next) {
        /* Set up pipes, if necessary.  */
        if (p->next) {
            if (pipe (mypipe) < 0) {
                perror ("pipe");
                exit (1);
            }
            outfile = mypipe[1];
        }
        else
            outfile = j->stdout;

        /* Fork the child processes.  */
        pid = fork ();
        if (pid == 0)
            /* This is the child process.  */
            launch_process (p, j->pgid, infile, outfile, j->stderr, foreground);
        else if (pid < 0) {
            /* The fork failed.  */
            perror ("fork");
            exit (1);
        }
        else {
            /* This is the parent process.  */
            p->pid = pid;
            if (shell_is_interactive) {
                if (!j->pgid)
                j->pgid = pid;
                setpgid (pid, j->pgid);
            }
        }

        /* Clean up after pipes.  */
        if (infile != j->stdin)
            close (infile);
        if (outfile != j->stdout)
            close (outfile);

        infile = mypipe[0];
    }

    format_job_info (j, "launched");

    if (!shell_is_interactive)
        wait_for_job (j);
    else if (foreground)
        put_job_in_foreground (j, 0);
    else
       put_job_in_background (j, 0);
}

int shell_processTokens(job *j, char ** args) {
    int i;
    for(i = 0; i < (sizeof(args) / sizeof(args[0])); i++) {
        if(strcmp(args[i],"|"))
            printf(" Pipe !\n" );
        else if(0)
            ;
        else
            printf("Invalid input\n" );
    }
    return i;
}

/*
 *  Does not allow quoting or backslash escaping in command line args
 */
char **shell_split_line(char *line) {
    int buffsize = SHELL_TOK_BUFFSIZE, position = 0;
    char **tokens = malloc(buffsize * sizeof(char*));
    char *token;
    if(!tokens) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, SHELL_TOK_DELIM);
    while(token != NULL) {
        tokens[position] = token;
        position++;
        if(position >= buffsize) {
            buffsize += SHELL_TOK_BUFFSIZE;
            tokens = realloc(tokens, buffsize * sizeof(char*));
            if(!tokens) {
                fprintf(stderr, "shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/*
 *  The code in this method could be replaced with the following:
 *
 *      char *line = NULL;
 *      ssize_t bufsize = 0; // have getline allocate a buffer for us
 *      getline(&line, &bufsize, stdin);
 *      return line;
 *
 *  Keeping it this way for more control of buffer reallocation later on
 */
char *shell_readLine(void)
{
    int buffsize = SHELL_RL_BUFFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffsize);
    int c;
    if(!buffer) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    while(1) {
        // Read a character
        c = getchar();
        // If EOF, replace with null and return
        if(c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        // Check if we have exceeded the buffer and reallocate if necessary
        if(position >= buffsize) {
            buffsize += SHELL_RL_BUFFSIZE;
            buffer = realloc(buffer, buffsize);
            if(!buffer) {
                fprintf(stderr, "shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int shell_launch(char **args) {
    pid_t pid, wpid;
    int status;
    pid=fork();
    if(pid==0) {
        if(execvp(args[0],args)==-1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        perror("shell");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int shell_execute(char **args) {
    int i;
    if(args[0] == NULL) {
        return 1;
    }
//    for(i=0; i<shell_num_builtins(); i++) {  ******** NOTE: shell_num function not implemented yet********
//        if(strcmp(args[0], builtin_str[i]) == 0) {
//            return (*builtin_func[i]) (args);
//        }
//    }
    return shell_launch(args);
}

void shell_loop(void)
{
    char *line;
    char **args;
    int status=1;
    do
    {
        printf("【ツ】 ");
        line = shell_readLine();
        args = shell_split_line(line);
        job *theJob = malloc(sizeof(job));
        shell_processTokens(theJob, args);
        launch_job(theJob, 1);
        status = shell_execute(args);
        free(line);
        free(args);
    } while (status);
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


int main(int argc, char **argv)
{
    // Config Files
    // Command Line Loop
    init_shell();
    shell_loop();
    // Shut Down / Clean Up
    return EXIT_SUCCESS;
}
