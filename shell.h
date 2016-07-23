#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define SHELL_TOK_BUFFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
#define SHELL_RL_BUFFSIZE 1024

/* A process is a single process.  */
typedef struct process process;
 struct process {
  struct process *next;          // next process in pipeline
  char **argv;                   // for exec
  pid_t pid;                     // process ID
  char completed;                // true if process has completed
  char stopped;                  // true if process has stopped
  int status;                    // reported status value
};

/* A job is a pipeline of processes.  */
typedef struct job job;
struct job {
  struct job *next;              // next active job
  char *command;                 // command line, used for messages
  struct process *first_process; // list of processes in this job
  pid_t pgid;                    // process group ID */
  char notified;                 // true if user told about stopped job
  struct termios tmodes;         // saved terminal modes
  int stdin, stdout, stderr;     // standard i/o channels
};

pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;
int shell_execute(char **args);
char *shell_readLine(void);
char **shell_split_line(char *line);
int shell_processTokens(job *j, char ** args);
void launch_job (job *j, int foreground);
job *first_job;
job *find_job (pid_t pgid);
int job_is_stopped (job *j);
int job_is_completed (job *j);
void put_job_in_foreground (job *j, int cont);
void put_job_in_background (job *j, int cont);
int mark_process_status (pid_t pid, int status);
void update_status (void);
void wait_for_job (job *j);
void format_job_info (job *j, const char *status);
void do_job_notification (void);
void free_job(void *j);

/*
 *  builtin commands
 */
int shell_num_builtins();
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

#endif
