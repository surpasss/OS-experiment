//g++ MemoryMonitor.cpp -lpsapi -lShlwapi -o MemoryMonitor

#include<iostream>
#include<cstdio>
#include<windows.h>
#include<tchar.h>
#include<psapi.h>
#include<tlhelp32.h>
#include<shlwapi.h>
#include<iomanip>
#include"conio.h"
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma warning(disable: 4996)
using namespace std;

#define DIV (1024*1024)


HANDLE GetProcessHandle(int ProcessID)
{
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
}

//��ʾ������ǣ��ñ�Ǳ�ʾ����Ӧ�ó�����ڴ���з��ʵ�����
inline bool TestSet(DWORD dwTarget, DWORD dwMask)
{
	return ((dwTarget &dwMask) == dwMask);
}


#define SHOWMASK(dwTarget, type) \
if (TestSet(dwTarget, PAGE_##type))\
{cout<<", "<<# type;}

void ShowProtection(DWORD dwTarget)
{
	//�����ҳ�汣����ʽ
	SHOWMASK(dwTarget, READONLY);
	SHOWMASK(dwTarget, GUARD);
	SHOWMASK(dwTarget, NOCACHE);
	SHOWMASK(dwTarget, READWRITE);
	SHOWMASK(dwTarget, WRITECOPY);
	SHOWMASK(dwTarget, EXECUTE);
	SHOWMASK(dwTarget, EXECUTE_READ);
	SHOWMASK(dwTarget, EXECUTE_READWRITE);
	SHOWMASK(dwTarget, EXECUTE_WRITECOPY);
	SHOWMASK(dwTarget, NOACCESS);
}


//�������������ڴ棬��ʾ�������������ַ�ռ䲼��
void WalkVM(HANDLE hProcess)
{
	SYSTEM_INFO si;                     //ϵͳ��Ϣ�ṹ
	ZeroMemory(&si, sizeof(si));		//��ʼ��
	GetSystemInfo(&si);                 //���ϵͳ��Ϣ
	MEMORY_BASIC_INFORMATION mbi;       //���������ڴ�ռ�Ļ�����Ϣ�ṹ
	ZeroMemory(&mbi, sizeof(mbi));      //���仺���������ڱ�����Ϣ				
	LPCVOID pBlock = (LPVOID)si.lpMinimumApplicationAddress; //ѭ������Ӧ�ó����ַ�ռ�
	while (pBlock < si.lpMaximumApplicationAddress)
	{
		//�����һ�������ڴ�����Ϣ
		if (VirtualQueryEx(hProcess,           //���̾��
			pBlock,                            //��ʼλ��
			&mbi,                              //������
			sizeof(mbi)) == sizeof(mbi))       //���ȵ�ȷ�ϣ����ʧ���򷵻�0
		{
			//��Ľ�βָ��
			LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;
			TCHAR szSize[MAX_PATH];
			StrFormatByteSize(mbi.RegionSize, szSize, MAX_PATH);

			//��ʾ���ַ�ͳ���
			cout.fill('0');
			cout << hex << setw(8) << (DWORD*)pBlock
				<< "--"
				<< hex << setw(8) << (DWORD*)pEnd
				<< "  " << szSize;
			
			//��ʾ���״̬
			switch (mbi.State) 
			{
				case MEM_COMMIT:
					printf("   Committed"); break;
				case MEM_FREE:
					printf("   Free"); break;
				case MEM_RESERVE:
					printf("   Reserved"); break;
			}

			//��ʾ����
			if (mbi.Protect == 0 && mbi.State != MEM_FREE) 
			{
				mbi.Protect = PAGE_READONLY;
			}
			ShowProtection(mbi.Protect);

			//��ʾ����
			switch (mbi.Type) 
			{
				case MEM_IMAGE:
					printf(", Image"); break;
				case MEM_MAPPED:
					printf(", Mapped"); break;
				case MEM_PRIVATE:
					printf(", Private"); break;
			}

			//����ִ�е�ӳ��
			TCHAR szFilename[MAX_PATH];
			if (GetModuleFileName(
				(HMODULE)pBlock,        //ʵ�������ڴ��ģ����
				szFilename,             //��ȫָ�����ļ�����
				MAX_PATH) > 0)            //ʵ��ʹ�õĻ���������
			{
				//��ȥ·������ʾ
				PathStripPath(szFilename);
				printf("%s", szFilename);
			}

			printf("\n");
			//�ƶ���ָ���Ի����һ����
			pBlock = pEnd;
		}
	}
	cout << endl;
}


//���ڵ�ǰϵͳ����Ϣ
void ShowProcessAddress()
{
	SYSTEM_INFO sys_info;  //ϵͳ��Ϣ�ṹ
	ZeroMemory(&sys_info, sizeof(sys_info));//��ʼ��
	while (!kbhit())
	{
		//���ϵͳ��Ϣ
		GetSystemInfo(&sys_info);
		printf("�����ڴ��ҳ��С: %d KB\n", sys_info.dwPageSize / 1024);
		printf("����������: %d\n", sys_info.dwNumberOfProcessors);
		printf("�������ܹ�: %d\n", sys_info.dwProcessorType);
		printf("�����ڴ�����: %d KB\n", sys_info.dwAllocationGranularity / 1024);
		printf("��ϵ�ṹ��صĴ������ȼ�: %d\n", sys_info.wProcessorLevel);
		printf("��ϵ�ṹ��صĴ������޶�: %x\n", sys_info.wProcessorRevision);
		printf("Ӧ����С��ַ: 0x%0.8x\n", sys_info.lpMinimumApplicationAddress);
		printf("Ӧ������ַ: 0x%0.8x\n", sys_info.lpMaximumApplicationAddress);
		printf("Ӧ�ÿ��������ڴ��С: %0.2f GB\n", ((DWORD*)sys_info.lpMaximumApplicationAddress
			- (DWORD*)sys_info.lpMinimumApplicationAddress) / (1024.0*1024.0*1024.0));
		cout << endl;
		Sleep(1000);
	}

}


void ShowPerformance()
{
	PERFORMANCE_INFORMATION perfor_info;
	perfor_info.cb = sizeof(perfor_info);
	while (!kbhit())
	{
		GetPerformanceInfo(&perfor_info, sizeof(perfor_info));
		cout << "��ҳ��С: " << perfor_info.PageSize / 1024 << "KB" << endl;
		cout << "ϵͳ�ύ��ҳ������: " << perfor_info.CommitTotal << " Pages" << endl;
		cout << "ϵͳ�ύ��ҳ������: " << perfor_info.CommitLimit << " Pages" << endl;
		cout << "ϵͳ�ύ��ҳ���ֵ: " << perfor_info.CommitPeak << " Pages" << endl;
		cout << "��ҳ����������ڴ�����: " << perfor_info.PhysicalTotal << " Pages" << endl;
		cout << "��ҳ����������ڴ������: " << perfor_info.PhysicalAvailable << " Pages" << endl;
		cout << "ϵͳ�����ڴ�ռ��: " << (perfor_info.PhysicalTotal - perfor_info.PhysicalAvailable)*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "ϵͳ�����ڴ����: " << perfor_info.PhysicalAvailable*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "ϵͳ�����ڴ�����: " << perfor_info.PhysicalTotal*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "ϵͳ���������� " << perfor_info.PhysicalAvailable << " Pages" << endl;
		cout << "ϵͳ�ں��ڴ�ռ��ҳ�������� " << perfor_info.KernelTotal << " Pages" << endl;
		cout << "ϵͳ�ں��ڴ�ռ�ݷ�ҳҳ������ " << perfor_info.KernelNonpaged << " Pages" << endl;
		cout << "ϵͳ�ں��ڴ�ռ�ݲ���ҳҳ������ " << perfor_info.KernelPaged << " Pages" << endl;
		cout << "ϵͳ��������� " << perfor_info.HandleCount << " Pages" << endl;
		cout << "ϵͳ���������� " << perfor_info.ProcessCount << " Pages" << endl;
		cout << "ϵͳ�߳������� " << perfor_info.ThreadCount << " Pages" << endl;
		cout << endl;
		Sleep(1000);
	}
}


void ShowMemory()
{
	MEMORYSTATUS total;
	total.dwLength = sizeof(total);
	while (!kbhit())
	{
		//�õ���ǰ�����ڴ�������ڴ�
		GlobalMemoryStatus(&total);
		cout << "���ص��ڴ�: " << total.dwMemoryLoad << "%\n";
		cout << "�ܵ������ڴ�: " << total.dwTotalPhys / DIV << "MB\n";
		cout << "���������ڴ�: " << total.dwAvailPhys / DIV << "MB\n";
		cout << "�ܵ������ڴ�: " << (total.dwTotalVirtual / DIV) << "MB\n";
		cout << "���������ڴ�: " << (total.dwAvailVirtual / DIV) << "MB\n";
		cout << "�ܵ�ҳ�Ĵ�С: " << total.dwTotalPageFile / DIV << "MB\n";
		cout << "����ҳ��С: " << total.dwAvailPageFile / DIV << "MB\n";
		cout << endl;
		Sleep(1000);
	}
}


//���pid Ϊ-1����ȡ���н���
void ShowAllProcess(int pid)
{

	PROCESSENTRY32 pe32;  //�洢������Ϣ
	pe32.dwSize = sizeof(pe32); //��ʹ������ṹǰ�����������Ĵ�С
	PROCESS_MEMORY_COUNTERS ppsmemCounter;//struct,�洢�����ڴ��ʹ����Ϣ�������ú���GetProcessMemoryInfo��ȡ���̵������Ϣ
	ppsmemCounter.cb = sizeof(ppsmemCounter); //��ʼ����С
	HANDLE hProcessSnap;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //���վ��
	HANDLE hProcess;//���̾��
	//�������̿��գ�������ʾÿ�����̵���Ϣ
	BOOL bMore = Process32First(hProcessSnap, &pe32);  //��ȡϵͳ���յ�һ�����̵���Ϣ��������ص�pe32�ṹ��
	printf("���̵Ĺ�������Ϣ:\n");
	while (bMore)
	{
		if (pid != -1)
		{
			if (pid == pe32.th32ProcessID)
			{
				cout << "��������:" << pe32.szExeFile << endl;//������Ϣ���洢��pe32�У�
				cout << "����ID:" << pe32.th32ProcessID << endl;
				cout << "�߳���:" << pe32.cntThreads << endl;
				hProcess = GetProcessHandle(pe32.th32ProcessID);
				GetProcessMemoryInfo(hProcess, &ppsmemCounter, sizeof(ppsmemCounter));//�����ڴ�ʹ����Ϣ���洢��ppsmemCounter�У�
				cout << "���ύ:" << ppsmemCounter.PagefileUsage / 1024 << " KB" << endl;
				cout << "������:" << ppsmemCounter.WorkingSetSize / 1024 << " KB" << endl;
				cout << "��������ֵ:" << ppsmemCounter.PeakWorkingSetSize / 1024 << " KB" << endl;
				cout << endl;
			}
			bMore = Process32Next(hProcessSnap, &pe32);//��ȡϵͳ������һ��������Ϣ
		}
		else
		{
			cout << "��������:" << pe32.szExeFile << endl;//������Ϣ���洢��pe32�У�
			cout << "����ID:" << pe32.th32ProcessID << endl;
			cout << "�߳���:" << pe32.cntThreads << endl;
			hProcess = GetProcessHandle(pe32.th32ProcessID);
			GetProcessMemoryInfo(hProcess, &ppsmemCounter, sizeof(ppsmemCounter));//�����ڴ�ʹ����Ϣ���洢��ppsmemCounter�У�
			cout << "���ύ:" << ppsmemCounter.PagefileUsage / 1024 << " KB" << endl;
			cout << "������:" << ppsmemCounter.WorkingSetSize / 1024 << " KB" << endl;
			cout << "��������ֵ:" << ppsmemCounter.PeakWorkingSetSize / 1024 << " KB" << endl;
			cout << endl;
			bMore = Process32Next(hProcessSnap, &pe32);//��ȡϵͳ������һ��������Ϣ
		}
	}
	CloseHandle(hProcessSnap); //�رտ���
}


void QuerySingleProcess()
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //���վ��
	HANDLE hProcess;//���̾��
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot ����ʧ��.\n");
		exit(0);
	}
	cout << "�������ID����ѯ���̵��ڴ�ֲ��ռ䣺" << endl;
	int PID = 0;
	cin >> PID;
	hProcess = GetProcessHandle(PID);
	ShowAllProcess(PID);
	WalkVM(hProcess);
	Sleep(1000);
	CloseHandle(hProcess);//�رս���
}


int main()
{
	while (1)
	{
		int mode = 0;
		cout << "ģʽѡ��" << endl;
		cout << "1.ʵʱ��ʾ���������Ϣ" << endl;
		cout << "2.ʵʱ��ʾϵͳ�����Ϣ" << endl;
		cout << "3.ʵʱ��ʾ�ڴ��ʹ�����" << endl;
		cout << "4.��ѯ���н��̿�����Ϣ" << endl;
		cout << "5.��ѯ�������̿�����Ϣ" << endl;
		cout << "6.�˳�" << endl;
		cin >> mode;
		switch (mode)
		{
			case 1:	ShowProcessAddress(); break;
			case 2:	ShowPerformance(); break;
			case 3:	ShowMemory(); break;
			case 4:	ShowAllProcess(-1); break;
			case 5:	QuerySingleProcess(); break;
			case 6: return 0;
			default: cout << "�����ʽ����ȷ���������������" << endl;
		}
	}
	return 0;
}
