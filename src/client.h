/* client.c
 *
 * Client side for simple file transfer app implementation with TCP
 * 
 * Usage:
 *    ./client_ftp <SERVER_HOSTNAME> <PORT>
 *
 * Data: 29.12.2020
 * 
 * Autor: Adrian Valentin Panaintescu <adrian.cert@gmail.com> (c)2020
 *
 * */

#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

/**
 * Parse command in cstruct
 */ 
int cmd_read(char * buf, struct command * cstruct, size_t size);

/**
 * Open data connection
 */
int open_connection(int sock_con);

/** 
 * Listing commmand
 * List line by line server output
 */
int list(int sock_data, int sock_con);

/**
 * Input: cmd struct with an a code and an arg
 * Concats code + arg into a string and sends to server
 */
int send_cmd(struct command *cmd);

/**
 * Get login details from user and
 * send to server for authentication
 */
void login();

#endif
