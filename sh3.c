#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
//#define DEBUG

#define ARGUMAXNUM 10
#define TOKEN_MAX_SIZE 30
#define TOKEN_MAX_NUM 10
#define INSTRUCTION_MAX_SIZE (TOKEN_MAX_SIZE+1)*TOKEN_MAX_NUM



//Based on the sh2,the following code is added to imply the tunnel function.
#define COMMAND_MAX_NUM 20
typedef struct {
    int token_count;   //the tokenNum before 
    char* command_tokens[TOKEN_MAX_NUM];//The commandArray before
    int input_redirect_flag;                 //
    int output_redirect_flag;               //redirect flags
    int fd_stdout_bak;
    int fd_stdin_bak;
    char* source_string;    //This is string for this command
}command;
command* command_vector[COMMAND_MAX_NUM];
unsigned int command_count;
command* command_context=NULL;
int fd_read_bak;
int fd_write_bak;


//Global command array
//It is set in the split function
//char *command_tokens[TOKEN_MAX_NUM];
//unsigned int token_count;



//This function analyze the string input and exact commands for the string
//compared with the split function analyzing a command string and exacting tokens from a single command string
void split_command(char* input){
    command_count=0;
    memset(command_vector,0,sizeof(command*)*COMMAND_MAX_NUM);

    char* temp1=(char*)malloc(sizeof(char)*INSTRUCTION_MAX_SIZE);
    strcpy(temp1,input); //do not change the source string.strtok function will change the source string
    

    char* temp2;
    temp2=strtok(temp1,"|");
    
    command_vector[command_count]=(command*)malloc(sizeof(command));
    command_vector[command_count]->source_string=(char*)malloc(INSTRUCTION_MAX_SIZE);
    command_vector[command_count]->input_redirect_flag=0;
    command_vector[command_count]->output_redirect_flag=0;
    strcpy(command_vector[command_count++]->source_string,temp2);

    //split_tokens(command_vector[command_count++]);

    //Here is a trick: if every time you get a command string,you call the split_tokens function
    //After you back from the split_tokens function,you will not get the next command string properly
    //This is beacuse you called the strtok function in the split_tokens functino.

    while ((temp2= strtok(NULL, "|")) != NULL) {
        command_vector[command_count]=(command*)malloc(sizeof(command));
        command_vector[command_count]->source_string=(char*)malloc(INSTRUCTION_MAX_SIZE);
        strcpy(command_vector[command_count++]->source_string,temp2);
        //split_tokens(command_vector[command_count++]);
    }

    int i;
    for(i=0;i<command_count;i++)
        split_tokens(command_vector[i]);
}


void split_tokens(command *cmd)
{
    //init
    cmd->token_count = 0;
    memset(cmd->command_tokens, 0, sizeof(char*)*TOKEN_MAX_NUM);

    char *variCommand =
	(char *) malloc(INSTRUCTION_MAX_SIZE * sizeof(char));
    strcpy(variCommand, cmd->source_string);
    
    //to get the tokens
    char *token = strtok(variCommand, " ");
    cmd->command_tokens[cmd->token_count] = malloc(sizeof(char) * TOKEN_MAX_SIZE);
    strcpy(cmd->command_tokens[(cmd->token_count)++], token);
    

    while ((token = strtok(NULL, " ")) != NULL) {
	//printf("%s\n",token);
        cmd->command_tokens[cmd->token_count] = malloc(sizeof(char) * TOKEN_MAX_SIZE);
        strcpy(cmd->command_tokens[(cmd->token_count)++], token);
    }
}

void pre_process(int *exit_code)
{
    
    if (strcmp(command_context->command_tokens[0], "exit") == 0)
	    exit(0);
    else if (strcmp(command_context->command_tokens[0], "pwd") == 0) {
	        char cwd[100];
	        getcwd(cwd, 100);
	        printf("%s\n", cwd);
	        *exit_code = 1;
        } else if (strcmp(command_context->command_tokens[0], "cd") == 0) {
	        chdir(command_context->command_tokens[1]);
	            printf("%s\n", command_context->command_tokens[1]);
	            *exit_code = 1;
                }else {
	                return;
        }
}

//Check if the redirect exists
//If exists,redirect by the dup2 and remove the redirect command argumets

void redirect_pre_process(){
    

    //Search if the redirect argument exist
    //Only needs to check the last argument 
    
    char* filename;
    //not exists
    if(command_context->command_tokens[command_context->token_count-1][0] != '>'&&
    command_context->command_tokens[command_context->token_count-1][0] != '<')
        return ;
    else if(command_context->command_tokens[command_context->token_count-1][0] == '>'){
        //output redirect
        

        filename=(char*)malloc(sizeof(char)*TOKEN_MAX_SIZE);
        strcpy(filename,command_context->command_tokens[command_context->token_count-1]+1); 

        #ifdef DEBUG
            printf("command:%s output direct to %s\n",command_context->command_tokens[0],filename);
        #endif

        int fd;
        fd = open(filename,O_CREAT|O_RDWR|O_TRUNC,0666);
        command_context->fd_stdout_bak=dup(1);
        dup2(fd,1);
        close(fd);
        
        free(command_context->command_tokens[command_context->token_count-1]);
        command_context->command_tokens[command_context->token_count-1]=NULL;
        command_context->token_count--;
        
        command_context->output_redirect_flag=1;
        return ;
    }
    else{
        //input direct
        filename=(char*)malloc(sizeof(char)*TOKEN_MAX_SIZE);
        strcpy(filename,command_context->command_tokens[command_context->token_count-1]+1); 

        
        #ifdef DEBUG
            printf("command:%s input direct from %s\n",command_context->command_tokens[0],filename);
        #endif

        int fd;
        fd = open(filename,O_RDONLY,0666);
        command_context->fd_stdin_bak=dup(0);
        dup2(fd,0);
        close(fd);

        free(command_context->command_tokens[command_context->token_count-1]);
        command_context->command_tokens[command_context->token_count-1]=NULL;
        command_context->token_count--;

        command_context->input_redirect_flag=1;
        return ;
    }
}

void redirect_post_process(){
    if(command_context->input_redirect_flag){
        close(0);
        dup2(command_context->fd_stdin_bak,0);
        close(command_context->fd_stdin_bak);
    }
        

    if(command_context->output_redirect_flag){
        close(1);
        dup2(command_context->fd_stdout_bak,1);
        close(command_context->fd_stdout_bak);
    }

}
void set_command_context(command* context){
    command_context=context;
}
void fd_bak(){
    dup2(0,fd_read_bak);
    dup2(1,fd_write_bak);

}
void fd_recover(){
    dup2(fd_read_bak,0);
    dup2(fd_write_bak,1);
}


/*The exec_cmd function does the following things
    1.preprocess:if the code is user-define command,handle them specially,and return or exit
    2.if it's the system-defined command,fork
        2.1 in the child process.excute the command
        2.2 wait for the child process and return.
*/
void exec_cmd(){
        #ifdef DEBUG
        printf("Exec_cmd:to execute command %s\n",command_context->command_tokens[0]);
        #endif

        int exit_code = 0;
        pre_process(&exit_code);
        if (exit_code == 1)
	    return;

        pid_t pid1;
        pid1 = fork();

        if (pid1 == 0)
        {   
           
            //fprintf(fd_write_bak,"Command:%s\n",command_context->command_tokens[0]);
            /*
            if(strcmp("wc",command_context->command_tokens[0])==0){
                printf("In grand child process\n");
                char temp[32];
                scanf("%s",temp);
                printf("Test printf:%s\n",temp);
            }
            */
            int error;
    	    error=execvp(command_context->command_tokens[0], command_context->command_tokens);
            if(error<0)
            {
                printf("sh3:Unrecognized command:%s\n",command_context->command_tokens[0]);
                exit(0);
            }
        }
        else{
            wait(NULL);
            
            return ;
        }

	    

        
}
//exec command[0] command[1]...command[child_count-1]
int exec_pipe(int child_count){
    
    
    if(child_count==1){
        set_command_context(command_vector[child_count-1]);
        redirect_pre_process();
        exec_cmd();
        redirect_post_process();
        return ;
    }

    int fd[2];
    pipe(fd);

    pid_t pid;
    pid=fork();


    if(pid==0){
        dup2(fd[0],0);
        close(fd[0]);
        close(fd[1]);   
        //sleep(2);

        set_command_context(command_vector[child_count-1]);
        redirect_pre_process();
        exec_cmd();
        redirect_post_process();
        exit(0);
    }
    else{
        dup2(fd[1],1);
        close(fd[0]);
        close(fd[1]);
        
        exec_pipe(child_count-1);
        close(1);
        //Here is another trick if you do not close.The output data in the 
        //buffer will not be transfered to the child process.In this way
        //The child process will wait for the parent (tunnel) data
        //At the same time,parent process will also wait The whole projects are blocked
        
        wait(NULL);
    }
    return 0;

}
/*This function did following things:
    1.Parse the command by calling the split_command function
    2.judge the command count.
        2.1 if the command count is less than two(it is one)
            run the corresponding command
        2.2 if the command count is more than two
            run the pipe command function
*/
void mysys(char *command)
{
    split_command(command); 
    if(command_count<2){
        set_command_context(command_vector[0]);
        redirect_pre_process();
        exec_cmd();
        redirect_post_process();
    }
    else{
        fd_bak();

        int error;
        error=exec_pipe(command_count);
        if(error<0){
            printf("Tunnel command excution error\n");
            exit(0);
        }
        fd_recover();
    }
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
