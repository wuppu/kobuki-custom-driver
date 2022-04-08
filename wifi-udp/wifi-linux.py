import socket
import time
from bridgeclient import BridgeClient

bridge = BridgeClient()

print("Initialize wifi udp bridge")

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("192.168.240.1", 5555))

while True:
        try:
                msg = sock.recv(1024)
                if msg:
                        bridge.put("D13", msg)
                print '[', datetime.datetime.now(), '] ', ''.join('{:02X}'.format(ord(x)) for x in msg)
        except KeyboardInterrupt:
                break
        except Exception as err:
                print err
                pass
