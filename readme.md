### Usage
Go to the root directory, run 

``> make``

It will compile and generate four folders containing executable files.
Go to server_dir, run 

``> ./tcp_server``

It will output the hostname and port for later use, default port is 30000 and you can modify it in tcp_server.c under 
```
line 18: #define PORT "30000"

```
eg:

```Server started...
  Host: Yuyuans-MBP
  Port: 30000
  Server: Waiting for connections...
``` 

Go to client_dir, run:

``> ./tcp_client <HOST> <PORT> // replace HOST and PORT with the value you get from above step``

After you start client, you will see:

```$xslt
Using hostname Yuyuans-MBP and port 30000
client: connecting to 172.16.1.84 on socket 6
Command supported:
1. add [KEY] [VALUE]
2. getvalue [KEY]
3. getall
4. remove [KEY]
5. quit
```

Then follow the usage with the add/getvalue/getall/remove/quit operation.

For proxy server, use the same instruction as client.

For UDP server, use the same instruction as server.