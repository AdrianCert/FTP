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
 * Receive a response from server
 * Returns -1 on error, return code on success
 */
int read_reply();

/**
 * Print response message
 */
void print_reply(int rc);

/**
 * Parse command in cstruct
 */ 
int cmd_read(char * buf, struct command * cstruct, size_t size);

/**
 * Do get <filename> command 
 */
int get(int data_sock, int sock_control, char* arg);

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
