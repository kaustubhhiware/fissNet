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
#define BUFFER_SIZE 1024

using namespace std;

// Mapping
std::map<string, string> filemap;
void showFileMap() {
    std::cout << "File Mapping:\n";
    std::map<string,string>::iterator it;
    for (it=filemap.begin(); it!=filemap.end(); ++it)
        std::cout << it->second << " => " << it->first << '\n';
}

int addfile (string filename, string ip) {
    filemap.insert ( std::pair<string,string>(filename,ip) );
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

string buftostr(char* buf, int size) {
    string s(buf,size);
    s = s.substr(3);
    return s;
}

void get_ip(char* buf, int size) {
    string ip;// = "127.0.0.1";
    string file = buftostr(buf,size);
    cout << "Received request for " << file << endl;
    if ( filemap.find(file) == filemap.end() ) {
        cout << "File not found." << endl;
        sprintf (buf, "-");
    } else {
        strcpy(buf, filemap[file].c_str());
    }
}

string getType(string a) {
    return a.substr(0,3);
    a = a.substr(4);
}

std::vector<std::string> split(std::string str, const char* delim) {
    std::vector<std::string> v;
    std::string tmp;

    for(std::string::const_iterator i = str.begin(); i <= str.end(); ++i) {
        if(*i != *delim && i != str.end()) {
            tmp += *i;
        } else {
            if (tmp.length() > 0) {
                v.push_back(tmp);
                cout << tmp;
            }
            tmp = "";
        }
    }
    return v;
}

void Tokenize(const string& str,
	vector<string>& tokens,
	const string& delimiters = ":") {
  // Skip delimiters at beginning.
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
	string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos) {
  	// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

string append_filenams() {
  string out;
  // view_file_list();
    std::map<string,string>::iterator it;
    for (it=filemap.begin(); it!=filemap.end(); ++it)
        out += it->first + ":";
  return out;
}

int main () {
    
    showFileMap();

    int sock, length, n;
    socklen_t fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;
    char buf[1024];


    sock=socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("Opening socket");
    length = sizeof(server);
    bzero(&server,length);
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(12000);
    if (bind(sock,(struct sockaddr *)&server,length)<0)
        error("binding");
    fromlen = sizeof(struct sockaddr_in);
    string type;
    while (1) {
        std::cout << "While loop" << '\n';
        n = recvfrom(sock,buf,1024,0,(struct sockaddr *)&from,&fromlen);
        printf("Datagram's IP address is: %s\n", inet_ntoa(from.sin_addr));
        printf("Datagram's port is: %d\n", (int) ntohs(from.sin_port));
        if (n < 0) error("recvfrom");

        type = getType(buf);
        // cout << type << endl;
        if (strcmp(type.c_str(),"REQ")==0) {
            // A request for file
            printf("This is a file request.\n");
            get_ip(buf, n);
        }
        else if (strcmp(type.c_str(),"ADD")==0) {
            // A list of files
            printf("This is file list\n");
            string flist = buftostr(buf, n);
            vector<string> tokens;
            Tokenize(flist, tokens);
            std::vector<std::string> vv = tokens;
            for (std::vector<string>::iterator it = vv.begin() ; it != vv.end(); ++it){
                // std::cout << ' ' << *it << endl;
                filemap[*it] = inet_ntoa(from.sin_addr);
            }
            showFileMap();
            strcpy(buf,"Files added\n");
        }
        else if (strcmp(type.c_str(),"UPD")==0) {
            // A list of files
            printf("File list requested.\n");
            string f = append_filenams();
            strcpy(buf,f.c_str());
        }
        n = sendto(sock,buf,strlen(buf),
            0,(struct sockaddr *)&from,fromlen);
        if (n  < 0) error("sendto");
    }
}
