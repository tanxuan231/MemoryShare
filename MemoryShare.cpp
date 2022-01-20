#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;

int main()
{
    const DWORD dwMemoryFileSize = 4 * 1024;            // 指定内存映射文件大小
    wstring strMapName(L"MyShareMemory");               // 内存映射对象名称
    wstring strEventName(L"MyShareMemoryEvent");
    
    HANDLE hEvent = OpenEvent(
        EVENT_ALL_ACCESS,   // 进程间事件指定该标志位
        NULL, strEventName.c_str());
    if (!hEvent) {
        cout << "event not exist, create it..." << endl;
        hEvent = CreateEvent(NULL,
            true,   // 手动置位(ResetEvent)
            false,  // 初始状态为无信号状态
            strEventName.c_str());
    } else {
        cout << "event exist" << endl;
    }
    if (!hEvent) {
        DWORD errorMessageID = ::GetLastError();
        cout << "event is null, error id: " << errorMessageID << endl;
        return -1;
    }

    wstring strComData(L"This is common data!");        // 共享内存中的数据
    LPVOID pBuffer;                                     // 共享内存指针

    HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, strMapName.c_str());
    if (!hMap) {    
        hMap = CreateFileMapping(INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            dwMemoryFileSize,
            strMapName.c_str());
        if (!hMap) {
            DWORD errorMessageID = ::GetLastError();
            cout << "file map is null, error id: "<< errorMessageID << endl;
            return -1;
        }

        Sleep(5*1000);
        cout << "wait event to write ..." << endl;        
        pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        lstrcpy((LPTSTR)pBuffer, strComData.c_str());
        cout << "write over" << endl;        
        SetEvent(hEvent);

        wcout << "写入共享内存数据：" << (TCHAR*)pBuffer << endl;
    } else {
        
        wstring readData;
        readData.resize(dwMemoryFileSize);
        cout << "wait event to read ..." << endl;
        WaitForSingleObject(hEvent, INFINITE);
        pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        lstrcpy((LPTSTR)readData.c_str(), (LPTSTR)pBuffer);        
        cout << "read over" << endl;        

        wcout << "读取共享内存数据：" << (TCHAR*)pBuffer << endl;
    }

    getchar();

    // 解除文件映射，关闭内存映射文件对象句柄
    UnmapViewOfFile(pBuffer);
    CloseHandle(hMap);
    system("pause");

    return 0;
}