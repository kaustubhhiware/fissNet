#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <utility>
#include <string>
#include <iostream>
#include <vector>
#include <map>
using namespace std;
#define MAXSIZE 512

/*
* Written by kingofools (Surya) and kaustubhhiware
* as a part of Networks Lab
*/

/* File information server
* run with
* ./fis port
*/

typedef pair<string, int> location;
typedef map<string, vector<location> > files;

// standard function to print all errors
void printerror(int x, const char* printmsg)
{
    if(x < 0)
    {
        fprintf(stderr, "+--- Error in %s : ",printmsg );
        perror("");
        exit(0);
    }
}


void print_files(files &file_list)
{
    files::iterator iter;
    int i=0;
    location loc;
    for(iter = file_list.begin(); iter != file_list.end(); iter++)
    {
        cout << endl << iter->first;
        for(i=0;i < iter->second.size();++i)
        {
            loc=iter->second.at(i);
            cout << " hosted on " << loc.first << "<port>" << loc.second << endl;
        }
    }
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    files files_to_ip = files();
    int sockfd,server_port;
    string s;
    socklen_t clilen;

    char buff[MAXSIZE], msg[MAXSIZE];
    int num_bytes;
    char *tok;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    int len = 512;
    char buff_2[len];
    char buff_3[len];

    if(argc < 2)
    {
        printf("+--- Need to specify ip! Use - ./fis port\n");
        exit(0);
    }
    else
    {
        server_port=atoi(argv[1]);
        if(server_port < 10000 || server_port > 15000)
        {
            printf("+--- Need to specify port between 10000 and 15000 !\n");
            exit(0);
        }
    }

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    bzero((char*)&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(server_port);

    int binder = bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    printerror(binder, "binding");

    listen(sockfd,5);
    while(1)
    {
        clilen=sizeof(client_addr);
        num_bytes=recvfrom(sockfd,msg,MAXSIZE,0,(struct sockaddr*)&client_addr,&clilen);
        printerror(num_bytes, "recfrom");

        // # af, src, dest, size
        inet_ntop(AF_INET,&(client_addr.sin_addr),buff_2,len);
        printf("\n%s %s\n",buff_2, msg);
        printf(" over Datagram port:%d\n",ntohs((client_addr.sin_port)));
        fflush(stdout);

        if(msg[0]=='S' && msg[1]=='H') // new files have been updated
        {
            printf("\nNew server now functional\n");
            fflush(stdout);
            files::iterator iter=files_to_ip.begin();

            tok = strtok(msg + 6, " ");
            location incoming_location=make_pair(string(buff_2),atoi(tok));
            tok = strtok(NULL,"\n");
            if(tok==NULL)
            {
                printf("Incorrect request\n");
                continue;
            }

            s = string(tok);
            iter = files_to_ip.find(s);
            if(iter!=files_to_ip.end())
            {
                // if exists, ass this file
                iter->second.push_back(incoming_location);
            }
            else
            {
                // if this is new, add as new server
                files::iterator iter = files_to_ip.begin();
                vector<location> x;
                x.push_back(incoming_location);
                pair<string,vector<location> > bar=make_pair(s,x);
                iter=files_to_ip.insert(iter,bar);
            }

            while((tok=strtok(NULL,"\n"))!=NULL)
            {
                s=string(tok);
                iter=files_to_ip.find(s);
                if(iter != files_to_ip.end())
                {
                    iter->second.push_back(incoming_location);
                }
                else{
                    iter=files_to_ip.begin();
                    vector<location> x;
                    x.push_back(incoming_location);
                    pair<string,vector<location> > bar=make_pair(s,x);
                    iter=files_to_ip.insert(iter,bar);
                }
            }
            print_files(files_to_ip);
            strcpy(buff,"Message Received");
        }
        else if(msg[0]=='R' && msg[1]=='E') // client requests for a file
        {
            s=string(msg+8);
            files::iterator iter=files_to_ip.begin();

            iter=files_to_ip.find(s);
            if(iter != files_to_ip.end())
            {
                location bar=iter->second.at(iter->second.size()-1);
                strcpy(buff,"SUCCESS ");
                strcpy(buff_2,bar.first.c_str());
                strcat(buff_2," ");
                sprintf(buff_3,"%d",bar.second);
                strcat(buff_2,buff_3);
                strcat(buff,buff_2);
            }
            else{
                strcpy(buff,"FAIL");
            }
        }
        else if(msg[0]=='I' && msg[1]=='N') // client asks for file list
        {
            s = string(msg + 6);
            files::iterator iter;
            strcpy(buff, "\n+--- Files available : \n");
            for(iter = files_to_ip.begin(); iter != files_to_ip.end(); iter++)
            {
                strcat(buff, iter->first.c_str());
                strcat(buff, "\n");
            }

        }
        fflush(stdout);
        num_bytes=sendto(sockfd,buff,MAXSIZE,0,(struct sockaddr*)&client_addr,clilen);
        printerror(num_bytes, "recfrom");
    }
}
