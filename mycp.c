#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include<fcntl.h>
#include <string.h>
//This code should run on the c99 standard
#define BUFFERMAXSIZE 16

char* charArrayToString(char* charArrayToString,int count){
	charArrayToString[count]='\0';
	return charArrayToString;
}
int main(int argc,char* argv[] ){

    char* sourceFile=argv[1];
    int sourceFd=open(sourceFile,O_RDONLY);
    if(sourceFd<0)
    	return -1;
    
    char* desFile=argv[2];
    //By using the O_TRUNCH flag,you can overwrite  the exsiting file 
    int desFd=open(desFile,O_WRONLY|O_CREAT|O_TRUNC,0777);
    if(desFd<0)
    	return -1;
	
    int count,result;
    char buffer[BUFFERMAXSIZE];
	while((count=read(sourceFd,buffer,BUFFERMAXSIZE))!=0){
		write(desFd,buffer,count);
	}
    
    return 0;
}
