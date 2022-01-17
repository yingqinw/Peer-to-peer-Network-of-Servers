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
 * @(#)$Id: my_socket.h,v 1.6 2020/12/08 20:29:21 william Exp $
 */

#ifndef _MY_SOCKET_H_
#define _MY_SOCKET_H_

/**
 * Use this code to create a master socket to be used by a server.
 * For this class, a server only serve on LOCALHOST (which is a compiler defined string and must be specified when you compile a module that #include this module).
 *
 * You should be able to use this function as it.
 *
 * @param port_number_string - port number of the well-known/welcome port.
 * @return master socket file descriptor.
 */
int create_master_socket(const std::string port_number_string);

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
std::string get_ip_and_port_for_server(int socket_fd, int server_side);

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
int my_accept(const int master_socket_fd);

/**
 * Use this code on the client side to return IP and port information in a string.
 *
 * You should be able to use this function as it.
 *
 * @param client_socket_fd - client socket returned from create_client_socket_and_connect().
 * @param client_side - if 0, return the peer's (i.e., server's) IP and port, otherwise, return client's IP and port.
 * @return a C++ string containing requested IP and port information (e.g., "127.0.0.1:33046").
 */
std::string get_ip_and_port_for_client(int client_socket_fd, int client_side);

/**
 * Use this code to create a client socket and connect to a server.
 *
 * You should be able to use this function as it.
 *
 * @param hostname_string - hostname of the server you would like to connect to (empty string means LOCALHOST).
 * @param port_number_string - port number of the server's well-known/welcome port.
 * @return client socket file descriptor.
 */
int create_client_socket_and_connect(const std::string hostname_string, const std::string port_number_string);

#endif /*_MY_SOCKET_H_*/
