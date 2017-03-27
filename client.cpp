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
#define BUFFER_SIZE 1024
using namespace std;

list<string> files;
vector<string> allfiles;

void error(const char *msg)
{
  perror(msg);
  exit(0);
}

// code for client for FIS
int sfd,wsfd;
char buffer[1024];
socklen_t addr_size;

void listDir() {
  DIR           *d;
  struct dirent *dir;
  d = opendir(".");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (dir->d_type == DT_REG) {
       // printf("%s\n", dir->d_name);
       files.push_back(dir->d_name);
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
  serv_addr.sin_port = htons(12000);
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
  for (std::vector<string>::iterator it = allfiles.begin() ; it != allfiles.end(); ++it){
    std::cout << ' ' << *it << endl;
  }
}
void view_file_list () {
  for(std::list<string>::iterator list_iter = files.begin();
    list_iter != files.end(); list_iter++)
  {
    std::cout<<*list_iter<<endl;
  }
}

int filesocket;
void send_fname(string fname)
{
  char buffer[BUFFER_SIZE];
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
    error("Connect: ");
  send_fname(file);
  FILE *f=fopen(saveas.c_str(),"w");
  char buffer[BUFFER_SIZE];
  int t;
//  recv(filesocket, buffer, BUFFER_SIZE, 0);
// int fsize = atoi(buffer);
 // cout << "Hey I got the file size: " << buffer << endl;
//  int fgetsize = 0;
  while((t=recv(filesocket, buffer, BUFFER_SIZE, MSG_WAITALL))>40) {
 // fgetsize+=t;
 // cout << fgetsize << endl;
    //cout << t << endl;
    for(int i=0;i<t;i++)
      putc(buffer[i],f);
//    if(t<BUFFER_SIZE) {
    //	printf("%d",t);
  //  	break;
//	  if (fgetsize==fsize)
//	  break;
    //}
  }
//  cout << "Out of loop." << endl;
  fclose(f);
  close(filesocket);
  cout << "File saved.\n";
}

int sock, n;
unsigned int length;
struct sockaddr_in server, from;
struct hostent *hp;

void connectfis() {
  sock= socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) error("socket");

  server.sin_family = AF_INET;
  hp = gethostbyname("localhost");
  if (hp==0) error("Unknown host");

  bcopy((char *)hp->h_addr,
   (char *)&server.sin_addr,
   hp->h_length);

  server.sin_addr.s_addr = inet_addr("10.147.48.208");
  server.sin_port = htons(12000);
  length=sizeof(struct sockaddr_in);
}

string append_filenams() {
  string out;
  // view_file_list();
  for(std::list<string>::iterator list_iter = files.begin();
    list_iter != files.end(); list_iter++)
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
  if (n < 0) error("Sendto");
  n = recvfrom(sock,buffer,BUFFER_SIZE,0,(struct sockaddr *)&from, &length);
  if (n < 0) error("recvfrom");
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
        // Skip delimiters.  Note the "not_of"
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
  if (n < 0) error("Sendto");
  n = recvfrom(sock,buffer,BUFFER_SIZE,0,(struct sockaddr *)&from, &length);
  if (n < 0) error("recvfrom");
  vector<string> tokens;
  string buf(buffer,n);
  Tokenize(buf, tokens);
  std::vector<std::string> vv = tokens;
  allfiles.clear();
  for (std::vector<string>::iterator it = vv.begin() ; it != vv.end(); ++it){
        // std::cout << ' ' << *it << endl;
    allfiles.push_back(*it);
  }
  view_allfile_list();
  close(sock);
  if (buffer[0]=='-') return -1;
  return 0;
}

void init () {
  connectfis();
  // printf("Please enter the message: ");
  string f = append_filenams();
  f = "ADD" + f;
  // cout << f << endl;
  strcpy(buffer, f.c_str());
  // bzero(buffer,BUFFER_SIZE);
  // fgets(buffer,BUFFER_SIZE,stdin);
  n=sendto(sock,buffer,
           strlen(buffer),0,(const struct sockaddr *)&server,length);
  if (n < 0) error("Sendto");
  n = recvfrom(sock,buffer,BUFFER_SIZE,0,(struct sockaddr *)&from, &length);
  if (n < 0) error("recvfrom");
 //  printf("Datagram's IP address is: %s\n", inet_ntoa(from.sin_addr));
 //  printf("Datagram's port is: %d\n", (int) ntohs(from.sin_port));

  write(1,"Got an ack: ",12);
  write(1,buffer,n);
  close(sock);
  update_file_list();
}

int main(int argc, char *argv[])
{
  listDir();
  init();
  // connect_to_fis();
  // send_file_list();
  // receive_file_list();
  printf("Connected\n");
  int input;
  string fname, save_as;
  while(1) {
    printf("What would you like to do?\n");
    printf("1. Update and view file list.\n");
    printf("2. Download a file.\n");
    printf("3. Shut down.\n");
    scanf("%d",&input);
    switch(input) {
      case 1:
      init();
        update_file_list();
        break;
      case 2: {
        printf("File name: ");
        cin >> fname;
        if (getDetails(fname) ==-1) {
          cout << "File not found." << endl;
          break;
        }
        printf("\nSave as: ");
        cin >> save_as;
        string ip(buffer,n);
        request_file(ip,fname,save_as);
      }
        break;
      case 3:
        break;
    }
    if(input==3) break;
  }

  return 0;
}
