#!/usr/bin/env python3

import sys
import socket

if len(sys.argv) < 2:
    sys.exit("Usage: server.py <port>")

port = int(sys.argv[1])
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', port))
sock.listen()

print(f"Waiting for the client to connect to ':{port}'.")
client = sock.accept()[0]
print('Client connected.')

while True:
    client.send(client.recv(1))
