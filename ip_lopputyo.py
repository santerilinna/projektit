#!/usr/bin/python
# -*- coding: utf-8 -*-
# python ip_lopputyo.py 195.148.20.105 10000 "HELLO"            (testaamiseen komentokehotteessa)

"""
Internetin perusteet -kurssin lopputyön koodi

Santeri Linna
2022
"""

import sys
import socket
import struct

def send_and_receive_tcp(address, port, message):
    """
    Luo TCP-socketin, lähettää HELLO viestin,
    vastaanottaa serverin vastauksen ja parsii siitä UDP-portin sekä Client-ID:n.
    """
    print(f"\nYou gave arguments: {address} {port} {message}\n\n======TCP======\n")
    tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcp_socket.connect((address, port))
    message_encoded = message.encode()
    tcp_socket.sendall(message_encoded)
    data = tcp_socket.recv(1024)
    data_decoded = data.decode()
    print(data_decoded)
    tcp_socket.close()
    data_split = data_decoded.split()
    cid = data_split[1]
    udp_port = int(data_split[2])
    message2 = (f"Hello from {cid}")
    send_and_receive_udp(address, udp_port, message2, cid)

def send_and_receive_udp(address, port, content, cid):
    """
    Luo UDP-socketin ja lähettää sekä vastaanottaa viestejä serverin kanssa.
    """
    print("======UDP=======\n")
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.connect((address, port))
    ack = True
    eom = False
    data_remaining = 0
    while True:
        data = struct.pack(">8s??HH128s", cid.encode(), ack, eom, data_remaining, len(content), content.encode())
        print(f"CLIENT: {content}")
        udp_socket.sendall(data)
        recv_data = udp_socket.recv(1024)
        _cid_encoded2, _ack2, eom2, _data_remaining2, _content_length2, content_wrong_order = struct.unpack("8s??HH128s", recv_data)
        print(f"SERVER: {content_wrong_order.decode()}")
        if  not eom2 or "Bye" not in recv_data.decode(): #tarkistaa onko viesti viimeinen, tarkistamalla, onko serverin lähettämän viestin EOM Tosi, tai onko viestissä sana Bye.
            content_wrong_order_decoded = content_wrong_order.decode().strip("\x00")
            content_split = content_wrong_order_decoded.split(" ")
            content_split.reverse()
            content = " ".join(content_split)
        else:
            break
    udp_socket.close()

def main():
    USAGE = 'usage: %s <server address> <server port> <message>' % sys.argv[0]
    try:
        # Get the server address, port and message from command line arguments
        server_address = str(sys.argv[1])
        server_tcpport = int(sys.argv[2])
        message = str(sys.argv[3])
    except IndexError:
        print("Index Error")
    except ValueError:
        print("Value Error")
    # Print usage instructions and exit if we didn't get proper arguments
        sys.exit(USAGE)
    send_and_receive_tcp(server_address, server_tcpport, message)

if __name__ == '__main__':
    main()
