#include <windows.h>
#include <stdio.h>
#include <iostream>
using namespace std;

void ShowTime(SYSTEMTIME start, SYSTEMTIME end);
 
int main(int argc, char **argv)
{
	SYSTEMTIME time_start, time_end;
	
	//进程启动相关信息的结构体
	STARTUPINFO si;	
	memset(&si,0,sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO); 
	si.dwFlags = STARTF_USESHOWWINDOW;	
	si.wShowWindow = SW_SHOW;
	
    PROCESS_INFORMATION pi;	//创建进程 
 	
	if (!CreateProcess(NULL, argv[1], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) 
	{
		cout <<"Create Fail!"<< endl;
		exit(1);
	}
	else 
	{
		cout <<"Create Success!"<< endl;
		GetSystemTime(&time_start);
		printf("Begin Time：%d:%d:%d-%d:%d:%d:%d\n", time_start.wYear, time_start.wMonth, time_start.wDay, time_start.wHour, time_start.wMinute, time_start.wSecond, time_start.wMilliseconds);
		cout << endl; 
	}
	
	if (argc == 2)
		WaitForSingleObject(pi.hProcess, INFINITE);
	else if(argc == 3)
	{
		//主进程休眠 
		double sleep_s;
		sleep_s = atof(argv[2]) * 1000;
		Sleep (sleep_s);
	
		//结束子进程 
		DWORD exitCode; 
		GetExitCodeProcess(pi.hProcess,&exitCode); //获取退出码  
		TerminateProcess(pi.hProcess, exitCode); //终止进程 
		//关闭句柄 
		CloseHandle(pi.hThread);  
		CloseHandle(pi.hProcess);  
		printf("Process has been killed\n"); 
	}
	
	GetSystemTime(&time_end);
	printf("End Time: %d:%d:%d-%d:%d:%d:%d\n", time_start.wYear, time_start.wMonth, time_start.wDay, time_end.wHour, time_end.wMinute, time_end.wSecond, time_end.wMilliseconds);
	ShowTime(time_start, time_end);
	return 0;
}

void ShowTime(SYSTEMTIME start, SYSTEMTIME end)
{
	int hour, min, sec, ms;
	ms = end.wMilliseconds - start.wMilliseconds;
	sec = end.wSecond - start.wSecond;
	min = end.wMinute - start.wMinute;
	hour = end.wHour - start.wHour;
	if (ms < 0) 
	{
		sec--;
		ms += 1000; 
	}
	if (sec < 0) 
	{
		min--;
		sec += 60; 
	}
	if (min < 0) 
	{
		hour--;
		min += 60; 
	}
	printf("Cost Time: %d hour %d min %d sec %d ms\n", hour, min, sec, ms);
}

