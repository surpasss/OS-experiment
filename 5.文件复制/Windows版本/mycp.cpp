#include<stdio.h>
#include<Windows.h>
#include<Windowsx.h>
#include<string.h>

#define buf_size 4096

//单个文件复制
void CopyFile(const char * fsource, const char * ftarget)
{
	WIN32_FIND_DATA lpfindfiledata;
	//查找指定文件路径
	HANDLE hfind = FindFirstFile(fsource, &lpfindfiledata);

	HANDLE hsource = CreateFile(fsource,//指向打开文件名的指针
		GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE htarget = CreateFile(ftarget,//指向创建文件名的指针
		GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	//源文件的大小
	LONG size = lpfindfiledata.nFileSizeLow - lpfindfiledata.nFileSizeHigh;
	DWORD wordbit;
	//新开缓冲区，保存数据
	int *BUFFER = new int[size];

	//源文件读数据
	ReadFile(hsource,//指定要读的文件句柄
		BUFFER,//指向存放从文件读的数据的缓冲区的地址指针
		size,//要从文件读的字节数
		&wordbit,//存放实际从文件中读的字节数的变量地址
		NULL);//同步方式打开设置为NULL

	//目标文件写数据
	WriteFile(htarget,//指定要写的文件对象的句柄
		BUFFER,//指向要写入文件的数据缓冲区指针
		size,//要写入文件的字节数
		&wordbit,//实际写入的字节数
		NULL);//同步方式打开设置为NULL
		
    SetFileTime(htarget,//指定文件的句柄
    	&lpfindfiledata.ftCreationTime,//文件创建时间
    	&lpfindfiledata.ftLastAccessTime,//文件最后访问时间
        &lpfindfiledata.ftLastWriteTime);//文件最后写入时间
                
	DWORD dwAttrs = GetFileAttributes(fsource);
	SetFileAttributes(ftarget, dwAttrs);
	
	CloseHandle(hfind);
	CloseHandle(hsource);
	CloseHandle(htarget);
}

//将源目录信息复制到目标目录下
void mycp(const char * fsource, const char * ftarget)
{
	WIN32_FIND_DATA lpfindfiledata;
	char source[buf_size];
	char target[buf_size];
	lstrcpy(source, fsource);
	lstrcpy(target, ftarget);
	lstrcat(source, "\\*.*");
	lstrcat(target, "\\");
	HANDLE hfind = FindFirstFile(source, &lpfindfiledata);//数据缓冲区
    while (FindNextFile(hfind, &lpfindfiledata) != 0)//循环查找FindFirstFile()函数搜索后的下一个文件
    {
        //查找下一个文件成功
        if ((lpfindfiledata.dwFileAttributes) == 16)//判断是否是目录(若为目录FILE_ATTRIBUTE_DIRECTORY是16)
        {
            if ((strcmp(lpfindfiledata.cFileName, ".") != 0) && (strcmp(lpfindfiledata.cFileName, "..") != 0))
            {
                memset(source, '0', sizeof(source));
                lstrcpy(source, fsource);
                lstrcat(source, "\\");
                lstrcat(source, lpfindfiledata.cFileName);//追加文件
                lstrcat(target, lpfindfiledata.cFileName);
                
                CreateDirectory(target,NULL);//为目标文件创建目录
                mycp(source, target);//进入子目录复制
                
                HANDLE hDir = CreateFile(target, GENERIC_READ|GENERIC_WRITE, 
					FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
				
				SetFileTime(hDir,
					&lpfindfiledata.ftCreationTime,
					&lpfindfiledata.ftLastAccessTime,
					&lpfindfiledata.ftLastWriteTime);
	
			    CloseHandle(hDir);
			    lstrcpy(source, fsource);
                lstrcat(source, "\\");
                lstrcpy(target, ftarget);
                lstrcat(target, "\\");
            }
        }
        else//无目录
        {
            memset(source, '0', sizeof(source));
            lstrcpy(source, fsource);
            lstrcat(source, "\\");
            lstrcat(source, lpfindfiledata.cFileName);
            lstrcat(target, lpfindfiledata.cFileName);
            CopyFile(source, target);//直接调用文件复制函数
            lstrcpy(source, fsource);
            lstrcat(source, "\\");
            lstrcpy(target, ftarget);
            lstrcat(target, "\\");
        }
    }
    CloseHandle(hfind);
}

int main(int argc, char *argv[])
{
	
	WIN32_FIND_DATA lpfindfiledata;
	if (argc != 3)
	{
        printf("Please enter valid parameters !\n");
        printf("For example: mycp sourceDir destinationDir\n");
	}
	else
	{
		if (FindFirstFile(argv[1], &lpfindfiledata) == INVALID_HANDLE_VALUE)
		{
			printf("Can not find the source directory\n");
		}
        else
        {
            if (FindFirstFile(argv[2], &lpfindfiledata) != INVALID_HANDLE_VALUE)
            {
                printf("The target directory is already exists\n"); 
            }
            else if (FindFirstFile(argv[2], &lpfindfiledata) == INVALID_HANDLE_VALUE)
            {
                printf("Create new directory %s\n",argv[2]);
                CreateDirectory(argv[2],NULL);//为目标文件创建目录
                mycp(argv[1], argv[2]);
                HANDLE hDir = CreateFile(argv[2], 
					GENERIC_READ|GENERIC_WRITE, 
					FILE_SHARE_READ, 
					NULL, 
					OPEN_EXISTING, 
					FILE_FLAG_BACKUP_SEMANTICS, 
					NULL);
				SetFileTime(hDir,
					&lpfindfiledata.ftCreationTime,
					&lpfindfiledata.ftLastAccessTime,
					&lpfindfiledata.ftLastWriteTime);			
                printf("Copy is done !\n");
            }
        }
	}
	return 0;
}
