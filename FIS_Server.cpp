#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <map>
#include <string>
#include <iostream>
#include <vector>
#define NEW 1
#define REQ 2
#define UPD 3
#define strvec vector<string>
/*
* 14CS30011 : Hiware Kaustubh Narendra
* 14CS30017 : Surya Midatala
*/

/* File information server
* run with
* ./fis port
*/
using namespace std;

map<string, string> files;
void showContent()
{
    cout << "Identifying hostable files on the servers\n";
    map<string,string>::iterator it;
    for (it=files.begin(); it!=files.end(); ++it)
    {
        cout << it->first << "@" << it->second << '\n';
    }
}

// int addfile (string filename, string ip)
// {
//     files.insert ( pair<string,string>(filename,ip) );
// }

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

// convert char* to str
string char_to_str(char* buf, int size)
{
    string s(buf,size);
    return s.substr(3);
}

void get_ip(char* buf, int size)
{
    string ip;
    string file = char_to_str(buf,size);
    cout << "Client wants to download a file " << file << endl;
    if (files.find(file) == files.end())
    {
        cout << "+--- Error: No such file exists" << endl;
        sprintf (buf, "-");
    }
    else strcpy(buf, files[file].c_str());
}

// strvec split(string str, const char* delim)
// {
//     strvec v;
//     string s;
//
//     for(string::const_iterator i = str.begin(); i <= str.end(); ++i)
//     {
//         if(*i != *delim && i != str.end())
//         {
//             s += *i;
//         }
//         else
//         {
//             if (s.length() > 0)
//             {
//                 v.push_back(s);
//                 cout << s;
//             }
//             s = "";
//         }
//     }
//     return v;
// }

void Tokenize(const string& str,
    strvec& tokens,
    const string& delimiters = ":") {
  // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos) {
      // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}


int main()
{
    showContent();

    int s, length, n;
    socklen_t fromlen;
    struct sockaddr_in server, from;
    char buf[1024];

    s = socket(AF_INET, SOCK_DGRAM, 0);
    printerror(s, "Opening socket");
    length = sizeof(server);
    bzero(&server,length);
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(12000);
    printerror(bind(s,(struct sockaddr *)&server,length), "binding");

    fromlen = sizeof(struct sockaddr_in);
    string type;
    while (1) {
        cout << "While loop" << '\n';
        n = recvfrom(s,buf,1024,0,(struct sockaddr *)&from,&fromlen);
        printf("Datagram's IP address is: %s\n", inet_ntoa(from.sin_addr));
        printf("Datagram's port is: %d\n", (int) ntohs(from.sin_port));
        printerror(n, "recvfrom");

        string temp(buf);
        type = temp.substr(0,3);
        // cout << type << endl;
        if (strcmp(type.c_str(),"REQ")==0) {
            // A request for file
            printf("This is a file request.\n");
            get_ip(buf, n);
        }
        else if (strcmp(type.c_str(),"ADD")==0) {
            // A list of files
            printf("This is file list\n");
            string flist = char_to_str(buf, n);
            strvec tokens;
            Tokenize(flist, tokens);
            for (strvec::iterator it = tokens.begin() ; it != tokens.end(); ++it)
            {
                files[*it] = inet_ntoa(from.sin_addr);
            }
            showContent();
            strcpy(buf,"Files added\n");
        }
        else if (strcmp(type.c_str(),"UPD")==0) {
            // A list of files
            printf("File list requested.\n");

            string f;
            map<string,string>::iterator it;
            for (it=files.begin(); it!=files.end(); ++it)
                f += it->first + ":";

            strcpy(buf,f.c_str());
        }
        n = sendto(s,buf,strlen(buf),
            0,(struct sockaddr *)&from,fromlen);
        printerror(n, "sendto");
    }
}
