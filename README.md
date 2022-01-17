# Network of Servers in C++
A simple application-layer network protocol, which is similar to the HTTP protocol in terms of message formats.

In the real world, layer 4 and below are implemented inside the operating system. Since we are not doing kernel hacking in this class, we will implement these functionalities in an overlay network, i.e., an application-level network, layered on top of the Internet (also known as "over-the-top network" mentioned in Ch 2). In this assignment, we will impmlement layer 2 (linke layer) and layer 3 (network layer) functionalities in such an overlay network. We will use graph terminology to describe the topology of an overlay network. An end-system in an overlay network will be referred to as a node. In Ch 2, we talked about the peer-to-peer architecture, which can also be considered as an overlay network.

Conceptually, we can start with your PA3 web server and turn it into an autonomous and independently operated node in an overlay network. Nodes in this network are part server, part client, and part router. We will refer to this overlay network as 353NET. A pair of directly connected nodes are referred to as neighbors and the "direct connection" between them is a persistent TCP connection (i.e., you can send multiple messages over such a connection). We will use standard graph terminology to describe our network of nodes where a vertex is a node and an edge is a connection between a pair of neighboring nodes. Not all nodes in the 353NET may be up and running. With some nodes being down, the 353NET can be partitioned and that is not unusual.

Neighboring nodes in a 353NET exchange messages by following the 353NET protocol described in the rest of this spec. The main goal we would like to achieve in this assignment is that every node in a 353NET knows the topology of the network partition it's in and maintains that information in an adjacency list data structure (similar to the one used in your PA1 assignment) and use this information to perform message routing for networking applications that would run on top of the 353NET in PA5. This needs to be achieved without an active central authority (although we will provide every node in the 353NET with an identical "network map").

When you start a node, you must specify a configuration file to be used for that node and each node must use a different configuration file. Inside each configuration file is a map of the entire 353NET (for this assignment, this map is guaranteed to be identical in all the configuration files). Using this network map, a node can figure out which nodes are supposed to be its neighbors. When functioning as a router, the main job of this node is to make connections to all of its neighbors, if they are up and running. If every node does that and if all the nodes in the 353NET are up and running, the network that's formed should be identical to the network map. If some nodes are not running or died, the network formed should be identical to the original network map with those nodes and links to those nodes removed.

**To Compile**

Enter 'make pa5' in the terminal

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

You should be able to easily adapt your code from PA3 to read and parse a 353NET message since the basic structure of 353NET messages is the same as HTTP messages.

There can be various reasons why a message is considered malformed. For example, if the first line in the header has an unknown MSGTYPE or the rest of the line does not contain the expected information, then the message is considered malformed. If some lines (except for the first line) in the header is missing a colon, the message is considered malformed. If the header does not contain a "Content-Length" key, the message is considered malformed. And so on. If a node receives a malformed message, it must immediately shutdown and close the socket and delete the connection. But since your node only has to work with other nodes running exactly the same code and written by yourself, your node should never see a malformed message!

Below are the details of each of the 353NET protocol messages.

**To run Cucumber tests:**

Make sure the web server is running when you run the Cucumber tests. Right-click "cucumber.launch" -> Run As -> "cucumber".
1. Make sure the web server is running when you run the Cucumber tests. 
2. Make sure the React Web App is running when you run the Cucumber tests. Go to "/src/main/webapp/reactjs" and follow the README.md inside that repository. Ideally run `yarn install` and `yarn start` will start the website.
3. Right-click "cucumber.launch" -> Run As -> "cucumber".

