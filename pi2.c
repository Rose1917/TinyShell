#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define precision_level 200000000
#define cpu_num 250
#define single_cpu_precision (precision_level/cpu_num)
/*
    Author-ID:161730231
    Author:RenYanjie
    In this function,the precision level and the cpu num are all setable.
    The precision level represents the Polynomial terms,and cpu number will represents the number of threads
*/


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

    pthread_t tid_array[cpu_num];
    param* param_array[cpu_num];
    

    int i;
    for(i=0;i<cpu_num;i++){
        param_array[i]=malloc(sizeof(param));
        param_array[i]->start=i*single_cpu_precision;
        param_array[i]->end=(i+1)*single_cpu_precision;
        pthread_create(&tid_array[i],NULL,&compute,(void*)param_array[i]);
    }
    

    float* result_array[cpu_num];
    float sum_result=0.0;
    for(i=0;i<cpu_num;i++){
        pthread_join(tid_array[i],(void*)&result_array[i]);
        sum_result+=(*result_array[i]);
    }
    sum_result*=4;

/*
    Do not use pointer to pointer easily except you really understand how it works and how it is used.
    For instance,if you define a float** variable and pass it to the pthread_join function
    there will be a segmentation fault.
    Cause the pthread_join function will do *variable=(void **)(the value of return).
    but the *variable is not defined,thus the segmentation occurred.

    Be careful of the dirty and wild pointer which could lead to very confusing errors.
*/


    printf("%.10f\n",sum_result);
    return 0;

}


