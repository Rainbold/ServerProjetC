#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <SDL_net.h>

#define NB_CLIENTS 2
#define NB_CLIENTS_MAX 4

typedef enum state
{
	OFFLINE = 0,
	ONLINE
} state_t;

struct client_info
{
	int id, x, y;
	IPaddress *remoteIP;
	TCPsocket sd, csd;
};

struct server_info
{
	int nb_clients;
	pthread_t thread_clients[NB_CLIENTS_MAX];
	state_t clients_state[NB_CLIENTS_MAX];
	struct client_info clients_info[NB_CLIENTS_MAX];
};

struct server_info* server_init(int nb_clients)
{
	struct server_info* serv = malloc(sizeof(*serv));
		serv->nb_clients = nb_clients;

	return serv;
}

static void* client_connection(void* p_data)
{
	struct client_info* client = p_data;
	state_t state = ONLINE;
	while(1)
	{
		if ((client->csd = SDLNet_TCP_Accept(client->sd)))
		{
			break;
		}
	}

	return (void*)state;
} 



static void* server_listening(void* p_data)
{
	struct server_info* serv = p_data;
	while(1)
	{
		for(int i=0; i<serv->nb_clients; i++) {
			if(serv->clients_state[i]) {
				printf("Client %d : ONLINE\n", i);
			}
			else
				printf("Client %d : OFFLINE\n", i);
		}
	}

	return NULL;
} 

int main(void)
{
	TCPsocket sd; /* Socket descriptor, Client socket descriptor */
	IPaddress ip;
	// int quit, quit2;
	// char buffer[512];
	pthread_t thread_server;

	printf("SDL_Net initialization.\n");

	if (SDLNet_Init() < 0)
	{
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	// Network interface is now listening on port 8888
	if (SDLNet_ResolveHost(&ip, NULL, 8888) < 0)
	{
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	// Open a connection with the given server type address
	if (!(sd = SDLNet_TCP_Open(&ip)))
	{
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	printf("Server initialization.\n");
	int ret = 0;
	struct server_info* serv = server_init(NB_CLIENTS);
	ret = pthread_create(&thread_server, NULL, server_listening, serv);

	if(ret)
		fprintf (stderr, "%s\n", strerror (ret));
	else {
		for(int i=0; i<serv->nb_clients; i++) {
			serv->clients_info[i].sd = sd;
			serv->clients_state[i] = OFFLINE;
			ret = pthread_create(&serv->thread_clients[i], NULL, client_connection, &serv->clients_info[i]);
			if(ret)
				fprintf (stderr, "%s\n", strerror (ret));
			pthread_join(serv->thread_clients[i], (void *)&serv->clients_state[i]);
		}
	}	


	return EXIT_SUCCESS;
}