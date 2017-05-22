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
#include <stdbool.h>
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

#define MAX 100 // Maximo de usuarios conectados simultaneamente

#define PORTA 2000 // Usuarios nao root so podem abrir porta da 1024 pra cima
#define LEN 4096 // Tamanho max do buffer da msg a ser lida

struct sockaddr_in local;
struct sockaddr_in remoto;

struct lista_clientes
{
    char nome[40];
    int id;
    char cor[10];
}lc[MAX];

int sockfd; // Descritor socket
long cliente; // Descritor cliente
int len = sizeof(remoto);
char buffer_entrada[LEN];
char buffer_saida[LEN];
int flag = 0;
bool flag2 = true;
pthread_t thread;
pthread_t threadc;

void abertura();
void *sessao();
char *nome_cliente(int cliente_sessao);
void elimina_cliente(int cliente_sessao);
void lista_clientes(int cliente_sessao);
void mensagem_privada(int cliente_sessao);
void mensagem_publica(int cliente_sessao);
void *cor();
void opcoes_cliente(int cliente_sessao);
void opcoes_servidor();
void *comandos();
void envia_teclado(bool op);
void lista_local();
void derruba_cliente(int id, char *cmd);
void encerra_servico(char *cmd);

int main(int argc, char *argv[]) // Ok
{
    system("clear");

    abertura();

    pthread_create(&threadc, NULL, comandos, NULL);

    // Accept clients ------------------------------------------------------------------------------------------------------

    while(1)
    {
        if( (cliente = accept(sockfd, (struct sockaddr*)&remoto, &len)) == -1 ) // permite que o socket aceite conexões
	    {
		    perror("Accept");
		    exit(1);
	    }

        if( pthread_create(&thread, NULL, sessao, (void *)cliente) )
        {
            close(cliente);
        }
    }
}

void *sessao(void* arg) // Ok
{
    //int cliente_sessao = (int) ((void *) arg);
    int cliente_sessao = (long) ((void *) arg);

    memset(buffer_entrada, 0x0, LEN);
    memset(buffer_saida, 0x0, LEN);

    strcpy(buffer_entrada, nome_cliente(cliente_sessao));

    // Insere o nome e id do cliente na lista -----------------------------------------------------------------------------
    for(int i=0; i < MAX; i++)
    {
        if(strcmp(lc[i].nome,"") == 0  && lc[i].id == 0)
        {
            strcpy(lc[i].nome, buffer_entrada);
            lc[i].id = cliente_sessao;
            if(strcmp(lc[i].cor,"") == 0) strcpy(lc[i].cor, cor());
            printf("%sCliente conectado: %s%s", GREEN, NORMAL, lc[i].nome);
            break;
        }
    }

    memset(buffer_entrada, 0x0, LEN);
    memset(buffer_saida, 0x0, LEN);

    sprintf(buffer_saida, "\n%s** Conexao bem sucedida! **%s\n\n", GREEN, NORMAL);
    write(cliente_sessao, buffer_saida, strlen(buffer_saida));
	memset(buffer_saida, 0x0, LEN);

    opcoes_cliente(cliente_sessao);

    // Atividade quando o cliente está conectado --------------------------------------------------------------------------

    //while( (slen = recv(cliente, buffer_entrada, LEN, 0)) > 0 )
    while( read(cliente_sessao,buffer_entrada,LEN-1) > 0)
    {
        buffer_entrada[strlen(buffer_entrada)-1] = '\0';

        if(strcmp(buffer_entrada,"/x") == 0) elimina_cliente(cliente_sessao);
        else if(strcmp(buffer_entrada,"/c") == 0) lista_clientes(cliente_sessao);
        else if(strcmp(buffer_entrada,"/p") == 0) mensagem_privada(cliente_sessao);
        else if(strcmp(buffer_entrada,"/h") == 0) opcoes_cliente(cliente_sessao);
        else if(strcmp(buffer_entrada,"/b") == 0) flag2 = false;
        else if(strcmp(buffer_entrada,"/l") == 0);
        else mensagem_publica(cliente_sessao);

        memset(buffer_entrada, 0x0, LEN);
    }
}

char *nome_cliente(int cliente_sessao) // Ok
{
    strcpy(buffer_saida, "Nome do usuario: ");
    write(cliente_sessao, buffer_saida, strlen(buffer_saida));
	memset(buffer_saida, 0x0, LEN);
    read(cliente_sessao,buffer_entrada,LEN);

    // Verifica a duplicidade do nome ------------------------------------------------------------------------------------
    for(int i=0; i < MAX; i++)
    {
        if(strcmp(buffer_entrada, lc[i].nome) == 0)
        {
            strcpy(buffer_saida, "Nome ja existe, insira um novo\n");
            write(cliente_sessao, buffer_saida, strlen(buffer_saida));
	        memset(buffer_saida, 0x0, LEN);
            memset(buffer_entrada, 0x0, LEN);
            nome_cliente(cliente_sessao);
        }
    }

    char *retorno = buffer_entrada;

    return(retorno);
}

void elimina_cliente(int cliente_sessao) // Ok
{
    // Elimina o nome e id do cliente na lista -----------------------------------------------------------------------------
    for(int i=0; i < MAX; i++)
    {
        if(lc[i].id == cliente_sessao)
        {
            printf("%sCliente desconectado: %s%s", RED, NORMAL, lc[i].nome);
            strcpy(lc[i].nome, "");
            lc[i].id = 0;
            close(cliente_sessao);
            break;
        }
    }
}

void lista_clientes(int cliente_sessao) // Ok
{
    for(int i=0; i < MAX; i++)
    {
        if(strcmp(lc[i].nome,"") != 0  && lc[i].id != 0)
        {
            sprintf(buffer_saida, "%d - %s", i, lc[i].nome);
            write(cliente_sessao, buffer_saida, strlen(buffer_saida));
	        memset(buffer_saida, 0x0, LEN);
        }
    }
}

void mensagem_privada(int cliente_sessao) // Ok
{
    lista_clientes(cliente_sessao);

    strcpy(buffer_saida, "Nº do usuario: ");
    write(cliente_sessao, buffer_saida, strlen(buffer_saida));
	memset(buffer_saida, 0x0, LEN);
    read(cliente_sessao,buffer_entrada,LEN-1);

    int id_destino = (int)strtol(buffer_entrada, (char **)NULL, 10);

    strcpy(buffer_saida, "Msg: ");
    write(cliente_sessao, buffer_saida, strlen(buffer_saida));
	memset(buffer_saida, 0x0, LEN);

    read(cliente_sessao,buffer_entrada,LEN-1);

    for(int i=0; i < MAX; i++)
    {
        if(lc[i].id == cliente_sessao)
        {
            if(flag2) strcpy(buffer_saida, RED);
            strcat(buffer_saida, "(MP) ");
            if(flag2) strcat(buffer_saida, lc[i].cor);
            strcat(buffer_saida, lc[i].nome);
            buffer_saida[strlen(buffer_saida)-1] = '\0';
            if(flag2) strcat(buffer_saida, RED);
            strcat(buffer_saida, " diz: ");
            if(flag2) strcat(buffer_saida, NORMAL);
            strcat(buffer_saida, buffer_entrada);
            break;
        }
    }

    write(lc[id_destino].id, buffer_saida, strlen(buffer_saida));
	
    memset(buffer_saida, 0x0, LEN);
    memset(buffer_entrada, 0x0, LEN);
}

void mensagem_publica(int cliente_sessao) // Ok
{
    for(int i=0; i < MAX; i++)
    {
        if(lc[i].id == cliente_sessao)
        {
            if(flag2) strcpy(buffer_saida, lc[i].cor); // Coloca cor no nome
            strcat(buffer_saida, lc[i].nome);
            break;
        }
    }
    
    buffer_saida[strlen(buffer_saida)-1] = '\0';
    strcat(buffer_saida, " diz: ");
    if(flag2) strcat(buffer_saida, NORMAL); // Volta a cor da letra ao normal
    strcat(buffer_saida, buffer_entrada);
    strcat(buffer_saida, "\n");

    for(int i=0; i < MAX; i++)
    {
        if(lc[i].nome != "" && lc[i].id != 0 && lc[i].id != cliente_sessao)
        {
            write(lc[i].id, buffer_saida, strlen(buffer_saida));
        }
    }

    memset(buffer_saida, 0x0, LEN);
    memset(buffer_entrada, 0x0, LEN);
}

void abertura() // Ok
{
    // Socket -----------------------------------------------------------------------------------------------------------

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd == -1) // Retorna -1 em caso de erro
	{
		perror("Socket");
		exit(1);
	}
	else 
		printf("%sSocket criado com sucesso!%s\n\n", GREEN, NORMAL);

	local.sin_family = AF_INET;
	local.sin_port = htons(PORTA);
    local.sin_addr.s_addr = INADDR_ANY;

	memset(local.sin_zero, 0x0, 8); // Setar sin_zero para zero

    // Binding -------------------------------------------------------------------------------------------------------------

	if ( bind(sockfd,(struct sockaddr *)&local, sizeof(local)) == -1)
	{
		perror("Bind");
		exit(1);
	}

    // Listening -----------------------------------------------------------------------------------------------------------

	listen(sockfd, 100); 
}

void *cor() // Ok
{
    char *cor;

    if(flag == 5) flag = 0;

    if(flag == 0) cor = GREEN;
    else if(flag == 1) cor = YELLOW;
    else if(flag == 2) cor = BLUE;
    else if(flag == 3) cor = MARGENTA;
    else if(flag == 4) cor = CYAN;

    flag++;

    return(cor);
}

void opcoes_cliente(int cliente_sessao) // Ok
{
    strcat(buffer_saida, "Digite a qualquer momento:");
    strcat(buffer_saida, "\n/c Para listar os clientes conectados");
    strcat(buffer_saida, "\n/p Para enviar uma mensagem privada");
    strcat(buffer_saida, "\n/l Para limpar a tela");
    strcat(buffer_saida, "\n/x Para encerrar a conexao");
    strcat(buffer_saida, "\n/h Para visualizar as opcoes");
    strcat(buffer_saida, "\n/b Caso tenha problemas para visualizar com cores");
    strcat(buffer_saida, "\nPara conversa publica basta digitar e enviar com  o ENTER\n\n");
    write(cliente_sessao, buffer_saida, strlen(buffer_saida));
	memset(buffer_saida, 0x0, LEN);
}

void opcoes_servidor() // Ok
{
    strcat(buffer_saida, "Digite a qualquer momento:");
    strcat(buffer_saida, "\n/c Para listar os clientes conectados");
    strcat(buffer_saida, "\n/p Para enviar uma mensagem a um cliente");
    strcat(buffer_saida, "\n/m Para enviar uma mensagem a todos os cliente");
    strcat(buffer_saida, "\n/d Para derrubar a conexão de um cliente");
    strcat(buffer_saida, "\n/l Para limpar a tela");
    strcat(buffer_saida, "\n/x Para encerrar o servico");
    strcat(buffer_saida, "\n/h Para visualizar as opcoes");
    strcat(buffer_saida, "\n/b Caso tenha problemas para visualizar com cores\n\n");
    printf("%s", buffer_saida);
	memset(buffer_saida, 0x0, LEN);
}

void *comandos()
{
    opcoes_servidor();

    char *cmd;
    cmd = (char *) malloc(10);

    while(1)
    {
        if( fgets(cmd, 10, stdin) > 0 )
        {
            setbuf(stdin, 0);

            if(strcmp(cmd,"/p\n") == 0)
	        {
                lista_local();
                envia_teclado(true);
            }
            else if(strcmp(cmd,"/d\n") == 0)
            {
                lista_local();
                int num;
                printf("Nº do usuario: \n");
                scanf("%d", &num);
                setbuf(stdin, 0);
                derruba_cliente(num, cmd);
                free(cmd);
            }
            else if(strcmp(cmd,"/c\n") == 0) lista_local();
            else if(strcmp(cmd,"/l\n") == 0) system("clear");
            else if(strcmp(cmd,"/m\n") == 0) envia_teclado(false);
            else if(strcmp(cmd,"/h\n") == 0) opcoes_servidor();
            else if(strcmp(cmd,"/b\n") == 0) flag2 = false;
            else if(strcmp(cmd,"/x\n") == 0) encerra_servico(cmd);
        }
    }
}

void encerra_servico(char *cmd)
{
    for(int i=0; i < MAX; i++)
    {
        if(lc[i].nome != "" && lc[i].id != 0)
        {
            derruba_cliente(i, cmd);
        }
    }
    pthread_cancel(thread);
    close(sockfd);
    pthread_cancel(threadc);
    exit(1);
}

void derruba_cliente(int id, char *cmd)
{
    write(lc[id].id, cmd, strlen(cmd));
    printf("%sCliente desconectado: %s%s", RED, NORMAL, lc[id].nome);
    strcpy(lc[id].nome, "");
    lc[id].id = 0;
}

void lista_local()
{
    for(int i=0; i < MAX; i++)
    {
        if(strcmp(lc[i].nome,"") != 0  && lc[i].id != 0)
        {
            printf("%d - %s", i, lc[i].nome);
        }
    }
}

void envia_teclado(bool op) // Ok
{
    int num;
    if(op)
    {
    printf("Nº do usuario: \n");
    scanf("%d", &num);
    setbuf(stdin, 0);
    }
    printf("Msg: \n");
    fgets(buffer_entrada, LEN, stdin);
    buffer_entrada[strlen(buffer_entrada)-1] = '\0';
    if(flag2) strcat(buffer_saida, RED);
    strcat(buffer_saida, "Admin diz: ");
    strcat(buffer_saida, buffer_entrada);
    if(flag2) strcat(buffer_saida, NORMAL);
    strcat(buffer_saida, "\n");

	if(op) write(lc[num].id, buffer_saida, strlen(buffer_saida));
    else
    {
        for(int i=0; i < MAX; i++)
        {
            if(lc[i].nome != "" && lc[i].id != 0)
            {
                write(lc[i].id, buffer_saida, strlen(buffer_saida));
            }
        }
    }

	setbuf(stdin, 0);
    memset(buffer_entrada, 0x0, LEN);
	memset(buffer_saida, 0x0, LEN);
}
