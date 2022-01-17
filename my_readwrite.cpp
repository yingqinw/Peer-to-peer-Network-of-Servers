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
 * @(#)$Id: my_readwrite.cpp,v 1.5 2020/11/30 19:52:58 william Exp $
 */

/* C++ standard include files first */
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

/* C system include files next */
#include <sys/socket.h>

/* C standard include files next */
#include <unistd.h>
#include <errno.h>

/* your own include last */
#include "my_readwrite.h"

using namespace std;

static
int non_ASCII(char ch)
{
    if (ch >= 0x20 && ch < 0x7f) return 0;
    switch (ch) {
    case '\r': return 0;
    case '\n': return 0;
    case '\t': return 0;
    default: break;
    }
    return 1;
}

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
int read_a_line(int socket_fd, string& line)
{
    
    string s = "";
    int idx = 0;
    char ch = '\0';
    int debug = 1;

    for (;;) {
        int bytes_read = read(socket_fd, &ch, 1);
        if (bytes_read < 0) {
            if (errno == EINTR) {
                /* not a real error, must retry */
                continue;
            }
            /* a real error, no need to return a line */
            return (-1);
        } else if (bytes_read == 0) {
            /*
             * according to man pages, 0 means end-of-file
             * if we don't break here, read() will keep returning 0!
             */
            if (idx == 0) {
                /* if no data has been read, just treat end-of-file as an error */
                return (-1);
            }
            /*
             * the last line does not terminate with '\n'
             * return the last line (which does not end with '\n')
             */
            break;
        } else {
            /*
             * being super paranoid and harsh here
             * if you are expecting binary data, you shouldn't be calling read_a_line()
             */
            if (debug && non_ASCII(ch)) {
                /*
                 * if you don't want to abort and crash your program here, you can set debug = 0 above
                 * although I would strongly encourage you not to do that and fix your bugs instead
                 */
                cerr << "Encountered a non-ASCII character (0x" << setfill('0') << setw(2) << hex << (int)ch << ") in read_a_line().  Abort program!" << endl;
                shutdown(socket_fd, SHUT_RDWR);
                close(socket_fd);
                exit(-1);
            }
            s += ch;
            idx++;
            if (ch == '\n') {
                break;
            }
        }
    }
    line = s;
    return idx;
}

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
int better_write(int fd, const char *buf, int bytes_to_write)
{
    int bytes_remaining = bytes_to_write;

    while (bytes_remaining > 0) {
        int bytes_written = write(fd, buf, bytes_remaining);

        if (bytes_written > 0) {
            bytes_remaining -= bytes_written;
            buf += bytes_written;
        } else if (bytes_written == (-1)) {
            if (errno == EINTR) {
                continue;
            }
            /* a real error, abort write() */
            return (-1);
        }
    }
    return bytes_to_write;
}
