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

// export LD_LIBRARY_PATH=/home/ldcas/Documents/GitHub/OS-Lab2/msh.c:$LD_LIBRARY_PATH

int accum = 0; // accumulation of mycalc

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
	/* Check if the command is mycalc */
	if (strcmp(argvv[0][0], "mycalc") == 0){

        int accum = 0;


		/* Check if the input command is composed by operand1 add/mod and operand 2 */
		if (argvv[0][1] != NULL && argvv[0][2] != NULL && argvv[0][3] != NULL){
			
			/* Get the operands */
			int op1 = atoi(argvv[0][1]);
			int op2 = atoi(argvv[0][3]);
			
			/* If add define the accumulator and show the result in the standard error output */
			if (strcmp(argvv[0][2],"add")==0){
                /*char buf_accum[50];
                sprintf(buf_accum, "%d", accum);
                const char *env = buf_accum;*/
               	int add = op1 + op2;
                accum += add;
				char buf_add[50];
                //setenv("Accumulator", env, 1);
				sprintf(buf_add, "[OK] %d + %d = %d; Acc %d\n", op1, op2, add, accum);
				
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
            sprintf(buff,"[OK] Copy has been successful between %s and %s\n",p1,p2);
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
                    if (filev[2][0] != '0'){
                        /* file[2] as stderr */
                        close(STDERR_FILENO);
                        int fd = open(filev[2], O_CREAT | O_WRONLY, S_IRWXU);                          
                    }

                    if (command_counter == 2){ // two commands, one pipe
                        /* create pipe */
                        int pipefd[2];
                        if (pipe(pipefd) < 0){
                            perror("Error in pipe");
                            exit(-1);
                        }

                        int pid = fork();
                        switch (pid){
                            case -1: /* error */
                                perror("Error in fork");
                                exit(-1);

                            case 0: /* children -> cmd 0*/
                                /* REDIRECTION */
                                if (filev[0][0] != '0'){ 
                                    /* redirection from filev[0] -> filev[0] as stdin */
                                    close(STDIN_FILENO); // free file desc. 0
                                    int fd = open(filev[0], O_RDONLY); // fd is now 0
                                }

                                close(pipefd[0]); // close read pipe
                                dup2(pipefd[1], STDOUT_FILENO); // write to write pipe
                                close(pipefd[1]); // close write pipe

                                /* execute the command */

                                /* INTERNAL COMMANDS */
                                if (strcmp(argvv[0][0],"mycp") == 0){
                                    /* execute mycpy */
                                    mycp(argvv);
                                    exit(0);
                                }

                                else if (strcmp(argvv[0][0], "mycalc") == 0){
                                    /* execute mycalc */
                                    mycalc(argvv);
                                    exit(0);
                                } else{
                                    execvp(argvv[0][0], argvv[0]);
                                    exit(0);
                                }
                                break;

                            default: /* parent -> cmd 1*/
                                if (filev[1][0] != '0'){
                                    /* redirection to filev[1] -> filev[1] as stdout */
                                    close(STDOUT_FILENO);
                                    int fd = open(filev[1], O_CREAT | O_WRONLY, S_IRWXU);                          
                                }
                                wait(NULL); // wait for child to die

                                close(pipefd[1]);
                                dup2(pipefd[0], STDIN_FILENO);
                                close(pipefd[0]);

                                /* execute the command */

                                /* INTERNAL COMMANDS */
                                if (strcmp(argvv[1][0],"mycp") == 0){
                                    /* execute mycpy */
                                    mycp(argvv);
                                    exit(0);
                                } else if (strcmp(argvv[1][0], "mycalc") == 0){
                                    /* execute mycalc */
                                    mycalc(argvv);
                                    exit(0);
                                } else{
                                    execvp(argvv[1][0], argvv[1]);
                                    exit(0);
                                }
                                break;
                        }

                    }

                    if (command_counter == 3){ // 3 commands, 2 pipes
                        int pid;
                        int pfd1[2];
                        int pfd2[2];
                        int i;

                        /* pipe creation */
                        if (pipe(pfd1) == -1){
                            perror("Error in pipe");
                            exit(-1);
                        }

                        if (pipe(pfd2) == -1){
                            perror("Error in pipe");
                            exit(-1);
                        }

                        for (i = 0; i < 3; i++){
                            pid = fork();
                            switch (pid){
                                case -1:
                                    /* error */
                                    perror("Error in fork");
                                    return -1;

                                case 0: /* children execute cmd 0 & 1 */
                                    if (i == 0){
                                        /* redirection */
                                        if (filev[0][0] != '0'){
                                            /* file[0] as stdin */
                                            close(STDIN_FILENO); // free file desc. 0
                                            int fd = open(filev[0], O_RDONLY); // fd is now 0
                                        }

                                        close(pfd1[0]);
                                        dup2(pfd1[1], STDOUT_FILENO);
                                        /* close pipes */
                                        close(pfd1[1]);
                                        close(pfd2[0,1]);
                                        /* INTERNAL COMMANDS */
                                        if (strcmp(argvv[0][0],"mycp") == 0){
                                            /* execute mycpy */
                                            mycp(argvv);
                                            exit(0);
                                        } else if (strcmp(argvv[0][0], "mycalc") == 0){
                                            /* execute mycalc */
                                            mycalc(argvv);
                                            exit(0);
                                        } else{
                                            execvp(argvv[0][0], argvv[0]);
                                            exit(0);
                                        }
                                        break;
                                    }

                                    if (i == 1){
                                        close(pfd1[1]);
                                        close(STDIN_FILENO);
                                        dup(pfd1[0]);
                                        close(pfd1[0,1]);

                                        close(pfd2[0]);
                                        close(STDOUT_FILENO);
                                        dup(pfd2[1]);
                                        close(pfd2[0,1]);
                                        /* INTERNAL COMMANDS */
                                        if (strcmp(argvv[1][0],"mycp") == 0){
                                            /* execute mycpy */
                                            mycp(argvv);
                                            exit(0);
                                        } else if (strcmp(argvv[1][0], "mycalc") == 0){
                                            /* execute mycalc */
                                            mycalc(argvv);
                                            exit(0);
                                        } else{
                                            execvp(argvv[1][0], argvv[1]);
                                            exit(0);
                                        }
                                        break;
                                    }
                                    break;


                                default: /* children execute cmd 2 */
                                    if (i == 2){
                                        if (filev[1][0] != '0'){
                                            /* file[1] as stdout */
                                            close(STDOUT_FILENO);
                                            int fd = open(filev[1], O_CREAT | O_WRONLY, S_IRWXU);                          
                                        }

                                        close(pfd2[1]);
                                        close(STDIN_FILENO);
                                        dup(pfd2[0]);
                                        close(pfd1[0,1]);
                                        close(pfd2[0,1]);
                                        /* INTERNAL COMMANDS */
                                        if (strcmp(argvv[2][0],"mycp") == 0){
                                            /* execute mycpy */
                                            mycp(argvv);
                                            exit(0);
                                        } else if (strcmp(argvv[2][0], "mycalc") == 0){
                                            /* execute mycalc */
                                            mycalc(argvv);
                                            exit(0);
                                        } else{
                                            execvp(argvv[2][0], argvv[2]);
                                            exit(0);
                                        }
                                        break;
                                    }
                            }
                        }
                    }

                    if (command_counter == 1){
                        int pid  = fork();

                        switch (pid){
                            case -1:
                                /* error */
                                perror("Error in fork");
                                return -1;

                            case 0: /* child process */

                                /* REDIRECTION */

                                if (filev[0][0] != '0'){
                                    /* file[0] as stdin */
                                    close(STDIN_FILENO); // free file desc. 0
                                    int fd = open(filev[0], O_RDONLY); // fd is now 0
                                }

                                if (filev[1][0] != '0'){
                                    /* file[1] as stdout */
                                    close(STDOUT_FILENO);
                                    int fd = open(filev[1], O_CREAT | O_WRONLY, S_IRWXU);                          
                                }

                                if (filev[2][0] != '0'){
                                    /* file[1] as stderr */
                                    close(STDERR_FILENO);
                                    int fd = open(filev[2], O_CREAT | O_WRONLY, S_IRWXU);                          
                                }

                                /* INTERNAL COMMANDS */
                            
                                char* pCmd = argvv[0][0]; // internal command is first command
                                
                                if (strcmp(pCmd,"mycp") == 0){
                                    /* execute mycpy */
                                    mycp(argvv);
                                    exit(0);
                                }

                                else if (strcmp(pCmd, "mycalc") == 0){
                                    /* execute mycalc */
                                    mycalc(argvv);
                                    exit(0);
                                }

                                else{
                                /* COMMAND EXECUTION */
                                    getCompleteCommand(argvv, command_counter);
                                    execvp(argvv[0][0], argvv[0]); //execute the comand
                                    exit(0);
                                    break;
                                }

                            default:
                                /* parent process */
                                /* BACKGROUND */
                                if (in_background != 1){
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
    }
	return 0;
}
