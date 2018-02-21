# Remote control robots server

The code is for remote control robots server at Control Systems Department of Saint Petersburg Electrotechnical University "LETI"

## Tech
This server provides remote access to robots on a table via TCP connection.
Server sends each connected client coordinates of robots' points and receives control actions for robots, that it sends to the IR transmitter module.
Maximum number of connected clients is specified in ```MAX_CONNECTIONS``` variable in ```server.h```

### Dependencies:
[Qt] - C++ Framework

[Qt]: <https://www.qt.io>
