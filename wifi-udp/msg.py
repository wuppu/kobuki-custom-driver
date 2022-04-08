#ArdUDP.py
import socket

UDP_IP = "192.168.240.1"
# UDP_IP = ""
UDP_PORT = 5555


sock = socket.socket(socket.AF_INET, #Internet
  socket.SOCK_DGRAM) #UDP

sock.sendto('[Debug] Command string message', (UDP_IP, UDP_PORT));
