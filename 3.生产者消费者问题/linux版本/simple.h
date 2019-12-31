#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/types.h>   
#include <sys/wait.h>

//2个生产者，每个生产者工作6次
#define Need_Producer 2
#define Works_Producer 6
//3个消费者，每个消费者工作4次
#define Need_Customer 3
#define Works_Customer 4
//缓冲区为3
#define buffer_len 3
#define MYBUF_LEN (sizeof(struct buf))
#define SHM_MODE 0600 //可读可写
#define SEM_ALL_KEY 1234
#define SEM_EMPTY 0
#define SEM_FULL 1

//缓冲区的结构（循环队列）
struct buf
{
	int buffer[buffer_len];
    int write;
	int read;
};


//P操作
void P(int sem_id, int sem_num)
{
	struct sembuf xx;
	xx.sem_num = sem_num; //信号量的索引
	xx.sem_op = -1; //信号量的操作值
	xx.sem_flg = 0; //访问标志
 	semop(sem_id, &xx, 1); //一次需进行的操作的数组sembuf中的元素数为1
}

//V操作
void V(int sem_id, int sem_num)
{
	struct sembuf xx;
	xx.sem_num = sem_num;
	xx.sem_op = 1;
	xx.sem_flg = 0;
	semop(sem_id, &xx, 1);
}
