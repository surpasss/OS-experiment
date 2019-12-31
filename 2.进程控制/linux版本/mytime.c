#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<unistd.h>
#include<wait.h>

int main(int argc, char* argv[])
{
    //����ϵͳʱ��
    struct timeval time_start;
    struct timeval time_end;
    
    //���Լ�¼�������е�ʱ��
    long time_use = 0; // us
    long time_left = 0; // us
    long time_hour = 0, time_min = 0, time_sec = 0, time_ms = 0, time_us = 0;
    

    pid_t pid = fork(); // return 0 to child process, return pid of child to parent process, return -1 while failure

    if (pid < 0) //�������
    {
        printf("Create Fail");
        exit(1);
    }
    else if (pid == 0) //������ӽ���
    {
        printf("Child process is running!\n");
        gettimeofday(&time_start, NULL);
        printf("time_start.tv_sec: %ld second\n", time_start.tv_sec); //�ӽ��̿�ʼ���е�ϵͳ����
        printf("time_start.tv_usec: %ld microsecond\n", time_start.tv_usec); //�ӽ��̿�ʼ���е�ϵͳ΢����
        printf("\n");
        execv(argv[1], &argv[1]); //���ӽ����е���execv��������������������һ������
    }
    else //����Ǹ����� 
    {
        printf("Parent process is running!\n");
        gettimeofday(&time_start, NULL);
        printf("time_start.tv_sec: %ld second\n", time_start.tv_sec); //�����̿�ʼ���е�ϵͳ����
        printf("time_start.tv_usec: %ld microsecond\n", time_start.tv_usec); //�����̿�ʼ���е�ϵͳ΢����
        printf("\n");
        if (argc == 2)
            wait(NULL); //�ȴ��ӽ��̽���
        if (argc == 3)
		{
			//���������� 
			double sleep_us;
            sleep_us = atof(argv[2]) * 1000000;
            usleep(sleep_us);
            
            //�����ӽ��� 
            long kill_ret_val = 0;
            kill_ret_val = kill(pid, SIGKILL);
            if (kill_ret_val == -1) // return -1, fail
            {
                printf("Kill Failed.\n");
                perror("Kill");
            }
            else if (kill_ret_val == 0) // return 0, success
            	printf("process %d has been killed\n", pid); 
            printf("\n");
        }
        
        //��������ʱ�� 
        gettimeofday(&time_end, NULL);
        printf("time_end.tv_sec: %ld second\n", time_end.tv_sec); //���̽�����ϵͳ����
        printf("time_end.tv_usec: %ld microsecond\n", time_end.tv_usec); //���̽�����ϵͳ΢����
        time_use = (time_end.tv_sec - time_start.tv_sec)*1000000 + (time_end.tv_usec - time_start.tv_usec);//������ʱ�� 
        time_hour = time_use / (60 * 60 * 1000 * 1000);//����Сʱ 
        time_left = time_use % (60 * 60 * 1000 * 1000);
        time_min = time_left / (60 * 1000 * 1000);//���з��� 
        time_left %= (60 * 1000 * 1000);
        time_sec = time_left / (1000 * 1000);//������ 
        time_left %= (1000 * 1000);
        time_ms = time_left / 1000;//���к��� 
        time_left %= 1000;
        time_us = time_left;//����΢�� 
        printf("Time Cost: %ld hour, %ld min, %ld sec, %ld ms, %ld us\n",time_hour,time_min,time_sec,time_ms,time_us);
    }
    return 0;
}

