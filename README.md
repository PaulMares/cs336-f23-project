# Computer Networks Project

By Paul Maresquier

A set of applications that can be used to detect compression on a network path.
These applications use the method outlined in the paper [End-to-End Detection of
Compression of Traffic Flows by Intermediaries](https://www.cs.usfca.edu/vahab/resources/compression_detection.pdf).


## Table of Contents

- Requirements
- Configuration File
- Part 1: client-server applications
	- Building
	- Running
- Part 2: standalone application
	- Building
 	- Running


## Requirements

To build and run this project, you must follow these requirements:

- For the standalone application only one computer or virtual machine is
	necessary, but for the client-server applications you will need two
	computers or VMs
- All computers or VMs must use linux
- To build, the computer or VM must have `make` and `gcc` installed


### Configuration File

The configuration file is a file of type .ini that must exist in the same folder
as the client or standalone application. The source code contains a default
config.ini file with the following options:

- `server-addr`: The IPv4 address of the server application, **must be set for
	the client and standalone applications to work**.
- `source-addr`: The IPv4 address of the client or standalone application,
	**must be set for the standalone application to work**.
- `source-port`: The port that the client will bind to when communicating with
	the server. (default value: 9876)
- `dest-udp-port`: The port that the server will listen on when receiving UDP
	packet trains from the client. (default value: 8765)
- `dest-tcp-port-head`: The server port that the standalone application will
	send the **head** SYN packet to. (default value: 9999)
- `dest-tcp-port-tail`: The server port that the standalone application will
	send the **tail** SYN packet to. (default value: 8888)
- `tcp-port-prev`: The port that the server will listen to during the
	**pre-probing** phase, **must be the same as the argument passed to the
	server when running the server application**. (default value: 7777)
- `tcp-port-post`: The port that the server will listen to during the
	**post-probing** phase. (default value: 6666)
- `udp-payload-size`: The size in bytes of each UDP payload sent by the client
	to the server, must be lower than MTU. (default value: 1000)
- `measurement-time`: Time in seconds between the low-entropy packet train and
	the high-entropy packet train. (default value: 15)
- `udp-train-len`: Number of UDP packets in each packet train. (default value:
	6000)
- `udp-packet-ttl`: Max number of hops allowed for UDP packets sent by the
	standalone application. (default value: 255)

**Notes**
- `port` numbers can be any number between 1 and 65535, but they cannot be a
  	reserved port
- Although 5 seconds is typically enough for `measurement-time`, 15
  	seconds is recommended for slower networks
- The effects of compression become more pronounced with a larger
	`udp-train-len`, with diminishing returns. Smaller values may be easier
	for smaller networks but it will be harder to detect compression
- `udp-packet-ttl` should be a number between 1 and 255. Values too small may
  	prevent the program for working, but can also be used to find
  	how many hops away the compression link is


## Part 1: client-server applications

Part 1 consists of two applications, one that is run by a client and another
that is run on a server.


### Building

1. Download the source code on both computers or virtual machines
1. Open the downloaded source code folder in a terminal
1. Use `make` to run the Makefile and compile the programs


### Running

Prior to running the client application, make sure to set the configuration file
parameter `server-addr` to the IPv4 address of the machine that will be running
the server and the parameter `tcp-port-prev` to the port passed when running
the server application.

Run an instance of the server application on one device, and run an instance of
the client application a few seconds later on another device.

The client and server applications both have only one option, `-v`, which
enables verbose mode. To run them you must open the folder containing the
application in a terminal and run the appropriate command:

- Server: `./compdetect_server [OPTIONS] [PORT]`
	- `PORT` is the network port that the server will listen to during the
		pre-probing phase. **Must be the same as the `tcp-port-prev` option in
		the config file passed to the client application**. If unspecified, it
		will default to `7777`.
- Client: `./compdetect_client [OPTIONS] [CONFIG_FILE]`
	- `CONFIG_FILE` is the filename of the configuration file. If unspecified,
		it will default to `config.ini`


## Part 2: standalone application

Part 2 consists of one application that will ping a remote server.


### Building

1. Download the source code on a computer or virtual machine
1. Open the downloaded source code folder in a terminal
1. Use `make` to run the Makefile and compile the program


### Running

Prior to running the standalone application, make sure to set the configuration file
parameters `server-addr` and `client-addr` to the IPv4 addresses of the server and
host respectively. The server can be any computer connected to the Internet that will
respond to TCP connections.

The standalone application only has one option, `-v`, which enables verbose mode.
To run it you must open the folder containing the application in a terminal and 
run the following command:

- Client: `sudo ./compdetect [OPTIONS] [CONFIG_FILE]`
	- `CONFIG_FILE` is the filename of the configuration file. If unspecified,
		it will default to `config.ini`
	- For the program to work, you **must use sudo**.

