# Peer-to-peer Network of Servers

**1. Basic Information**

This network application impmlements layer 2 (link layer) and layer 3 (network layer) and layer 4 (transport Layer) functionalities in an peer-to-peer overlay network. Nodes in this network are part server, part client, and part router. 

We will refer to this overlay network as 353NET. A pair of directly connected nodes are referred to as neighbors and the "direct connection" between them is a persistent TCP connection (i.e., you can send multiple messages over such a connection). We will use standard graph terminology to describe our network of nodes where a vertex is a node and an edge is a connection between a pair of neighboring nodes. Not all nodes in the 353NET may be up and running. With some nodes being down, the 353NET can be partitioned and that is not unusual.

When you start a node, you must specify a configuration file to be used for that node and each node must use a different configuration file. Inside each configuration file is a map of the entire 353NET. Using this network map, a node can figure out which nodes are supposed to be its neighbors. When functioning as a router, the main job of this node is to make connections to all of its neighbors, if they are up and running. If every node does that and if all the nodes in the 353NET are up and running, the network that's formed should be identical to the network map. If some nodes are not running or died, the network formed should be identical to the original network map with those nodes and links to those nodes removed.

**2. Commandline Syntax**

To compile the application, simply enters in the termianl:

    make pa5 
An executable named pa5 is created and the compiler command that gets run must start with "g++ -g -Wall -std=c++11". 

To run the application, the commandline syntax is as follows:

    pa5 CONFIGFILE
The CONFIGFILE is a configuration file that contains all the information your node needs in order to function in the 353NET.
Unless otherwise specified, output of your program must go to cout and error messages must go to stderr/cerr.

**3. 353NET Protocol Message Format**

The 353NET protocol message format is similar in design to the HTTP protocol message format. All 353NET protocol messages has a message header which is followed by an empty line and an optional message body. The first line of a message must be in the following format:

    353NET/1.0 MSGTYPE NONE/1.0\r\n
The first field specifies the protocol name and version to which the message conforms. For this assignment, the protocol name must be "353NET" and we are using version 1.0 of the 353NET protocol. The 2nd field is a message type (the rest of this section specifies the format for each type of messages). The 3rd field specifies the application-level protocol and version to which the message conforms (and it's always "NONE/1.0" for this application). 

The message header consists of lines of text, each of which is terminated by a "\r\n" sequence. Each line in the header (except for the first line) has the following format (similar to HTTP):

    KEY:VALUE\r\n
where VALUE may have leading and trailing space characters. The end of the message header is an empty line (i.e., only contains "\r\n").

One of the lines in the message header must have the "Content-Length" key. The corresponding value specifies the exact number of bytes in the message body which immediately follows the empty line after the message header. Since we are using persistent TCP connections, within a connection, multiple messages can be sent. At the end of a message body, another message begins immediately.

Below are the details of each of the 353NET protocol messages.

![截屏2022-02-14 下午8 46 01](https://user-images.githubusercontent.com/35575612/153994333-9b5daa17-27b2-4ceb-ae6e-22630b3b0157.png)

**4. Configuration File Format**

A configuration file is a file in the INI format. Below is an example of a configuration file that you would use:

    [startup]
    host=
    port=12000
    logfile=pa4data/12000.log

    [params]
    max_ttl=9
    msg_lifetime=8
    neighbor_retry_interval=4

    [topology]
    :12000=:12002
    :12002=:12000,:12004,:12010,:12012
    :12004=:12002,:12006,:12012
    :12006=:12004,:12008
    :12008=:12006,:12010
    :12010=:12002,:12008
    :12012=:12002,:12004

    [map]
    ;                    +-------+
    ;                 /--+ 12010 +--------------------------\
    ;                 |  +-------+                          |
    ;                 |                                     |
    ; +-------+   +---+---+     +-------+   +-------+   +---+---+
    ; | 12000 +---+ 12002 +-----+ 12004 +---+ 12006 +---+ 12008 |
    ; +-------+   +---+---+     +---+---+   +-------+   +-------+
    ;                 |             |
    ;                 |  +-------+  |
    ;                 \--+ 12012 +--/
    ;                    +-------+

The "[startup]" section is required and it must contain the following key=value lines (can come in any order and additional keys can be ignored):

* host=hostname - hostname is name of the machine your server must serve on.
* port=number - number is the Control Port Number your server must listen on. 
* pidfile=pidfilename - pidfilename is the file name of a file for storing the process ID of your server.
* logfile=logfilename - logfilename is the file name of a file for logging messages.

All the other sections of the configuration files are identical for all nodes in the 353NET. The only section that's different in all the configuration files is the "[startup]" section.

* max_ttl=number - This is referred to as the Max TTL of your node. number is the TTL value your node must put into a message header when it initiates a flooded or routed message and it must be an integer > 0 and < 256.
* msg_lifetime=number - This is referred to as the Message Life Time of your node.
* neighbor_retry_interval=number - This is referred to as the Neighbor Retry Interval of your node.

The "[topology]" section is required and it contains key=value lines where a key is a NodeID and value is a list of comma-separated NodeIDs to mean that the node named in key should be neighbors with all the nodes listed in the corresponding value.

The "[map]" section is optional and it's empty (since everything in it are comments). This section a graphic representation of the network topology when all the nodes in the 353NET are up and running and connected to all its neighbors.

**5. User Console**

Each node must have an interactive commandline interface. Your program must use "NODEID> " (i.e., the NodeID, followed by a "greater than" symbol, followed by a space character) as the command prompt to tell that user which node they are on and that you are expecting the user to enter a line of command. Unless the user shutdowns the node using the quit command, the node should run forever.

The commands and their meanings are:

![截屏2022-02-14 下午8 48 22](https://user-images.githubusercontent.com/35575612/153994517-a8405c13-9a9a-4c9b-a586-1edd0af9663f.png)
![截屏2022-02-14 下午8 48 33](https://user-images.githubusercontent.com/35575612/153994533-dd4bbf6a-69ac-40e8-8d35-971349725bea.png)
![截屏2022-02-14 下午8 51 06](https://user-images.githubusercontent.com/35575612/153994847-f11d9424-5b76-4d7c-9f52-64357a88f265.png)
![截屏2022-02-14 下午8 51 21](https://user-images.githubusercontent.com/35575612/153994857-4e776b57-11a5-4fba-88e5-c691e7de49f8.png)
![截屏2022-02-14 下午8 51 32](https://user-images.githubusercontent.com/35575612/153994864-5e56baff-1b0a-402b-bbf7-2d03ecf9d87a.png)
![截屏2022-02-14 下午8 51 48](https://user-images.githubusercontent.com/35575612/153994873-e7e56c4b-79ce-485a-bdd9-32b503d413a4.png)
![截屏2022-02-14 下午8 48 43](https://user-images.githubusercontent.com/35575612/153994582-3aeaaa31-a93b-4164-b27f-4b2169b97316.png)
