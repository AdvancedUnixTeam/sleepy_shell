#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

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
