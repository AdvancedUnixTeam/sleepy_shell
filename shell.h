#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;

void init_shell(void);
void shell_loop(void);

/* A process is a single process.  */
typedef struct process process;

 struct process {
  struct process *next;       /* next process in pipeline */
  char **argv;                /* for exec */
  pid_t pid;                  /* process ID */
  char completed;             /* true if process has completed */
  char stopped;               /* true if process has stopped */
  int status;                 /* reported status value */
};

/* A job is a pipeline of processes.  */
typedef struct job job;

struct job {
  struct job *next;           /* next active job */
  char *command;              /* command line, used for messages */
  struct process *first_process;     /* list of processes in this job */
  pid_t pgid;                 /* process group ID */
  char notified;              /* true if user told about stopped job */
  struct termios tmodes;      /* saved terminal modes */
  int stdin, stdout, stderr;  /* standard i/o channels */
};

/* The active jobs are linked into a list.  This is its head.   */
job *first_job = NULL;

/* Find the active job with the indicated pgid.  */
job *find_job (pid_t pgid);

/* Return true if all processes in the job have stopped or completed.  */
int job_is_stopped (job *j);

/* Return true if all processes in the job have completed.  */
int job_is_completed (job *j);

/* Put job j in the foreground.  If cont is nonzero,
   restore the saved terminal modes and send the process group a
   SIGCONT signal to wake it up before we block.  */
void put_job_in_foreground (job *j, int cont);

/* Put a job in the background.  If the cont argument is true, send
   the process group a SIGCONT signal to wake it up.  */
void put_job_in_background (job *j, int cont);

/* Store the status of the process pid that was returned by waitpid.
   Return 0 if all went well, nonzero otherwise.  */
int mark_process_status (pid_t pid, int status);

/* Check for processes that have status information available,
   without blocking.  */
void update_status (void);

/* Check for processes that have status information available,
   blocking until all processes in the given job have reported.  */
void wait_for_job (job *j);

/* Format information about job status for the user to look at.  */
void format_job_info (job *j, const char *status);

/* Notify the user about stopped or terminated jobs.
   Delete terminated jobs from the active job list.  */
void do_job_notification (void);

/*
 *  builtin commands
 */
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &shell_cd,
    &shell_help,
    &shell_exit
};

int shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "shell: expected arg to \"cd\"\n");
    } else {
        if(chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

int shell_help(char **args) {
    int i;
    printf("\nThis is a shell.\n\n");
    printf("The following commands are built in:\n");
    for(i=0;i<shell_num_builtins();i++) {
        printf("    %s\n",builtin_str[i]);
    }
    printf("\na man is a dogs best friend\n");
    printf("...good day\n\n");
    return 1;
}

int shell_exit(char **args) {
    return 0;
}
