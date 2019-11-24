#!/usr/bin/env python3

import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', 33000))
sock.listen()

print('Waiting for the client to connect...')
client = sock.accept()[0]
print('Client connected.')

while True:
    client.send(client.recv(1))
