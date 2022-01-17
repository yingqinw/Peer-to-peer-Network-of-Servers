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
 * Author:      Bill Cheng (bill.cheng@usc.edu)
 *
 * @(#)$Id: my_timestamp.h,v 1.4 2020/12/22 06:43:33 william Exp $
 */

#ifndef _MY_TIMESTAMP_H_
#define _MY_TIMESTAMP_H_
#include <string>

/**
 * Format the timestamp argument in the DoW MON DA YEAR HH:MM:SS.MICROS format and return it in a string.
 *
 * @param timestamp - timestamp to be formatted
 * @return a formatted timestamp.
 */
std::string format_timestamp(struct timeval *timestamp);

/**
 * Return a string that represents the current time.
 *
 * @return a string that represents the current time in the DoW MON DA YEAR HH:MM:SS.MICROS format.
 */
std::string get_timestamp_now(void);

/**
 * This function compares the two timestamps t1 and t2.  It returns an intger less than, equal to, or greater than zero if
 *         t1 is found, respectively, to be less than (earlier than), equal to, or greater than (later than) t2.
 * This function is analogous to strcmp(s1,s2) which returns an integer less than, equal to, or greater than zero if
 *         s1 is found, respectively, to be less than, equal to, or be greater than s2.
 *
 * @param t1 - a timestamp
 * @param t2 - a timestamp
 * @return an integer less than, equal to, or greater than zero if t1 is earlier than, equal to, or ater than t2, respectively.
 */
int timestamp_cmp(struct timeval *t1, struct timeval *t2);

/**
 * Return the difference between two timestamps that were obtained from calling gettimeofday().
 *
 * @param older - an older timestamp
 * @param newer - a newer timestamp
 * @return the (newer timestamp - older timestamp) in the unit of seconds.
 */
double timestamp_diff_in_seconds(struct timeval *older, struct timeval *newer);

/**
 * Format and return the difference between two timestamps that were obtained from calling gettimeofday().
 *
 * @param older - an older timestamp
 * @param newer - a newer timestamp
 * @return the (newer timestamp - older timestamp) in the unit of seconds.
 */
std::string str_timestamp_diff_in_seconds(struct timeval *older, struct timeval *newer);

/**
 * Converts time (in seconds) in the 1st argument to a timestamp and return the timesamp in the 2nd argument.
 *
 * @param d_seconds - number of seconds to convert
 * @param timestamp_return - returned timestamp with (*newer_return = d_seconds)
 */
void convert_seconds_to_timestamp(double d_seconds, struct timeval *timestamp_return);

/**
 * Add time (in seconds) in the 2nd argument to the timestamp in the 1st argument and return the sum in the 3rd argument.
 *
 * @param older - an older timestamp
 * @param d_seconds - number of seconds to add to older
 * @param newer_return - returned timestamp with (*newer_return = *older + d_seconds)
 */
void add_seconds_to_timestamp(struct timeval *older, double d_seconds, struct timeval *newer_return);

#endif /*_MY_TIMESTAMP_H_*/
