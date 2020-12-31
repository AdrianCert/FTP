#include "common.h"

int socket_create(int port)
{
    int sockfd;
    int yes = 1;
    struct sockaddr_in sock_addr;

    // create new socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() error");
        return -1;
    }

    // set local address info
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        close(sockfd);
        perror("setsockopt() error");
        return -1;
    }

    // bind
    if (bind(sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0)
    {
        close(sockfd);
        perror("bind() error");
        return -1;
    }

    // begin listening for incoming TCP requests
    if (listen(sockfd, 5) < 0)
    {
        close(sockfd);
        perror("listen() error");
        return -1;
    }
    return sockfd;
}

int socket_accept(int sd)
{
    int sockfd;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    // Wait for incoming request, store client info in client_addr
    sockfd = accept(sd, (struct sockaddr *)&client_addr, &len);

    return sockfd;
}

int socket_connect(int port, char *host)
{
    int sockfd;
    struct sockaddr_in dest_addr;

    // create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("error creating socket");
        return -1;
    }

    // create server address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(host);

    // Connect on socket
    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        perror("error connecting to server");
        return -1;
    }
    return sockfd;
}

int recv_data(int sockfd, char *buf, int bufsize)
{
    size_t num_bytes;
    memset(buf, 0, bufsize);
    num_bytes = recv(sockfd, buf, bufsize, 0);
    if (num_bytes < 0)
    {
        return -1;
    }
    return num_bytes;
}

void strtrim(char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (isspace(str[i]))
            str[i] = 0;
        if (str[i] == '\n')
            str[i] = 0;
    }
}

int send_response(int sockfd, int rc)
{
    int conv = htonl(rc);
    if (send(sockfd, &conv, sizeof conv, 0) < 0)
    {
        perror("error sending...\n");
        return -1;
    }
    return 0;
}

void read_input(char *buffer, int size)
{
    char *nl = NULL;
    memset(buffer, 0, size);

    if (fgets(buffer, size, stdin) != NULL)
    {
        nl = strchr(buffer, '\n');
        if (nl)
            *nl = '\0';
    }
}

int readconfig(char *dest, char *path, char *key)
{
    FILE *file;
    size_t len = 0;
    char * fn = NULL;
    char delimiter[] = " ";

    if ((file = fopen(path, "r")) == NULL)
    {
        perror("Error open file");
        exit(err_file);
    }

    if ((fn = (char *)malloc(520 * sizeof(char))) == NULL)
    {
        perror("Unable to allocate fn");
        exit(err_alloc);
    }

    while (getline(&fn, &len, file) != -1)
    {
        char * c_key;
        char * c_value;

        c_key = strtok_r(fn, delimiter, &c_value);

        if (strcmp(key, c_key) == 0)
        {
            strtrim(c_value);
            strcpy(dest, c_value);

            fclose(file);
            free(fn);

            return strlen(dest);
        }
    }

    free(fn);
    fclose(file);

    return 0;
}

void tree(char *basePath, const int root, FILE * file)
{
    int i;
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            for (i=0; i<root; i++) 
            {
                if (i%2 == 0 || i == 0)
                {
                    fprintf(file, "%c", ':');
                    // fprintf(file, "%c", 179);

                }
                else
                {
                    fprintf(file, " ");
                }

            }

            //fprintf(file ,"%c%c%s\n", 195, 196, dp->d_name);
            fprintf(file ,"%c%c%s\n", '>', '-', dp->d_name);

            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            tree(path, root + 2, file);
        }
    }

    closedir(dir);
}