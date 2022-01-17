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
 * @(#)$Id: my_readwrite.h,v 1.2 2020/11/30 19:11:24 william Exp $
 */

#ifndef _MY_READWRITE_H_
#define _MY_READWRITE_H_

/**
 * Read a line from the socket and return a C++ string.
 *         Can call this function repeatedly to read one line at a time.
 *         A line ends with a '\n'.
 *         The last line may be a "partial line", i.e., not ends with a '\n'.
 * Return -1 if there is an error (must ignore the returned C++ string).
 *         End-of-file/end-of-input is considered an error.
 *         After this function returns -1, should continue to return -1 if called again.
 * Otherwise, return the length of the returned C++ string.
 *
 * You should be able to use this function as it.
 * You should only call this function if you are expecting a line of text from socket_fd.
 *
 * @param socket_fd - client socket created by create_client_socket().
 * @param line - returned C++ string.
 */
int read_a_line(int socket_fd, std::string& line);

/**
 * Use this code to write to a socket (not a good idea to call write() directly).
 * Return 0 if successful.
 * Return -1 if there is an error.
 *
 * You should be able to use this function as it.
 *
 * @param fd - socket file descriptor or a regular file descriptor.
 * @param buf - buffer address.
 * @param bytes_to_wrte - number of bytes to write, starting at buf.
 */
int better_write(int fd, const char *buf, int bytes_to_write);

#endif /*_MY_READWRITE_H_*/
