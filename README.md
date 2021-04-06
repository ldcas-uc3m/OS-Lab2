# Operating Systems - Lab2: Minishell
By team 89-09: Ignacio Arnaiz Tierraseca, Luis Daniel Casais Mezquida & Juan Del Pino Vega

## Lab Statement

This lab allows the student to familiarize with the services for process management that are provided by POSIX. Moreover, one of the objectives is to understand how a Shell works in UNIX/Linux. In summary, a shell allows the user to communicate with the kernel of the Operating System using simple or chained commands.<br/>
For the management of processes, you will use the POSIX system calls such as fork, wait, exit. For process communication pipe, dup, close and signal system calls.<br/>
The student must design and implement, in C language and over the UNIX/Linux Operating System, a program that acts like a shell.<br/>

## Parser
For the development of this lab, a parser is given to the student. This parser can read the commands introduced by the user. The student should only work to create a command interpreter. The syntax used by the parser is the following:
* A **space** is a space or a tab.
* A **separator** is a character with a special meaning (| , < , > , & ), a new line or the end of
file (CTRL-C).
* A **string** is any sequence of characters delimited by a space or a separator.
* A **command** is a sequence of strings separated by spaces. The first string is the name of the command to be executed. The remaining strings are the arguments of the commands. For instance, in the command `$ ls –l`, 'ls' is the command and '–l' is the argument. The name of the command is to be passed as the argument 0 to the `execvp` command. Each command must execute as an immediate child of the minishell, spawned by `fork` command. The value of a command is its termination status, returned by exit function from the child and received by wait function in the father. If the execution fails, the error must be notified by the shell to the user through the standard error.
* A **command sequence** is a list of commands separated by ‘|’. The standard output of each command is connected through an unnamed pipe to the standard input of the following command. A shell typically waits for the termination of a sequence of commands before requesting the next input line. The value of a sequence is the value returned by the last command in the sequence.

**Redirection.** The input or the output of a command sequence can be redirected by the following syntax added at the end of the sequence:
* `< <file>` → Use _file_ as the standard input after opening it for reading.
* `> <file>` → Use _file_ as the standard output. If the file does not exist, it is created. If the
file exists, it is truncated.
* `!> <file>` → Use _file_ as the standard error. If the file does not exist, it is created. If thefile
exists, it is truncated.

In case of a redirection error, the execution of the line must be suspended, and the user should be notified using the standard error.<br/>
<br/>
**Background (&).** A command or a sequence of commands finishing in ‘&’ must execute in background, i.e., the minishell is not blocked waiting for its completion. The minishell must execute the command without waiting and print on the screen the identifier of the child process in the following format: `[ %d] \n`<br/>
<br/>
The **prompt** is a message indicating that the shell is ready to accept commands from the user. By default, it's "MSH>>".

### Command line parsing
In order to obtain the parsed command line introduced by the user you can use the function `read_command()`:<br/>
`int read_command(char ***argvv, char **filev, int *in_background);`<br/>
The function returns 0 if the user types Control-C (EOF) and -1 in case of error. If successful, the function returns the number of commands (The input format must be respected for the correct functioning of the parser: `[<command> <args> [ | <command> <args>]* [< input_file ] [ > output_file] [!> outer_file] [&]`). For example:
* For `ls -l` returns 1.
* For `ls -l | sort` returns 2.

The argument 'argvv' contains the commands entered by the user.<br/>
The argument 'filev' contains the files employed in redirections, if any:
* filev[0] contains the file name to be used in standard input redirection and zero if there
is no such redirection.
* filev[1] contains the file name to be used in standard output redirection and zero if there
is no such redirection.
* filev[2] contains the file name to be used in standard error redirection and zero if there
is no such redirection.

The argument 'in_background' is 1 if the command or command sequence are to be executed in background.<br/>
<br/>
In the file msh.c the function `read_command()` is invoked, and the next loop executed:<br/>
`if (command_counter > 0) {`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;`if (command_counter > MAX_COMMANDS)`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;`else {`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`// Print command`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`print_command(argvv, filev, in_background);`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;`}`<br/>
`}`<br/>
In this code appears the function `print_command()`, which is in charge of printing the stored information captured from the command line, and its code is:<br/>
`for (int i = 0; i < num_commands; i++){`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;`for (int j = 0; argvv[i][j] != NULL; j++){`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`printf("%s\n", argvv[i][j]);`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;`}`<br/>
`}`<br/>
`printf("Redir IN: %s\n", filev[0]);`<br/>
`printf("Redir OUT: %s\n", filev[1]);`<br/>
`printf("Redir ERR: %s\n", filev[2]);`<br/>
`if (in_background == 0){`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;`printf("No Bg\n");`<br/>
`}`<br/>
`else{`<br/>
&nbsp;&nbsp;&nbsp;&nbsp;`printf("Bg\n");`<br/>
`}`<br/>

## Development
To develop the minishell we recommend following the next steps, so that it is implemented incrementally. Each step will add a new functionality.
* Execution of simple commands such as `$ ls -l`, `$ who`, etc.
* Execution of simple commands in background.
* Execution of simple commands with redirection (input, output and error).
* Execution of sequences of commands connected through pipes. The number of commands is limited to 3, e.g. `$ ls –l | sort | wc`. The implementation of a version that accepts an arbitrary number of commands will be considered for additional marks.
* Execution of simple commands and sequence of commands with redirections (input, output and error), in background.
* Execution of internal commands. An internal command is a command, which maps directly to a system call or a command internally implemented inside the shell. It must be implemented and executed inside the minishell (in the parent process). If it finds any error (not enough arguments or other type of error), a notification will appear (using standard error output). The internal commands to be implemented are:

### mycalc
It works as a simple calculator in the command line. It takes a simple equation, following the format: `<operand> <operator> <operand>`, where operand is an integer number and operator can be sum (add) or module (mod) in which the integer quotient as well as the remainder must be computed.<br/>
For the sum operation, the values will be stored in an environment variable called “Acc”. This variable will start with the value 0, and later the results of the sums will be added (not the results from the module operation). For the module operation, the results will be shown in the following way: `<dividend> = <divisor> * <quotient> + <remainder>`.<br/>
If the operation is successful, the command will show (using the standard error output), the result of solving the computation preceded by the label `"[OK]"`. In the case of the sum operation, the accumulated value must be also shown.<br/>
If the operator does not correspond with the ones previously described, or not all the terms of the equation were introduced, the following message will be shown (in the standard output): `“[ERROR] The structure of the command is <operand1> <add/mod> <operand2>”`.

### mycp
It works similarly to the command `cp`. It takes a source file and a destination and tries to copy the source file to the specified destination, having the same name as the source file.<br/>
If any of the source/destination cannot be opened, the following message will be printed (in the standard output): `“[ERROR] Error opening original file”` or `“[ERROR] Error opening the copied file”`, depending on the error.<br/>
If the function is not called with two parameters, it will show (using the standardoutput) the following message: `“[ERROR] The structure of the command is mycp <original file> <copied file>”`.<br/>
If the copy is successful it will show (using the standard output): `“[OK] Copy has been successful between <original> and <copied>”`.<br/>

## Files
* **Makefile**: Input file for the make tool. It serves to recompile automatically only the source code that is modified.
* **libparser.so**: Shared library with the parser functions. It allows to recognize and parser the input commands.
* **probador_ssoo_p2.sh**: Shell script that carries out an auto-correction of the student’s code. To run the script the students must change the permissions:<br/>
`$ chmod +x probador_ssoo_p2.sh`,<br/>
and run it:<br/>
`$ ./corrector_ssoo_p2.sh <zip file>`.<br/>
At the end the students will see printed on screen the estimated grade of their code.
* **msh.c**: C source file which shows an example on how to use the parser. The current version simply implements an echo (print) of the commands given to msh as arguments, which are syntactically correct. This functionality must be removed and substitutes by the lines of code that implement the lab.

## Compilation
In order to compile and run the code, it’s mandatory to link the shared library libparser.so, which is in charge of providing to the programmer (students) a useful function that extracts the list of commands given as input to the minishell. These commands are saved in a global structure that can be manipulated by the programmer.
1. Create a directory in your preferred folder and remember the path.
2. Unzip the initial code in path.
3. Compile or clean:
    1. With the command `$ make` you can compile your code.
    2. With the command `$ make clean` the generated files will be deleted.
4. Run the minishell. If there is an error because the library is not found:<br/>
`$ export LD_LIBRARY_PATH=/home/<username>/<path>:$LD_LIBRARY_PATH`, and run the minishell again.
