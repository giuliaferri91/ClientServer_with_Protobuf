/* !----- THIS IS AN AUTOGENERATED CODE DON'T MODIFY -------! */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <netdb.h>
#include <errno.h>
#include <iostream>
#include <fstream>
//include 
--include--

#define MAXSIZE 4096
#define PORT 8000
#define HOSTNAME "localhost"

using std::cout;
using std::endl;
using std::string;

--enum--

class err{
    public:
        err(const char* namefile, bool verbosity);
        ~err() {log.close();}
        void warning_err(const char * string, int errnumb);
        void exit_err(const char * string, int errnumb);
        void setVerbosity(bool verbosity){ this->verbosity = verbosity; }
    private:
        bool verbosity;
        std::string namefile;
        std::ofstream log;
};

class autoserial {

	protected:
		int mysend(int fd, void* buf, int size, err* error_handler);
		int receive(int fd, void* buf, err* error_handler);
		// user declared
		--virtual1--
};

class client:public autoserial{
	private:
		struct hostent *he;
		struct sockaddr_in myserver;
		int general_descriptor;
		char buf[4096];
		char* hostname;
		int port;
		// define the name of log file and verbosity option
		err* log_file = new err("./log/client.txt",true);
	public:
		client();
		client(char* hostname, char* port);
		int client_init();
		int getDescriptor(){return general_descriptor;};
		void client_connect(int fd);
		void client_close();
		// user declared
		--virtual2--
};

class server: public autoserial {	
	private:
		char buf[4096];
		int opt;
		int port;
		struct sockaddr_in client;
		int general_descriptor;
		int session_descriptor;
		// define the name of log file and verbosity option
		err* log_file = new err("./log/server.txt",true);
	public:
		server(){port = PORT;};
		server(char *p){port = atoi(p);};
		int server_init();
		int getDescriptor(){return general_descriptor;};
		int getSessionDescriptor(){return session_descriptor;};
		int server_accept(int fd_s);
		void accept_close(int fd_s);
		void server_close();
		void request_handler();
		// user declared
		--virtual2--
};