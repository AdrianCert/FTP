#ifndef SERVER_H
#define SERVER_H

#include "common.h"

/**
 * Send file specified in filename over data connection, sending
 * control message over control connection
 * Handles case of null or invalid filename
 */
void retr(int sock_control, int sock_data, char* filename);

/**
 * Send list of files in current directory
 * over data connection
 * Return -1 on error, 0 on success
 */
int serv_list(int sock_data, int sock_control, char * path);

int serv_tree(int sock_data, int sock_control, char * path);
/**
 * Open data connection to client 
 * Returns: socket for data connection
 * or -1 on error
 */
int start_data_conn(int sock_control);

/**
 * Authenticate a user's credentials
 * Return 1 if authenticated, 0 if not
 */
int check_user(char*user, char*pass);

/** 
 * Log in connected client
 */
int login(int sock_control);

/**
 * Wait for command from client and send response
 * Returns response code
 */
int recv_cmd(int sock_control, char*cmd, char*arg);

/** 
 * Child process handles connection to client
 */
void serve_process(int sock_control);

#endif