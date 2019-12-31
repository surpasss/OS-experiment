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

//显示保护标记，该标记表示允许应用程序对内存进行访问的类型
inline bool TestSet(DWORD dwTarget, DWORD dwMask)
{
	return ((dwTarget &dwMask) == dwMask);
}


#define SHOWMASK(dwTarget, type) \
if (TestSet(dwTarget, PAGE_##type))\
{cout<<", "<<# type;}

void ShowProtection(DWORD dwTarget)
{
	//定义的页面保护方式
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


//遍历整个虚拟内存，显示单个进程虚拟地址空间布局
void WalkVM(HANDLE hProcess)
{
	SYSTEM_INFO si;                     //系统信息结构
	ZeroMemory(&si, sizeof(si));		//初始化
	GetSystemInfo(&si);                 //获得系统信息
	MEMORY_BASIC_INFORMATION mbi;       //进程虚拟内存空间的基本信息结构
	ZeroMemory(&mbi, sizeof(mbi));      //分配缓冲区，用于保存信息				
	LPCVOID pBlock = (LPVOID)si.lpMinimumApplicationAddress; //循环整个应用程序地址空间
	while (pBlock < si.lpMaximumApplicationAddress)
	{
		//获得下一个虚拟内存块的信息
		if (VirtualQueryEx(hProcess,           //进程句柄
			pBlock,                            //开始位置
			&mbi,                              //缓冲区
			sizeof(mbi)) == sizeof(mbi))       //长度的确认，如果失败则返回0
		{
			//块的结尾指针
			LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;
			TCHAR szSize[MAX_PATH];
			StrFormatByteSize(mbi.RegionSize, szSize, MAX_PATH);

			//显示块地址和长度
			cout.fill('0');
			cout << hex << setw(8) << (DWORD*)pBlock
				<< "--"
				<< hex << setw(8) << (DWORD*)pEnd
				<< "  " << szSize;
			
			//显示块的状态
			switch (mbi.State) 
			{
				case MEM_COMMIT:
					printf("   Committed"); break;
				case MEM_FREE:
					printf("   Free"); break;
				case MEM_RESERVE:
					printf("   Reserved"); break;
			}

			//显示保护
			if (mbi.Protect == 0 && mbi.State != MEM_FREE) 
			{
				mbi.Protect = PAGE_READONLY;
			}
			ShowProtection(mbi.Protect);

			//显示类型
			switch (mbi.Type) 
			{
				case MEM_IMAGE:
					printf(", Image"); break;
				case MEM_MAPPED:
					printf(", Mapped"); break;
				case MEM_PRIVATE:
					printf(", Private"); break;
			}

			//检测可执行的映像
			TCHAR szFilename[MAX_PATH];
			if (GetModuleFileName(
				(HMODULE)pBlock,        //实际虚拟内存的模块句柄
				szFilename,             //完全指定的文件名称
				MAX_PATH) > 0)            //实际使用的缓冲区长度
			{
				//除去路径并显示
				PathStripPath(szFilename);
				printf("%s", szFilename);
			}

			printf("\n");
			//移动块指针以获得下一个块
			pBlock = pEnd;
		}
	}
	cout << endl;
}


//关于当前系统的信息
void ShowProcessAddress()
{
	SYSTEM_INFO sys_info;  //系统信息结构
	ZeroMemory(&sys_info, sizeof(sys_info));//初始化
	while (!kbhit())
	{
		//获得系统信息
		GetSystemInfo(&sys_info);
		printf("虚拟内存分页大小: %d KB\n", sys_info.dwPageSize / 1024);
		printf("处理器总数: %d\n", sys_info.dwNumberOfProcessors);
		printf("处理器架构: %d\n", sys_info.dwProcessorType);
		printf("虚拟内存粒度: %d KB\n", sys_info.dwAllocationGranularity / 1024);
		printf("体系结构相关的处理器等级: %d\n", sys_info.wProcessorLevel);
		printf("体系结构相关的处理器修订: %x\n", sys_info.wProcessorRevision);
		printf("应用最小地址: 0x%0.8x\n", sys_info.lpMinimumApplicationAddress);
		printf("应用最大地址: 0x%0.8x\n", sys_info.lpMaximumApplicationAddress);
		printf("应用可用虚拟内存大小: %0.2f GB\n", ((DWORD*)sys_info.lpMaximumApplicationAddress
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
		cout << "分页大小: " << perfor_info.PageSize / 1024 << "KB" << endl;
		cout << "系统提交的页面总数: " << perfor_info.CommitTotal << " Pages" << endl;
		cout << "系统提交的页面限制: " << perfor_info.CommitLimit << " Pages" << endl;
		cout << "系统提交的页面峰值: " << perfor_info.CommitPeak << " Pages" << endl;
		cout << "按页分配的物理内存总数: " << perfor_info.PhysicalTotal << " Pages" << endl;
		cout << "按页分配的物理内存可用量: " << perfor_info.PhysicalAvailable << " Pages" << endl;
		cout << "系统物理内存占用: " << (perfor_info.PhysicalTotal - perfor_info.PhysicalAvailable)*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "系统物理内存可用: " << perfor_info.PhysicalAvailable*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "系统物理内存总数: " << perfor_info.PhysicalTotal*(perfor_info.PageSize / 1024)*1.0 / DIV << "GB" << endl;
		cout << "系统缓存总量： " << perfor_info.PhysicalAvailable << " Pages" << endl;
		cout << "系统内核内存占据页面总数： " << perfor_info.KernelTotal << " Pages" << endl;
		cout << "系统内核内存占据分页页面数： " << perfor_info.KernelNonpaged << " Pages" << endl;
		cout << "系统内核内存占据不分页页面数： " << perfor_info.KernelPaged << " Pages" << endl;
		cout << "系统句柄总量： " << perfor_info.HandleCount << " Pages" << endl;
		cout << "系统进程总量： " << perfor_info.ProcessCount << " Pages" << endl;
		cout << "系统线程总量： " << perfor_info.ThreadCount << " Pages" << endl;
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
		//得到当前物理内存和虚拟内存
		GlobalMemoryStatus(&total);
		cout << "加载的内存: " << total.dwMemoryLoad << "%\n";
		cout << "总的物理内存: " << total.dwTotalPhys / DIV << "MB\n";
		cout << "可用物理内存: " << total.dwAvailPhys / DIV << "MB\n";
		cout << "总的虚拟内存: " << (total.dwTotalVirtual / DIV) << "MB\n";
		cout << "可用虚拟内存: " << (total.dwAvailVirtual / DIV) << "MB\n";
		cout << "总的页的大小: " << total.dwTotalPageFile / DIV << "MB\n";
		cout << "可用页大小: " << total.dwAvailPageFile / DIV << "MB\n";
		cout << endl;
		Sleep(1000);
	}
}


//如果pid 为-1，获取所有进程
void ShowAllProcess(int pid)
{

	PROCESSENTRY32 pe32;  //存储进程信息
	pe32.dwSize = sizeof(pe32); //在使用这个结构前，先设置它的大小
	PROCESS_MEMORY_COUNTERS ppsmemCounter;//struct,存储进程内存的使用信息，便于用函数GetProcessMemoryInfo获取进程的相关信息
	ppsmemCounter.cb = sizeof(ppsmemCounter); //初始化大小
	HANDLE hProcessSnap;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //快照句柄
	HANDLE hProcess;//进程句柄
	//遍历进程快照，轮流显示每个进程的信息
	BOOL bMore = Process32First(hProcessSnap, &pe32);  //获取系统快照第一个进程的信息，结果返回到pe32结构里
	printf("进程的工作集信息:\n");
	while (bMore)
	{
		if (pid != -1)
		{
			if (pid == pe32.th32ProcessID)
			{
				cout << "进程名称:" << pe32.szExeFile << endl;//进程信息（存储于pe32中）
				cout << "进程ID:" << pe32.th32ProcessID << endl;
				cout << "线程数:" << pe32.cntThreads << endl;
				hProcess = GetProcessHandle(pe32.th32ProcessID);
				GetProcessMemoryInfo(hProcess, &ppsmemCounter, sizeof(ppsmemCounter));//进程内存使用信息（存储于ppsmemCounter中）
				cout << "已提交:" << ppsmemCounter.PagefileUsage / 1024 << " KB" << endl;
				cout << "工作集:" << ppsmemCounter.WorkingSetSize / 1024 << " KB" << endl;
				cout << "工作集峰值:" << ppsmemCounter.PeakWorkingSetSize / 1024 << " KB" << endl;
				cout << endl;
			}
			bMore = Process32Next(hProcessSnap, &pe32);//获取系统快照下一个进程信息
		}
		else
		{
			cout << "进程名称:" << pe32.szExeFile << endl;//进程信息（存储于pe32中）
			cout << "进程ID:" << pe32.th32ProcessID << endl;
			cout << "线程数:" << pe32.cntThreads << endl;
			hProcess = GetProcessHandle(pe32.th32ProcessID);
			GetProcessMemoryInfo(hProcess, &ppsmemCounter, sizeof(ppsmemCounter));//进程内存使用信息（存储于ppsmemCounter中）
			cout << "已提交:" << ppsmemCounter.PagefileUsage / 1024 << " KB" << endl;
			cout << "工作集:" << ppsmemCounter.WorkingSetSize / 1024 << " KB" << endl;
			cout << "工作集峰值:" << ppsmemCounter.PeakWorkingSetSize / 1024 << " KB" << endl;
			cout << endl;
			bMore = Process32Next(hProcessSnap, &pe32);//获取系统快照下一个进程信息
		}
	}
	CloseHandle(hProcessSnap); //关闭快照
}


void QuerySingleProcess()
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //快照句柄
	HANDLE hProcess;//进程句柄
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot 调用失败.\n");
		exit(0);
	}
	cout << "输入进程ID，查询进程的内存分布空间：" << endl;
	int PID = 0;
	cin >> PID;
	hProcess = GetProcessHandle(PID);
	ShowAllProcess(PID);
	WalkVM(hProcess);
	Sleep(1000);
	CloseHandle(hProcess);//关闭进程
}


int main()
{
	while (1)
	{
		int mode = 0;
		cout << "模式选择" << endl;
		cout << "1.实时显示进程相关信息" << endl;
		cout << "2.实时显示系统相关信息" << endl;
		cout << "3.实时显示内存的使用情况" << endl;
		cout << "4.查询所有进程控制信息" << endl;
		cout << "5.查询单个进程控制信息" << endl;
		cout << "6.退出" << endl;
		cin >> mode;
		switch (mode)
		{
			case 1:	ShowProcessAddress(); break;
			case 2:	ShowPerformance(); break;
			case 3:	ShowMemory(); break;
			case 4:	ShowAllProcess(-1); break;
			case 5:	QuerySingleProcess(); break;
			case 6: return 0;
			default: cout << "输入格式不正确，请重新输出数字" << endl;
		}
	}
	return 0;
}
