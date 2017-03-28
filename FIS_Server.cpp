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
* 14CS30011 : Hiware Kaustubh Narendra
* 14CS30017 : Surya Midatala
*/

/* File information server
* run with
* ./fis port
*/

typedef pair<string, int> location;
typedef map<string, vector<location> > files;

void print_files(files &f)
{
    files::iterator it = f.begin();
    int i=0;
    location l;
    for( ; it != f.end(); it++)
    {
        cout<<endl<<it->first;
        for(i=0;i < it->second.size();++i)
        {
            l=it->second.at(i);
            cout << "\n" << l.first << " <port>" << l.second << endl;
        }
    }
    fflush(stdout);
}

// standard function to print all errors
void printerror(int x, const char* printmsg)
{
    if(x < 0)
    {
        fprintf(stderr, "+--- Error in %s : ",printmsg );
        perror("");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    files files_to_ip = files();
    int sockfd,fis_port;
    string s;
    socklen_t clilen;

    char buffer[MAXSIZE], message[MAXSIZE];
    int nbytes;
    char *token;
    struct sockaddr_in server_loc;
    struct sockaddr_in client_loc;
    int len=100;
    char buffer_2[len], buffer_3[len];

    if(argc < 2)
    {
        printf("Need to specify port! Use ./fis port\n");
    }
    else
    {
        fis_port = atoi(argv[1]);
    }

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    bzero((char*)&server_loc,sizeof(server_loc));
    server_loc.sin_family = AF_INET;
    server_loc.sin_addr.s_addr = INADDR_ANY;
    server_loc.sin_port = htons(fis_port);

    int binding = bind(sockfd,(struct sockaddr*)&server_loc,sizeof(server_loc)
    printerror(binding, "binding");

    listen(sockfd,5);
    while(1)
    {
        clilen=sizeof(client_loc);
        nbytes=recvfrom(sockfd,message,MAXSIZE,0,(struct sockaddr*)&client_loc,&clilen);
        printerror(nbytes, "recfrom");

    }
    inet_ntop(AF_INET,&(client_loc.sin_addr),buffer_2,len);
    printf("\nMESSAGE RECEIVED: %s\n",message);
    printf("\n\taddress:%s\n",buffer_2);
    printf("\n\tDatagram port:%d\n",ntohs((client_loc.sin_port)));
    fflush(stdout);
    if(message[0]=='S'&&message[1]=='H'){
      printf("\nNew peer connected to share stuff\n");
      fflush(stdout);
      files::iterator it=files_to_ip.begin();
      token=strtok(message+6," ");
      location incoming_location=make_pair(string(buffer_2),atoi(token));
      token=strtok(NULL,"\n");
      if(token==NULL){
        printf("\nWrong Format");
        continue;
        //exit(1);
      }
      s=string(token);
      it=files_to_ip.find(s);
      if(it!=files_to_ip.end()){
        it->second.push_back(incoming_location);
      }
      else{
        files::iterator it=files_to_ip.begin();
        vector<location> x;
        x.push_back(incoming_location);
        pair<string,vector<location> > bar=make_pair(s,x);
        it=files_to_ip.insert(it,bar);
      }
      while((token=strtok(NULL,"\n"))!=NULL){
        s=string(token);
        it=files_to_ip.find(s);
        if(it!=files_to_ip.end()){
          it->second.push_back(incoming_location);
        }
        else{
          it=files_to_ip.begin();
          vector<location> x;
          x.push_back(incoming_location);
          pair<string,vector<location> > bar=make_pair(s,x);
          it=files_to_ip.insert(it,bar);
        }
      }
      print_files(files_to_ip);
      strcpy(buffer,"Message Received");
    }
    else if(message[0]=='R'&&message[1]=='E'){
      s=string(message+8);
      files::iterator it=files_to_ip.begin();
      it=files_to_ip.find(s);
      if(it!=files_to_ip.end()){
        location bar=it->second.at(it->second.size()-1);
        strcpy(buffer,"SUCCESS ");
        strcpy(buffer_2,bar.first.c_str());
        strcat(buffer_2," ");
        sprintf(buffer_3,"%d",bar.second);
        strcat(buffer_2,buffer_3);
        strcat(buffer,buffer_2);
      }
      else{
        strcpy(buffer,"FAIL");
      }
    }
    fflush(stdout);
    nbytes=sendto(sockfd,buffer,MAXSIZE,0,(struct sockaddr*)&client_loc,clilen);
    if(nbytes<0){
      perror("sendto (server)");
    }
  }
  return 0;
}
