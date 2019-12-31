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

	//����һ���ź�������(�ź�����Ϊ2)������ֵΪ�ź������ϵı�ʶ��(�ؼ��֣��ź�������������򿪵ı�־)
	sem_id = semget(SEM_ALL_KEY, 2,IPC_CREAT|0660);
	//���ź���ִ�п��Ʋ���(�ź������ϱ�ʶ���ź�����������Ҫִ�еĲ���������û򷵻��ź����Ĳ���)
	semctl(sem_id, SEM_EMPTY, SETVAL, buffer_len);
	semctl(sem_id, SEM_FULL, SETVAL, 0);

	//����һ�������ڴ������ɹ�����Ϊ�����ڴ����ı�ʶ
	shm_id = shmget(IPC_PRIVATE, MYBUF_LEN, SHM_MODE);
	if (shm_id < 0)
	{
		printf("Error on shmget.\n"); //���빲���ڴ���ʧ��
		exit(1);
	}
	//������θ��ӵ�����ͨ�ŵĽ��̿ռ䣻�ɹ�ʱ���ع����ڴ渽�ӵ����̿ռ�����ַ��ʧ��ʱ����-1
	shmptr = shmat(shm_id, 0, 0);
	if (shmptr == (void *)-1)
	{
		printf("Error on shmat.\n"); //������θ��ӵ�����ͨ�ŵĽ��̿ռ�ʧ��
		exit(1);
	}
	shmptr->read = 0;
	shmptr->write = 0;


	//2�������߽���
	while (num_Producer < Need_Producer)
	{
		pid_p = fork();
		num_Producer++;
		if (pid_p < 0)
		{
			printf("Error on fork.\n"); //��������ʧ��
			exit(1);
		}

		//������������ӽ��̣���ʼ����������
		if (pid_p == 0)
		{
			printf("Producer %d is created.\n", num_Producer);
			//������θ��ӵ�����ͨ�ŵĽ��̿ռ䣻�ɹ�ʱ���ع����ڴ渽�ӵ����̿ռ�����ַ��ʧ��ʱ����-1
			shmptr = shmat(shm_id, 0, 0);
			if (shmptr == (void *)-1)
			{
				printf("Error on shmat.\n"); //������θ��ӵ�����ͨ�ŵĽ��̿ռ�ʧ��
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
			//������������֮��������
			shmdt(shmptr);
			exit(0);
		}
	}

	//3�������߽���
	while (num_Customer < Need_Customer)
	{
		pid_c = fork();
		num_Customer++;
		if (pid_c < 0)
		{
			printf("Error on fork.\n"); //��������ʧ��
			exit(1);
		}

		//������������ӽ��̣���ʼ����������
		if (pid_c == 0)
		{
			printf("Counsumer %d is created.\n", num_Customer);
			//������θ��ӵ�����ͨ�ŵĽ��̿ռ䣻�ɹ�ʱ���ع����ڴ渽�ӵ����̿ռ�����ַ��ʧ��ʱ����-1
			shmptr = shmat(shm_id, 0, 0);
			if (shmptr == (void *)-1)
			{
				printf("Error on shmat.\n"); //������θ��ӵ�����ͨ�ŵĽ��̿ռ�ʧ��
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
			//������������֮��������
			shmdt(shmptr);
			exit(0);
		}
	}

	//����������˳�
	while (wait(0) != -1);
	//������������֮��������
	shmdt(shmptr);
	//�Թ����ڴ���ִ�п��Ʋ���
	shmctl(shm_id,IPC_RMID,0); //��cmdΪIPC_RMIDʱ��ɾ���ù���
	shmctl(sem_id,IPC_RMID,0);
	//printf("Main process ends.\n"); //���������н���
	fflush(stdout);
	exit(0);
}

 
