#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <utility>
#include <string>
#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <netdb.h>
#include <map>

#define CHUNKSIZE 32
#define MAXSIZE 512

using namespace std;

/*
* Written by kingofools (Surya) and kaustubhhiware
* as a part of Networks Lab
*/

/* Transfer client
* run with
* ./client fis_ip port
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

int main(int argc, char *argv[])
{
    int sockfd,port_no;
    socklen_t len;
    char *tok;
    int num_bytes;
    char buff[MAXSIZE], buff_2[MAXSIZE], buff_3[MAXSIZE], buffer_4[MAXSIZE];
    struct hostent *server;
    struct sockaddr_in server_loc;
    if(argc < 3)
    {
        printf("+--- Need to specify ip and port! Use - ./client ip port\n");
        exit(1);
    }
    port_no=atoi(argv[2]);

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    printerror(sockfd, "opening socket");

    server=gethostbyname(argv[1]);
    if(server == NULL)
    {
        printf("ERROR, no such host\n");
        exit(0);
    }
    bzero((char*)&server_loc,sizeof(server_loc));
    server_loc.sin_family=AF_INET;
    bcopy((char*)server->h_addr,(char*)&server_loc.sin_addr.s_addr,server->h_length);
    server_loc.sin_port=htons(port_no);
    len = sizeof(server_loc);

    int input = 1;
    char instruction[100];
    while(1)
    {
        // clear the terminal
        const char* blank = "\e[1;1H\e[2J";
        write(STDOUT_FILENO,blank,12);

        printf("+------------------------------------------------------------------------------+\n");
        printf("|                             Welcome to fissNet !                             |\n");
        printf("+------------------------------------------------------------------------------+\n");
        printf("|                          What are you looking for ?                          |\n");
        printf("|                     1 to view files available at server                      |\n");
        printf("|                             2 to download a file                             |\n");
        printf("|                            Anything else to exit                             |\n");
        printf("+------------------------------------------------------------------------------+\n");
        scanf("%s",instruction);
        input = 0;
        sscanf(instruction,"%d",&input);

        if(input == 1)
        {
            strcpy(buff, "INFORM");

            num_bytes=sendto(sockfd,buff,MAXSIZE,0,(struct sockaddr*)&server_loc,len);
            printerror(num_bytes, "sending");

            num_bytes=recvfrom(sockfd,buff_2,MAXSIZE,0,(struct sockaddr*)&server_loc,&len);
            printf("\n%s\n",buff_2);

            printf("+--- Press [ENTER] to continue ...");
            char buf;
            scanf ("%c%*c", &buf);
        }
        else if(input != 2) exit(0);

        printf("\nEnter filename to download: ");
        scanf("%s",buff_2);
        strcpy(buff,"REQUEST ");
        strcat(buff,buff_2);
        strcpy(buff_3,buff);
        strcpy(buffer_4,buff_2);

        num_bytes=sendto(sockfd,buff,MAXSIZE,0,(struct sockaddr*)&server_loc,len);
        printerror(num_bytes, "sending");

        num_bytes=recvfrom(sockfd,buff_2,MAXSIZE,0,(struct sockaddr*)&server_loc,&len);
        printf("\nGot ack: %s\n",buff_2);

        if(num_bytes > 0)
        {
            // what if transfer fails
            if(buff_2[0] == 'F' && buff_2[1] == 'A')
            {
                printf("\n File not available\n");
            }
            else
            {
                tok=strtok(buff_2+8," ");
                int sockfd_stream, portno_stream, n;
                struct sockaddr_in server_loc_stream;
                struct hostent *server_stream;
                char buffer_stream[256];
                portno_stream=10011;
                sockfd_stream=socket(AF_INET, SOCK_STREAM, 0);
                printerror(sockfd_stream, "opening socket");

                server_stream=gethostbyname(tok);
                tok=strtok(NULL," ");
                portno_stream=atoi(tok);
                if(server_stream == NULL)
                {
                    fprintf(stderr,"+--- Errror in connecting to host\n");
                    exit(0);
                }

                bzero((char*)&server_loc_stream,sizeof(server_loc_stream));
                server_loc_stream.sin_family=AF_INET;
                bcopy((char*)server_stream->h_addr,(char*)&server_loc_stream.sin_addr.s_addr,server_stream->h_length);

                server_loc_stream.sin_port=htons(portno_stream);
                while(connect(sockfd_stream,(struct sockaddr*)&server_loc_stream,sizeof(server_loc_stream)) < 0);
                num_bytes=write(sockfd_stream,buff_3,strlen(buff_3));

                printerror(num_bytes, "writing");
                fflush(stdout);
                num_bytes=read(sockfd_stream,buff,20);
                fflush(stdout);
                printerror(num_bytes, "reading");

                if(strcmp(buff,"SUCCESS") == 0)
                {
                    fflush(stdout);
                    char *filename=basename(buffer_4);
                    int nwritten,num_read;

                    strcpy(buff_3,filename);
                    // open if file exists, or create one
                    int fd_to=open(buff_3,O_WRONLY|O_CREAT,0777);

                    if(fd_to<0)
                    {
                        perror("Can't write/create");
                    }
                    else
                    {
                        int i;
                        while(num_bytes=read(sockfd_stream,buff,CHUNKSIZE),num_bytes > 0)
                        {
                            fflush(stdout);
                            char *out_ptr=buff;
                            do
                            {
                                nwritten=write(fd_to,out_ptr,num_bytes);
                                if(nwritten >= 0)
                                {
                                    num_bytes -= nwritten;
                                    out_ptr += nwritten;
                                }
                                else if(errno != EINTR)
                                {
                                    if(fd_to >= 0)
                                    close(fd_to);
                                    return -1;
                                }
                            }while(num_bytes > 0);
                        }
                        close(sockfd_stream);
                        close(fd_to);
                        printf("\nFile transfer complete\n");
                    }
                }
                else printf("\nCan't read\n");
            }
        }
        else printf("No response from FIS\n");

        printf("+--- Press [ENTER] to continue ...");
        char buf;
        scanf ("%c%*c", &buf);
    }
}
