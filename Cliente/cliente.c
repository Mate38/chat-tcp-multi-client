/**
    Desenvolvedor: Mateus Cardoso
    Email: matecardoso38@gmail.com
    Ano: 2017
**/
/*
	Para testar a conexão com o servidor no terminal: $ nc localhost 2000
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // Biblioteca para tratamento de erros
#include <unistd.h> // Para o close()
// Headers para socket
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h> // man pthread_create

#define BLACK "\033[30m" 
#define RED "\033[31m" 
#define GREEN "\033[32m" 
#define YELLOW "\033[33m" 
#define BLUE "\033[34m"
#define MARGENTA "\033[35m" 
#define CYAN "\033[36m" 
#define WHITE "\033[37m"
#define NORMAL "\033[39m"

#define PORTA 2000 // Usuarios nao root so podem abrir porta da 1024 pra cima
#define LEN 4096 // Tamanho max do buffer da msg a ser lida

struct sockaddr_in remoto;

int sockfd; // Descritor socket
int len = sizeof(remoto);
char buffer_entrada[LEN];
char buffer_saida[LEN];
int flag = 0;
pthread_t thread;

void abertura();
void *recebe();
void envia_teclado();
void ip_servidor();

int main(int argc, char**argv) // Ok
{
	system("clear");

	ip_servidor();

	pthread_create(&thread, NULL, recebe, NULL);

	while(1)
	{
		envia_teclado();
		if(flag == 1) break;
	}

	pthread_exit(NULL);
	close(sockfd);
	exit(1);
}

void ip_servidor()
{
	char op[1];
	printf("Usar IP armazenado? (s/n)\n");
	scanf("%c", op);

	if(strcmp(op,"s") == 0)
	{
		char url[]="config", info[20];
		FILE *arq;
		arq = fopen(url, "r");
		if(arq == NULL)
		{
				printf("Erro, nao foi possivel abrir o arquivo\n");
		}
		else
			while( (fgets(info, sizeof(info), arq))!=NULL );

		fclose(arq);
		setbuf(stdin, 0);
		system("clear");

		abertura(info);
	}
	else if(strcmp(op,"n") == 0)
	{
		char info[20];

		printf("IP do servidor: \n");
		scanf("%s20", info);
		setbuf(stdin, 0);
		system("clear");
		
		abertura(info);
	}
	else 
	{
		printf("Comando inexistente!\n");
		exit(1);
	}
}

void envia_teclado() // Ok
{
	fgets(buffer_saida, LEN, stdin);
	write(sockfd, buffer_saida, strlen(buffer_saida));

	//__fpurge(stdin);
	setbuf(stdin, 0);

	if(strcmp(buffer_saida,"/x\n") == 0)
	{
		//pthread_exit(recebe);
		//close(sockfd);
		exit(1);
	}
	else if(strcmp(buffer_saida,"/l\n") == 0) system("clear");

	memset(buffer_saida, 0x0, LEN);
}

void *recebe() // Ok
{	
	while(1)
	{
		if( read(sockfd,buffer_entrada,LEN) > 0)
		{	
			if(strcmp(buffer_entrada,"/d\n") == 0)
			{
				printf("%sSua conexao foi interrompida pelo servidor!%s\n", RED, NORMAL);
				exit(1);
			}
			else if(strcmp(buffer_entrada,"/x\n") == 0)
			{
				printf("%sSua conexao foi interrompida pelo servidor!%s\n", RED, NORMAL);
				exit(1);
			}
			buffer_entrada[strlen(buffer_entrada)-1] = '\0';	
			printf("%s\n", buffer_entrada);
			memset(buffer_entrada, 0x0, LEN);
		}
	}
}

void abertura(char * ip) // Ok
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd == -1) // Retorna -1 em caso de erro
	{
		perror("Socket");
		exit(1);
	}
	else 
		printf("Socket criado com sucesso!\n\n");

	remoto.sin_family = AF_INET;
	remoto.sin_port = htons(PORTA);
	//remoto.sin_addr.s_addr = inet_addr("127.0.0.1");
	//remoto.sin_addr.s_addr = inet_addr("192.168.2.104");
	remoto.sin_addr.s_addr = inet_addr(ip);

	memset(remoto.sin_zero, 0x0, 8); // Setar sin_zero para zero

	if( connect(sockfd, (struct sockaddr*)&remoto, len) == -1)
	{
		perror("Connect");
		exit(1);
	}
}
