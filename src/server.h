#ifndef SERVER_H
#define SERVER_H

#include "common.h"

/**
 * Send list of files in current directory
 * over data connection
 * Return -1 on error, 0 on success
 */
int serv_list(int sock_data, int sock_control);

/**
 * Send tree view current directory
 * over data connection
 * Return -1 on error, 0 on success
 */
int serv_tree(int sock_data, int sock_control);

/**
 * Create a directory
 * Return -1 on error, 0 on success
 */
int serv_mdir(int sock_control, char *path);

/**
 * Change current directory
 * Return -1 on error, 0 on success
 */
int serv_cdir(int sock_control, char *path);

/**
 * Remove a file or directory
 * Return -1 on error, 0 on success
 */
int serv_rm(int sock_control, char *path);

/**
 * Rename a file or directory
 * Return -1 on error, 0 on success
 */
int serv_rename(int sock_control, char *arg);

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
int check_user(char *user, char *pass);

/** 
 * Log in connected client
 */
int login(int sock_control);

/**
 * Wait for command from client and send response
 * Returns response code
 */
int recv_cmd(int sock_control, char *cmd, char *arg);

/** 
 * Child process handles connection to client
 */
void serve_process(int sock_control);

#endif