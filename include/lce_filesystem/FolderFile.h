#pragma once

#include <vector>
#include <unordered_map>
#include <windows.h>

#include "../Minecraft.World/ArrayWithLength.h"

using namespace std;

class FolderFile
{
private:
    wstring m_folderPath;
    unordered_map<wstring, wstring> m_filePaths; // filename -> full path
    
    void _buildFileIndex();
    void _buildFileIndexRecursive(const wstring& currentPath, const wstring& relativePath);
    
public:
    FolderFile(wstring folderPath);
    ~FolderFile();

    vector<wstring>* getFileList();
    bool hasFile(const wstring &filename);
    int getFileSize(const wstring &filename);
    byteArray getFile(const wstring &filename);
};
