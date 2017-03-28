#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <list>
#include <string>
#include <iostream>
#include <vector>
using namespace std;

list<string> serverfiles;
vector<string> allfiles;
// usage - ./client fisip
//

// standard function to print all errors > assgn4/B.c
void printerror(const char* printmsg)
{
    fprintf(stderr, "+--- %s : ",printmsg );
    perror("");
    exit(0);
}

// code for client for FIS
int sfd,wsfd;
char buffer[1024];
char fis_ip[20];
socklen_t addr_size;

// list the FILES only in current folder
void listDir()
{
    DIR *d;
    struct dirent *dir;
    d = opendir("."); // get the files in server locations
    if (d) {
    while ((dir = readdir(d)) != NULL)
    {
        if (dir->d_type == DT_REG)
        {
            serverfiles.push_back(dir->d_name);
        }
    }
    closedir(d);
 }
}

char sendBuff[1025];
int sockfd;
char recvBuff[1024];
struct sockaddr_in serv_addr;

int connect_to_fis() {
    sockfd = 0;

    memset(recvBuff, '0' ,sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))< 0) {
    printf("\n Error : Could not create socket \n");
    return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(12500);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0) {
    printf("\n Error : Connect Failed \n");
    return 1;
    }
}

void send_file_list() {
    strcpy(sendBuff, "File list sent by client");
    // write(sockfd, sendBuff, strlen(sendBuff));
struct sockaddr_storage sender;
socklen_t sendsize = sizeof(sender);
bzero(&sender, sizeof(sender));
sendto(sockfd, sendBuff, sizeof(sendBuff), 0, (struct sockaddr *)&sender, sendsize);
}


void receive_file_list () {
    int n = 0;
    while((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0) {
    recvBuff[n] = 0;
    if(fputs(recvBuff, stdout) == EOF) {
    printf("\n Error : Fputs error");
    }
    printf("\n");
    }
    // printf("<= %s\n", recvBuff);
}

void view_allfile_list () {
    cout << "Files available to download:" << endl;
    for (vector<string>::iterator it = allfiles.begin() ; it != allfiles.end(); ++it){
    cout << ' ' << *it << endl;
    }
}
void view_file_list () {
    for(list<string>::iterator list_iter = serverfiles.begin();
    list_iter != serverfiles.end(); list_iter++)
    {
    cout<<*list_iter<<endl;
    }
}

int filesocket;
void send_fname(string fname)
{
    char buffer[1024];
    strcpy(buffer,fname.c_str());
    send(filesocket,buffer,strlen(buffer),0);
}

void request_file(string ip,string file,string saveas)
{
    socklen_t addr_size;
    sockaddr_in serverAddr;

    filesocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12002);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof serverAddr;
    if(connect(filesocket, (struct sockaddr *) &serverAddr, addr_size)<0)
    printerror("Connect: ");
    send_fname(file);
    FILE *f=fopen(saveas.c_str(),"w");
    char buffer[1024];
    int t;
//    recv(filesocket, buffer, 1024, 0);
// int fsize = atoi(buffer);
 // cout << "Hey I got the file size: " << buffer << endl;
//    int fgetsize = 0;
    while((t=recv(filesocket, buffer, 1024, MSG_WAITALL))>40) {
 // fgetsize+=t;
 // cout << fgetsize << endl;
    //cout << t << endl;
    for(int i=0;i<t;i++)
    putc(buffer[i],f);
//    if(t<1024) {
    //	printf("%d",t);
    //    	break;
//	    if (fgetsize==fsize)
//	    break;
    //}
    }
//    cout << "Out of loop." << endl;
    fclose(f);
    close(filesocket);
    cout << "File saved.\n";
}

int sock, n;
unsigned int length;
struct sockaddr_in server, from;
struct hostent *host_ip;

void connectfis()
{
    sock= socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) printerror("socket");

    server.sin_family = AF_INET;
    host_ip = gethostbyname("localhost");
    if (host_ip==0) printerror("Unknown host");

    bcopy((char *)host_ip->h_addr,
     (char *)&server.sin_addr,
     host_ip->h_length);

    // server.sin_addr.s_addr = inet_addr("10.145.210.31");
    server.sin_addr.s_addr = inet_addr(fis_ip);
    server.sin_port = htons(12500);
    length=sizeof(struct sockaddr_in);
}

string append_filenams() {
    string out;
    // view_file_list();
    for(list<string>::iterator list_iter = serverfiles.begin();
    list_iter != serverfiles.end(); list_iter++)
    {
    out += *list_iter + ":";
    }
    return out;
}


int getDetails (string file) {
    file = "REQ"+file;
    connectfis();
    strcpy(buffer,file.c_str());
    n=sendto(sock,buffer,
     strlen(buffer),0,(const struct sockaddr *)&server,length);
    if (n < 0) printerror("Sendto");
    n = recvfrom(sock,buffer,1024,0,(struct sockaddr *)&from, &length);
    if (n < 0) printerror("recvfrom");
    write(1,"Got an ack: ",12);
    write(1,buffer,n);
    close(sock);
    if (buffer[0]=='-') return -1;
    return 0;
}

void Tokenize(const string& str,
    vector<string>& tokens,
    const string& delimiters = ":") {
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.    Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
    }
}

int update_file_list() {
    string s = "UPD";
    connectfis();
    strcpy(buffer,s.c_str());
    n=sendto(sock,buffer,
     strlen(buffer),0,(const struct sockaddr *)&server,length);
    if (n < 0) printerror("Sendto");
    n = recvfrom(sock,buffer,1024,0,(struct sockaddr *)&from, &length);
    if (n < 0) printerror("recvfrom");
    vector<string> tokens;
    string buf(buffer,n);
    Tokenize(buf, tokens);
    vector<string> vv = tokens;
    allfiles.clear();
    for (vector<string>::iterator it = vv.begin() ; it != vv.end(); ++it){
    // cout << ' ' << *it << endl;
    allfiles.push_back(*it);
    }
    view_allfile_list();
    close(sock);
    if (buffer[0]=='-') return -1;
    return 0;
}

void init()
{
    connectfis();
    // printf("Please enter the message: ");
    string f = append_filenams();
    f = "ADD" + f;
    // cout << f << endl;
    strcpy(buffer, f.c_str());
    // bzero(buffer,1024);
    // fgets(buffer,1024,stdin);
    n=sendto(sock,buffer,
     strlen(buffer),0,(const struct sockaddr *)&server,length);
    if (n < 0) printerror("Sendto");
    n = recvfrom(sock,buffer,1024,0,(struct sockaddr *)&from, &length);
    if (n < 0) printerror("recvfrom");
 //    printf("Datagram's IP address is: %s\n", inet_ntoa(from.sin_addr));
 //    printf("Datagram's port is: %d\n", (int) ntohs(from.sin_port));

    write(1,"Got an ack: ",12);
    write(1,buffer,n);
    close(sock);
}

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("+--- Need to mention IP and port! Exitting...\n");
    }

    sscanf(argv[1],"%s",fis_ip);
    if(argc < 2)
    {
        printf("+--- Need to enter IP address !\nCorrect usage - ./client ip\n");
        exit(0);
    }
    sscanf(argv[1],"%s",fis_ip);
    listDir(); // get all the files listed by server
    init();
    // connect_to_fis();
    // send_file_list();
    // receive_file_list();
    printf("Connected\n");
    int input = 1;
    char instr[100];
    string fname, save_as;
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
        scanf("%s",instr);
        input = 0;
        sscanf(instr,"%d",&input);
        // if case string does not contain a int, input is set as 0
        if(input==1)
        {
            init();
            update_file_list();
        }
        else if(input==2)
        {
            printf("File name: ");
            cin >> fname;
            if (getDetails(fname) ==-1)
            {
                cout << "File not found." << endl;
                break;
            }
            printf("\nSave as: ");
            cin >> save_as;
            string ip(buffer,n);
            request_file(ip,fname,save_as);
        }
        else exit(0);

        printf("+--- Press [ENTER] to continue ...");
        char buf;
        scanf ("%c%*c", &buf);
    }
}
