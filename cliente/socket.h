#ifndef _SOCKET_H_
#define _SOCKET_H_



int connect_socket(int sockfd);
int create_socket(char*addr,int port);
int write_socket(char * msg, int sockfd);


#endif
