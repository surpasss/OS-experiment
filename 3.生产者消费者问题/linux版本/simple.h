#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/types.h>   
#include <sys/wait.h>

//2�������ߣ�ÿ�������߹���6��
#define Need_Producer 2
#define Works_Producer 6
//3�������ߣ�ÿ�������߹���4��
#define Need_Customer 3
#define Works_Customer 4
//������Ϊ3
#define buffer_len 3
#define MYBUF_LEN (sizeof(struct buf))
#define SHM_MODE 0600 //�ɶ���д
#define SEM_ALL_KEY 1234
#define SEM_EMPTY 0
#define SEM_FULL 1

//�������Ľṹ��ѭ�����У�
struct buf
{
	int buffer[buffer_len];
    int write;
	int read;
};


//P����
void P(int sem_id, int sem_num)
{
	struct sembuf xx;
	xx.sem_num = sem_num; //�ź���������
	xx.sem_op = -1; //�ź����Ĳ���ֵ
	xx.sem_flg = 0; //���ʱ�־
 	semop(sem_id, &xx, 1); //һ������еĲ���������sembuf�е�Ԫ����Ϊ1
}

//V����
void V(int sem_id, int sem_num)
{
	struct sembuf xx;
	xx.sem_num = sem_num;
	xx.sem_op = 1;
	xx.sem_flg = 0;
	semop(sem_id, &xx, 1);
}
