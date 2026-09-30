#include "header.h"

/* Function definitions */

void scan_input(char *input_string)
{
	int type;
	char *command;

	/* To store external commands into the array of pointers */
	extract_external_commands(external_commands); 

	/* To extract the signal */
	signal(SIGINT, signal_handler);
	signal(SIGTSTP, signal_handler); 
	signal(SIGCHLD, signal_handler); 
	
    while(1)
    {
        printf(ANSI_COLOR_GREEN"%s"ANSI_COLOR_RESET, prompt);

		if(scanf("%[^\n]", input_string) == 0)
		{
			getchar();
			continue;
		}
		getchar();

		/* To remove trailing spaces from input_string */
		int len = strlen(input_string);		

		/* Check if input_string is ending with spaces */
		while(len >= 0 && isspace(input_string[len - 1]))	
		{
			input_string[len - 1] = '\0';	
			len--;		
		}

        if(strncmp(input_string, "PS1=", 4) == 0)
        {
            // To check if space is present in the string or not
            int i = 4, count = 0;

            while(input_string[i] != '\0')
            {
                if(input_string[i] == ' ')
                {
                    count = 1;
                }
                else if(count == 1)
                {
                    printf("%s: Command not found!\n", input_string + 4);
                    break;
                }
                i++;
            }
            if(input_string[i] == '\0')
            {
                /* To remove trailing zeroes */
                int j = strlen(input_string) - 1;

                while(j >=0 && isspace(input_string[j]))
                {
                    input_string[j] = '\0';
                    j--;
                }

                strcpy(prompt, input_string + 4);		
            }
            continue;
        }

		command = get_command(input_string);	
		
		type = check_command_type(command);

		if(type == BUILTIN)
		{			
			execute_internal_commands(input_string);
		}
		else if(type == EXTERNAL)
		{
			
			pid = fork();

			/* Child process */
			if(pid == 0)
			{				
				signal(SIGINT, SIG_DFL);		
				signal(SIGTSTP, SIG_DFL);

				execute_external_commands(input_string);
				exit(0);     // Exit from the child
			}
			/* Parent Process */
			else if(pid > 0)
			{
				waitpid(pid, &status, WUNTRACED);
				
				if(WIFEXITED(status))
					last_st = WEXITSTATUS(status);
				
				else if(WIFSIGNALED(status))
					last_st = 128 + WTERMSIG(status);
				
				else if(WIFSTOPPED(status))
				{
					last_st = 128 + WSTOPSIG(status);
					insert_atlast_jobs(pid, input_string);		
				}
				pid = 0;		// reset pid 
			}
		}
		else
		{			
			printf("%s : Command not found\n", command);
			last_st = 127;		// to print signal exit status
		}
    }
}