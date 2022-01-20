#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;

int main()
{
    const DWORD dwMemoryFileSize = 4 * 1024;            // 指定内存映射文件大小
    wstring strMapName(L"MyShareMemory");               // 内存映射对象名称
    
    wstring strComData(L"This is common data!");        // 共享内存中的数据
    LPVOID pBuffer;                                     // 共享内存指针

    HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, strMapName.c_str());
    if (NULL == hMap) {    
        hMap = CreateFileMapping(INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            dwMemoryFileSize,
            strMapName.c_str());

        // 映射对象的一个视图，得到指向共享内存的指针，设置里面的数据
        pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        lstrcpy((LPTSTR)pBuffer, strComData.c_str());

        wcout << "写入共享内存数据：" << (TCHAR*)pBuffer << endl;
    } else {    
        // 打开成功，映射对象的一个视图，得到指向共享内存的指针，显示出里面的数据
        pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        wcout << "读取共享内存数据：" << (TCHAR*)pBuffer << endl;
    }

    getchar();

    // 解除文件映射，关闭内存映射文件对象句柄
    UnmapViewOfFile(pBuffer);
    CloseHandle(hMap);
    system("pause");

    return 0;
}