/*
http_upload by sincoder
www.sincoder.com
*/
#include <Windows.h>
#include <WinInet.h>
#include <stdio.h>
#pragma comment( lib,"Wininet.lib")

DWORD LoadFileIntoMemery(WCHAR *FileName,LPBYTE *pBuffer)
{
    DWORD bytes = 0;
    HANDLE hfile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    if(INVALID_HANDLE_VALUE != hfile)
    {
        DWORD filesize = GetFileSize(hfile,NULL);
        if(filesize)
        {
            *pBuffer = (LPBYTE)malloc(filesize);
            if(*pBuffer)
            {
                ReadFile(hfile,*pBuffer,filesize,&bytes,NULL);
                if(bytes)
                {

                }
                else
                {
                    free(*pBuffer);
                    *pBuffer = NULL;
                }
            }
        }
        CloseHandle(hfile);
    }
    return bytes;
}

int wmain(int argc,WCHAR** argv)
{
    HINTERNET hSession;
    WCHAR *filename = NULL;
    LPBYTE filedata = NULL;
    DWORD  filesize = 0;
    WCHAR  http_host[256]={0};
    WCHAR  http_req[256]={0};
    WCHAR *p;
    if(argc < 3)
    {
        printf("Usage:http_upload.exe url filename \n");
        return -1;
    }
    filesize = LoadFileIntoMemery(argv[2],&filedata);
    if(0 == filesize)
    {
         printf("can not open file !! \n");
         return -2;
    }
    filename = wcsrchr(argv[2],L'\\');
    if(filename)
        filename++;
    else
        filename = argv[2];
    if(0 == wcsnicmp(argv[1],L"http://",7))
    {
        wcsncpy(http_host,&argv[1][7],256);
    }
    else
        wcsncpy(http_host,&argv[1][0],256);
    p = wcschr(http_host,'/');
    if(p)
    {
        wcsncpy(http_req,p,256);
        *p = 0;
    }
    else
    {
        wcscpy(http_req,L"/");
    }

    hSession = InternetOpenW(L"Mozilla/5.0",INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(hSession)
    {
        HINTERNET hConnect = InternetConnect(hSession, 
            http_host,
            INTERNET_DEFAULT_HTTP_PORT, 
            NULL, NULL, 
            INTERNET_SERVICE_HTTP, 0, 0);
        if(hConnect)
        {
            PCTSTR rgpszAcceptTypes[] = {L"text/html", NULL};
            HINTERNET hRequest = HttpOpenRequest(hConnect, 
                L"POST",
                http_req, 
                NULL, 
                NULL, rgpszAcceptTypes, 
                INTERNET_FLAG_NO_UI, 0);
            if(hRequest)
            {
                WCHAR *strBoundary = L"---------------------------2552205232316";
                char  *strLastBoundary = "\r\n-----------------------------2552205232316--\r\n";
                WCHAR strAppendHeader[256];
                wcscpy(strAppendHeader,L"Content-Type: multipart/form-data; boundary=");
                wcscat(strAppendHeader,strBoundary);
                if(HttpAddRequestHeadersW(hRequest,
                    strAppendHeader,
                    -1,
                    HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE))
                {
                    int len = filesize + 1024;
	                LPBYTE RequestData = (LPBYTE)malloc(len);
					if(RequestData)
					{
                        int count = _snprintf(RequestData,1024,"--%S\r\n"
                            "Content-Disposition: form-data; name=\"file\"; "
                            "filename=\"%S\"\r\n"
                            "Content-Type: application/octet-stream\r\n\r\n",
                            strBoundary,
                            filename);
                        memcpy(RequestData + count,filedata,filesize);
                        memcpy(RequestData + count + filesize,strLastBoundary,strlen(strLastBoundary));
                        len = count + filesize + strlen(strLastBoundary);
                        if(HttpSendRequestW(hRequest,NULL,0,RequestData,len))
                        {
                            printf("send request to %S OK ! \n",http_host);
                        }
                        else
                        {
                            printf("failed to send request !! \n");
                        }
						free(RequestData);
					}
                }
                else
                {

                }
                InternetCloseHandle(hRequest);
            }
            InternetCloseHandle(hConnect);
        }
        else
        {
            printf("connect to host %S failed !! \n",http_host);
        }
        InternetCloseHandle(hSession);
    }
    free(filedata);
    return 0;
}
