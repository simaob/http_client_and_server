
#include <stdio.h>
#include <stdlib.h>

//handle files
#include <fcntl.h>

//use of mkdir
#include <sys/stat.h>

//Stat()
#include <unistd.h>

#include <string.h>

//Defines e cenas
#define MAXLEN 1024

//MakeArgv
#include "helper.h"

//Estrutura Global que guarda a informa√√o do servidor
//struct sockaddr_in server_addr;

#include "socket.h"



#include <time.h>


//Porta a ser utilizada:
int porta = 80;


typedef struct{
	char * path;
	char * lastmodified;
}cachefile;

cachefile cache[100];
int cached = 0;

typedef struct{
	char *wday;
	char *mday;
	char *month;
	char *time;
	char *year;
}timestamp;

int fill_cache(FILE* file)
{
	int i=0;
	int j=0;

	while((cache[i].path = (char *)malloc(1024)) && fgets(cache[i].path,35,file)!=NULL)
	{
		cache[i].lastmodified = (char *)malloc(1024);
		fgets(cache[i].lastmodified,35,file);
		i++;
		cached++;
	}
	for(i=0;i<cached;i++)
	{
		while(cache[i].path[j]!='\n')j++;
		cache[i].path[j]='\0';
		j=0;
		while(cache[i].lastmodified[j]!='\n')j++;
		cache[i].lastmodified[j] ='\0';
		j=0;
	}
	return 1;
}

int save_cache(FILE*file)
{
	int i=0;
	for(i;i<cached;i++)
	{
		fprintf(file,"%s\n",cache[i].path);
		fprintf(file,"%s\n",cache[i].lastmodified);
	}
	return 1;
}

int menu()
{
	char resposta[50];
	int i;
	printf("\n  -----------------------------------------------\n ");
	printf(" -- RCOM | Redes de Computadores // FEUP 2007 --\n ");
	printf(" --                                           --\n ");
	printf(" -- Cliente HTTP | ei01091.ei03032.ei04100    --\n ");
	printf(" -----------------------------------------------\n ");
	printf("\n");
	printf("\nTem disponiveis as seguintes opcoes:\n");
	printf("1- Visitar um website\n");
	printf("2- Visitar um website, se tiver sido actualizado\n");
	printf("3- Visitar um website e descarregar todo o seu conteudo [Nao implementado]\n");
	printf("4- Visitar um website via proxy server [Nao implementado]\n");
	printf("5- Alterar a porta a utilizar nas ligacoes [Porta definida: %d]\n",porta);
	printf("6- Sair\n> ");
	fgets(resposta,sizeof(resposta),stdin);
	i =0;
	while(resposta[i]!='\n')
		i++;
	resposta[i]='\0';
	return atoi(resposta);
}

char * check_modified(char **headers,int nrheaders)
{
	int i;
	char * aux;
	for(i=0;i<nrheaders;i++)
	{
		if(strncmp(headers[i],"Last-Modified:",strlen("Last-Modified:"))==0)
		{
			strtok(headers[i]," ");	
			aux = strtok(NULL,"\r\n");
			return aux;
		}
	}
	return "No Info";
}
/**
 *	200 OK - return 2
 *	3xx Redirection - return 3
 *	301 Moved Permanently - return 31
 *	304 Not Modified - return 34
 *	4xx Client Error - return 4
 *	400 Bad Request - return 40
 *	404 Not Found - return 44
 *	5xx Server Error - return 5
 *	
 */
int check_errors(char*headers,int nrheaders)
{
	int i;
	char * aux;
	strtok(headers," ");
	aux = strtok(NULL," ");
	i = atoi(aux);
	switch(i)
	{
		case 200: return 2;
		case 301: return 31;
		case 304: return 34;
		case 400: return 40;
		case 404: return 44;
		default:break;
	}
	i = (int) i/100;
	return i;
}
int check_length(char **headers,int nrheaders)
{
	int i;
	for(i=0;i<nrheaders;i++)
	{
		if(strncmp(headers[i],"Content-Length: ",strlen("Content-Length: "))==0)
		{
			strtok(headers[i]," ");	
			return atoi(strtok(NULL,"\r\n"));
		}
	}
	return 0;	
}

int check_chunked(char ** headers,int nrheaders)
{
	int i;
	for(i=0;i<nrheaders;i++)
	{
		if(strncmp(headers[i],"Transfer-Encoding: chunked",strlen("Transfer-Encoding: chunked"))==0)
				return 1;
	}
	return 0;
}


void get_chunked(int filed,char * cenas, int sockfd)
{
	long length;
	int i,j=0;

	char byte;
	char * tamanho = (char*)malloc(sizeof(char)*500);
	int next=0;
	int k=0;
	char ** strip;
	int ntokens;

	ntokens = makeargv(cenas,"\r\n",&strip);
	length=strtol(strip[0],NULL,16);
	if(length ==0)
		return;
	for(i=1;i<ntokens;i++)
	{
		if(next)
		{
			length = strtol(strip[i],NULL,16);
			if(length ==0)
				return;
			next =0;
			k=0;
		}
		else
		{
			for(j=0;strip[i][j]!='\n';j++)
			{
				write(filed,&strip[i][j],1);
				k++;		
				if(k>=length)
				{
					next =1;
					break;
				}
			}
		}
	}
	while(1)
	{
		if(k<length)
		{
			read(sockfd,&byte,1);
			write(filed,&byte,1);
			k++;
		}
		else
		{
			i=0;
			do{
				read(sockfd,&byte,1);
				strcat(tamanho,&byte);
			}while(byte!='\n');
			length=strtol(tamanho,NULL,16);
			k = 0;	
		}
		if(length==0)
			return;
	}
}

int getFicheiro(int sockfd, char * ficheiro, char*url)
{
	int bytes;
	int fd;
	char answer[1024];
	char ant1,act1,ant2,act2;
	int i,j;
	

	int length;
	int aux;
	char ** partes;
	int nrpartes;
	
	//Separar os Headers do resto da informacao
	char ** headers;
	int nrheaders;

	int incache = 0;
	char * lastmodaux;
	
	int erro;
	int not_modified=0;

	int chunked = check_chunked(partes,nrpartes);
	fd = open(ficheiro, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	bytes = read(sockfd,answer,MAXLEN);


	nrpartes = makeargv(answer,"\r\n\r\n",&partes);

	//Verifica a existencia de erros na resposta do servidor
	i = check_errors(partes[0],nrpartes);

	length = check_length(partes,nrpartes);
	switch(i)
	{
		case 2:
			printf("Estado: HTTP 200 OK\n");
			break;
		case 3:
			printf("Estado: 3xx Redirection\n");
			break;
		case 31:
			printf("Estado: 301 Moved Permanently\n");
			break;
		case 34:
			printf("Estado: 304 Not Modified\n");
			not_modified=1;
			break;
		case 4:
			printf("Estado: 4xx Client Error\n");
			break;
		case 40:
			printf("Estado: 400 Bad Request\n");
			break;
		case 44:
			printf("Estado: 404 Not Found\n");
			break;
		case 5:
			printf("Estado: 5xx Server Error\n");
			break;
	}
	if(!not_modified){	

	//Verifica a data de modificacao do ficheiro e guarda na cache
	lastmodaux= check_modified(partes,nrpartes);
	for(i=0;i<cached;i++)
		if(strcmp(cache[i].path,url)==0)
		{
			incache=1;
			cache[i].lastmodified = lastmodaux;
			break;
		}
	if(!incache)
	{
		cache[cached].lastmodified = lastmodaux;
		cached++;
	}

		
	for (i = 3; i<=1024; i++)
	{
		ant1 = answer[i-3];
		act1 = answer[i-2];
		ant2 = answer[i-1];
		act2 = answer[i];

		if((ant1==ant2) && (act1==act2) && (act1=='\n') && (ant1=='\r'))
		{
			if(chunked)
				get_chunked(fd,answer+i-1,sockfd);
			else
			{
				for(j=0;j<i-2;j++)
					printf("%c",answer[j]);
				if(length<1024 && length>0)
				{
					write(fd,answer+i+1,length);
					aux=length;
				}
				else
				{
					aux = bytes-i-1;
					write(fd,answer+i+1,aux);
				}
			}
			break;
		}
	}
	if(!chunked){
		if(length!=0)
		{
			i = length-aux;
			do
			{
				if(i<=0)
					break;
				if(i>1024)
				{
					read(sockfd, answer,1024);
					write(fd,answer,1024);
					i-=1024;
				}
				else
				{
					read(sockfd,answer,i);
					write(fd,answer,i);
					break;
				}
			}while(1);
		}
		else
		{
			do
			{
				bytes = read(sockfd,answer,1024);
				write(fd,answer,bytes);
			}while(bytes>0);
		}
	}
	printf("Transferencia concluida\n");	
	}
	return 1;
}


int ver_site(char* url, int condicional)
{

	int sockfd;

	//String a ser enviada para o servidor
	char buff[1024];

	//Vari·veis para utilizaÁ„o do makeargv
	char ** tokens;
	int ntokens;
	int i;

	//Tratamento do URL
	char filename[50];
	char * directorio;

	char lastmodified[50];
	char * lastmodaux;
	int incache =0;
	//Alterar posteriormente para mudar a porta... DEFAULT: 80
	ntokens = makeargv(url,"/",&tokens);

	//Guardar o path no ficheiro de cache
	for(i=0;i<cached;i++)
	{
		if(strcmp(cache[i].path,url)==0)
		{
			incache=1;
			break;
		}
	}
	if(!incache)
		cache[cached].path = url;

	//Fazer parse do URL
	if(ntokens>1)
	{
		strtok(url,"/");
		directorio = strtok(NULL,"\0");
	}
	else directorio = " ";

	for(i=0;i<(ntokens-1);i++)
	{
		mkdir(tokens[i],S_IRWXU |S_IRWXG | S_IRWXO);
		chdir(tokens[i]);
	}

	if(ntokens==1)
	{
		mkdir(tokens[0],S_IRWXU|S_IRWXG|S_IRWXO);
		chdir(tokens[0]);
		strcpy(filename,"index.html");
	}
	else
		strcpy(filename,tokens[ntokens-1]);
	

	printf("Full path: %s/%s\n",url,filename);
	if(condicional)
	{
		if(incache)
		{
			for(i=0;i<cached;i++)
			{
				if(strcmp(cache[i].path,url)==0)
				{
					lastmodaux = cache[i].lastmodified;
					incache = 2;
					break;
				}
			}
			if(incache!=2||(strcmp(lastmodaux,"No Info")==0))
				lastmodaux = "";
		}
		else
			lastmodaux = "";
			
		i = sprintf(buff,"GET /%s HTTP/1.1\r\nHOST: %s\r\nIf-Modified-Since: %s\r\n\r\n",directorio,tokens[0],lastmodaux);
		buff[i] ='\0';
	}
	else
	{//Preencher a string a enviar
		i = sprintf(buff,"GET /%s HTTP/1.1\r\nHOST: %s\r\n\r\n",directorio,tokens[0]);
		buff[i] ='\0';
	}

	if((sockfd = create_socket(tokens[0],porta))<0)
	{
		perror("create_socket()");
		exit(0);
	}
	freemakeargv(tokens);
	if(write_socket(buff,sockfd)>0)
	{
		printf("Escrito com sucesso\n%s\n",buff);
	}
	if(getFicheiro(sockfd,filename,url)<0)
	{
		perror("getFicheiro()");
	}
	for(i = 0; i<(ntokens-1);i++)
		chdir("..");
	if(ntokens==1)
		chdir("..");

	close(sockfd);
	return 1;
}

int main(int argc, char*argv[])
{
	int sockfd;
	int portaAUX;

	//Variaveis auxiliares
	int i, opcao;

	//bytes lidos
	int bytes;

	//Teste das Sockets
	char pedido[100];
	
	char answer[MAXLEN];

	FILE * cachefile;

	
	if((cachefile = fopen("cache","a+"))==NULL)
	{
		perror("fopen()");
		return -1;
	}
	else
		if(fill_cache(cachefile)==-1)
		{
			printf("Erro ao ler o ficheiro de cache.\n");
			return -1;
		}
	fclose(cachefile);
	chdir("cachedir");
	printf("Cliente de HTTP/1.1 iniciado com sucesso!\n");

	while((opcao=menu())!=6)
	{
		switch(opcao){
			case 1:
				/*Caso simples */
				printf("Por favor escreva o URL do site a visitar:\n");
				fgets(pedido,sizeof(pedido),stdin);
				i=0;
				while(pedido[i]!='\n')
					i++;
				pedido[i] ='\0';
				ver_site(pedido,0);
				break;
			case 2:
				/* Caso Get -Cond */
				printf("Por favor escreva o URL do site a visitar:\n");
				fgets(pedido,sizeof(pedido),stdin);
				i=0;
				while(pedido[i]!='\n')
					i++;
				pedido[i]='\0';
				ver_site(pedido,1);
				break;
			case 3:
				/* Sacar a cena toda */
				break;
			case 4:
				/* Utilizando um proxy server, mas isto sao contas de outro rosario */
				break;
			case 5:
				/* alterar a porta a utilizar */
				printf("Neste momento estamos a utilizar a porta %d\n", porta);
				printf("Que porta pretende utilizar? Por favor indique de seguida [Porta deve ser > 1024]:\n");
				fgets(pedido,sizeof(pedido),stdin);
				i=0;
				while(pedido[i]!='\n')
					i++;
				pedido[i]='\0';
				portaAUX = atoi(pedido);
				if(portaAUX>1024)
				{
					porta = portaAUX;
					printf("Nova porta a utilizar È: %d\n",porta);
				}
				else
					printf("Valor inv·lido: %d\n Porta definida: %d\n", portaAUX, porta);
				break;
			default:
				printf("Opcao invalida!\n");
				break;
		}
	}
	chdir("..");
	cachefile=fopen("cache","w");
	save_cache(cachefile);	
	fclose(cachefile);
	return 1;
}


