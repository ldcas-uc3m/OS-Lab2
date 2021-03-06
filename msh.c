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

// $ export LD_LIBRARY_PATH=/home/ldcas/Documents/GitHub/OS-Lab2/msh.c:$LD_LIBRARY_PATH

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

void mycalc(char ***argvv){

    // TODO: fix accum

	/* Check if the command is mycalc */
	if (strcmp(argvv[0][0], "mycalc") == 0){

		/* Check if the input command is composed by operand1 add/mod and operand 2 */
		if (argvv[0][1] != NULL && argvv[0][2] != NULL && argvv[0][3] != NULL){
			
			/* Get the operands */
			int op1 = atoi(argvv[0][1]);
			int op2 = atoi(argvv[0][3]);
			
			/* If add define the accumulator and show the result in the standard error output */
			if (strcmp(argvv[0][2],"add")==0){
				int accum = accum + op1 + op2 ;
                int add = op1 + op2;
				char buf_add[50];
				sprintf(buf_add, "[OK] %d + %d = %d Acc %d\n", op1, op2, add, accum);
				
				/* Write in standard output error and if there is an error show the error */
				if (write(2, buf_add, strlen(buf_add)) < strlen(buf_add)){
					perror("Error in write\n");
				}
			}
			
			/* If mod calculate the remainider the quotient and show the result in the standard error output */
			else if (strcmp(argvv[0][2],"mod")==0){
				int rem = op1 % op2;
				int quo = op1 / op2;
				char buf_mod[50];
				sprintf(buf_mod, "[OK] %d %% %d = %d * %d + %d\n", op1, op2, op2, quo, rem);
				
				/* Write in standard output error and if there is an error show the error */
				if (write(2, buf_mod, strlen(buf_mod)) < strlen(buf_mod)){
					perror("Error in write\n");
				}
			}
			
			/* If the input does not follow the the structure show the error in the standard output */
			else{
				/* Write in standard output and if there is an error show the error */
				if (write(1,"[ERROR] The structure of the command is <operand 1> <add/mod> <operand 2>\n", strlen("[ERROR] The structure of the command is <operand 1> <add/mod> <operand 2>\n")) < strlen("[ERROR] The structure of the command is <operand 1> <add/mod> <operand 2>\n")){
					perror("Error in write\n");
				}
			}
		} 
		
		/* If the input does not follow the the structure show the error in the standard output */
		else{
			/* Write in standard output and if there is an error show the error */
			if (write(1,"[ERROR] The structure of the command is <operand 1> <add/mod> <operand 2>\n", strlen("[ERROR] The structure of the command is <operand 1> <add/mod> <operand 2>\n")) < strlen("[ERROR] The structure of the command is <operand 1> <add/mod> <operand 2>\n")){
				perror("Error in write\n");
			}
		}
	}
    return;           
}

void mycp(char ***argvv){
    struct stat fileOrg; // data from the origin file
        
    char* p1  = argvv[0][1]; // origin file
    char* p2  = argvv[0][2]; // destination file

    if (p1 == NULL || p2 == NULL){
        write(1,"[ERROR] The structure of the comand is mycpy <original file> <copied file>\n", strlen("[ERROR] The structure of the comand is mycpy <original file> <copied file>\n"));
    }
    else{
        int existOrg = stat(p1, &fileOrg); // check if origin file exists
                                    
        if (existOrg==-1){
            write(1,"[ERROR] Error opening Original file: No such file or directory\n", strlen("[ERROR] Error opening Original file: No such file or directory\n"));
        }
        else{
            char buff[50];
            sprintf(buff,"[OK] Copy has been successfull between %s and %s\n",p1,p2);
            write(1,buff, strlen(buff));
            execvp("cp", argvv[0]); 
        }
    }
    return;
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


	while (1){
		int status = 0;
	    int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		/* Prompt */ 
		write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

		/* Get command */
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

        int curr_command = -1; // identifies which child is in charge of which command

	    if (command_counter > 0) {
            if (command_counter > MAX_COMMANDS){
                printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
            } else {
                /* PIPES */
                /* setup pipes */
                int pipe0[2], pipe1[2];
                pipe(pipe0);
                pipe(pipe1);

                /* create subprocesses */

                int pid = 1;
                for (int i = 0; i < command_counter; i++){
                    if (pid != 0){ // only father can fork
                        if ((pid = fork()) == 0){ /* child */
                            curr_command = i;
                        }
                    } else break;
                }

                if (pid == -1){ /* error */
                    perror("Error in fork");
                    return -1;
                } else if (pid != 0){ /* parent process */

                    /* close pipes */
                    close(pipe0[0,1]);
                    close(pipe1[0,1]);

                    /* BACKGROUND */
                           
                    if (in_background != 1){
                        while (wait(&status) != pid){ // wait for children to finish
                            if (status != 0){
                                perror("Error executing the child");
                            } 
                        }
                    }
                } else { /* child process */
                           
                    /* REDIRECTION */
                    
                    if ((curr_command == 0) && (filev[0][0] != '0')){
                        /* redirect from input, file[0] as stdin */
                        close(STDIN_FILENO); // free file desc. 0
                        int fd = open(filev[0], O_RDONLY); // fd is now 0
                    } else if ((curr_command == command_counter - 1) && (filev[1][0] != '0')){
                        /* redirect to output, file[1] as stdout */
                        close(STDOUT_FILENO);
                        int fd = open(filev[1], O_CREAT | O_RDWR, S_IRWXU);                          
                    }
                    if (filev[2][0] != '0'){
                        /* redirect error, file[1] as stderr */
                        close(STDERR_FILENO);
                        int fd = open(filev[2], O_CREAT | O_RDWR, S_IRWXU);                          
                    }

                    /* PIPES */

                    if (command_counter > 1){
                        if (curr_command == 0){ /* first command - pipe0 out */
                            close(pipe1[0,1]);
                            close(pipe0[0]);
                            dup2(pipe0[1], STDOUT_FILENO); // stdout is now pipe write
                            close(pipe0[1]);
                        } else if (curr_command == command_counter - 1){ /* last command - pipe in */
                            if ((curr_command % 2) != 0){ // odd - reads from pipe0
                                close(pipe1[0,1]);
                                close(pipe0[1]);
                                dup2(pipe0[0], STDIN_FILENO); // stdin is now pipe read
                                close(pipe0[0]);
                            } else{ /* even - pipe1 in */
                                close(pipe0[0,1]);
                                close(pipe1[1]);
                                dup2(pipe1[0], STDIN_FILENO);
                                close(pipe1[0]);
                            }
                        } else if ((curr_command % 2) != 0){ /* intermediate command, odd - pipe in/out */
                            close(pipe0[1]);
                            close(pipe1[0]);
                            dup2(pipe0[0], STDIN_FILENO);
                            dup2(pipe1[1], STDOUT_FILENO);
                        } else{ /* intermediate command, even - pipe in/out */
                            close(pipe1[1]);
                            close(pipe0[0]);
                            dup2(pipe1[0], STDIN_FILENO); // as it's even, we have to alternate to complete the loop
                            close(pipe1[0]);
                            dup2(pipe0[1], STDOUT_FILENO);
                            close(pipe0[1]);
                        }
                        /* close pipes *
                        close(pipe0[0,1]);
                        close(pipe1[0,1]);*/
                    }
                    /* execute current command */

                    /* INTERNAL COMMANDS */
                         
                    if (strcmp(argvv[curr_command][0],"mycp") == 0){
                        /* execute mycpy */
                        mycp(argvv);
                        exit(0);
                    } else if (strcmp(argvv[curr_command][0], "mycalc") == 0){
                        /* execute mycalc */
                        mycalc(argvv);
                        exit(0);
                    } else{
                        /* COMMAND EXECUTION */

                        getCompleteCommand(argvv, curr_command);
                        execvp(argvv[curr_command][0], argvv[curr_command]); //execute the comand
                        perror("Error in execvp\n");
                        exit(0);
                    }
                }
            }
        }
    }
    return 0;
}
