#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define array_max_length 10000


typedef struct {
    int* array;
    int from;
    int to;
} param;
       
void* select_sort(void * p){

    param* local_param=p;

    int i,j,min,temp;
    for(i=local_param->from;i<local_param->to-1;i++){
        min=i;
        for(j=i+1;j<local_param->to;j++)
            if(local_param->array[j]<local_param->array[min])min=j;
        
        temp=local_param->array[i];
        local_param->array[i]=local_param->array[min];
        local_param->array[min]=temp;
    }
    return NULL;
}

 void merge (int array[],int left,int mid,int right){
        
        int i = left;
        int j = mid;
        int t = 0;
        int *temp=malloc(sizeof(array_max_length));

        while (i<=mid && j<=right){
            if(array[i]<=array[j]){
                temp[t++] = array[i++];
            }else {
                temp[t++] = array[j++];
            }
        }


        while(i<=mid){//将左边剩余元素填充进temp中
            temp[t++] = array[i++];
        }
        while(j<=right){//将右序列剩余元素填充进temp中
            temp[t++] = array[j++];
        }

        t = 0;
        while(left<=right){
            array[left++] = temp[t++];
        }
    }

int main(){
    int array[10]={-1,-9,211,2,32,88,1,0,999,1000};
    int array_length=sizeof(array)/sizeof(int);
    param* param_1=malloc(sizeof(param));
    param* param_2=malloc(sizeof(param));

    pthread_t tid_1;
    param_1->array=array;
    param_1->from=0;
    param_1->to=array_length-1;
    pthread_create(&tid_1,NULL,&select_sort,param_1);

    pthread_t tid_2;
    param_2->array=array;
    param_2->from=array_length/2;
    param_2->to=array_length-1;
    pthread_create(&tid_2,NULL,&select_sort,param_2);

    pthread_join(tid_1,NULL);
    pthread_join(tid_2,NULL);

    merge(array,0,array_length/2-1,array_length-1);
    
    int i;
    for(i=0;i<array_length;i++)
        printf("%d ",array[i]);
    printf("\n");
    return 0;

}