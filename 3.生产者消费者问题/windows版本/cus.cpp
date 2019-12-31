#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<time.h>

//2个生产者，每个生产者工作6次
#define Need_Producer 2
#define Works_Producer 6
//3个消费者，每个消费者工作4次
#define Need_Customer 3
#define Works_Customer 4
//缓冲区为3
#define buffer_len 3

static HANDLE hMutexMapping = INVALID_HANDLE_VALUE;
int num = 0;
HANDLE lpHandle[10];

struct buf
{
	int num;
	int read;
	int write;
	int buffer[buffer_len];
};

BOOL StartClone()
{
	int i;
	BOOL bCreateOK;
	PROCESS_INFORMATION pi;
	TCHAR szFilename[MAX_PATH];
	GetModuleFileName(NULL, szFilename, MAX_PATH);
	TCHAR szCmdLine[MAX_PATH];
	for (i = 1; i <= Need_Producer; i++)
	{
		sprintf(szCmdLine, "\"%s\" Producer %d", szFilename, i);
		STARTUPINFO si;
		ZeroMemory(reinterpret_cast<void *>(&si), sizeof(si));
		si.cb = sizeof(si);
		bCreateOK = CreateProcess(szFilename, szCmdLine, NULL, NULL, 
				FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi);
		if (!bCreateOK)
			return false;
		else
		{
			printf("Producer %d is created\n", i);
			lpHandle[num] = pi.hProcess;
			num++;
		}
	}
	for (i = 1; i <= Need_Customer; i++)
	{
		sprintf(szCmdLine, "\"%s\" Consumer %d", szFilename, i);
		STARTUPINFO si;
		ZeroMemory(reinterpret_cast<void *>(&si), sizeof(si));
		si.cb = sizeof(si);
		bCreateOK = CreateProcess(szFilename, szCmdLine, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi);
		if (!bCreateOK)
			return false;
		else
		{
			printf("Consumer %d is created\n", i);
			lpHandle[num] = pi.hProcess;
			num++;
		}
	}
	return true;
}

void Parent()
{
	hMutexMapping = CreateMutex(NULL, true, "mutex"); //创建互斥体对象 
	//创建共享内存区的文件映射
	HANDLE hMapping = CreateFileMapping(NULL, NULL, PAGE_READWRITE, 0, sizeof(LONG), "map");
	if (hMapping != INVALID_HANDLE_VALUE)
	{
		//把文件映射到进程的地址空间，用读写内存的方式操作和处理文件数据
		LPVOID pData = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0); 
		if (pData != NULL)
			ZeroMemory(pData, sizeof(LONG));
		struct buf *shmptr = reinterpret_cast<struct buf *>(pData);
    	shmptr->read = 0;
        shmptr->write = 0;
		shmptr->num = 0;
		memset(shmptr->buffer, 0, sizeof(shmptr->buffer));
		UnmapViewOfFile(pData); //从进程的地址空间撤销文件数据的映像
	}
	CreateSemaphore(NULL, 3, 3, "EMPTY");
	CreateSemaphore(NULL, 0, 3, "FULL");
	//创建5个子进程
	StartClone();
	ReleaseMutex(hMutexMapping); //释放互斥体 
}

void Producer(int n)
{
	int j;
	//为现有的已命名互斥体对象创建一个新句柄
	hMutexMapping = OpenMutex(MUTEX_ALL_ACCESS, true, "mutex"); 
	HANDLE hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "map"); //打开文件映射对象 
	if (hMapping == INVALID_HANDLE_VALUE)
		printf("error\n");
	//打开已经存在的命名信号量对象 
	HANDLE semEmpty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "EMPTY");
	HANDLE semFull = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FULL");
	for (int i = 0; i < Works_Producer; i++)
	{
		WaitForSingleObject(semEmpty, INFINITE);
		srand((unsigned)time(0));
		Sleep(rand()%1000);
		WaitForSingleObject(hMutexMapping, INFINITE);
		//把文件映射到进程的地址空间，用读写内存的方式操作和处理文件数据
		LPVOID pFile = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pFile != NULL)
		{
			struct buf *shmptr = reinterpret_cast<struct buf *>(pFile);
			shmptr->buffer[shmptr->write] = 1;
			shmptr->num++;
			SYSTEMTIME time;
			GetSystemTime(&time);
			printf("\n%04d/%02d/%02d-%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour+8, time.wMinute, time.wSecond);
			printf("\tProducer %d puts data at Position [%d]", n, shmptr->write + 1);
			printf("\tCurrent buffer: ");
			for (j = 0; j < buffer_len; j++)
				printf("%d ", shmptr->buffer[j]);
			shmptr->write = (shmptr->write + 1) % buffer_len;
		}
		UnmapViewOfFile(pFile); //解除文件映射 
		pFile = NULL;
		ReleaseSemaphore(semFull, 1, NULL); //释放信号量 
		ReleaseMutex(hMutexMapping); //释放互斥体 
	}
	printf("\tProducer %d works over.", n);
}

void Consumer(int n)
{
	int j;
	hMutexMapping = OpenMutex(MUTEX_ALL_ACCESS, true, "mutex");
	HANDLE hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "map");
	if (hMapping == INVALID_HANDLE_VALUE)
		printf("error\n");
	HANDLE semEmpty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "EMPTY");
	HANDLE semFull = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FULL");
	for (int i = 0; i < Works_Customer; i++)
	{
		WaitForSingleObject(semFull, INFINITE);
		srand((unsigned)time(0));
		Sleep(rand()%1000);
		WaitForSingleObject(hMutexMapping, INFINITE);
		LPVOID pFile = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pFile != NULL)
		{
			struct buf *shmptr = reinterpret_cast<struct buf *>(pFile);
			shmptr->buffer[shmptr->read] = 0;
			shmptr->num--;
			SYSTEMTIME time;
			GetSystemTime(&time);
			printf("\n%04d/%02d/%02d-%02d:%02d:%02d", time.wYear, time.wMonth, 
				time.wDay, time.wHour+8, time.wMinute, time.wSecond);
			printf("\tConsumer %d gets data from Position [%d]", n, shmptr->read + 1);
			printf("\tCurrent buffer: ");
			for (j = 0; j < buffer_len; j++)
				printf("%d ", shmptr->buffer[j]);
			shmptr->read = (shmptr->read + 1) % buffer_len;
		}
		UnmapViewOfFile(pFile);
		pFile = NULL;
		ReleaseSemaphore(semEmpty, 1, NULL);
		ReleaseMutex(hMutexMapping);
	}
	printf("\tConsumer %d works over.", n);
}

int main(int argc, char **argv)
{
	if (argc>1 && strcmp(argv[1], "Producer") == 0)
		Producer(atoi(argv[2]));
	else if (argc>1 && strcmp(argv[1], "Consumer") == 0)
		Consumer(atoi(argv[2]));
	else
	{
		Parent();
		WaitForMultipleObjects(num, lpHandle, true, INFINITE);
	}
	return 0;
}
