#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include<fcntl.h>
#include <string.h>
//This code should run on the c99 standard
#define BUFFERMAXSIZE 8192

int main(int argc,char* argv[] ){
    //int i;
    char* filePath=argv[1];
    printf("%s\n",filePath);
    
    int fd=open(filePath,O_RDONLY);
    if(fd<0)
    	return -1;
    
    char buffer[BUFFERMAXSIZE];
    int count=read(fd,buffer,BUFFERMAXSIZE);
    write(1,buffer,count);
    //todo here is a potential bug.
	//if the buffer size is less the file size  
    //
    
    return 0;
}
