#include <windows.h>

int main()
{
    HANDLE heap = GetProcessHeap();
    char* pool = (char*) HeapAlloc(heap, HEAP_ZERO_MEMORY, 3);
    pool[0] = 'H';
    pool[1] = 'i';
    pool[2] = '\0';
    HANDLE stdout = GetStdHandle(-11);
    WriteFile(stdout, pool, 3, NULL, NULL);
    HeapFree(heap, 0x00, (void*) pool);
}