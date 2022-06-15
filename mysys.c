#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ARGUMAXNUM 10
#define TOKEN_MAX_SIZE 30
#define TOKEN_MAX_NUM 10
#define INSTRUCTION_MAX_SIZE (TOKEN_MAX_SIZE+1)*TOKEN_MAX_NUM
 
//Global command array
//It is set in the split function
char* commandArray[TOKEN_MAX_NUM];
unsigned int tokenNum;
 


void split(char* command){
	
	//init
	tokenNum=0;
    memset(commandArray,0,sizeof(commandArray));

	char* variCommand=(char*)malloc(INSTRUCTION_MAX_SIZE*sizeof(char));
	strcpy(variCommand,command);
	//printf("%s\n",variCommand);
	
	//to get the tokens
	char* token=strtok(variCommand," ");
    commandArray[tokenNum]=malloc(sizeof(char)*TOKEN_MAX_SIZE);
	strcpy(commandArray[tokenNum++],token);
		
	while((token=strtok(NULL," "))!=NULL){
		//printf("%s\n",token);
        commandArray[tokenNum]=malloc(sizeof(char)*TOKEN_MAX_SIZE);
		strcpy(commandArray[tokenNum++],token);
	}
}

void mysys(char *command)
{	
    split(command);
    
    pid_t pid;
    pid=fork();
    
    if(pid==0)
        execvp(commandArray[0],commandArray);
    else
        wait(NULL);
    	
}

int main()
{
    printf("--------------------------------------------------\n");
    mysys("echo HELLO WORLD");
    printf("--------------------------------------------------\n");
    mysys("ls /");
    printf("--------------------------------------------------\n");
    return 0;
}