#include<stdio.h>
#include<Windows.h>
#include<Windowsx.h>
#include<string.h>

#define buf_size 4096

//�����ļ�����
void CopyFile(const char * fsource, const char * ftarget)
{
	WIN32_FIND_DATA lpfindfiledata;
	//����ָ���ļ�·��
	HANDLE hfind = FindFirstFile(fsource, &lpfindfiledata);

	HANDLE hsource = CreateFile(fsource,//ָ����ļ�����ָ��
		GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE htarget = CreateFile(ftarget,//ָ�򴴽��ļ�����ָ��
		GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	//Դ�ļ��Ĵ�С
	LONG size = lpfindfiledata.nFileSizeLow - lpfindfiledata.nFileSizeHigh;
	DWORD wordbit;
	//�¿�����������������
	int *BUFFER = new int[size];

	//Դ�ļ�������
	ReadFile(hsource,//ָ��Ҫ�����ļ����
		BUFFER,//ָ���Ŵ��ļ��������ݵĻ������ĵ�ַָ��
		size,//Ҫ���ļ������ֽ���
		&wordbit,//���ʵ�ʴ��ļ��ж����ֽ����ı�����ַ
		NULL);//ͬ����ʽ������ΪNULL

	//Ŀ���ļ�д����
	WriteFile(htarget,//ָ��Ҫд���ļ�����ľ��
		BUFFER,//ָ��Ҫд���ļ������ݻ�����ָ��
		size,//Ҫд���ļ����ֽ���
		&wordbit,//ʵ��д����ֽ���
		NULL);//ͬ����ʽ������ΪNULL
		
    SetFileTime(htarget,//ָ���ļ��ľ��
    	&lpfindfiledata.ftCreationTime,//�ļ�����ʱ��
    	&lpfindfiledata.ftLastAccessTime,//�ļ�������ʱ��
        &lpfindfiledata.ftLastWriteTime);//�ļ����д��ʱ��
                
	DWORD dwAttrs = GetFileAttributes(fsource);
	SetFileAttributes(ftarget, dwAttrs);
	
	CloseHandle(hfind);
	CloseHandle(hsource);
	CloseHandle(htarget);
}

//��ԴĿ¼��Ϣ���Ƶ�Ŀ��Ŀ¼��
void mycp(const char * fsource, const char * ftarget)
{
	WIN32_FIND_DATA lpfindfiledata;
	char source[buf_size];
	char target[buf_size];
	lstrcpy(source, fsource);
	lstrcpy(target, ftarget);
	lstrcat(source, "\\*.*");
	lstrcat(target, "\\");
	HANDLE hfind = FindFirstFile(source, &lpfindfiledata);//���ݻ�����
    while (FindNextFile(hfind, &lpfindfiledata) != 0)//ѭ������FindFirstFile()�������������һ���ļ�
    {
        //������һ���ļ��ɹ�
        if ((lpfindfiledata.dwFileAttributes) == 16)//�ж��Ƿ���Ŀ¼(��ΪĿ¼FILE_ATTRIBUTE_DIRECTORY��16)
        {
            if ((strcmp(lpfindfiledata.cFileName, ".") != 0) && (strcmp(lpfindfiledata.cFileName, "..") != 0))
            {
                memset(source, '0', sizeof(source));
                lstrcpy(source, fsource);
                lstrcat(source, "\\");
                lstrcat(source, lpfindfiledata.cFileName);//׷���ļ�
                lstrcat(target, lpfindfiledata.cFileName);
                
                CreateDirectory(target,NULL);//ΪĿ���ļ�����Ŀ¼
                mycp(source, target);//������Ŀ¼����
                
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
        else//��Ŀ¼
        {
            memset(source, '0', sizeof(source));
            lstrcpy(source, fsource);
            lstrcat(source, "\\");
            lstrcat(source, lpfindfiledata.cFileName);
            lstrcat(target, lpfindfiledata.cFileName);
            CopyFile(source, target);//ֱ�ӵ����ļ����ƺ���
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
                CreateDirectory(argv[2],NULL);//ΪĿ���ļ�����Ŀ¼
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
