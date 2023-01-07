#include <bits/stdc++.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>

/////////////////////////////
#include <iostream>
#include <assert.h>
#include <tuple>
using namespace std;
/////////////////////////////

// Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define ANSI_RESET "\x1b[0m"
int curr_node = 0;
int prev_node = 0;

typedef long long LL;

#define pb push_back
#define debug(x) cout << #x << " : " << x << endl
#define part cout << "-----------------------------------" << endl;

///////////////////////////////
#define MAX_CLIENTS 4
#define PORT_ARG 694212
int port_count = 1234579;

const int initial_msg_len = 256;

////////////////////////////////////

const LL buff_sz = 1048575;
///////////////////////////////////////////////////

struct edge
{
    int s;
    int r;
    int port;
    int socket_s;
    int socket_r;
};

int nodes, connections;
vector<vector<pair<int, int>>> adj;
vector<vector<int>> rt;
vector<int> d;
vector<int> p;
vector<edge> edges;
vector<pthread_t> threads_comm;

const int INF = 1000000000;

pair<string, int>
read_string_from_socket(const int &fd, int bytes)
{
    std::string output;
    output.resize(bytes);

    int bytes_received = read(fd, &output[0], bytes - 1);
    debug(bytes_received);
    if (bytes_received <= 0)
    {
        cerr << "Failed to read data from socket. \n";
    }

    output[bytes_received] = 0;
    output.resize(bytes_received);
    // debug(output);
    return {output, bytes_received};
}

int send_string_on_socket(int fd, const string &s)
{
    // debug(s.length());
    int bytes_sent = write(fd, s.c_str(), s.length());
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA via socket.\n";
    }

    return bytes_sent;
}


// ///////////////////////////////

void ack(int dest, string s)
{
    cout << "Data received at node : ";
    cout << curr_node << " ";
    cout << "Source : ";
    cout << prev_node << " ";
    cout << "Destination : ";
    cout << dest << " ";
    cout << "Forwaded_Destination : ";
    cout << rt[curr_node][dest] << " ";
    cout << "Message : ";
    cout << s << endl;
}

void handle_connection(int client_socket_fd)
{
    // int client_socket_fd = *((int *)client_socket_fd_ptr);
    //####################################################

    int received_num, sent_num;

    /* read message from client */
    int ret_val = 1;

    while (true)
    {
        string cmd;
        tie(cmd, received_num) = read_string_from_socket(client_socket_fd, buff_sz);
        ret_val = received_num;
        // debug(ret_val);
        // printf("Read something\n");
        if (ret_val <= 0)
        {
            // perror("Error read()");
            printf("Server could not read msg sent from client\n");
            goto close_client_socket_ceremony;
        }

        cout << "Client sent : " << cmd << endl;
        string msg_to_send_back = "Ack: " + cmd;

        if (cmd == "exit")
        {
            cout << "Exit pressed by client" << endl;
            goto close_client_socket_ceremony;
        }
        else if (cmd == "pt")
        {
            send_string_on_socket(client_socket_fd, "\ndest forw delay\n");

            for (int i = 1; i < nodes; i++)
            {
                send_string_on_socket(client_socket_fd, to_string(i) + " " + to_string(rt[0][i]) + " " + to_string(d[i]) + "\n");
            }
        }
        else
        {
            int dest;
            string s;

            for (int i = 0; i < cmd.size(); i++)
            {
                if (cmd[i] == ' ')
                {
                    dest = cmd[i + 1] - '0';
                    for (int j = i + 3; j < cmd.size(); j++)
                    {
                        s.push_back(cmd[j]);
                    }
                    break;
                }
            }

            ack(dest, s);
            for (int i = 0; i < 2 * connections; i++)
            {
                if (edges[i].r == rt[curr_node][dest] && edges[i].s == curr_node)
                {
                    send_string_on_socket(edges[i].socket_s, cmd);
                    prev_node = curr_node;
                    curr_node = rt[curr_node][dest];
                    break;
                }
            }

            while (curr_node != dest)
            {
                continue;
            }

            ack(dest, s);

            curr_node = 0;
            prev_node = 0;
        }

        ////////////////////////////////////////
        // "If the server write a message on the socket and then close it before the client's read. Will the client be able to read the message?"
        // Yes. The client will get the data that was sent before the FIN packet that closes the socket.

        int sent_to_client = send_string_on_socket(client_socket_fd, msg_to_send_back);
        // debug(sent_to_client);
        if (sent_to_client == -1)
        {
            perror("Error while writing to client. Seems socket has been closed");
            goto close_client_socket_ceremony;
        }
    }

close_client_socket_ceremony:
    close(client_socket_fd);
    printf(BRED "Disconnected from client" ANSI_RESET "\n");
    // return NULL;
}

int get_socket_fd(struct sockaddr_in *ptr, int port)
{
    struct sockaddr_in server_obj = *ptr;

    // socket() creates an endpoint for communication and returns a file
    //        descriptor that refers to that endpoint.  The file descriptor
    //        returned by a successful call will be the lowest-numbered file
    //        descriptor not currently open for the process.
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Error in socket creation for CLIENT");
        exit(-1);
    }
    /////////////////////////////////////////////////////////////////////////////////////
    int port_num = port;

    memset(&server_obj, 0, sizeof(server_obj)); // Zero out structure
    server_obj.sin_family = AF_INET;
    server_obj.sin_port = htons(port_num); // convert to big-endian order

    // Converts an IP address in numbers-and-dots notation into either a
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    // https://stackoverflow.com/a/20778887/6427607

    /////////////////////////////////////////////////////////////////////////////////////////
    /* connect to server */

    if (connect(socket_fd, (struct sockaddr *)&server_obj, sizeof(server_obj)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(-1);
    }

    // part;
    //  printf(BGRN "Connected to server\n" ANSI_RESET);
    //  part;
    return socket_fd;
}

void *thread_run(void *arg)
{
    int index = *(int *)arg;

    int wel_socket_fd, client_socket_fd, port_number;
    socklen_t clilen;

    struct sockaddr_in serv_addr_obj, client_addr_obj;
    /////////////////////////////////////////////////////////////////////////
    /* create socket */
    /*
    The server program must have a special door—more precisely,
    a special socket—that welcomes some initial contact
    from a client process running on an arbitrary host
    */
    // get welcoming socket
    // get ip,port
    /////////////////////////
    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        perror("ERROR creating welcoming socket");
        exit(-1);
    }

    //////////////////////////////////////////////////////////////////////
    /* IP address can be anything (INADDR_ANY) */
    bzero((char *)&serv_addr_obj, sizeof(serv_addr_obj));
    port_number = PORT_ARG;
    serv_addr_obj.sin_family = AF_INET;
    // On the server side I understand that INADDR_ANY will bind the port to all available interfaces,
    serv_addr_obj.sin_addr.s_addr = INADDR_ANY;
    serv_addr_obj.sin_port = htons(edges[index].port); // process specifies port

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* bind socket to this port number on this machine */
    /*When a socket is created with socket(2), it exists in a name space
       (address family) but has no address assigned to it.  bind() assigns
       the address specified by addr to the socket referred to by the file
       descriptor wel_sock_fd.  addrlen specifies the size, in bytes, of the
       address structure pointed to by addr.  */

    // CHECK WHY THE CASTING IS REQUIRED
    if (bind(wel_socket_fd, (struct sockaddr *)&serv_addr_obj, sizeof(serv_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket: ");
        exit(-1);
    }
    //////////////////////////////////////////////////////////////////////////////////////

    /* listen for incoming connection requests */

    listen(wel_socket_fd, MAX_CLIENTS);
    clilen = sizeof(client_addr_obj);

    edges[index].socket_r = wel_socket_fd;

    struct sockaddr_in server_obj;
    int socket_fd = get_socket_fd(&server_obj, edges[index].port);

    edges[index].socket_s = socket_fd;

    while (1)
    {
        int client_socket_fd = accept(edges[index].socket_r, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }

        int received_num, sent_num;

        /* read message from client */
        int ret_val = 1;

        while (true)
        {
            string cmd;
            tie(cmd, received_num) = read_string_from_socket(client_socket_fd, buff_sz);
            ret_val = received_num;

            int dest;
            string s;

            for (int i = 0; i < cmd.size(); i++)
            {
                if (cmd[i] == ' ')
                {
                    dest = cmd[i + 1] - '0';
                    for (int j = i + 3; j < cmd.size(); j++)
                    {
                        s.push_back(cmd[j]);
                    }
                    break;
                }
            }

            ack(dest, s);

            for (int i = 0; i < 2 * connections; i++)
            {
                if (edges[i].r == rt[curr_node][dest] && edges[i].s == curr_node)
                {
                    send_string_on_socket(edges[i].socket_s, cmd);
                    prev_node = curr_node;
                    curr_node = rt[curr_node][dest];
                    break;
                }
            }
        }
    }

    return NULL;
}

void dijkstra(int s)
{
    int i = 0;

    int n = adj.size();

    vector<bool> u(n, false);
    d.assign(n, INF);

    d[s] = 0;
    p.assign(n, -1);

    while (i < n)
    {
        int v = -1;
        int j = 0;
        while (j < n)
        {
            if (!u[j])
            {
                if ((v == -1 || d[j] < d[v]))
                {
                    v = j;
                }
            }
            j++;
        }

        if (d[v] != INF)
            ;
        else
            break;

        u[v] = false;
        u[v] = true;
        for (auto edge : adj[v])
        {
            int x = d[v] + edge.second;
            int y = d[edge.first];
            if (x < y)
            {
                d[edge.first] = x;
                p[edge.first] = v;
            }
        }
        i++;
    }
    i = 0;
    while (i < nodes)
    {
        if (i != s)
        {
            vector<int> path;
            int j = i;
            while (j != s)
            {
                path.push_back(j);

                j = p[j];
            }

            rt[s][i] = path[path.size() - 1];
        }
        i++;
    }
}

int main()
{
    cin >> nodes >> connections;

    for (int i = 0; i < nodes; i++)
    {
        vector<pair<int, int>> temp1;
        vector<int> temp2;
        for (int j = 0; j < nodes; j++)
        {
            temp2.push_back(j);
        }
        rt.push_back(temp2);
        adj.push_back(temp1);
    }

    int count = 0;

    for (int i = 0; i < connections; i++)
    {
        int a, b, d;
        cin >> a >> b >> d;
        adj[a].push_back({b, d});
        adj[b].push_back({a, d});
        struct edge temp1;
        temp1.s = a;
        temp1.r = b;
        temp1.port = ++port_count;
        temp1.socket_s = -1;
        temp1.socket_r = -1;
        struct edge temp2;
        temp2.s = b;
        temp2.r = a;
        temp2.port = ++port_count;
        temp2.socket_s = -1;
        temp2.socket_r = -1;
        edges.push_back(temp1);
        edges.push_back(temp2);
        pthread_t t1;
        pthread_t t2;
        threads_comm.push_back(t1);
        threads_comm.push_back(t2);
    }

    for (int i = nodes - 1; i >= 0; i--)
    {
        dijkstra(i);
    }

    int i, j, k, t;

    int wel_socket_fd, client_socket_fd, port_number;
    socklen_t clilen;

    struct sockaddr_in serv_addr_obj, client_addr_obj;
    /////////////////////////////////////////////////////////////////////////
    /* create socket */
    /*
    The server program must have a special door—more precisely,
    a special socket—that welcomes some initial contact
    from a client process running on an arbitrary host
    */
    // get welcoming socket
    // get ip,port
    /////////////////////////
    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        perror("ERROR creating welcoming socket");
        exit(-1);
    }

    //////////////////////////////////////////////////////////////////////
    /* IP address can be anything (INADDR_ANY) */
    bzero((char *)&serv_addr_obj, sizeof(serv_addr_obj));
    port_number = PORT_ARG;
    serv_addr_obj.sin_family = AF_INET;
    // On the server side I understand that INADDR_ANY will bind the port to all available interfaces,
    serv_addr_obj.sin_addr.s_addr = INADDR_ANY;
    serv_addr_obj.sin_port = htons(port_number); // process specifies port

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* bind socket to this port number on this machine */
    /*When a socket is created with socket(2), it exists in a name space
       (address family) but has no address assigned to it.  bind() assigns
       the address specified by addr to the socket referred to by the file
       descriptor wel_sock_fd.  addrlen specifies the size, in bytes, of the
       address structure pointed to by addr.  */

    // CHECK WHY THE CASTING IS REQUIRED
    if (bind(wel_socket_fd, (struct sockaddr *)&serv_addr_obj, sizeof(serv_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket: ");
        exit(-1);
    }
    //////////////////////////////////////////////////////////////////////////////////////

    /* listen for incoming connection requests */

    listen(wel_socket_fd, MAX_CLIENTS);
    cout << "Server has started listening on the LISTEN PORT" << endl;
    clilen = sizeof(client_addr_obj);

    for (int i = 0; i < threads_comm.size(); i++)
    {
        int *a = (int *)malloc(sizeof(int));
        *a = i;
        pthread_create(&threads_comm[i], NULL, thread_run, a);
    }

    while (1)
    {
        /* accept a new request, create a client_socket_fd */
        /*
        During the three-way handshake, the client process knocks on the welcoming door
of the server process. When the server “hears” the knocking, it creates a new door—
more precisely, a new socket that is dedicated to that particular client.
        */
        // accept is a blocking call
        printf("Waiting for a new client to request for a connection\n");
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }

        printf(BGRN "New client connected from port number %d and IP %s \n" ANSI_RESET, ntohs(client_addr_obj.sin_port), inet_ntoa(client_addr_obj.sin_addr));

        handle_connection(client_socket_fd);
    }

    for (int i = 0; i < threads_comm.size(); i++)
    {
        pthread_join(threads_comm[i], NULL);
    }

    close(wel_socket_fd);
    return 0;
}