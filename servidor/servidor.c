


//FICHEIROS
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <string.h>
#include "../cliente/helper.h"


#include <time.h>

#define MAXLEN 1024
#define SERVER_PORT 8080



#define CONTENT "Content-Type: text/html; charset=iso8859-1"
#define SERVER "Server: Claudio && Simao && Goncalo - HTTP Server - Beta Version - Catchim"


#define BADREQUEST "HTTP/1.1 400 Bad Request"

#define FILENOTFOUND "HTTP/1.1 404 File Not Found"

#define FILEOK "HTTP/1.1 200 OK"

void fireman(void)
{
	while(waitpid(-1, NULL, WNOHANG)>0)
		;
}

int getFileSize(int fd)
{
	struct stat file;
	if(!fstat(fd,&file))
	{
		return file.st_size;
	}
	return 0;
}

int check_host(char**headers,int numero)
{
	int i =0;
	for(i;i<numero;i++)
	{

		if((strcmp(strtok(headers[i],":"),"HOST")==0)&& (strcmp(strtok(headers[i],":"),"Host")==0))
			return 1;
	}
	return 0;
}

time_t check_modifiedsince(char**headers,int numero)
{
	int i =0;

	time_t req_date;
	struct tm request_date;
	char *dateString;
	for(i;i<numero;i++)
	{
		
		if((strncmp(strtok(headers[i],": "),"If-Modified-Since",strlen("If-Modified-Since"))==0))
		{
			dateString = strtok(NULL,"\n");
			printf("A date String: %s\n",dateString);
			if(strptime(dateString," %a %b %d %T %Y",&request_date)==NULL)
			{
				return -1;
			}
			return (req_date = mktime(&request_date));
		}
	}
	return -1;
}

int handle_request(int sockfd,char*request)
{

		int filed;

		//Utilizado para "obter" a palavra "GET" do request
		char * token;

		//Variaveis utilizados para fazer o parse do request por "\r\n"
		char ** stripped;
		int ntokens;

		//Auxilio
		char ** stripped2;
		int ntokens3;
		
		//Guarda o directorio do ficheiro a ser buscado
		char * directorio;
	
		//HOST
		char * pagina;
		
		//Variaveis utilizadas para fazer o parse do "path" 
		char ** path;
		int ntokens2;
	
		//Guarda a informacao a ser enviada para os clientes
		char answer[1024];

		//Nome do ficheiro .html
		char ficheiro[50];

		int i;

		struct stat file_info;
		
		char *auxiliar;
	
		//Variaveis para controlar o Header If-Modified-Since
		time_t oldTime;
		time_t timemodified;
		double diffdate;
		

		//HTTP/1.1 ?
		char * http;
		
		time_t now;
		struct tm *tnow;
		time(&now);
		tnow=localtime(&now);


		
		if((ntokens = makeargv(request,"\r\n",&stripped))==-1)
			return -1;
	
		chdir("public_html");

		token = strtok(stripped[0], " "); //vai buscar a primeira palavra, a ver se é um get
		directorio = strtok(NULL," "); //guardar o directorio
		http = strtok(NULL," ");	

		//Bad Command
		if(strcmp(token,"GET")!=0)
		{
			sprintf(answer,"HTTP/1.1 4xx Client Error\r\n%s\r\n%s\r\nDate:%s\r\n",CONTENT,SERVER,asctime(tnow));
			write(sockfd,answer,strlen(answer));
			freemakeargv(stripped);
			freemakeargv(path);
			return -1;
		}
		if(strcmp(http,"HTTP/1.1")!=0)
		{
			sprintf(answer,"400 Error - Bad Request - HTTP Version 1.1!!\n");
			write(sockfd, answer, strlen(answer));
			freemakeargv(stripped);
			freemakeargv(path);
			return -1;
		}

		
		//Bad Request 400
			
		if(check_host(stripped,ntokens))
		{

			filed = open("erros/400",O_RDONLY);
			i=getFileSize(filed);
			sprintf(answer,"%s\r\n%s\r\n%s\r\nContent-Length: %d\r\nConnection: close\r\nDate:%s\r\n",BADREQUEST,CONTENT,SERVER,i,asctime(tnow));
			printf("Enviado:\n%s\n", answer);
			write(sockfd,answer,strlen(answer));
			printf("O tamanho total é: %d\n",i);
			do{
				if(i<=0)
				{
					break;	
				}
				if(i>1024)
				{
					read(filed,answer,1024);
					write(sockfd,answer,1024);
					i -=1024;
				}
				else
				{
					read(filed,answer,i);
					write(sockfd,answer,i);
					break;
				}
			}
			while(1);
			close(filed);	
			freemakeargv(stripped);
			freemakeargv(path);
			return 1;
		}

		
		if((ntokens2 = makeargv(directorio,"/",&path))==-1) //Separar as várias pastas do path
			return -1;

		switch(ntokens2)
		{
			case 0:
				strcpy(ficheiro,"index.html");
				break;
			case 1:
				strcpy(ficheiro,path[0]);
				break;
			default:
				for(i=0;i<(ntokens-1);i++)
				{
					if(chdir(path[i])==-1)
					{

						filed = open("erros/404",O_RDONLY);
						i=getFileSize(filed);
						sprintf(answer,"%s\r\n%s\r\n%s\r\nContent-Length: %d\r\nConnection: close\r\nDate:%s\r\n",FILENOTFOUND,CONTENT,SERVER,i,asctime(tnow));
						printf("Enviado:\n%s\n",answer);
						write(sockfd,answer,strlen(answer));
						do{
							if(i<=0)
							{
								break;	
							}
							if(i>1024)
							{
								read(filed,answer,1024);
								write(sockfd,answer,1024);
								i -=1024;
							}
							else
							{
								read(filed,answer,i);
								write(sockfd,answer,i);
								break;
							}
						}while(1);
						close(filed);
						freemakeargv(stripped);
						freemakeargv(path);
						return 1;
					}
				}
				strcpy(ficheiro,path[i]);
				break;
		}

		ntokens3 = makeargv(request,"\r\n",&stripped2);
		//Verifica a existencia do Header "If-Modified Since"
		timemodified = check_modifiedsince(stripped2,ntokens3);
		if((filed=open(ficheiro,O_RDONLY))!=-1)
		{
			i=getFileSize(filed);
			fstat(filed,&file_info);
			oldTime = (time_t)&file_info.st_mtime;
		
			if(timemodified>1)
			{
				diffdate = difftime(oldTime,timemodified);
				//Not-Modified
				if(diffdate<0)
				{
					printf("Estado: Ficheiro nao modificado\n");
					sprintf(answer,"HTTP/1.1 304 Not Modified\r\nDate: %s%s\r\n%s\r\nContent-Length: 0\r\n",asctime(tnow),SERVER,CONTENT);
					printf("Enviado:\n%s\n",answer);
					write(sockfd,answer,strlen(answer));
					freemakeargv(stripped);
					freemakeargv(stripped2);
					freemakeargv(path);
					return 1;
				}
			}
			sprintf(answer,"%s\r\n%s\r\n%s\r\nContent-Length: %d\r\nLast-Modified: %sConnection: close\r\nDate:%s\r\n",FILEOK,CONTENT,SERVER,i,ctime(&file_info.st_mtime),asctime(tnow));
			printf("Enviado:\n%s\n",answer);
			write(sockfd,answer,strlen(answer));
			do{
				if(i<=0)
				{
					break;	
				}
				if(i>1024)
				{
					read(filed,answer,1024);
					write(sockfd,answer,1024);
					i -=1024;
				}
				else
				{
					read(filed,answer,i);
					write(sockfd,answer,i);
					break;
				}
			}while(1);
			close(filed);
			freemakeargv(stripped2);
			freemakeargv(stripped);
			freemakeargv(path);
			return 1;
		}
		else
		{
			filed =open("erros/404",O_RDONLY);
			i=getFileSize(filed);
			sprintf(answer,"%s\r\n%s\r\n%s\r\nContent-Length: %d\r\nConnection: close\r\nDate:%s\r\n",FILENOTFOUND,CONTENT,SERVER,i,asctime(tnow));
			printf("Enviado:\n%s\n",answer);
			write(sockfd,answer,strlen(answer));
			do{
				if(i<=0)
				{
					break;	
				}
				if(i>1024)
				{
					read(filed,answer,1024);
					write(sockfd,answer,1024);
					i -=1024;
				}
				else
				{
					read(filed,answer,i);
					write(sockfd,answer,i);
					break;
				}
			}while(1);
			close(filed);
			freemakeargv(stripped2);
			freemakeargv(stripped);
			freemakeargv(path);
			return 1;
		}

		freemakeargv(stripped2);
		freemakeargv(stripped);
		freemakeargv(path);
}
int main(int argc, char**argv)
{
	int sockfd, newsockfd, client_size;

	struct sockaddr_in server_addr, client_addr;
	int childpid;
	char buf[MAXLEN];
	int bytes;
	
	signal(SIGCHLD, fireman);

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
        	exit(0);
    	}
	/*bind the local address so that a client can connect*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	
	server_addr.sin_port = htons(SERVER_PORT);	/*server TCP port must be network byte ordered */
    
	if (bind(sockfd, 
		 (struct sockaddr *)&server_addr, 
		 sizeof(server_addr)) < 0){
        	perror("bind()");
		exit(0);
	}

	printf("\n  -----------------------------------------------\n ");
	printf(" -- RCOM | Redes de Computadores // FEUP 2007 --\n ");
	printf(" --                                           --\n ");
	printf(" -- Servidor HTTP | ei01091.ei03032.ei04100   --\n ");
	printf(" -----------------------------------------------\n ");
	printf("\n");
	listen(sockfd, 5);


	for(;;) {
		/*wait for a connection request (concurrent server)*/
		client_size = sizeof(client_addr);
		/*blocks until a connection request is done*/
		/*then returns the new socket descriptor, now complete*/
		newsockfd = accept(sockfd, 
			           (struct sockaddr *)&client_addr, 
				   &client_size);
		if (newsockfd < 0) {
			perror("accept()");
			exit(0);
		}
		/*fork the server so one process handle the connection
		and other keeps waiting incoming connection requests*/
		if ((childpid = fork()) < 0) {
			perror("fork()");
			exit(0);
		}
		else if (childpid == 0){	/*child process*/
			close(sockfd);	/*continue processing with the new descriptor*/
			bytes = read(newsockfd, buf, MAXLEN);
			printf("Bytes lidos = %d\n%s\n", bytes, buf);
			handle_request(newsockfd,buf);
			exit(0);
		}
	}
	return 1;
}
