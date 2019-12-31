#include "simple.h"

int main(int argc, char *argv[])
{
	int i, j;
	int shm_id, sem_id;
	int num_Producer = 0, num_Customer = 0;
	struct buf *shmptr;

	time_t now;
	struct tm *timenow;
	pid_t pid_p, pid_c;

	//创建一个信号量集合(信号量数为2)，返回值为信号量集合的标识号(关键字，信号量数，创建或打开的标志)
	sem_id = semget(SEM_ALL_KEY, 2,IPC_CREAT|0660);
	//对信号量执行控制操作(信号量集合标识，信号量的索引，要执行的操作命令，设置或返回信号量的参数)
	semctl(sem_id, SEM_EMPTY, SETVAL, buffer_len);
	semctl(sem_id, SEM_FULL, SETVAL, 0);

	//申请一个共享内存区，成功返回为共享内存区的标识
	shm_id = shmget(IPC_PRIVATE, MYBUF_LEN, SHM_MODE);
	if (shm_id < 0)
	{
		printf("Error on shmget.\n"); //申请共享内存区失败
		exit(1);
	}
	//将共享段附加到申请通信的进程空间；成功时返回共享内存附加到进程空间的虚地址，失败时返回-1
	shmptr = shmat(shm_id, 0, 0);
	if (shmptr == (void *)-1)
	{
		printf("Error on shmat.\n"); //将共享段附加到申请通信的进程空间失败
		exit(1);
	}
	shmptr->read = 0;
	shmptr->write = 0;


	//2个生产者进程
	while (num_Producer < Need_Producer)
	{
		pid_p = fork();
		num_Producer++;
		if (pid_p < 0)
		{
			printf("Error on fork.\n"); //创建进程失败
			exit(1);
		}

		//如果是生产者子进程，开始创建生产者
		if (pid_p == 0)
		{
			printf("Producer %d is created.\n", num_Producer);
			//将共享段附加到申请通信的进程空间；成功时返回共享内存附加到进程空间的虚地址，失败时返回-1
			shmptr = shmat(shm_id, 0, 0);
			if (shmptr == (void *)-1)
			{
				printf("Error on shmat.\n"); //将共享段附加到申请通信的进程空间失败
				exit(1);
			}
			
		 	for (i=0; i<Works_Producer; i++)
			{
				P(sem_id, SEM_EMPTY);
                srand((unsigned)(getpid() + time(0)));
		        sleep(rand()%10);
			
				time(&now);
				timenow = localtime(&now);
				printf("%04d/%02d/%02d-%02d:%02d:%02d", timenow->tm_year+1900, 
					timenow->tm_mon+1, timenow->tm_mday, timenow->tm_hour, 
					timenow->tm_min, timenow->tm_sec);
				
				shmptr->buffer[shmptr->write] = 1;
				printf("\tProducer %d puts data at Position [%d]", num_Producer, shmptr->write + 1);
				shmptr->write = (shmptr->write + 1) % buffer_len;
				
				
				printf("\tCurrent buffer: ");
                for (j = 0; j < buffer_len; j++)
				    printf("%d ", shmptr->buffer[j]);

				if(i == Works_Producer - 1)
					printf("\tProducer %d works over", num_Producer);				
				printf("\n");

				fflush(stdout);
				V(sem_id,SEM_FULL);
			}
			//将共享段与进程之间解除连接
			shmdt(shmptr);
			exit(0);
		}
	}

	//3个消费者进程
	while (num_Customer < Need_Customer)
	{
		pid_c = fork();
		num_Customer++;
		if (pid_c < 0)
		{
			printf("Error on fork.\n"); //创建进程失败
			exit(1);
		}

		//如果是消费者子进程，开始创建消费者
		if (pid_c == 0)
		{
			printf("Counsumer %d is created.\n", num_Customer);
			//将共享段附加到申请通信的进程空间；成功时返回共享内存附加到进程空间的虚地址，失败时返回-1
			shmptr = shmat(shm_id, 0, 0);
			if (shmptr == (void *)-1)
			{
				printf("Error on shmat.\n"); //将共享段附加到申请通信的进程空间失败
				exit(1);
			}

		 	for (i=0; i<Works_Customer; i++)
			{
				P(sem_id, SEM_FULL);
                srand((unsigned)(getpid() + time(0)));
		        sleep(rand()%10);

				time(&now);
				timenow = localtime(&now);
				printf("%04d/%02d/%02d-%02d:%02d:%02d", timenow->tm_year+1900, timenow->tm_mon+1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min, timenow->tm_sec);

				shmptr->buffer[shmptr->read] = 0;
				printf("\tConsumer %d gets data from Position [%d]", num_Customer, shmptr->read + 1);
				shmptr->read = (shmptr->read + 1) % buffer_len;	

				printf("\tCurrent buffer: ");
                for (j = 0; j < buffer_len; j++)
				    printf("%d ", shmptr->buffer[j]);

				if(i == Works_Customer - 1)
					printf("\tCustomer %d works over", num_Customer);		
				printf("\n");

				fflush(stdout);
				V(sem_id,SEM_EMPTY);
			}
			//将共享段与进程之间解除连接
			shmdt(shmptr);
			exit(0);
		}
	}

	//主进程最后退出
	while (wait(0) != -1);
	//将共享段与进程之间解除连接
	shmdt(shmptr);
	//对共享内存区执行控制操作
	shmctl(shm_id,IPC_RMID,0); //当cmd为IPC_RMID时，删除该共享
	shmctl(sem_id,IPC_RMID,0);
	//printf("Main process ends.\n"); //主进程运行结束
	fflush(stdout);
	exit(0);
}

 
