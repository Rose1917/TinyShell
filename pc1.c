#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define BUFFER_1 1
#define BUFFER_2 2

//#define DEBUG
/*
    Function Test
    */

#define BUFFER_SIZE 4
#define ITEM_COUNT 8

typedef struct{
    char buffer[4];
    int pointer_in;
    int pointer_out;
    pthread_mutex_t mutex;
    pthread_cond_t wait_empty_buffer;
    pthread_cond_t wait_full_buffer;
    int buffer_size;
}Buffer;

Buffer* buffer_1;
Buffer* buffer_2;



Buffer* buffer_array[3];

int is_empty_buffer(int num){
        return buffer_array[num]->pointer_in==buffer_array[num]->pointer_out;
}

int is_full_buffer(int num){
    char c;
    return ((buffer_array[num]->pointer_in + 1) % buffer_array[num]->buffer_size) ==buffer_array[num]->pointer_out;
}

int buffer_get_item(int num ){
    Buffer* temp_buffer=buffer_array[num];
    
    char temp_data=temp_buffer->buffer[temp_buffer->pointer_out];
    temp_buffer->pointer_out=(temp_buffer->pointer_out+1)%temp_buffer->buffer_size;
    return temp_data;
}
int buffer_put_item(int  num,char data){
    Buffer* temp_buffer=buffer_array[num];

    temp_buffer->buffer[temp_buffer->pointer_in]=data;
    temp_buffer->pointer_in=(temp_buffer->pointer_in+1)%temp_buffer->buffer_size;
    return 0;
}

void* produce(void * p){
    #ifdef DEBUG
        printf("Produce\n");
    #endif

    char temp_data;
    
    for(temp_data='a';temp_data<'i';temp_data++){
        pthread_mutex_lock(&(buffer_array[BUFFER_1]->mutex));
        #ifdef DEBUG
        printf("producer: Locked buffer 1\n");

        //printf("Producer:is buffer 1 full? %d\n",is_full_buffer(BUFFER_1));
        #endif

        
        while (is_full_buffer(BUFFER_1)) 
            pthread_cond_wait(&(buffer_1->wait_empty_buffer),&(buffer_1->mutex));
        
        #ifdef DEBUG
        printf("produce: got the buffer 1\n");
        #endif
        buffer_put_item(BUFFER_1,temp_data); 

        #ifdef DEBUG
        printf("producer: put the data %c in the buffer 1\n",temp_data);
        //printf("Producer:is buffer 1 full? %d\n",is_full_buffer(BUFFER_1));
        #endif
        pthread_cond_signal(&(buffer_1->wait_full_buffer));
        pthread_mutex_unlock(&(buffer_1->mutex));
        #ifdef DEBUG
        printf("producer: Unlocked buffer 1\n");
        //printf("Producer:is buffer 1 full? %d\n",is_full_buffer(BUFFER_1));
        #endif
    }
    return NULL;
}


void* compute(void* p){

    char temp_data;
    #ifdef DEBUG
        printf("compute\n");
    #endif

    
    int i;
    for(i=0;i<ITEM_COUNT;i++){
        pthread_mutex_lock(&(buffer_1->mutex));
        #ifdef DEBUG
        printf("compute: Locked buffer 1\n");
        #endif
        while (is_empty_buffer(BUFFER_1)) 
            pthread_cond_wait(&(buffer_1->wait_full_buffer),&(buffer_1->mutex));

        #ifdef DEBUG
        printf("compute: got the buffer 1\n");
        #endif

        pthread_mutex_lock(&(buffer_2->mutex));
        #ifdef DEBUG
        printf("compute: Locked buffer 2\n");
        #endif

        while(is_full_buffer(BUFFER_2))
            pthread_cond_wait(&(buffer_2->wait_empty_buffer),&(buffer_2->mutex));

        #ifdef DEBUG
        printf("compute: get the buffer 2\n");
        #endif

        temp_data=buffer_get_item(BUFFER_1);
        temp_data+=('A'-'a');
        buffer_put_item(BUFFER_2,temp_data);
        #ifdef DEBUG
        printf("compute: put a data  %c in buffer 2\n",temp_data);
        //printf("Producer:is buffer 1 full? %d\n",is_full_buffer(BUFFER_1));
        #endif
        pthread_cond_signal(&(buffer_2->wait_full_buffer));
        pthread_cond_signal(&(buffer_1->wait_empty_buffer));

        pthread_mutex_unlock(&(buffer_1->mutex));
        pthread_mutex_unlock(&(buffer_2->mutex));

        #ifdef DEBUG
        printf("compute: Unlocked the buffer 1 and 2\n");
        //printf("Producer:is buffer 1 full? %d\n",is_full_buffer(BUFFER_1));
        #endif
    }
    return NULL;
}
void* consume(void* p){
    #ifdef DEBUG
        printf("consume\n");
    #endif

    int i;
    char temp_data;
    for(i=0;i<ITEM_COUNT;i++){
        
        pthread_mutex_lock(&(buffer_2->mutex));
        

        #ifdef DEBUG
        printf("consume: Locked buffer 2\n",i);
        #endif
        
        while (is_empty_buffer(BUFFER_2)) 
            pthread_cond_wait(&(buffer_2->wait_full_buffer),&(buffer_2->mutex));

        #ifdef DEBUG
        printf("compute: got the buffer 2\n");
        #endif
        temp_data=buffer_get_item(BUFFER_2);
        printf("%c \n",temp_data);

        pthread_cond_signal(&(buffer_2->wait_empty_buffer));
        pthread_mutex_unlock(&(buffer_2->mutex));
        #ifdef DEBUG
        printf("consume: Unlocked the buffer 2\n");
        #endif
        
    }
    printf("\n");
    return NULL;
}

/*In this function,the we apply the memory for the 
  two buffers,init the mutex value and we also set the buffer arary values
  */
void init(){
    
    buffer_1=malloc(sizeof(Buffer));
    buffer_2=malloc(sizeof(Buffer));
    buffer_array[1]=buffer_1;
    buffer_array[2]=buffer_2;

    buffer_1->pointer_in=0;
    buffer_1->pointer_out=0;
    buffer_1->buffer_size=BUFFER_SIZE;
    pthread_mutex_init(&(buffer_1->mutex),NULL);
    pthread_cond_init(&(buffer_1->wait_empty_buffer), NULL);
    pthread_cond_init(&(buffer_1->wait_full_buffer),NULL);

    buffer_2->pointer_in=0;
    buffer_2->pointer_out=0;
    buffer_2->buffer_size=BUFFER_SIZE;
    pthread_mutex_init(&(buffer_2->mutex),NULL);
    pthread_cond_init(&(buffer_2->wait_empty_buffer), NULL);
    pthread_cond_init(&(buffer_2->wait_full_buffer),NULL);

}
int main(){

    init();

    pthread_t tid_producer;
    pthread_create(&tid_producer,NULL,&produce,NULL);

    pthread_t tid_computer;
    pthread_create(&tid_computer,NULL,&compute,NULL);

    pthread_t tid_consumer;
    pthread_create(&tid_consumer,NULL,&consume,NULL);
    

    pthread_join(tid_producer,NULL);
    pthread_join(tid_computer,NULL);
    pthread_join(tid_consumer,NULL);

    pthread_mutex_destroy(&(buffer_1->mutex));
    pthread_mutex_destroy(&(buffer_2->mutex));

}