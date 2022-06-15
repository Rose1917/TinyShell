#include <stdio.h>
//This code should run on the c1 standard

int main(int argc,char* argv[] ){
    
    //The program received n+1 arguments.(Here the n reperesents the number of arguments you want to send
    //The first one (argc[0] is your binary file.
    //Thus we only need to get the following arguments.
    int i;
	for(i=1;i<argc;i++)
        printf("%s ",argv[i]);
    printf("\n");
    return 0;
}
