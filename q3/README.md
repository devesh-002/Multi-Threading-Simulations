- Client-server communication using sockets was implemented.

- Dijkstra's shortest path algorithm is used to determine the shortest paths between nodes in the network built in the file server.cpp.

- A thread is present on each edge and is used to connect to the nodes.

- The handle connection() function manages client communications. - Send system call:

1. print the information as it appears in the document

2. Next, we locate the edge connecting source (0) and the destination that was sent (0).

3. Varibales are revised as necessary

4. After publishing the data that was received, we send the message to the edge thread, which then passes it along to other edge threads using Dijkstra's shortest path from 0 to n.

5. All nodes that receive the data, excluding the destination node  print the data.

- Join all threads
- Tutorial code has been used extensively.


Follow Up Question:

1. We can alert the client with the appropriate error message in the event of a server failure, informing them of what went wrong.
   Segfaults, timed connections, addresses that have previously been utilised, etc. can all result in server failures.
   We can resend the client's request if a server is down. 
