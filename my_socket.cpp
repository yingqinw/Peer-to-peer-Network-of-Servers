/******************************************************************************/
/* Important CSCI 353 usage information:                                      */
/*                                                                            */
/* This fils is part of CSCI 353 programming assignments at USC.              */
/*         53616c7465645f5faf454acd8de2536dc9ae92d5904738f3b41eed2fa2af       */
/*         35d28e0a435e5f50414281de61958c65444cbd5b46aa7eb42f2c0258ca8a       */
/*         cf3ad79d6e72cd90806987130037a9068c552efff12cec1996639568           */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/*         You are also NOT permitted to distribute or publically display     */
/*         any file (or ANY PART of it) that uses/includes this file.         */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block if you    */
/*         submit this file for grading.                                      */
/******************************************************************************/

/*
 * Author:      William Chia-Wei Cheng (bill.cheng@usc.edu)
 *
 * @(#)$Id: my_socket.cpp,v 1.16 2021/01/21 05:00:33 william Exp $
 */

/* C++ standard include files first */
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

/* C system include files next */
#include <arpa/inet.h>
#include <netdb.h>

/* C standard include files next */
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* your own include last */
#include "my_socket.h"

/**
 * Use this code to setup a struct addrinfo data structure.
 *
 * @param host_name - hostname C++ string of an Internet host.
 * @param port_number_string - TCP port number in a C++ string.
 * @param result - addressing information to be used to create a socket, this is a return pointer value.
 */
static
void my_getaddrinfo(const string hostname_string, const string port_number_string, struct addrinfo **result)
{
    struct addrinfo hints;

    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_NUMERICSERV|AI_ADDRCONFIG;

    if (getaddrinfo(hostname_string.c_str(), port_number_string.c_str(), &hints, result) != 0) {
        perror("getaddrinfo() system call");
        exit(-1);
    }
}

/**
 * Use this code to create a server master socket or a client socket.
 *
 * You should be able to use this function as it.
 *
 * @param host_name - hostname C++ string of an Internet host.
 * @param port_number_string - TCP port number in a C++ string.
 * @param result - additional information about the socket, this is a return pointer value.
 * @return a socket file descriptor.
 */
static
int create_stream_socket_for_server(const string hostname_string, const string port_number_string, struct addrinfo **result)
{
    int socket_fd = (-1);

    my_getaddrinfo(hostname_string, port_number_string, result);
    socket_fd = socket((*result)->ai_family, (*result)->ai_socktype, (*result)->ai_protocol);
    if (socket_fd == (-1)) {
        perror("socket() system call");
        exit(-1);
    }
    return socket_fd;
}

/**
 * Use this code to create a master socket to be used by a server.
 * For this class, a server only serve on LOCALHOST (which is a compiler defined string and must be specified when you compile a module that #include this module).
 *
 * You should use this function as it.
 * When you run g++ to compile this module, you must begin g++ with:
 *     g++ -g -Wall -std=c++11 -DLOCALHOST=\"127.0.0.1\"
 *
 * @param port_number_string - port number of the well-known/welcome port.
 * @return master socket file descriptor.
 */
int create_master_socket(const string port_number_string)
{
    struct addrinfo* res = NULL;
    int socket_fd = (-1);
    int reuse_addr = 1;
    string hostname_string = LOCALHOST;

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal() system call");
        exit(-1);
    }
    socket_fd = create_stream_socket_for_server(hostname_string, port_number_string, &res);
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)(&reuse_addr), sizeof(int)) == (-1)) {
        perror("setsockopt() system call");
        exit(-1);
    }
    if (::bind(socket_fd, res->ai_addr, res->ai_addrlen) == (-1)) {
        perror("bind() system call");
        exit(-1);
    }
    freeaddrinfo(res);
    if (listen(socket_fd, 5) == (-1)) {
        perror("listen() system call");
        exit(-1);
    }
    return socket_fd;
}

/**
 * Use this code on the server side to return client's IP and port information in a string.
 *
 * You should be able to use this function as it.
 *
 * @param socket_fd - either master socket returned by create_master_socket() or newsockfd returned by my_accept().
 * @param server_side - if 0, return client's IP and port, otherwise, return peer's (i.e., server's) IP and port.
 *                      must == 1 if socket_fd is the master socket.
 * @return a C++ string containing client's IP and port information (e.g., "127.0.0.1:33046").
 */
string get_ip_and_port_for_server(int socket_fd, int server_side)
{
    struct sockaddr_in socket_info;
    socklen_t socket_info_addr_len = (socklen_t)sizeof(socket_info);
    stringstream ss;
    /* char buf[80]; */

    if (server_side) {
        getsockname(socket_fd, (struct sockaddr *)(&socket_info), &socket_info_addr_len);
        /* snprintf(buf, sizeof(buf), "%s:%1d", inet_ntoa(socket_info.sin_addr), (int)htons((uint16_t)(socket_info.sin_port & 0x0ffff))); */
    } else {
        getpeername(socket_fd, (struct sockaddr *)(&socket_info), &socket_info_addr_len);
        /* snprintf(buf, sizeof(buf), "%s:%1d", inet_ntoa(socket_info.sin_addr), (int)htons((uint16_t)(socket_info.sin_port & 0x0ffff))); */
    }
    ss << inet_ntoa(socket_info.sin_addr) << ":" << (int)htons((uint16_t)(socket_info.sin_port & 0x0ffff));
    return ss.str();
}

/**
 * Call accept() on the master socket to wait for a client to connect.
 * If no client connects, the function would block indefinitely (unless an error occurs).
 * If a client connects, this function would create a new socket file descriptor for communicating with the client.
 *
 * You should be able to use this function as it.
 *
 * @param master_socket_fd - master socket created by create_master_socket().
 * @return (-1) if there is an error.
 */
int my_accept(const int master_socket_fd)
{
    int newsockfd = (-1);

    while (newsockfd == (-1)) {
        struct sockaddr_in cli_addr;
        unsigned int clilen = sizeof(cli_addr);

        newsockfd = accept(master_socket_fd, (struct sockaddr *)(&cli_addr), &clilen);
        if (newsockfd == (-1)) {
            if (errno == EINTR) {
                /* not an error, try again */
                continue;
            }
            /* error, will return (-1) */
            return (-1);
        }
        break;
    }
    return newsockfd;
}

/**
 * Use this code on the client side to return IP and port information in a string.
 *
 * You should be able to use this function as it.
 *
 * @param client_socket_fd - client socket returned from create_client_socket_and_connect().
 * @param client_side - if 0, return the peer's (i.e., server's) IP and port, otherwise, return client's IP and port.
 * @return a C++ string containing requested IP and port information (e.g., "127.0.0.1:33046").
 */
string get_ip_and_port_for_client(int client_socket_fd, int client_side)
{
    struct sockaddr_in socket_info;
    socklen_t socket_info_addr_len = (socklen_t)sizeof(socket_info);
    char buf[80];

    if (client_side) {
        getsockname(client_socket_fd, (struct sockaddr *)(&socket_info), &socket_info_addr_len);
        snprintf(buf, sizeof(buf), "%s:%1d", inet_ntoa(socket_info.sin_addr), (int)htons((uint16_t)(socket_info.sin_port & 0x0ffff)));
    } else {
        getpeername(client_socket_fd, (struct sockaddr *)(&socket_info), &socket_info_addr_len);
        snprintf(buf, sizeof(buf), "%s:%1d", inet_ntoa(socket_info.sin_addr), (int)htons((uint16_t)(socket_info.sin_port & 0x0ffff)));
    }
    return buf;
}

/**
 * Use this code to create a client socket and connect to a server.
 *
 * You should be able to use this function as it.
 *
 * @param hostname_string - hostname of the server you would like to connect to (empty string means LOCALHOST).
 * @param port_number_string - port number of the server's well-known/welcome port.
 * @return client socket file descriptor.
 */
int create_client_socket_and_connect(const string hostname_string, const string port_number_string)
{
    int client_socket_fd = (-1);
    struct addrinfo *res = NULL;
    string host = ((hostname_string[0] == '\0') ? LOCALHOST : hostname_string);

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal() system call");
        exit(-1);
    }
    my_getaddrinfo(host, port_number_string, &res);
    struct addrinfo *ai_ptr = NULL;
    for (ai_ptr = res; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {
        client_socket_fd = socket(ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
        if (client_socket_fd == (-1)) {
            continue;
        }
        if (connect(client_socket_fd, ai_ptr->ai_addr, ai_ptr->ai_addrlen) != (-1)) {
            break;
        }
        close(client_socket_fd);
        client_socket_fd = (-1);
    }
    freeaddrinfo(res);

    return client_socket_fd;
}

