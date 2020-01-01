#!/usr/bin/env bash

# Certificate Authority
openssl req -new -x509 -days 10000 -keyout ca.key -out ca.crt

# Server
openssl genrsa -out server.key 2048
openssl req -out server.csr -key server.key -new
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 10000
