#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define DEBUG 1
#define MAXSIZE 512
#define CLIENT_PORT 30020

#define dprintf( out, in, ...); if (DEBUG) { fprintf(out, in, __VA_ARGS__); fflush(out); }
#define dprint(in, ... ); dprintf(stdout , in , __VA_ARGS__ );

struct command
{
    char code;
    char arg[255];
};

enum command_codes {
    cmd_begin = 0x20,
    cmd_list, // list
    cmd_tree, // tree
    cmd_get, // get <filename>
    cmd_post, // push <filename>
    cmd_mdir, // mkdir <foldername>
    cmd_cdir, // cd <foldername>
    cmd_remove, // rm <path>
    cmd_rename, // rename <path_a> <path_b>
    cmd_user,
    cmd_pass,
    cmd_quit, // quit
    cmd_end
};

/**
 * Create listening socket on remote host
 * Returns -1 on error, socket fd on success
 */
int socket_create(int port);

/**
 * Create new socket for incoming client connection request
 * Returns -1 on error, or fd of newly created socket
 */
int socket_accept(int sd);

/**
 * Connect to remote host at given port
 * Returns socket fd on success, -1 on error
 */
int socket_connect(int port, char *host);

/**
 * Receive data on sockfd
 * Returns -1 on error, number of bytes received 
 * on success
 */
int recv_data(int sockfd, char* buf, int bufsize);

/**
 * Receive a response from sockfd
 * Returns -1 on error, return code on success
 */
int recv_code(int sockfd);

/**
 * Send resposne code on sockfd
 * Returns -1 on error, 0 on success
 */
int send_response(int sockfd, int rc);

/**
 * Print the message for reply code
 */
void print_reply(int rc);

/**
 * Recive a file on sock data
 * Save in path, comunicate with sock_control
 * return 0 for succes
 */
int file_recive(int sock_data, int sock_control, char * path, int show_progress);

/**
 * Send a file on sock data
 * Save in path, comunicate with sock_control
 * return 0 for succes
 */
int file_send(int sock_data, int sock_control, char * path, int show_progress);

/************************************************************/
/******************** UTILITY FUNCTIONS *********************/
/************************************************************/

/**
 * Trim whiteshpace and line ending
 * characters from a string
 */
void strtrim(char * str);

/**
 * Lower each string characters.
 */
void strlow(char *str);

/** 
 * Read input from command line
 */
void read_input(char * buffer, int size);

/**
 * Copy key value on destination under
 * the key in the file from path
 */
int readconfig(char * dest, char * path, char * key);

/**
 * Print in a file a tree view of base path
 */
void tree(char *basePath, const int root, FILE * file);

/**
 * Encript mess with key
 */
void cripto(char * mess, char * key, int encoding);

/**
 * Return a key
 */
char * statkey( char * srt);

/**
 * Return if it is a file
 */
int is_file(const char *path);

/**
 * Remove directory recursively
 */
int remove_directory(const char *path);

/**
 * Show progress bar
 */
int progressbar(long int p,long int t);

/**
 * Return file size
 */
long int filesize(char *path);

#endif
