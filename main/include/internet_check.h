///@file internet_check.h
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief 
///@version 1.0
///@date 2025-02-14
///
///@copyright Copyright (c) 2025
///

#ifndef __INTERNET_CHECK_H__
#define __INTERNET_CHECK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

int check_internet_connection();

#endif // __INTERNET_CHECK_H__
