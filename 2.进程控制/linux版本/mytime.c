#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<unistd.h>
#include<wait.h>

int main(int argc, char* argv[])
{
    //调用系统时间
    struct timeval time_start;
    struct timeval time_end;
    
    //用以记录进程运行的时间
    long time_use = 0; // us
    long time_left = 0; // us
    long time_hour = 0, time_min = 0, time_sec = 0, time_ms = 0, time_us = 0;
    

    pid_t pid = fork(); // return 0 to child process, return pid of child to parent process, return -1 while failure

    if (pid < 0) //如果出错
    {
        printf("Create Fail");
        exit(1);
    }
    else if (pid == 0) //如果是子进程
    {
        printf("Child process is running!\n");
        gettimeofday(&time_start, NULL);
        printf("time_start.tv_sec: %ld second\n", time_start.tv_sec); //子进程开始运行的系统秒数
        printf("time_start.tv_usec: %ld microsecond\n", time_start.tv_usec); //子进程开始运行的系统微秒数
        printf("\n");
        execv(argv[1], &argv[1]); //在子进程中调用execv函数在命令行中来运行一个程序
    }
    else //如果是父进程 
    {
        printf("Parent process is running!\n");
        gettimeofday(&time_start, NULL);
        printf("time_start.tv_sec: %ld second\n", time_start.tv_sec); //父进程开始运行的系统秒数
        printf("time_start.tv_usec: %ld microsecond\n", time_start.tv_usec); //父进程开始运行的系统微秒数
        printf("\n");
        if (argc == 2)
            wait(NULL); //等待子进程结束
        if (argc == 3)
		{
			//主进程休眠 
			double sleep_us;
            sleep_us = atof(argv[2]) * 1000000;
            usleep(sleep_us);
            
            //结束子进程 
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
        
        //计算运行时间 
        gettimeofday(&time_end, NULL);
        printf("time_end.tv_sec: %ld second\n", time_end.tv_sec); //进程结束的系统秒数
        printf("time_end.tv_usec: %ld microsecond\n", time_end.tv_usec); //进程结束的系统微秒数
        time_use = (time_end.tv_sec - time_start.tv_sec)*1000000 + (time_end.tv_usec - time_start.tv_usec);//总运行时间 
        time_hour = time_use / (60 * 60 * 1000 * 1000);//运行小时 
        time_left = time_use % (60 * 60 * 1000 * 1000);
        time_min = time_left / (60 * 1000 * 1000);//运行分钟 
        time_left %= (60 * 1000 * 1000);
        time_sec = time_left / (1000 * 1000);//运行秒 
        time_left %= (1000 * 1000);
        time_ms = time_left / 1000;//运行毫秒 
        time_left %= 1000;
        time_us = time_left;//运行微妙 
        printf("Time Cost: %ld hour, %ld min, %ld sec, %ld ms, %ld us\n",time_hour,time_min,time_sec,time_ms,time_us);
    }
    return 0;
}

