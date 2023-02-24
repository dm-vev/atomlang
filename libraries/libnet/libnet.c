#include "atom.h"
#include "lauxlib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

static int l_socket(atom_State *L) {
    int domain = atomL_checkinteger(L, 1);
    int type = atomL_checkinteger(L, 2);
    int protocol = atomL_checkinteger(L, 3);
    int sockfd = socket(domain, type, protocol);
    atom_pushinteger(L, sockfd);
    return 1;
}

static int l_connect(atom_State *L) {
    const char *host = atomL_checkstring(L, 1);
    int port = atomL_checkinteger(L, 2);
    int sockfd = atomL_checkinteger(L, 3);
    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(host);
    if (server == NULL) {
        atomL_error(L, "ERROR, no such host\n");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        atomL_error(L, "ERROR connecting\n");
    }
    return 0;
}

static int l_send(atom_State *L) {
    int sockfd = atomL_checkinteger(L, 1);
    const char *message = atomL_checkstring(L, 2);
    int len = strlen(message);
    int bytes_sent = send(sockfd, message, len, 0);
    if (bytes_sent < 0) {
        atomL_error(L, "ERROR writing to socket\n");
    }
    atom_pushinteger(L, bytes_sent);
    return 1;
}

static int l_recv(atom_State *L) {
    int sockfd = atomL_checkinteger(L, 1);
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        atomL_error(L, "ERROR reading from socket\n");
    }
    atom_pushlstring(L, buffer, bytes_received);
    return 1;
}


static int close_socket(atom_State *L) {
    int sockfd = atomL_checkinteger(L, 1);
    close(sockfd);
    return 0;
}

int atomopen_libnet(atom_State* L) {
    atomL_Reg socket_funcs[] = {
        {"socket", l_socket},
        {"connect", l_connect},
        {"send", l_send},
        {"recv", l_recv},
        {"close", close_socket},
        {NULL, NULL}
    };

    /* Регистрируем таблицу socket */
    atom_newtable(L);
    atom_pushinteger(L, AF_INET);
    atom_setfield(L, -2, "AF_INET");

    atom_pushinteger(L, SOCK_STREAM);
    atom_setfield(L, -2, "SOCK_STREAM");
    atomL_setfuncs(L, socket_funcs, 0);
    atom_setglobal(L, "socket");

    return 1;
}
