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
 * @(#)$Id: my_timestamp.cpp,v 1.2 2020/12/22 06:34:57 william Exp $
 */

/* C++ standard include files first */
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

/* C system include files next */
#include <sys/time.h>

/* C standard include files next */
#include <string.h>

/* your own include last */
#include "my_timestamp.h"

string format_timestamp(struct timeval *timestamp)
{
    char time_buf[26];
    char timestamp_buf[25];

    strcpy(time_buf, ctime(&timestamp->tv_sec));
    /* time_buf now contains something like "Thu Jan  9 08:37:19 2020\n" */
    /* first, copy the time-of-day, month, date, and a space character */
    for (int i=0; i < 11; i++) {
        timestamp_buf[i] = time_buf[i];
    }
    /* then copy the year */
    for (int i=11; i < 15; i++) {
        timestamp_buf[i] = time_buf[i+9];
    }
    /* then copy the space character, followed by hour, minute, second */
    for (int i=15; i < 24; i++) {
        timestamp_buf[i] = time_buf[i-5];
    }
    timestamp_buf[24] = '\0';

    stringstream ss(timestamp_buf, ios::ate|ios::in|ios::out);
    ss << "." << setfill('0') << setw(6) << ((int)timestamp->tv_usec);

    return ss.str();
}

string get_timestamp_now()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return format_timestamp(&now);
}

int timestamp_cmp(struct timeval *t1, struct timeval *t2)
    /* return negative, 0, or positive value if t1 is less than, equal to, or grader than t2 */
{
    int diff = ((int)(t1->tv_sec)) - ((int)(t2->tv_sec));

    if (diff != 0) {
        return diff;
    }
    return ((int)(t1->tv_usec)) - ((int)(t2->tv_usec));
}

double timestamp_diff_in_seconds(struct timeval *older, struct timeval *newer)
{
    struct timeval elapsed;
    timersub(newer, older, &elapsed);
    return ((double)(elapsed.tv_sec)) + ((double)(elapsed.tv_usec)) / ((double)1000000);
}

string str_timestamp_diff_in_seconds(struct timeval *older, struct timeval *newer)
{
    struct timeval elapsed;
    timersub(newer, older, &elapsed);

    char buf[40];
    snprintf(buf, sizeof(buf), "%d.%06d", (int)(elapsed.tv_sec), (int)(elapsed.tv_usec));
    return buf;
}

void convert_seconds_to_timestamp(double d_seconds, struct timeval *timestamp_return)
{
    int sec = (int)d_seconds;
    double remainder = (d_seconds - ((double)sec));
    int usec = (int)(remainder*((double)1000000));
    if (usec < 0) {
        usec = 0;
    }
    timestamp_return->tv_sec = sec;
    timestamp_return->tv_usec = usec;
}

void add_seconds_to_timestamp(struct timeval *older, double d_seconds, struct timeval *newer_return)
{   
    struct timeval interval;
    convert_seconds_to_timestamp(d_seconds, &interval);
    timeradd(older, &interval, newer_return);
}   

