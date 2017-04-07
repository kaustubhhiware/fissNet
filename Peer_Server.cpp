#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <utility>
#include <string>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <netinet/in.h>
#include <netdb.h>
#include <map>

#define MAXSIZE 512
#define CHUNKSIZE 32
using namespace std;

/*
* Written by kingofools (Surya) and kaustubhhiware
* as a part of Networks Lab
*/

/* Transfer server
* run with
* ./server fis_ip port
*/

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

void my_handler(int n)
{
    pid_t t = waitpid(-1, NULL, WNOHANG);
    printf("\nTransfer complete\n");
}

int main(int argc, char *argv[])
{
    int sockfd,newsockfd,port_no;
    int serv_here_port=10011;
    int num_files;
    pid_t pid;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    socklen_t len;
    int num_bytes;
    char buff[MAXSIZE], buff_2[MAXSIZE], buff_3[MAXSIZE];
    struct hostent *server;
    struct sockaddr_in server_loc,server_loc_stream;

    if(argc < 3)
    {
        printf("+--- Need to specify ip and port! Use ./server ip port\n");
        exit(0);
    }
    port_no = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    printerror(sockfd, "opening socket");

    server=gethostbyname(argv[1]);
    if(server==NULL)
    {
        printf("+--- Error in finding host\n");
        exit(0);
    }

    bzero((char*)&server_loc,sizeof(server_loc));
    server_loc.sin_family = AF_INET;
    bcopy((char*)server->h_addr,(char*)&server_loc.sin_addr.s_addr,server->h_length);
    server_loc.sin_port = htons(port_no);
    len = sizeof(server_loc);

    // upload all files in current directory
    struct dirent **listr;
    int i, listn = scandir(".", &listr, 0, alphasort);
    if (listn >= 0)
    {
        printf("+--- Total %d objects in this directory\n",listn-2);
        for(i = 0; i < listn; i++ )
        {
            if(listr[i]->d_type == DT_REG) // only files are to be uploaded
            {
                strcat(buff_2, "\n");
                strcat(buff_2, listr[i]->d_name);
                printf("Adding %s to shared list\n", listr[i]->d_name);
            }
        }
        printf("\n");
    }
    else
    {
        perror ("+--- Error in reading current directory contents");
    }

    strcpy(buff,"SHARE ");
    int sockfd_stream=socket(AF_INET,SOCK_STREAM,0);
    printerror(sockfd_stream, "opening socket");

    bzero((char*)&server_loc_stream,sizeof(server_loc_stream));
    server_loc_stream.sin_family = AF_INET;
    server_loc_stream.sin_addr.s_addr=INADDR_ANY;
    server_loc_stream.sin_port=htons(serv_here_port);

    int binder = bind(sockfd_stream,(struct sockaddr*)&server_loc_stream,sizeof(server_loc_stream));
    while (binder < 0)
    {
        printf("\nERROR on binding to %d",serv_here_port);
        serv_here_port=rand()%5001+10001;
        server_loc_stream.sin_port=htons(serv_here_port);
        binder = bind(sockfd_stream,(struct sockaddr*)&server_loc_stream,sizeof(server_loc_stream));
    }

    printf("\nUsing port %d for socket stream",serv_here_port);
    sprintf(buff_3,"%d ",serv_here_port);
    strcat(buff,buff_3);
    strcat(buff,buff_2);

    num_bytes=sendto(sockfd,buff,MAXSIZE,0,(struct sockaddr*)&server_loc,len);
    if(num_bytes < 0)
    {
        perror("Can't send");
    }
    num_bytes=recvfrom(sockfd,buff_2,MAXSIZE,0,(struct sockaddr*)&server_loc,&len);
    if(num_bytes<0)
    {
        perror("no Ack");
    }
    else printf("\nGot ack: %s\n",buff_2);

    listen(sockfd_stream,5);
    clilen=sizeof(cli_addr);
    while(1)
    {
        fflush(stdout);
        newsockfd=accept(sockfd_stream,(struct sockaddr*)&cli_addr,&clilen);
        fflush(stdout);

        if(newsockfd<0)
            perror("ERROR on accept");
        else
        {
            fflush(stdout);
            pid=fork();

            if(pid==0)
            {
                close(sockfd_stream);
                fflush(stdout);
                int n,num_read,f;
                char buff[256];
                bzero(buff,256);
                n = read(newsockfd, buff, 50);
                printerror(n, "reading from socket");

                printf("\n%s\n",buff);
                if(buff[0]=='R'&&buff[1]=='E')
                {
                    if( (f = open(buff+8, O_RDONLY)) >= 0)
                    {
                        strcpy(buff,"SUCCESS");
                        fflush(stdout);
                        n = write(newsockfd,buff,20);
                        printerror(n, "writing to socket");

                        while( (num_read = read(f,buff,CHUNKSIZE)) > 0)
                        {
                            fflush(stdout);
                            n=write(newsockfd,buff,num_read);
                            printerror(n, "write");
                        }
                        printf("\nFile sent successfully\n");
                    }
                    else
                    {
                        strcpy(buff,"FAIL");
                        printf("\n%s",buff);
                        fflush(stdout);
                        n = write(newsockfd,buff,20);
                        printerror(n, "writing to socket");
                    }
                }
                close(newsockfd);
                exit(0);
            }
            else
            {
                signal(SIGCHLD,my_handler);
                close(newsockfd);
            }
        }
    }
    close(sockfd);
}
