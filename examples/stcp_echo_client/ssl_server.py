#!/usr/bin/env python3

import sys
import socket
import ssl

context = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
context.load_cert_chain(certfile="server.crt",
                        keyfile="server.key")

port = 33001
sock = socket.socket()
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', port))
sock.listen()

print(f"Waiting for the client to connect to ':{port}'.")
client = sock.accept()[0]
print('Client connected.')
client = context.wrap_socket(client, server_side=True)

while True:
    client.send(client.recv(1))
