# TCP-Echo

## TCP Echo Server and Client programs

### Server build:
```
$ g++ -std=c++14 -pthread TCPEchoServer.cpp -o executables/TCPEchoServer
```

### Client build:
```
$ g++ -std=c++14 -pthread TCPEchoClient.cpp -o executables/TCPEchoClient
```

### Server run command:
```
$ ./executables/TCPEchoServer <port>
```

### Client run command:
```
$ ./executables/TCPEchoClient <host> <port> <message> <optional_loop_cnt>
```

