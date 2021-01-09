#include "client.h"

int sock_control;

int cmd_read(char * buf, struct command * cstruct, size_t size)
{
    size_t size_buf = sizeof buf;
    memset(cstruct->arg, 0, sizeof(cstruct->arg));

    printf("ftpclient> ");
    fflush(stdout);

    // wait for user to enter a command
    read_input(buf, size);

    char *arg;
    buf = strtok_r(buf, " ", &arg);

    strlow(buf);

    if (strcmp(buf, "list") == 0)
    {
        cstruct->code = cmd_list;
    }
    else if (strcmp(buf, "tree") == 0)
    {
        cstruct->code = cmd_tree;
    }
    else if (strcmp(buf, "get") == 0)
    {
        cstruct->code = cmd_get;
    }
    else if (strcmp(buf, "push") == 0)
    {
        cstruct->code = cmd_post;
    }
    else if (strcmp(buf, "mkdir") == 0)
    {
        cstruct->code = cmd_mdir;
    }
    else if (strcmp(buf, "cd") == 0)
    {
        cstruct->code = cmd_cdir;
    }
    else if (strcmp(buf, "rm") == 0)
    {
        cstruct->code = cmd_remove;
    }
    else if (strcmp(buf, "rename") == 0)
    {
        cstruct->code = cmd_rename;
    }
    else if (strcmp(buf, "quit") == 0)
    {
        cstruct->code = cmd_quit;
    }
    else
    {
        return -1;
    }

    if (arg != NULL)
    {
        // store the argument if there is one
        strncpy(cstruct->arg, arg, strlen(arg));
    }

    // store code in beginning of buffer
    memset(buf, 0, size_buf );
    buf[0] = cstruct->code;

    // if there's an arg, append it to the buffer
    if (arg != NULL)
    {
        strncat(buf, cstruct->arg, strlen(cstruct->arg));
    }

    return 0;
}

int open_connection(int sock_control)
{
    int sock_listen = socket_create(CLIENT_PORT);

    // send an ACK on control conn
    int ack = 1;
    if ((send(sock_control, (char *)&ack, sizeof(ack), 0)) < 0)
    {
        printf("client: ack write error :%d\n", errno);
        exit(1);
    }

    int sd = socket_accept(sock_listen);
    close(sock_listen);
    return sd;
}

int list(int sock_data, int sock_control)
{
    size_t bread;
    char buf[MAXSIZE]; // hold a filename received from server
    int tmp = 0;

    // Wait for server starting message
    if (recv(sock_control, &tmp, sizeof tmp, 0) < 0)
    {
        perror("client: error reading message from server\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    while ((bread = recv(sock_data, buf, MAXSIZE, 0)) > 0)
    {
        printf("%s", buf);
        memset(buf, 0, sizeof(buf));
    }

    if (bread < 0)
    {
        perror("error");
    }

    // Wait for server done message
    if (recv(sock_control, &tmp, sizeof tmp, 0) < 0)
    {
        perror("client: error reading message from server\n");
        return -1;
    }
    return 0;
}

int send_cmd(struct command *cmd)
{
    char buffer[MAXSIZE];
    int rc;

    sprintf(buffer, "%c%s", cmd->code, cmd->arg);

    // Send command string to server
    rc = send(sock_control, buffer, (int)strlen(buffer), 0);
    if (rc < 0)
    {
        perror("Error sending command to server");
        return -1;
    }

    return 0;
}

void login()
{
    struct command cmd;
    char user[256];
    memset(user, 0, 256);

    // Get username from user
    printf("Name: ");
    fflush(stdout);
    read_input(user, 256);

    // Send USER command to server
    cmd.code = cmd_user;
    strcpy(cmd.arg, user);
    send_cmd(&cmd);

    // Wait for go-ahead to send password
    int wait;
    recv(sock_control, &wait, sizeof wait, 0);

    // Get password from user
    fflush(stdout);
    char * pass = getpass("Password: ");
    char * key = statkey(user);
    cripto(pass, key, 1);
    free(key);

    // Send PASS command to server
    cmd.code = cmd_pass;
    strcpy(cmd.arg, pass);
    send_cmd(&cmd);

    // wait for response
    int retcode = recv_code(sock_control);
    switch (retcode)
    {
    case 430:
        printf("Invalid username/password.\n");
        exit(0);
    case 230:
        printf("Successful login.\n");
        break;
    default:
        perror("error reading message from server");
        exit(1);
        break;
    }
}

int main(int argc, char *argv[])
{
    int sock_data, retcode, s;
    char buffer[MAXSIZE];
    struct command cmd;
    struct addrinfo hints, *res, *rp;

    if (argc != 3)
    {
        printf("usage: %s hostname port\n", argv[0]);
        exit(0);
    }

    char * host = argv[1];
    char * port = argv[2];

    // Get matching addresses
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    s = getaddrinfo(host, port, &hints, &res);
    if (s != 0)
    {
        printf("getaddrinfo() error %s", gai_strerror(s));
        exit(1);
    }

    // Find an address to connect to & connect
    for (rp = res; rp != NULL; rp = rp->ai_next)
    {
        sock_control = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (sock_control < 0)
            continue;

        if (connect(sock_control, res->ai_addr, res->ai_addrlen) == 0)
        {
            break;
        }
        else
        {
            perror("connecting stream socket");
            exit(1);
        }
        close(sock_control);
    }
    freeaddrinfo(rp);

    // Get connection, welcome messages
    print_reply(recv_code(sock_control));

    /* Get name and password and send to server */
    login();

    // loop until user types quit
    while (1)
    {
        // Get a command from user
        if (cmd_read(buffer, &cmd, sizeof buffer) < 0)
        {
            printf("Invalid command\n");
            continue; // loop back for another command
        }

        // Send command to server
        if (send(sock_control, buffer, (int)strlen(buffer), 0) < 0)
        {
            close(sock_control);
            exit(1);
        }

        retcode = recv_code(sock_control);

        if( retcode == 403)
        {
            print_reply(403);
            continue;
        }

        if (retcode != 200)
        {
            print_reply(retcode);
            break;
        }

        // open data connection
        if ((sock_data = open_connection(sock_control)) < 0)
        {
            perror("Error opening socket for data connection");
            exit(1);
        }

        int rcrequest = 1;
        // execute command
        switch (cmd.code)
        {
        case cmd_list:
        case cmd_tree:
            list(sock_data, sock_control);
            rcrequest = 0;
            break;
        case cmd_get:
            // wait for reply (is file valid)
            if ( recv_code(sock_control) == 550)
            {
                print_reply(550);
                close(sock_data);
                continue;
            }
            file_recive(sock_data, sock_control, cmd.arg, 1);
            break;
        case cmd_post:
            file_send(sock_data, sock_control, cmd.arg, 1);
            break;
        case cmd_mdir:
        case cmd_cdir:
        case cmd_remove:
        case cmd_rename:
            break;
        case cmd_quit:
            /* code */
            break;
        default:
            break;
        }

        close(sock_data);
        if(rcrequest)
        {
            print_reply(recv_code(sock_control));
        }
        // loop back to get more user input
    }

    // Close the socket (control connection)
    close(sock_control);
    return 0;
}