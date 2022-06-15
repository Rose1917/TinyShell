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





typedef struct {
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} sema_t;

typedef struct{
    char buffer[4];
    int pointer_in;
    int pointer_out;
    sema_t mutex_sema;
    sema_t empty_buffer_sema;
    sema_t full_buffer_sema;
    int buffer_size;
}Buffer;

Buffer* buffer_1;
Buffer* buffer_2;
Buffer* buffer_array[3];

void sema_init(sema_t *sema, int value)
{
    sema->value = value;
    pthread_mutex_init(&sema->mutex, NULL);
    pthread_cond_init(&sema->cond, NULL);
}

void sema_wait(sema_t *sema)
{
    pthread_mutex_lock(&sema->mutex);
    while (sema->value <= 0)
        pthread_cond_wait(&sema->cond, &sema->mutex);
    sema->value--;
    pthread_mutex_unlock(&sema->mutex);
}

void sema_signal(sema_t *sema)
{
    pthread_mutex_lock(&sema->mutex);
    ++sema->value;
    pthread_cond_signal(&sema->cond);
    pthread_mutex_unlock(&sema->mutex);
}




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
        sema_wait(&(buffer_1->empty_buffer_sema)); 
        sema_wait(&(buffer_1->mutex_sema));

        buffer_put_item(BUFFER_1,temp_data); 

        sema_signal(&(buffer_1->mutex_sema));
        sema_signal(&(buffer_1->full_buffer_sema));
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
        

       
        sema_wait(&(buffer_1->full_buffer_sema));
        sema_wait(&(buffer_1->mutex_sema));

        sema_wait(&(buffer_2->empty_buffer_sema));
        sema_wait(&(buffer_2->mutex_sema));

        temp_data=buffer_get_item(BUFFER_1);
        temp_data+=('A'-'a');
        buffer_put_item(BUFFER_2,temp_data);
        
        sema_signal(&(buffer_1->mutex_sema));
        sema_signal(&(buffer_1->empty_buffer_sema));
        
        sema_signal(&(buffer_2->mutex_sema));
        sema_signal(&(buffer_2->full_buffer_sema));
        
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
        
        sema_wait(&(buffer_2->full_buffer_sema));
        sema_wait(&(buffer_2->mutex_sema));
        temp_data=buffer_get_item(BUFFER_2);
        printf("%c \n",temp_data);

        sema_signal(&(buffer_2->mutex_sema));
        sema_signal(&(buffer_2->empty_buffer_sema));
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

    sema_init(&buffer_1->mutex_sema, 1);
    sema_init(&buffer_1->empty_buffer_sema, BUFFER_SIZE - 1);
    sema_init(&buffer_1->full_buffer_sema, 0);

    buffer_2->pointer_in=0;
    buffer_2->pointer_out=0;
    buffer_2->buffer_size=BUFFER_SIZE;
    
    sema_init(&buffer_2->mutex_sema, 1);
    sema_init(&buffer_2->empty_buffer_sema, BUFFER_SIZE - 1);
    sema_init(&buffer_2->full_buffer_sema, 0);
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
}