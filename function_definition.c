#include "header.h"

/* 2D array for builtin commands */
char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
					"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
					"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", "fg",
				    "bg", "jobs", NULL};

/* Declaring and initializing  */
char *external_commands[153];
char cmd[30];
pid_t pid = 0;
int status, last_st = 0;
jobs* head = NULL;
int job_id = 1;

/* Extracting the External commands */
void extract_external_commands(char **external_commands)
{
	char ch, buffer[30];
	int count;

	int fd = open("ext_cmd.txt", O_RDONLY);
	
	if(fd < 0)
	{
		perror("open");
		return;
	}
	/* total 153 commands */
	for(int i = 0; i < 153; i++)
	{
		count = 0;
		while(read(fd, &ch, 1) == 1)
		{
			/* Check if ch is '\n' */
			if(ch == '\n')
				break;

			buffer[count++] = ch;				
		}
		buffer[count] = '\0';	
		/* Dynamically allocating memory for row of array of pointers */
		external_commands[i] = (char *) malloc((count+1) * sizeof(char)); 	
		strcpy(external_commands[i], buffer);	// Copying the buffer to array of pointers
	}

	close(fd);	
}

/* For getting the Command */
char *get_command(char *input_string)
{
	int i = 0;
	while(input_string[i] != ' ' && input_string[i] != '\0')
	{
		cmd[i] = input_string[i];	// Copy command to the cmd string
		i++;
	}
	cmd[i] = '\0';

	return cmd;						
}

/* For checking the Command type */
int check_command_type(char *command)
{
	for(int i = 0; builtins[i] != NULL; i++)
	{
		if(strcmp(builtins[i], command) == 0)
			return BUILTIN;
	}
	/* total 153 commands */
	for(int i = 0; i < 153; i++)
	{
		if(strcmp(external_commands[i], command) == 0)
		  return EXTERNAL;
	}
	/* If the command is neither internal nor external */
	return NO_COMMAND;
}

/* For executing the Internal commands by using system calls */
void execute_internal_commands(char *input_string)
{
	/* Command 1 -> 'exit' */
	if(strncmp(input_string, "exit", 4) == 0)
		exit(0);

	/* Command 2 -> 'pwd' */
	else if(strncmp(input_string, "pwd", 3) == 0)
	{
		char buffer[100];
		getcwd(buffer, sizeof(buffer));
		printf(ANSI_COLOR_BLUE"%s\n"ANSI_COLOR_RESET, buffer);		
	}
	/* Commmand 3 -> 'cd ' */
	else if(strncmp(input_string, "cd ", 3) == 0)
	{
		/* For changing the directory */
		if(chdir(input_string + 3) == -1)
			perror("cd");

		char buffer2[100];
		getcwd(buffer2, sizeof(buffer2));
		printf(ANSI_COLOR_BLUE"%s\n"ANSI_COLOR_RESET, buffer2);		
	}
	/* Command 4 -> 'echo $$' */
	else if(strcmp(input_string, "echo $$") == 0)
		printf("%d\n", getpid());
	/* Command 5 -> 'echo $? */
	else if(strcmp(input_string, "echo $?") == 0)
	{
		/* For the previous commands executed successfully or not */
		printf("%d\n", last_st);
	}
	/* command 6 -> 'echo $SHELL' */
	else if(strncmp(input_string, "echo $", 6) == 0)
	{
		char *ptr = getenv(input_string + 6);
		if(ptr != NULL)
			printf("%s\n", ptr);		// Print the path
	}
	/* Command 7 -> 'jobs' */
	else if(strcmp(input_string, "jobs") == 0)
	{
		printing_jobs(head);
	}
	/* Command 8 -> 'fg' */
	/* fg -> will block the terminal and executes the command which is stopped in foreground */
	else if(strcmp(input_string, "fg") == 0)
	{
		/* If there are no jobs in the foreground */
		if(head == NULL)
		{
			printf("bash: fg: current: No such job\n");
			return;
		}
		
		jobs *temp = head;
		jobs *prev = NULL;
		/* Moving to last job */
		while(temp->next != NULL)
		{
			prev = temp;
			temp = temp->next;
		}
		pid = temp->job_pid;			
		kill(pid, SIGCONT);				
		printf("%s\n", temp->job_cmd);	
		temp->state = 1;
		waitpid(temp->job_pid, &status, WUNTRACED);	 		
		pid = 0;						

		/* To remove the last node which is resumed again */
		if(prev == NULL)
			head = NULL;
		else
			prev->next = NULL;

		return;
	}
	/* Command 9 -> 'bg' */
	/* bg -> will resume/continue the stopped process in the background without blocking the terminal */
	else if(strcmp(input_string, "bg") == 0)
	{
		/* bg -> resumes only last stopped jobs */
		
		jobs *temp = head;
		jobs *prev = NULL;			
		
		while(temp != NULL)
		{
			if(temp->state == 0)
				prev = temp;

			temp = temp->next;		
		}
		
		if(prev == NULL)
		{
			printf("bash: bg: current: No such job\n");
			return;
		}
		
		kill(prev->job_pid, SIGCONT);
		prev->state = 1;		
		printf("[%d] %d %s &\n", prev->job_id, prev->job_pid, prev->job_cmd);
	}
}

/* For Executing the external commands */
void execute_external_commands(char *input_string)
{
	/* Array of pointers to store the input string */
	char *argv[20];
	// Convert the input string into 2D array
	int argc = 0;
	char *token = strtok(input_string, " ");
	
	while(token != NULL)
	{
		argv[argc++] = token;
		token = strtok(NULL, " ");
	}
	argv[argc] = NULL;
	/* Checking the number of times the pipes is present */
	int pipe_count = 0;
	for(int j = 0; j < argc; j++)
	{
		if(strcmp(argv[j], "|") == 0)
			pipe_count++;			
	}
	/* Checking pipe is there or not*/
	if(pipe_count == 0)
	{
		// If there is only 1 command  
		execvp(argv[0], argv);		// Replace old program with new program
		perror("execvp");
		exit(1);
	}
	/* If the pipe is present  */
	else
	{
		// Logic for the n pipe
		int *cmd_index = malloc(argc * sizeof(int));
		int index = 0, fd[2];       
		cmd_index[index++] = 0;
		/* Storing index into the cmd */
		for(int i = 0; i < argc; i++)
		{			
			if(strcmp(argv[i], "|") == 0)
			{
				argv[i] = NULL;
				cmd_index[index++] = i + 1;
			}
		}
		/* creating the pipe */
		for(int i = 0; i < index; i++)
		{
			/* If the command is n then we have to create n-1 pipes*/
			if(i != index-1)
				pipe(fd);   

			pid_t pid1 = fork();      
			if(pid1 > 0)
			{
				// parent
				if(i != index - 1)
				{
					close(fd[1]);
					dup2(fd[0], 0);
					close(fd[0]);
				}
				wait(NULL);       // Wait til child get terminated
			}
			else if(pid1 == 0)
			{
				//  child
				if(i != index - 1)
				{
					close(fd[0]);
					dup2(fd[1], 1);
					close(fd[1]);
				}

				execvp(argv[cmd_index[i]], argv + cmd_index[i]);    // Replace old program with new
			}
		}
		
		free(cmd_index);
	}
	exit(0);	
}

/* For handling the signal */
void signal_handler(int signum)
{	
	/* Checking which signal has triggered */
	if(signum == SIGINT)
	{
		/*  Parent */
		if(pid == 0)
		{			
			printf(ANSI_COLOR_GREEN"\n%s"ANSI_COLOR_RESET, prompt); 
			fflush(stdout);		
		}
		else
			kill(pid, SIGINT);	
	}
	else if(signum == SIGTSTP)
	{
		/* parent */
		if(pid == 0)
		{			
			printf(ANSI_COLOR_GREEN"\n%s"ANSI_COLOR_RESET, prompt);
			fflush(stdout);		
		}
		else
			kill(pid, SIGTSTP);	
	}
	
	else if(signum == SIGCHLD)
	{
		
		pid_t child;		
		while((child = waitpid(-1, &status, WNOHANG)) > 0)		
		{			
			jobs *temp = head, *prev = NULL;

			while(temp)
			{
				if(temp->job_pid == child)
				{			

					if(prev == NULL)
						head = temp->next;
					else
						prev->next = temp->next;	

					free(temp);						
					break;
				}

				prev = temp;					
				temp = temp->next;				
			}
		}
	}
}

/* For inserting the jobs at first */
void insert_atlast_jobs(pid_t pid, char *input_string)
{
    jobs *new = malloc(sizeof(jobs));			
	new->job_pid = pid;							
	new->job_id = job_id++;
	strcpy(new->job_cmd, input_string);			
	new->state = 0;								
	new->next = NULL;

	/* For inserting the job in the first node */
	if(head == NULL)
		head = new;								
	else
	{
		jobs *temp = head;
		while(temp->next != NULL)
			temp = temp->next;

		temp->next = new;						
	}
	/* For printing the job which is inserted */
	printf("\n[%d] %d stopped\t\t%s\n", new->job_id, new->job_pid, new->job_cmd);
}

/* Printing the jobs which are inserted */
void printing_jobs(struct job *head)
{
	/* Checking if the Linked list is empty */
	if(head == NULL)
		return;

	jobs *temp = head;							
	while(temp)
	{
		/* Print the jobs stored by their states(running or stopped)*/
		if(temp->state == 0)
			printf("[%d] stopped\t\t%s\n", temp->job_id, temp->job_cmd);
		else 
			printf("[%d] running\t\t%s\n", temp->job_id, temp->job_cmd);

		temp = temp->next;						
	}
}