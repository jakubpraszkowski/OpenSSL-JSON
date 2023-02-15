# JSON Communication Client-Server Program

This program consists of a client and a server that communicate with each other using JSON-formatted frames. The client sends information to the server about the city, current temperature, and pressure, and then closes the connection. The server can handle only one connection at a time.

To ensure the security of the connection, the program uses certificates generated for clients. The server verifies the identity of the client after establishing the connection and rejects the connection if the client does not have a valid certificate.

This program was written in C++ using OpenSSL.
