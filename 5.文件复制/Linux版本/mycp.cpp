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

//�����ļ�����
void CopyFile(char *fsource, char *ftarget)
{
	int fd = open(fsource, 0);//���ļ����ļ�������
	int fdr;//Ŀ���ļ�������
	char buf[buf_size];
	struct stat statbuf, statebuf1;
	struct utimbuf timeby;
	lstat(fsource, &statebuf1);
	if (S_ISLNK(statebuf1.st_mode))//�������� 
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
	char BUFFER[buf_size];//�¿�����������������
	int wordbit;
	stat(fsource, &statbuf);
	fdr = creat(ftarget, statbuf.st_mode);//�������ļ�,�����ļ�������
	while ((wordbit = read(fd, BUFFER, buf_size)) > 0)//��ȡԴ�ļ��ֽ���>0
	{
		write(fdr, BUFFER, wordbit);//д��Ŀ���ļ�
	}
	
	timeby.actime = statbuf.st_atime;//�޸�ʱ������
	timeby.modtime = statbuf.st_mtime;
	utime(ftarget, &timeby);

	close(fd); 
	close(fdr);
}

//��ԴĿ¼��Ϣ���Ƶ�Ŀ��Ŀ¼��
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
	dir = opendir(source);//��Ŀ¼,����ָ��DIR�ṹ��ָ��
	while ((entry = readdir(dir)) != NULL)//��Ŀ¼
	{
		if (entry->d_type == 4)//���ļ���,�ݹ鸴�� 
		{
			if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
			{
				strcat(source, "/");
				strcat(source, entry->d_name);
				strcat(target, "/");
				strcat(target, entry->d_name);
				stat(source, &statbuf);//ͳ���ļ�������Ϣ

				mkdir(target, statbuf.st_mode);//����Ŀ��Ŀ¼
				mycp(source, target);
				timeby.actime = statbuf.st_atime;
				timeby.modtime = statbuf.st_mtime;//�޸��ļ���ȡ���޸�ʱ��
				utime(target, &timeby);

				strcpy(source, fsource);
				strcpy(target, ftarget);
			}
		}
		else//��Ŀ¼ 
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
				mkdir(argv[2], statbuf.st_mode);//����Ŀ¼
				printf("Create new directory %s\n",argv[2]);
				mycp(argv[1], argv[2]);//��ʼ����
				utime(argv[2], &timeby);
				printf("Copy is done !\n");
			}
		}
	}
	return 0;
}



