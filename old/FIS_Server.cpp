#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#define strvec vector<string>
using namespace std;
/*
* 14CS30011 : Hiware Kaustubh Narendra
* 14CS30017 : Surya Midatala
*/

/* File information server
* run with
* ./fis
*/

map<string, string> files;
// standard function to print all errors
void printerror(int x, const char* printmsg)
{
    if(x < 0)
    {
        fprintf(stderr, "+--- %s : ",printmsg );
        perror("");
        exit(0);
    }
}

void getFiles(const string& str, strvec& tokens, const string& delimiters = ":")
{
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}


int main()
{
    int s, l, n;
    struct sockaddr_in server, from;
    socklen_t addrsize = sizeof(struct sockaddr_in);
    char buff[1024];

    s = socket(AF_INET, SOCK_DGRAM, 0);
    printerror(s, "Error in opening socket");

    l = sizeof(server);
    bzero(&server,l);

    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(12500);
    int binder = bind(s,(struct sockaddr *)&server,l);
    printerror(binder, "Error in binding");

    printf("\nStarted server\n");
    while (1)
    {
        n = recvfrom(s,buff,1024,0,(struct sockaddr *)&from,&addrsize);
        printf("\nPeer communication over port %d@ %s\n", (int) ntohs(from.sin_port), inet_ntoa(from.sin_addr));
        printerror(n, "recvfrom");

        string temp(buff);
        string type = temp.substr(0,3);
        printf("type: %s\n", type.c_str());
        if (strcmp(type.c_str(),"REQ")==0)
        {
            string ip;
            string filestr(buff, n);
            string file = filestr.substr(3);
            cout << "Client wants to download " << file << endl;
            if (files.find(file) == files.end())
            {
                cout << "+--- Error: No such file exists" << endl;
                sprintf (buff, "-");
            }
            else strcpy(buff, files[file].c_str());
        }
        else if (strcmp(type.c_str(),"ADD")==0)
        {
            printf("Currently available files: \n");
            string fileliststr(buff, n);
            string flist = fileliststr.substr(3);
            strvec tokens;
            getFiles(flist, tokens);
            for (strvec::iterator it = tokens.begin() ; it != tokens.end(); ++it)
            {
                files[*it] = inet_ntoa(from.sin_addr);
            }

            printf("%s\n", buff);
            // show available content
            cout << "Identifying hostable files on the servers\n";
            map<string,string>::iterator it;
            for (it=files.begin(); it!=files.end(); ++it)
            {
                cout << it->first << " @ " << it->second << '\n';
            }
        }
        else if (strcmp(type.c_str(),"UPD")==0)
        {
            printf("Client has requested for file list\n");

            string f;
            map<string,string>::iterator it;
            for (it=files.begin(); it!=files.end(); ++it)
                f += it->first + ":";

            strcpy(buff,f.c_str());
        }
        n = sendto(s,buff,strlen(buff),0,(struct sockaddr *)&from,addrsize);
        printerror(n, "Error in sendto");
    }
}
