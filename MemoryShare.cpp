#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;

HANDLE OpenSharedEvent(wstring strEventName)
{
    HANDLE hEvent = OpenEvent(
        EVENT_ALL_ACCESS,   // 进程间事件指定该标志位
        NULL, strEventName.c_str());
    if (!hEvent) {
        wcout << "event("<< strEventName<<") not exist, create it..." << endl;
        hEvent = CreateEvent(NULL,
            true,   // 手动置位(ResetEvent)
            false,  // 初始状态为无信号状态
            strEventName.c_str());
    } else {
        wcout << "event(" << strEventName << ") exist" << endl;
    }
    if (!hEvent) {
        DWORD errorMessageID = ::GetLastError();
        wcout << "event(" << strEventName << ") is null, error id: " << errorMessageID << endl;
        return nullptr;
    }
    
    return hEvent;
}

LPVOID OpenSharedMemory(wstring strMemoryName, const DWORD dwMemorySize, bool& isCreate)
{
    isCreate = false;
    HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, strMemoryName.c_str());
    if (!hMap) {
        wcout << "shared memory(" << strMemoryName << ") not exist, create it..." << endl;
        hMap = CreateFileMapping(INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            dwMemorySize,
            strMemoryName.c_str());
        if (!hMap) {
            DWORD errorMessageID = ::GetLastError();
            cout << "file map is null, error id: " << errorMessageID << endl;
            return nullptr;
        }
        isCreate = true;
    } else {
        wcout << "shared memory(" << strMemoryName << ") exist" << endl;
    }
    return hMap;
}

int main()
{
    const DWORD dwMemoryFileSize = 4 * 1024;            // 指定内存映射文件大小
    wstring strMapName(L"MyShareMemory");               // 内存映射对象名称
    wstring strReadEventName(L"MyShareMemoryReadEvent");
    wstring strWriteEventName(L"MyShareMemoryWriteEvent");
    wstring strComData(L"This 2222222222222!");        // 共享内存中的数据
    LPVOID pBuffer;                                     // 共享内存指针
    bool isCreate;
    DWORD pid = GetCurrentProcessId();

    HANDLE hReadEvent = OpenSharedEvent(strReadEventName);
    if (!hReadEvent) {
        return -1;
    }
    HANDLE hWriteEvent = OpenSharedEvent(strWriteEventName);
    if (!hWriteEvent) {
        return -1;
    }

    HANDLE hMap = OpenSharedMemory(strMapName, dwMemoryFileSize, isCreate);
    if (!hMap) {
        return -1;
    }
    
    if (isCreate) {
        // 写数据
        cout << "[" << pid << "]wait event to write ..." << endl;
        //WaitForSingleObject(hReadEvent, INFINITE);
        //WaitForSingleObject(hWriteEvent, INFINITE);

        pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        lstrcpy((LPTSTR)pBuffer, strComData.c_str());
        Sleep(10 * 1000);
        cout << "[" << pid << "]write over" << endl;
        SetEvent(hWriteEvent);
        SetEvent(hReadEvent);

        wcout << "[" << pid << "]写入共享内存数据：" << strComData << endl;
    }
    else {
        // 读数据
        wstring readData;
        readData.resize(dwMemoryFileSize);
        cout << "[" << pid << "]wait event to read ..." << endl;
        WaitForSingleObject(hWriteEvent, INFINITE);
        
        pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        lstrcpy((LPTSTR)readData.c_str(), (LPTSTR)pBuffer);
        cout << "[" << pid << "]read over" << endl;
        SetEvent(hReadEvent);

        wcout << "[" << pid << "]读取共享内存数据：" << (TCHAR*)pBuffer << endl;
    }

    // 解除文件映射，关闭内存映射文件对象句柄
    UnmapViewOfFile(pBuffer);
    CloseHandle(hMap);
    system("pause");

    return 0;
}