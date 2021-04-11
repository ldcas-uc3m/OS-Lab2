//  MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMANDS 8


// ficheros por si hay redirección
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];

void siginthandler(int param)
{
	printf("****  Saliendo del MSH **** \n");
	//signal(SIGINT, siginthandler);
        exit(0);
}

/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
    //reset first
    for(int j = 0; j < 8; j++)
        argv_execvp[j] = NULL;

    int i = 0;
    for ( i = 0; argvv[num_command][i] != NULL; i++)
        argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main sheell  Loop  
 */
int main(int argc, char* argv[])
{
    /**** Do not delete this code.****/
    int end = 0; 
    int executed_cmd_lines = -1;
    char *cmd_line = NULL;
    char *cmd_lines[10];

    if (!isatty(STDIN_FILENO)) {
        cmd_line = (char*)malloc(100);
        while (scanf(" %[^\n]", cmd_line) != EOF){
            if(strlen(cmd_line) <= 0) return 0;
            cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
            strcpy(cmd_lines[end], cmd_line);
            end++;
            fflush (stdin);
            fflush(stdout);
        }
    }

    /*********************************/

    char ***argvv = NULL;
    int num_commands;


	while (1) 
	{
		int status = 0;
	        int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		// Prompt 
		write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

		// Get command
                //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
                executed_cmd_lines++;
                if( end != 0 && executed_cmd_lines < end) {
                    command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
                }else if( end != 0 && executed_cmd_lines == end)
                    return 0;
                else
                    command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
                //************************************************************************************************


              /************************ STUDENTS CODE ********************************/
	       if (command_counter > 0) {
                if (command_counter > MAX_COMMANDS)
                    printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
                else {
            	    // Print command
		            //print_command(argvv, filev, in_background);
      
                    char*  pCmd = NULL;
                    char*  p1;
                    char*  p2;
                    struct stat fileOrg;
                    int    existOrg;
                    int    pid  = fork();

                    switch (pid){
                    
                    case -1:
                        /* error */
                        perror("Error in fork");
                        return -1;
                        break;

                    case 0:
                        /* child process */

                        /* execvp(argvv[0][0], argvv[0]); //execute the comand */
                       
                        pCmd = argvv[0][0];
                        //printf("CMD: %s\n",pCmd);
                        
                        if ( strcmp(pCmd,"mycp")==0 )
                           { /* execute mycpy */

                             p1  = argvv[0][1];
                             p2  = argvv[0][2];        
                             
                             //printf("P1:  %s\n",p1);
                             //printf("P2:  %s\n",p2);

                             if ( p1==NULL || p2==NULL)
                                {
                                  printf("The structure of the comand is mycpy <original file> <copied file>\n");
                                }
                             else
                                {
                                  existOrg = stat(p1,&fileOrg);
                                  
                                  if (existOrg==-1)
                                     {
                                       printf("Error opening Original file: No such file or directory\n");
                                     }
                                  else
                                     {
                                      printf("Command executed\n");
                                      execvp("cp", argvv[0]); 
                                     }
                                }
                          }

                       else if ( strcmp(pCmd,"mycalc")==0 )
                          { /* execute mycalc */
                            printf("MI mycalc\n");
                          }


                        else
                           { /* execute the comand */
                             execvp(argvv[0][0], argvv[0]); 
                           }

                          exit(0);
                          break;

                    default:
                        /* parent */
                        if (argvv[1][0] != "&"){
                            while (wait(&status) != pid){
                                if (status != 0){
                                    perror("Error executing the child");
                                }
                            }
                        }
                        break;
                    }
                }
              }
        }
	return 0;
}
