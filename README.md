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

## Running TCP Download Server
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


## Running UDP Download Server
