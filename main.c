
#include "header.h"

/* Initialize the prompt*/
char prompt[25] = "minishell$ ";        

int main()
{
    system("clear");               
    char input_string[25];
    scan_input(input_string);      
    return 0;
}