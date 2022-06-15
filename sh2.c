#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define ARGUMAXNUM 10
#define TOKEN_MAX_SIZE 30
#define TOKEN_MAX_NUM 10
#define INSTRUCTION_MAX_SIZE (TOKEN_MAX_SIZE+1)*TOKEN_MAX_NUM

//Global command array
//It is set in the split function
char *commandArray[TOKEN_MAX_NUM];
unsigned int tokenNum;
int fd_stdout_bak;


void split(char *command)
{

    //init
    tokenNum = 0;
    memset(commandArray, 0, sizeof(commandArray));

    char *variCommand =
	(char *) malloc(INSTRUCTION_MAX_SIZE * sizeof(char));
    strcpy(variCommand, command);
    //printf("%s\n",variCommand);

    //to get the tokens
    char *token = strtok(variCommand, " ");
    commandArray[tokenNum] = malloc(sizeof(char) * TOKEN_MAX_SIZE);
    strcpy(commandArray[tokenNum++], token);

    while ((token = strtok(NULL, " ")) != NULL) {
	//printf("%s\n",token);
	commandArray[tokenNum] = malloc(sizeof(char) * TOKEN_MAX_SIZE);
	strcpy(commandArray[tokenNum++], token);
    }
}

void pre_process(int *exit_code)
{

    if (strcmp(commandArray[0], "exit") == 0)
	    exit(0);
    else if (strcmp(commandArray[0], "pwd") == 0) {
	        char cwd[100];
	        getcwd(cwd, 100);
	        printf("%s\n", cwd);
	        *exit_code = 1;
        } else if (strcmp(commandArray[0], "cd") == 0) {
	        chdir(commandArray[1]);
	            printf("%s\n", commandArray[1]);
	            *exit_code = 1;
                }else {
	                return;
        }
}

//Check if the redirect exists
//If exists,redirect by the dup2 and remove the redirect command argumets
void redirect_process(){

    //Search if the redirect argument exist
    //Only needs to check the last argument 
    
    char* filename;
    //not exists
    if(commandArray[tokenNum-1][0] != '>')
        return ;
    else{
        //exists
        
        filename=(char*)malloc(sizeof(char)*TOKEN_MAX_SIZE);
        strcpy(filename,commandArray[tokenNum-1]+1); 
        printf("Found the redirect command,the filename is %s\n",filename);


        int fd;
        fd = open(filename,O_CREAT|O_RDWR,0666);
        fd_stdout_bak=dup(1);
        dup2(fd,1);
        close(fd);
        
        free(commandArray[tokenNum-1]);
        commandArray[tokenNum-1]=NULL;
        tokenNum--;

        return ;
    }
}

void redirect_post_process(){
    dup2(fd_stdout_bak,1);
}
void mysys(char *command)
{
    int exit_code = 0;
    split(command);
    redirect_process();
    pre_process(&exit_code);
    if (exit_code == 1)
	return;


    pid_t pid;
    pid = fork();

    if (pid == 0)
	execvp(commandArray[0], commandArray);
    else
	wait(NULL);

    redirect_post_process();

}

int main()
{
    while (1) {
	    printf(">");
	    char line[80];
	    fgets(line, 80, stdin);
	    line[strlen(line) - 1] = '\0';
	    mysys(line);
    }
    return 0;
}
