//Single File FTP Dumper

#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <filesystem>
#include <string>
#include "FTPClient.h"
#include <conio.h>
#using <System.dll>
#include <cstring>
#include <wininet.h>

#pragma comment(lib, "Wininet")

//Define Varibles And Use Namespaces
namespace fs = std::filesystem;
using namespace System;

void nointernet();
void getDir(std::string f);
void getDir2(std::string f);
int upload(std::filesystem::path p);
void mkDir(System::String^ _FileName, System::String^ _UploadPath, System::String^ _FTPUser, System::String^ _FTPPass);
void findAndReplaceAll(std::string& data, const std::string& match, const std::string& replace);

std::string get_current_dir();
std::string getexepath();

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

#define GetCurrentDir _getcwd

std::wstring s2ws(const std::string& s);

std::string homeDir;
std::string USERNAME;
std::string PASSWRD;
std::string ip;
std::string drive;
std::string driveM;

HINTERNET hInternet;
HINTERNET hFtpSession;

int main(int argc, char* argv[])
{ 
       //Make Sure All Arguments Are There
        if (sizeof(argv) <= 0) {
            std::cout << "Please Add Arguments To Command \n Usage:    Dumper <Drive Letter> <FTP USERNAME> <FTP PASSWORD> <FTP ip> <Folder To Save Dump To>" << std::endl;
            exit(1);
        }
        else {
            //Initalize Arguments Into Varibles
            drive = std::string(argv[1]);
            USERNAME = std::string(argv[2]);
            PASSWRD = std::string(argv[3]);
            ip = std::string(argv[4]);
            homeDir = std::string(argv[5]);

            //Modify Drive Letter
            driveM = drive;
            driveM.resize(driveM.size() - 2);

            //Give Wait 5 Seconds To Start
            SetConsoleTextAttribute(hConsole, 14);
            std::cout << "Starting Dump In 5 Seconds\n";
            Sleep(5000);
            
            //Open FTP Server Connection
            hInternet = InternetOpenA(NULL, 1, NULL, NULL, 0);
            hFtpSession = InternetConnectA(hInternet, ip.c_str(), 21, USERNAME.c_str(), PASSWRD.c_str(), 1, 0x08000000, 0);
            //Create Root Folder For FTP Dump
            FtpCreateDirectoryA(hFtpSession, homeDir.c_str());
            FtpSetCurrentDirectoryA(hFtpSession, homeDir.c_str());
            FtpCreateDirectoryA(hFtpSession, driveM.c_str());
            FtpSetCurrentDirectoryA(hFtpSession, driveM.c_str());

            //Start Dumping Files
            getDir(drive);

            //Close FTP Connection
            InternetCloseHandle(hFtpSession);
            InternetCloseHandle(hInternet);
        }
    
}

void getDir(std::string f) {
    //Define Iterator
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
    for (const auto& entry : fs::directory_iterator(f)) {
        try {
            //Check If Item In Question Is A Directory
            if (entry.is_directory()) {
                //UI Shit
                SetConsoleTextAttribute(hConsole, 14);
                std::cout << "DIR> " + entry.path().u8string() << std::endl;
                std::string dirN = entry.path().filename().u8string();
                SetConsoleTextAttribute(hConsole, 7);
                //Create The Directory
                FtpCreateDirectory(hFtpSession, dirN.c_str());
                FtpSetCurrentDirectory(hFtpSession, dirN.c_str());
                //Delay So It DOsnt Think You're A DDOS Attack
                Sleep(100);
                //Loop Through New Directory
                getDir(entry.path().u8string());
            }
            else {
                //Define Current Working Directory
                LPSTR cwd = new TCHAR[255];
                DWORD b = strtol("255", 0, 0);
                FtpGetCurrentDirectory(hFtpSession, cwd, &b);
                //UI SHit
                SetConsoleTextAttribute(hConsole, 7);
                std::cout << entry.path().u8string() << " To " << cwd << std::endl;
                //Call Upload Function
                upload(entry);
                //Delay So It DOsnt Think You're A DDOS Attack
                Sleep(100);
            }

        }
        //Catch Errors
        catch (const std::exception& e) {
            SetConsoleTextAttribute(hConsole, 12);
            std::cout << "Error: ", e;
            exit(0);
        }
    }
        //Back Out When You Finish Uploading Folder
        if (FtpSetCurrentDirectory(hFtpSession, "..")) {
            //UI Shit
            SetConsoleTextAttribute(hConsole, 10);
            std::cout << "Finished Uploading Files In Dir" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        else {
            //Ui Shit
            SetConsoleTextAttribute(hConsole, 12);
            std::cout << "Error Reversing Directory" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
}

//Upload Function
int upload(std::filesystem::path p)
{
    //Define Local Varibles
    std::string filename = p.filename().u8string();
    std::string localPath = p.u8string();

    std::wstring wLocalPath = s2ws(localPath);

    std::string onlinePath = localPath;
    
    //Upload File To FTP Server
    FtpPutFile(hFtpSession, localPath.c_str(), filename.c_str(), FTP_TRANSFER_TYPE_BINARY, 0);

    //UI Shit
    SetConsoleTextAttribute(hConsole, 10);
    std::cout << "File Uploaded." << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
    

    return 1;
}

//Swap std::string to std::wstring
std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

//find and replace all charictors in a std::string
void findAndReplaceAll(std::string& data, const std::string& match, const std::string& replace)
{
    size_t pos = data.find(match);
    while (pos != std::string::npos)
    {
        data.replace(pos, match.size(), replace);
        pos = data.find(match, pos + replace.size());
    }
}

std::string get_current_dir() {
    char buff[FILENAME_MAX]; 
    GetCurrentDir(buff, FILENAME_MAX);
    std::string current_working_dir(buff);
    return current_working_dir;
}

std::string getexepath()
{
    char result[MAX_PATH];
    return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}