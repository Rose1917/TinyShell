#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define precision_level 200
#define cpu_num 2
#define single_cpu_precision (precision_level/cpu_num)

typedef struct{
    int start;
    int end;
} param;

//[start,end)
void* compute(void * p){
    int i;
    int positive_flag=1;
    param* local_param=(param*)p;

    float* local_sum=malloc(sizeof(float));
    float local_temp=0.0;
    *local_sum=0.0;
    for(i=local_param->start;i<local_param->end;i++){
        local_temp=1.0/(2*i+1.0)*positive_flag;
        *local_sum+=local_temp;
        positive_flag=-positive_flag;    
    }

    return (void*)local_sum;
}
/*
    Here is a trick:
        You can not return a local address
        Cuz the memory will lose after this function return.
        But malloc will apply memory from head instead of function stack
        
*/

int main(){

    pthread_t auxiliary_tid;
    param* auxiliary_param=malloc(sizeof(param));
    if(auxiliary_param==NULL){
        printf("Malloc memory error!\n");
        exit(0);
    }

    auxiliary_param->start=single_cpu_precision;
    auxiliary_param->end=precision_level;
    pthread_create(&auxiliary_tid,NULL,&compute,(void*)auxiliary_param);
    
    param* main_param=malloc(sizeof(param));
    if(main_param==NULL){
        printf("Malloc memory error!\n");
        exit(0);
    }

    float* main_result;
    main_param->start=0;
    main_param->end=single_cpu_precision;
    main_result=(float*)compute(main_param);

    float * auxiliary_result;
    pthread_join(auxiliary_tid,&auxiliary_result);
/*
    Do not use pointer to pointer easily except you really understand how it works and how it is used.
    For instance,if you define a float** variable and pass it to the pthread_join function
    there will be a segmentation fault.
    Cause the pthread_join function will do *variable=(void **)(the value of return).
    but the *variable is not defined,thus the segmentation occurred.

    Be careful of the dirty and wild pointer which could lead to very confusing errors.
*/

    float sum_result=0.0;
    sum_result=*main_result+*auxiliary_result;
    sum_result*=4.0;

    printf("%.9f\n",sum_result);
    return 0;

}


