
#include "socket.h"


// the use of socket(*)
#include <sys/types.h>
#include <sys/socket.h>

// gethostbyname()
#include <netdb.h>

// bzero!
#include <strings.h>

//inet_ntoa
#include <arpa/inet.h>
#include <netinet/in.h>



//strlen
#include <string.h>

struct sockaddr_in server_addr;

int connect_socket(int sockfd)
{
	if(connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
		return -1;
	}
	else return 1;
}



int create_socket(char * addr, int port)
{

	int fd;
	struct hostent * h;
	char * hostIPnumber;
	
	h = gethostbyname(addr);
	hostIPnumber = inet_ntoa(*(struct in_addr*)h->h_addr_list[0]);

	//Criar a socket
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(hostIPnumber);
	server_addr.sin_port = htons(port);

	if((fd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		return -1;
	}

	else
	{
		if(connect_socket(fd)<0)
			return -1;
		return fd;
	}
}

int write_socket(char * msg, int sockfd)
{
	int bytes;
	bytes = write(sockfd,msg, strlen(msg));
	return bytes;
}
