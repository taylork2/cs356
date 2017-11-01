CS356 Thomson
HW2: UDP Ping Client and Server

Taylor Tu

Usage:

g++ -o main client.cpp && ./main [IP address of server] [IP port of server]

g++ -o main server.cpp && ./main 

Example commandline:

g++ -o main server.cpp && ./main

g++ -o main client.cpp && ./main 127.0.0.1 2222

Notes:

* Server is hardcoded to be on port 2222 because I like that number 
* Sending over char array instead of struct via packet 

