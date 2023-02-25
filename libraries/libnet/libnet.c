#include "lua.h"
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

static int l_socket(lua_State *L) {
    int domain = luaL_checkinteger(L, 1);
    int type = luaL_checkinteger(L, 2);
    int protocol = luaL_checkinteger(L, 3);
    int sockfd = socket(domain, type, protocol);
    lua_pushinteger(L, sockfd);
    return 1;
}

static int l_connect(lua_State *L) {
    const char *host = luaL_checkstring(L, 1);
    int port = luaL_checkinteger(L, 2);
    int sockfd = luaL_checkinteger(L, 3);
    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(host);
    if (server == NULL) {
        luaL_error(L, "ERROR, no such host\n");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        luaL_error(L, "ERROR connecting\n");
    }
    return 0;
}

static int l_send(lua_State *L) {
    int sockfd = luaL_checkinteger(L, 1);
    const char *message = luaL_checkstring(L, 2);
    int len = strlen(message);
    int bytes_sent = send(sockfd, message, len, 0);
    if (bytes_sent < 0) {
        luaL_error(L, "ERROR writing to socket\n");
    }
    lua_pushinteger(L, bytes_sent);
    return 1;
}

static int l_recv(lua_State *L) {
    int sockfd = luaL_checkinteger(L, 1);
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        luaL_error(L, "ERROR reading from socket\n");
    }
    lua_pushlstring(L, buffer, bytes_received);
    return 1;
}

static int l_bind(lua_State *L) {
    int sockfd = luaL_checkinteger(L, 1);
    const char *host = luaL_checkstring(L, 2);
    int port = luaL_checkinteger(L, 3);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host);
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        luaL_error(L, "ERROR on binding\n");
    }
    return 0;
}

static int l_accept(lua_State *L) {
    int sockfd = luaL_checkinteger(L, 1);

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
        //luaL_error(L, "ERROR on accept\n");
        return 0;    
    }

    char ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(cli_addr.sin_addr), ip_address, INET_ADDRSTRLEN);

    lua_pushinteger(L, newsockfd);
    lua_pushstring(L, ip_address);
    lua_pushinteger(L, ntohs(cli_addr.sin_port));
    return 3;
}

static int l_sendto(lua_State *L) {
    int sockfd = luaL_checkinteger(L, 1);
    const char *message = luaL_checkstring(L, 2);
    const char *host = luaL_checkstring(L, 3);
    int port = luaL_checkinteger(L, 4);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host);
    serv_addr.sin_port = htons(port);

    int len = strlen(message);
    int bytes_sent = sendto(sockfd, message, len, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (bytes_sent < 0) {
        luaL_error(L, "ERROR writing to socket\n");
    }
    lua_pushinteger(L, bytes_sent);
    return 1;
}

static int l_listen(lua_State *L) {
    int sockfd = luaL_checkinteger(L, 1);
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    char buffer[BUFFER_SIZE];
    int bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);

    if (bytes_received < 0) {
        //luaL_error(L, "ERROR reading from socket\n");
        return 0;
    }

    lua_newtable(L);
    lua_pushstring(L, inet_ntoa(cli_addr.sin_addr));
    lua_setfield(L, -2, "ip");
    lua_pushinteger(L, ntohs(cli_addr.sin_port));
    lua_setfield(L, -2, "port");
    lua_pushlstring(L, buffer, bytes_received);
    lua_setfield(L, -2, "data");

    return 1;
}

static int close_socket(lua_State *L) {
    int sockfd = luaL_checkinteger(L, 1);
    close(sockfd);
    return 0;
}

int luaopen_libnet(lua_State* L) {
    luaL_Reg socket_funcs[] = {
        {"socket", l_socket},
        {"connect", l_connect},
        {"send", l_send},
        {"recv", l_recv},
        {"close", close_socket},
        {"bind", l_bind},
        {"listen", l_listen},
        {"sendto", l_sendto},
        {"accept", l_accept},
        {NULL, NULL}
    };

    /* Регистрируем таблицу socket */
    lua_newtable(L);
    lua_pushinteger(L, AF_INET);
    lua_setfield(L, -2, "AF_INET");

    lua_pushinteger(L, SOCK_STREAM);
    lua_setfield(L, -2, "SOCK_STREAM");
    luaL_setfuncs(L, socket_funcs, 0);
    lua_setglobal(L, "socket");

    return 1;
}
