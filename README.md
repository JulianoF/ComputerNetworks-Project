# ComputerNetworks-Project

## Running Hello TCP Server
Go into the Directory HelloWorldTCP
```
cd HelloWorldTCP
```
IF you are running the Server:
```
make server
./helloServer [PORT NUM]
```
If you are running the Client 
```
make client
./helloClient [PORT NUM]
```

<details>
<summary style="font-size: 40px; font-weight: 500; cursor: pointer;"> TCP Download Server </summary>
Go into the TCP_DownloadServer directory
```
cd TCP_DownloadServer
```
### If you are running the Server
```
cd TCP_Server
```
Run Make
```
make
```
Start server with your information

> [!IMPORTANT]  
> If NO Port And IP Specififed it will use 8000 and 127.0.0.1 by default

```
./TCP-Server [PORT] [IP ADDRESS]
```

### If you are running the Client
```
cd TCP_Client
```
Run Make
```
make
```
Start server with your information

> [!IMPORTANT]  
> If NO Port And IP Specififed it will use 8000 and 127.0.0.1 by default

```
./TCP-Client [PORT] [IP ADDRESS]
```

</details>

<details>
<summary style="font-size: 40px; font-weight: 500; cursor: pointer;"> UDP Download Server </summary>

## Build Both UDP Client & Servers:

- In `UDP_DownloadServer` directory run `make all` altenratively, you can build Client & Server seperate
- Server: `make UDP-Server`, Client: `make UDP-Client`
- To clean (del) both app makes. run `make clean`
- To build (both apps) in **debug** mode, run `make d` *(For enhanced error messages with GDB)*

### Run both Applications on VMs.

1. On Server VM, run `ifconfig enp0s3` command, and find the **inet ipv4 address**, something like 10.0.0.X generally
2. On Server VM, execute: `make UDP-Server`, then: `./UDP-Server` in terminal
3. On Client VM, execute: `make UDP-Client`, then: `./UDP-Client <VM1's IPv4>` in terminal, this is the ip you found with command 1.


### If you're trying to run UDP server/client locally for dev purposes, you can try:

`./UDP-Server` in the terminal

`./UDP-Client 127.0.0.1` in a second terminal window

**127.0.0.1** could also be **0.0.0.0**

</details>