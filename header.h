#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>

#define BUILTIN		1
#define EXTERNAL	2
#define NO_COMMAND  3

/* Macros for the color */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct job
{
    int job_id;         // ID of stopped process
    pid_t job_pid;      //  pid of job
    int state;          // (0 = stopped, 1 = running)
    char job_cmd[100];  // For storing the command
    struct job *next;   // For storing the next node address
}jobs;


extern char *external_commands[153];
extern char prompt[25];
extern jobs *head;
extern pid_t pid;
extern int status;
extern int last_st;
extern int job_id;

/* Function declarations */

/* Function for detecting the input/output and handling it */
void scan_input(char *input_string);

/* Function for getting the command what user passes */
char *get_command(char *input_string);

/* Function for checking the command type */
int check_command_type(char *command);

/* Function for executing the internal commands */
void execute_internal_commands(char *input_string);

/* Function for handling the Signals if triggered */
void signal_handler(int sig_num);

/* Function for extracting the internal commands */
void extract_external_commands(char **external_commands);

/* Function for executing the external commands */
void execute_external_commands(char *input_string);

/* Function for inserting the stopped jobs */
void insert_atlast_jobs(pid_t pid, char *input_string);

/* Function for printing the jobs which are inserted */
void printing_jobs(struct job *jobs);     

#endif