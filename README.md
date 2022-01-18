# Peer-to-peer Network of Servers
A simple application-layer network protocol, which is similar to the HTTP protocol in terms of message formats.

This application impmlements layer 2 (linke layer) and layer 3 (network layer) and layer 4 (transport Layer) functionalities in an peer-to-peer overlay network. We will use graph terminology to describe the topology of an overlay network. An end-system in an overlay network will be referred to as a node. Nodes in this network are part server, part client, and part router. 

We will refer to this overlay network as 353NET. A pair of directly connected nodes are referred to as neighbors and the "direct connection" between them is a persistent TCP connection (i.e., you can send multiple messages over such a connection). We will use standard graph terminology to describe our network of nodes where a vertex is a node and an edge is a connection between a pair of neighboring nodes. Not all nodes in the 353NET may be up and running. With some nodes being down, the 353NET can be partitioned and that is not unusual.

When you start a node, you must specify a configuration file to be used for that node and each node must use a different configuration file. Inside each configuration file is a map of the entire 353NET (for this assignment, this map is guaranteed to be identical in all the configuration files). Using this network map, a node can figure out which nodes are supposed to be its neighbors. When functioning as a router, the main job of this node is to make connections to all of its neighbors, if they are up and running. If every node does that and if all the nodes in the 353NET are up and running, the network that's formed should be identical to the network map. If some nodes are not running or died, the network formed should be identical to the original network map with those nodes and links to those nodes removed.

**To Compile**

when the grader simply enters:

    make pa5 
An executable named pa4 is created and the compiler command that gets run must start with "g++ -g -Wall -std=c++11". 

**Commandline Syntax**

The commandline syntax (also known as "usage information") for the application is as follows:

    pa5 CONFIGFILE
The CONFIGFILE is a configuration file that contains all the information your node needs in order to function in the 353NET.
Unless otherwise specified, output of your program must go to cout and error messages must go to stderr/cerr.

**353NET Protocol Message Format**

The 353NET protocol message format is similar in design to the HTTP protocol message format. All 353NET protocol messages has a message header which is followed by an empty line and an optional message body. The first line of a message must be in the following format:

    353NET/1.0 MSGTYPE NONE/1.0\r\n
The first field specifies the protocol name and version to which the message conforms. For this assignment, the protocol name must be "353NET" and we are using version 1.0 of the 353NET protocol. The 2nd field is a message type (the rest of this section specifies the format for each type of messages). The 3rd field specifies the application-level protocol and version to which the message conforms (and it's always "NONE/1.0" for this assignment and you can either ignore this field or not implement it).
The message header consists of lines of text, each of which is terminated by a "\r\n" sequence. Each line in the header (except for the first line) has the following format (similar to HTTP):

    KEY:VALUE\r\n
where VALUE may have leading and trailing space characters. The end of the message header is an empty line (i.e., only contains "\r\n").
One of the lines in the message header must have the "Content-Length" key. The corresponding value specifies the exact number of bytes in the message body which immediately follows the empty line after the message header. Since we are using persistent TCP connections, within a connection, multiple messages can be sent. At the end of a message body, another message begins immediately. You must treat the message body as binary data when you are detecting message boundaries. You must reliably detect message boundaries or your node can get very confused.

**User Console**

Each node must have an interactive commandline interface. Your program must use "NODEID> " (i.e., the NodeID, followed by a "greater than" symbol, followed by a space character) as the command prompt to tell that user which node they are on and that you are expecting the user to enter a line of command. Unless the user shutdowns the node using the quit command, the node should run forever.
The commands and their meanings are:
![截屏2022-01-17 上午3 25 48](https://user-images.githubusercontent.com/35575612/149761703-b6abfb02-377a-4d54-8f04-1b634718e58a.png)

**Logging**

Logging is extremely important in a networking application like this one. It should be very useful to help you debug this assignment. It's also extremely important because part of the grading will be done by looking at the log file your node produced. Without looking at the log, it would be very difficult for the grader to tell what your node is doing. The bottomline is that if you don't log all the required messages correctly, you could end up losing a lot of points!
Every message coming into a node and every message going out of a node must be logged. The format for a line of log entry is as follows:

    [TIMESTAMP] {r|i|d|f} MSGTYPE NEIGHBOR TTL FLOOD CONTENT_LENGTH msg-dependent-data
TIMESTAMP is the current time in the same format as a PA2 timestamp. The next field is referred to as the "category" field. It's a single character and it can have 4 possible values:

You must use "r" if the message was "received" by this node.

You must use "i" if the message was sent and "initiated" by this node.

You must use "d" if the message was sent due to "flooding" by this node (and not initiated by this node).

You must use "f" if the message was sent due to "forwarding" by this node (i.e., the message is a UCASTAPP message and is being "routed" by this node to reach a particular target/destination node and this node is not the initiator).

If the "category" is "r", then the NEIGHBOR field is the NodeID of the neighbor from which you received the corresponding message.

If the "category" is "i", "d", or "f", then the NEIGHBOR field is the NodeID of the neighbor to which you sent the corresponding message.


