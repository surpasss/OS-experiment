#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <utime.h>
#include <sys/time.h>

#define buf_size 4096

//单个文件复制
void CopyFile(char *fsource, char *ftarget)
{
	int fd = open(fsource, 0);//打开文件，文件描述符
	int fdr;//目标文件描述符
	char buf[buf_size];
	struct stat statbuf, statebuf1;
	struct utimbuf timeby;
	lstat(fsource, &statebuf1);
	if (S_ISLNK(statebuf1.st_mode))//是软链接 
	{
		int rslt = readlink(fsource, buf, 511);
		buf[rslt] = '\0';
		symlink(buf, ftarget);	
		struct timeval times[2];
		times[0].tv_sec = statebuf1.st_atime;
		times[0].tv_usec = 0;
		times[1].tv_sec = statebuf1.st_mtime;
		times[1].tv_usec = 0;
		lutimes(ftarget, times);	
		return;
	}
	char BUFFER[buf_size];//新开缓冲区，保存数据
	int wordbit;
	stat(fsource, &statbuf);
	fdr = creat(ftarget, statbuf.st_mode);//创建新文件,返回文件描述符
	while ((wordbit = read(fd, BUFFER, buf_size)) > 0)//读取源文件字节数>0
	{
		write(fdr, BUFFER, wordbit);//写入目标文件
	}
	
	timeby.actime = statbuf.st_atime;//修改时间属性
	timeby.modtime = statbuf.st_mtime;
	utime(ftarget, &timeby);

	close(fd); 
	close(fdr);
}

//将源目录信息复制到目标目录下
void mycp(const char *fsource, const char *ftarget)
{
	char source[buf_size];
	char target[buf_size];
	char buf[buf_size];
	struct stat statbuf, statebuf1;
	struct utimbuf timeby;
	DIR *dir;
	struct dirent * entry;
	strcpy(source, fsource);
	strcpy(target, ftarget);
	dir = opendir(source);//打开目录,返回指向DIR结构的指针
	while ((entry = readdir(dir)) != NULL)//读目录
	{
		if (entry->d_type == 4)//是文件夹,递归复制 
		{
			if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
			{
				strcat(source, "/");
				strcat(source, entry->d_name);
				strcat(target, "/");
				strcat(target, entry->d_name);
				stat(source, &statbuf);//统计文件属性信息

				mkdir(target, statbuf.st_mode);//创建目标目录
				mycp(source, target);
				timeby.actime = statbuf.st_atime;
				timeby.modtime = statbuf.st_mtime;//修改文件存取和修改时间
				utime(target, &timeby);

				strcpy(source, fsource);
				strcpy(target, ftarget);
			}
		}
		else//无目录 
		{
			strcat(source, "/");
			strcat(source, entry->d_name);
			strcat(target, "/");
			strcat(target, entry->d_name);
			CopyFile(source, target);
			strcpy(source, fsource);
			strcpy(target, ftarget);
		}
	}
}

int main(int argc, char *argv[])
{
	struct stat statbuf;     
	struct utimbuf timeby;  
	DIR * dir;             
	if (argc != 3)          
	{
		printf("Please enter valid parameters !\n");
        printf("For example: mycp sourceDir destinationDir\n");
	}
	else
	{
		if ((dir = opendir(argv[1])) == NULL)
		{
			printf("Can not find the source directory\n");
		}
		else
		{
			if ((dir = opendir(argv[2])) != NULL)
			{
				printf("The target directory is already exists\n"); 
			}
			else if ((dir = opendir(argv[2])) == NULL)
			{
				stat(argv[1], &statbuf);
				timeby.actime = statbuf.st_atime;	
				timeby.modtime = statbuf.st_mtime;
				mkdir(argv[2], statbuf.st_mode);//创建目录
				printf("Create new directory %s\n",argv[2]);
				mycp(argv[1], argv[2]);//开始复制
				utime(argv[2], &timeby);
				printf("Copy is done !\n");
			}
		}
	}
	return 0;
}



