#include "server.h"

int serv_list(int sock_data, int sock_control)
{
    char data[MAXSIZE];
    size_t num_read;
    FILE *fd;

    if (system("ls -l | tail -n+2 > .tmp") < 0)
    {
        exit(1);
    }

    if (!(fd = fopen(".tmp", "r")))
    {
        exit(2);
    }

    /* Seek to the beginning of the file */
    fseek(fd, SEEK_SET, 0);

    send_response(sock_control, 1); //starting

    memset(data, 0, MAXSIZE);
    while ((num_read = fread(data, 1, MAXSIZE, fd)) > 0)
    {
        if (send(sock_data, data, num_read, 0) < 0)
        {
            perror("err");
        }
        memset(data, 0, MAXSIZE);
    }

    fclose(fd);
    system("rm -f .tmp");

    send_response(sock_control, 226); // send 226

    return 0;
}

int serv_tree(int sock_data, int sock_control)
{
    char data[MAXSIZE];
    size_t num_read;
    FILE *fd;

    if (!(fd = fopen("../.tmp", "w")))
    {
        exit(1);
    }

    tree(".", 0, fd);

    fclose(fd);
    if (!(fd = fopen("../.tmp", "r")))
    {
        exit(2);
    }

    /* Seek to the beginning of the file */
    fseek(fd, SEEK_SET, 0);

    send_response(sock_control, 1); //starting

    memset(data, 0, MAXSIZE);
    while ((num_read = fread(data, 1, MAXSIZE, fd)) > 0)
    {
        fflush(stdout);

        if (send(sock_data, data, num_read, 0) < 0)
        {
            perror("err");
        }
        memset(data, 0, MAXSIZE);
    }

    fclose(fd);
    system("rm -f ../.tmp");

    send_response(sock_control, 226); // send 226

    return 0;
}

int serv_mdir(int sock_control, char *path)
{
    struct stat st = {0};
    if (stat(path, &st) == -1)
    {
        if(!mkdir(path, 0700))
        {
            send_response(sock_control, 226);
            return 0;
        }
    }
    send_response(sock_control, 551);
    return -1;
}

int serv_cdir(int sock_control, char *path)
{
    if(chdir(path) == 0)
    {
        if( access( ".auth", F_OK ) == 0 )
        {
            chdir("data");
            send_response(sock_control, 552);
            return -1;
        }
        send_response(sock_control, 226);
        return 0;
    }
    send_response(sock_control, 552);
    return -1;
}

int serv_rm(int sock_control, char *path)
{
    if(is_file(path))
    {
        dprint("This is a file %c", '!');
        if (remove(path) == 0)
        {
            send_response(sock_control, 226);
            return 0;
        }
        send_response(sock_control, 555);
    }
    else
    {
        if( remove_directory(path) == 0)
        {
            send_response(sock_control, 226);
            return 0;
        }
        send_response(sock_control, 555);
    }
    
    return -1;
}

int start_data_conn(int sock_control)
{
    char buf[1024];
    int wait, sock_data;

    // Wait for go-ahead on control conn
    if (recv(sock_control, &wait, sizeof wait, 0) < 0)
    {
        perror("Error while waiting");
        return -1;
    }

    // Get client address
    struct sockaddr_in client_addr;
    socklen_t len = sizeof client_addr;
    getpeername(sock_control, (struct sockaddr *)&client_addr, &len);
    inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

    // Initiate data connection with client
    if ((sock_data = socket_connect(CLIENT_PORT, buf)) < 0)
        return -1;

    return sock_data;
}

int check_user(char *user, char *pass)
{
    char username[MAXSIZE];
    char password[MAXSIZE];
    char *pch;
    char buf[MAXSIZE];
    char *line = NULL;
    size_t num_read;
    size_t len = 0;
    FILE *fd;
    int auth = 0;

    fd = fopen(".auth", "r");
    if (fd == NULL)
    {
        perror("file not found");
        exit(1);
    }

    while ((num_read = getline(&line, &len, fd)) != -1)
    {
        memset(buf, 0, MAXSIZE);
        strcpy(buf, line);

        pch = strtok(buf, " ");
        strcpy(username, pch);

        if (pch != NULL)
        {
            pch = strtok(NULL, " ");
            strcpy(password, pch);
        }

        // remove end of line and whitespace
        strtrim(password);

        if ((strcmp(user, username) == 0) && (strcmp(pass, password) == 0))
        {
            auth = 1;
            break;
        }
    }
    free(line);
    fclose(fd);
    return auth;
}

int login(int sock_control)
{
    char buf[MAXSIZE];
    char user[MAXSIZE];
    char pass[MAXSIZE];
    memset(user, 0, MAXSIZE);
    memset(pass, 0, MAXSIZE);
    memset(buf, 0, MAXSIZE);

    // Wait to recieve username
    if ((recv_data(sock_control, buf, sizeof(buf))) == -1)
    {
        perror("recv error\n");
        exit(1);
    }

    strcpy(user, buf + 1);

    char *key = statkey(user);

    // tell client we're ready for password
    send_response(sock_control, 331);

    // Wait to recieve password
    memset(buf, 0, MAXSIZE);
    if ((recv_data(sock_control, buf, sizeof(buf))) == -1)
    {
        perror("recv error\n");
        exit(1);
    }

    strcpy(pass, buf + 1);
    cripto(pass, key, 0);

    return (check_user(user, pass));
}

int recv_cmd(int sock_control, char *cmd, char *arg)
{
    int rc = 200;
    char buffer[MAXSIZE];

    memset(buffer, 0, MAXSIZE);
    memset(arg, 0, MAXSIZE);

    // Wait to recieve command
    if ((recv_data(sock_control, buffer, sizeof(buffer))) == -1)
    {
        perror("recv error\n");
        return -1;
    }

    *cmd = buffer[0];
    strcpy(arg, buffer + 1);

    if (*cmd == cmd_quit)
    {
        rc = 221;
    }
    else if (*cmd > cmd_begin && cmd_end > *cmd)
    {
        rc = 200;
    }
    else
    {
        rc = 502;
    }

    send_response(sock_control, rc);
    return rc;
}

void serve_process(int sock_control)
{
    int sock_data;
    char cmd[1];
    char arg[MAXSIZE];

    // Send welcome message
    send_response(sock_control, 220);

    // Authenticate user
    if (login(sock_control) == 1)
    {
        send_response(sock_control, 230);
    }
    else
    {
        send_response(sock_control, 430);
        exit(0);
    }

    chdir("data");

    while (1)
    {
        // Wait for command
        int rc = recv_cmd(sock_control, cmd, arg);
        printf("serverftp> comand %d %s\n", (int)*cmd, arg);
        fflush(stdout);

        if ((rc < 0) || (rc == 221))
        {
            break;
        }

        if (rc == 200)
        {
            // Open data connection with client
            if ((sock_data = start_data_conn(sock_control)) < 0)
            {
                close(sock_control);
                exit(1);
            }

            // Execute command
            switch (*cmd)
            {
            case cmd_list:
                serv_list(sock_data, sock_control);
                break;
            case cmd_tree:
                serv_tree(sock_data, sock_control);
                break;
            case cmd_get:
                file_send(sock_data, sock_control, arg);
                break;
            case cmd_post:
                if ((rc = recv_code(sock_control)) == 150)
                {
                    file_recive(sock_data, sock_control, arg);
                    recv_code(sock_control);
                    send_response(sock_control, 226);
                }
                else
                {
                    send_response(sock_control, 550);
                }
                break;
            case cmd_mdir:
                serv_mdir(sock_control, arg);
                break;
            case cmd_cdir:
                serv_cdir(sock_control, arg);
                break;
            case cmd_remove:
                serv_rm(sock_control, arg);
                break;
            case cmd_quit:
                /* code */
                break;

            default:
                break;
            }

            // Close data connection
            close(sock_data);
        }
    }
}

int main(int argc, char **argv)
{
    int sock_listen, sock_control, port, pid;
    char s_port[25];

    system("mkdir -p ./data");

    if (readconfig(s_port, "server.cfg", "port") == 0)
    {
        perror("Error reading config");
        exit(0);
    }

    port = atoi(s_port);

    printf("servftp> Server start\n");
    fflush(stdout);

    // create socket
    if ((sock_listen = socket_create(port)) < 0)
    {
        perror("Error creating socket");
        exit(err_net);
    }

    while (1)
    {
        // wait for client request
        // create new socket for control connection
        if ((sock_control = socket_accept(sock_listen)) < 0)
        {
            perror("accept() error");
            break;
        }

        // create child process to do actual file transfer
        if ((pid = fork()) < 0)
        {
            perror("Error forking child process");
        }
        else if (pid == 0)
        {
            close(sock_listen);
            serve_process(sock_control);
            close(sock_control);
            exit(0);
        }

        close(sock_control);
    }

    close(sock_listen);

    return 0;
}