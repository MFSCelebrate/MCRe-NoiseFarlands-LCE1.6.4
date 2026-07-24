#include "stdafx.h"

#include "../Minecraft.World/StringHelpers.h"
#include "../Minecraft.World/compression.h"

#include "FolderFile.h"

void FolderFile::_buildFileIndex()
{
    _buildFileIndexRecursive(m_folderPath, L"");
}

void FolderFile::_buildFileIndexRecursive(const wstring& currentPath, const wstring& relativePath)
{
    wstring searchPath = currentPath + L"\\*";
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        app.DebugPrintf("Failed to open folder: %ls\n", m_folderPath.c_str());
        return;
    }

    do
    {
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
            continue;

        wstring filename = findData.cFileName;
        wstring fullRelativePath = relativePath.empty() ? filename : relativePath + L"\\" + filename;
        wstring fullPath = currentPath + L"\\" + filename;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            _buildFileIndexRecursive(fullPath, fullRelativePath);
        }
        else
        {
            m_filePaths[fullRelativePath] = fullPath;
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    
    app.DebugPrintf("Indexed %d files from folder\n", (int)m_filePaths.size());
}

FolderFile::FolderFile(wstring folderPath)
{
    m_folderPath = folderPath;
    app.DebugPrintf("Loading folder file...\n");
    
#ifndef _CONTENT_PACKAGE
    char buf[256];
    wcstombs(buf, folderPath.c_str(), 256);
    app.DebugPrintf("folder path - %s\n", buf);
#endif

    // Check if folder exists
    DWORD attrib = GetFileAttributesW(folderPath.c_str());
    if (attrib == INVALID_FILE_ATTRIBUTES || !(attrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        app.DebugPrintf("Failed to load folder - directory doesn't exist!\n");
        app.FatalLoadError();
    }

    _buildFileIndex();
    app.DebugPrintf("Finished loading folder file\n");
}

FolderFile::~FolderFile()
{
    
}

vector<wstring>* FolderFile::getFileList()
{
    vector<wstring>* out = new vector<wstring>();
    
    for (const auto& it : m_filePaths)
    {
        out->push_back(it.first);
    }

    return out;
}

bool FolderFile::hasFile(const wstring &filename)
{
    return m_filePaths.find(filename) != m_filePaths.end();
}

int FolderFile::getFileSize(const wstring &filename)
{
    auto it = m_filePaths.find(filename);
    if (it == m_filePaths.end())
    {
        return -1;
    }

    WIN32_FILE_ATTRIBUTE_DATA fileData;
    if (GetFileAttributesExW(it->second.c_str(), GetFileExInfoStandard, &fileData))
    {
        return (int)fileData.nFileSizeLow;
    }

    return -1;
}

byteArray FolderFile::getFile(const wstring &filename)
{
    byteArray out;
    auto it = m_filePaths.find(filename);

    if (it == m_filePaths.end())
    {
        app.DebugPrintf("Couldn't find file in folder\n");
        app.DebugPrintf("Failed to find file '%ls' in folder\n", filename.c_str());
#ifndef _CONTENT_PACKAGE
        __debugbreak();
#endif
        app.FatalLoadError();
        return out;
    }

    HANDLE hFile = CreateFileW(
        it->second.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        app.DebugPrintf("Failed to open file: %ls\n", it->second.c_str());
        app.FatalLoadError();
        return out;
    }

    DWORD fileSize = GetFileSize(hFile, nullptr);
    if (fileSize == INVALID_FILE_SIZE)
    {
        app.DebugPrintf("Failed to get file size: %ls\n", it->second.c_str());
        CloseHandle(hFile);
        app.FatalLoadError();
        return out;
    }

    PBYTE pData = new BYTE[fileSize];
    DWORD bytesRead = 0;

    BOOL success = ReadFile(hFile, pData, fileSize, &bytesRead, nullptr);
    CloseHandle(hFile);

    if (!success || bytesRead != fileSize)
    {
        app.DebugPrintf("Failed to read file: %ls\n", it->second.c_str());
        delete[] pData;
        app.FatalLoadError();
        return out;
    }

    out = byteArray(pData, fileSize);

    // Compressed filenames are preceeded with an asterisk.
    if (filename[0] == L'*' && out.data != nullptr)
    {
        /* 4J-JEV:
        * If a compressed file is accessed before compression object is 
        * initialized it will crash here (Compression::getCompression).
        */
        ///4 279 553 556

        ByteArrayInputStream bais(out);
        DataInputStream dis(&bais);
        unsigned int decompressedSize = dis.readInt();
        dis.close();

        PBYTE uncompressedBuffer = new BYTE[decompressedSize];
        Compression::getCompression()->Decompress(uncompressedBuffer, &decompressedSize, out.data + 4, out.length - 4);

        delete[] out.data;

        out.data = uncompressedBuffer;
        out.length = decompressedSize;
    }

    return out;
}
