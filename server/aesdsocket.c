#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <stdbool.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#define THEPORT "9000"
#define BUF_SIZE 1024
#define VARTMPFILE "/var/tmp/aesdsocketdata"
#define BACKLOG 10

int server_socket;
int graceful_stop = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t timer_thread;


int sock_to_peer(int sockfd, char *buf, size_t buf_size)
{
    // Get the peer address information
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    if (getpeername(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1)
    {
        perror("getpeername failed");
        return -1;
    }

    // Convert the client address to a string representation
    char clientIP[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN) == NULL)
    {
        perror("inet_ntop failed");
        return -1;
    }
    strncpy(buf, clientIP, buf_size);
    return 0;
}

// returns socket fd or -1 on error 
int get_listening_socket(char*port) {
    struct addrinfo hints, *res, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status;
    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        // syslog(LOG_ERR, gai_strerror(status));
        return -1;
    }
    int sockfd;
    int yes = 1; // For setsockopt() SO_REUSEADDR, below
    for (p = res; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0)
        {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        break;
    }
    if (sockfd == -1)
    {
        perror("socket()");
        return -1;
    }
    // If we got here, it means we didn't get bound
    if (p == NULL)
    {
        return -1;
    }
    syslog(LOG_DEBUG, "socket() ok");

    status = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if (status == -1)
    {
        perror("bind()");
        return -1;
    }
    syslog(LOG_DEBUG, "bind() ok");

    status = listen(sockfd, BACKLOG);
    if (status == -1)
    {
        perror("listen()");
        return -1;
    }
    syslog(LOG_DEBUG, "listening on %s...", port);
    freeaddrinfo(res);
    return sockfd;
}

int send_file_content(char *filename, int sockfd)
{
    char buf[BUF_SIZE];
    size_t bytesRead;
    ssize_t bytesSent;
    FILE *readfile = fopen(filename, "rb");
    while ((bytesRead = fread(buf, 1, sizeof(buf), readfile)) > 0)
    {
	pthread_mutex_lock(&mutex);
        bytesSent = send(sockfd, buf, bytesRead, 0);
        if (bytesSent == -1)
        {
            perror("send failed");
            fclose(readfile);
	    pthread_mutex_unlock(&mutex);
            return -1;
        }
    }
    fclose(readfile);
    pthread_mutex_unlock(&mutex);
    return 0;
}

// return client fd
int accept_connection(int sockfd)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    int status;
    char buf[BUF_SIZE];

    int clientfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    if (graceful_stop) {
        return -1;
    }
    if (clientfd == -1)
    {
        perror("accept()");
        return -1;
    }
    status = sock_to_peer(clientfd, buf, INET_ADDRSTRLEN);
    if (status == -1)
    {
        fprintf(stderr, "sock_to_peer(): failed to get peer from socket\n");
        return -1;
    }

    syslog(LOG_DEBUG, "Accepted connection from %s", buf);
    return clientfd;
}

int handle_client_connection(int clientfd) {
    char buf[BUF_SIZE];
    char peername[INET_ADDRSTRLEN + 10];
    int status;
    status = sock_to_peer(clientfd, peername, INET_ADDRSTRLEN);
    if (status == -1)
    {
        fprintf(stderr, "sock_to_peer(): failed to get peer from socket\n");
        return -1;
    }

    struct pollfd pfd;
    pfd.fd = clientfd;
    pfd.events = POLLIN;
    int fd_count = 1;
    if (!graceful_stop) {
        for(;!graceful_stop;) {
            int poll_count = poll(&pfd, fd_count, -1);
            if (graceful_stop) {
                break;
            }
            if (poll_count == -1)
            {
                perror("poll");
                return -1;
            }

            if (pfd.revents & POLLIN) {
                FILE *file = fopen(VARTMPFILE, "a");
                ssize_t bytesRead = recv(pfd.fd, buf, sizeof(buf) - 1, 0);
                syslog(LOG_DEBUG, "Bytes read %ld", bytesRead);

                if (bytesRead == -1 )
                {
                    perror("recv failed");
                    fclose(file);
                    return -1;
                }
                // client ended the connection
                if (bytesRead == 0) {
		    fclose(file);
                    break;
                }


		fwrite(buf, 1, bytesRead, file);
                fclose(file);

                if (buf[bytesRead-1] == '\n') {
                    status = send_file_content(VARTMPFILE, clientfd);
                    if (status == -1)
                    {
                        fprintf(stderr, "handle_client_connection(): sending file to client %s\n", VARTMPFILE);
                        return -1;
                    }
                }
            }
        }
    }
    syslog(LOG_DEBUG, "convert sock to peer");

    if (close(clientfd) == -1)
    {
        perror("close failed");
        return -1;
    }
    syslog(LOG_DEBUG, "Close connection from %s", peername);
    return 0;
}

void daemonize()
{
    // Fork the parent process
   pid_t pid = fork();

    if (pid < 0)
    {
        // Forking failed
        printf("Error: Forking process failed.\n");
        exit(1);
    }

    if (pid > 0)
    {
        // Exit the parent process
        printf("daemon pid: %d\n", pid);
        exit(0);
    }

    // Redirect standard input, output, and error to /dev/null
    int null_fd = open("/dev/null", O_RDWR);
    if (null_fd < 0)
    {
        printf("Error: Failed to open /dev/null.\n");
        exit(1);
    }
    dup2(null_fd, STDIN_FILENO);
    dup2(null_fd, STDOUT_FILENO);
    dup2(null_fd, STDERR_FILENO);
    close(null_fd);
}

void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        graceful_stop = 1;
        close(server_socket);
	pthread_cancel(timer_thread);
    }
}

void* timer_thread_handler(void* arg) {
    while (!graceful_stop) {
        time_t current_time = time(NULL);
        char timestamp[100];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&current_time));
        pthread_mutex_lock(&mutex);
        FILE* file = fopen(VARTMPFILE, "a");
        if (file == NULL) {
            perror("Failed to open file");
            pthread_mutex_unlock(&mutex);
            break;
        }
	fprintf(file, "timestamp:%s\n", timestamp);
	fclose(file);
        pthread_mutex_unlock(&mutex);

        sleep(10);
    }

    pthread_exit(NULL);
}

void cleanup_threads()
{
    pthread_cancel(timer_thread);
    pthread_join(timer_thread, NULL);
}

void cleanup()
{
    cleanup_threads();
    pthread_mutex_destroy(&mutex);
}

int run(bool daemon) {
    int status = 0;
    openlog("aesdsocket", LOG_PID, LOG_USER);
    server_socket = get_listening_socket(THEPORT);
    if (server_socket == -1)
    {
        perror("get_listening_socket()");
        syslog(LOG_ERR, "Failed to create server socket");
        closelog();
	return -1;
    }
    if (daemon) {
        daemonize();
    	// Open the system log for the daemon
    	openlog("aesdsocket_daemon", LOG_PID | LOG_CONS, LOG_USER);
    	syslog(LOG_DEBUG, "aesdsocket_daemon started");
    }
    else{
	// Run in the foreground
        openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_DEBUG, "aesdsocket started");
    }
    
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&timer_thread, NULL, timer_thread_handler, NULL);

    while (1)
    {
        int clientfd = accept_connection(server_socket);
        if (graceful_stop) {
            break;
        }
        if (clientfd == -1)
        {
            fprintf(stderr, "accept_connection() failed\n");
            cleanup();
	    return -1;
        }

        if (!graceful_stop)
            status = handle_client_connection(clientfd);

        if (status != 0)
        {
            fprintf(stderr, "handle_client_connection, status = %d\n", status);
            cleanup();
	    return -1;
        }
    }

    cleanup();
    closelog();
    // Close the socket
    if (close(server_socket) == -1 && !graceful_stop)
    {
        perror("close failed");
        return -1;
    }
    remove(VARTMPFILE);
    return 0;
}

int main(int argc, char *argv[])
{
    bool daemon = 0;
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        daemon = 1;
    }
    printf("pids: %ld %ld\n", (long)getpid(), (long)getppid());
    // Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    return run(daemon);
}
