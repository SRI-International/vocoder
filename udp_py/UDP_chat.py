#  KTDavis 13June202
# Simple chat program running on UDP.  (local) sender talk to (remote) receiver to IP / PORT Pair
# Remote running same chat program sends to our local IP and listens on agreed PORT.  I have been a
# Different PORT to send on and receive on....but no reason why they can be the same port.


import os
#os.system("tput setaf 3")
print("UDP Chat App with Multi-Threading")# importing modules for the chat appimport socket
import time
import threading
import sys
import socket
import pyaudio # PortAudio bindings

# PyAudio parameters
CHUNK    = 1024
FORMAT   = pyaudio.paInt16
CHANNELS = 1
RATE     = 8000

def callback(in_data, frame_count, time_info, status):
    return

# AF_INET = Network Address Family : IPv4
# SOCK_DGRAM = DataGram Socket : UDP

# Function for receiving
def receiver():
    print("receiver starting\n")
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind((ip_sender, port_sender)) #binding the IP address and port number
    while True:
        msg = s.recvfrom(1024)
        print("\n RX >>> "+msg[0].decode()+f"\n{name} Tx: ", end='')  #reprint prompt for sender input
        if "exit" in msg[0].decode() or "bye" in msg[0].decode():
            print("bye bye from receiver")
            sys.exit()
            return

#Function for sending
def sender():
    print("sender starting....\n")
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    text = "hello"
    if ip_sender == ip_receiver:   #To print prompt first time before and received data
        print(f'{name} TX: ',end='')
    while True:
        if "bye" in text or "exit" in text in text:
            print("bye bye from sender")
            s.sendto(text.encode(), (ip_sender, port_sender))  # SPECIAL to kill (local) receiver if not attached to REAL Sender....
                                                               # Receiver could block infinitely waiting on sender
            sys.exit()
            return
        else:
            if ip_sender == ip_receiver:   #data received after each send reprints input prompt, prevent double prompt
                text = input()
            else:
                text = input(f'{name} TX: ')
            text = name+":"+text
            s.sendto(text.encode(), (ip_receiver, port_receiver))

print("Initializing....")
print("KTDavis MP_VPN address and ports 128.18.167.251  ports 3478 (default),3479 ")
print("\nenter 'exit' or 'bye' in message to terminate\n\n")

s = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)  # Being used to gather current IP address
s.connect(("8.8.8.8", 80))
currentIP=s.getsockname()[0]
currentHostName=socket.gethostname()
s.close()
print("\tyour current IP is       "+currentIP)
print("\tyour current HostName is "+currentHostName)

#Gets a bunch of input from the user ... where to send and where to receive data
ip_sender = input("\nEnter the IP of your system : ")  #Default to local host, could use current IP
if not ip_sender :
    ip_sender="127.0.0.1"
    print("\tusing default IP: "+ip_sender)
port = input("Enter the port of your system: ")
if not port :
    port_sender = 3478
    print(f"\tusing default Port: {port_sender:5d}")
else:
    port_sender = int(port)

ip_receiver = input("\nEnter the IP of receiver: ")
if not ip_receiver :
    ip_receiver="127.0.0.1"
    print("\tusing default IP: "+ip_receiver)
port = input("Enter the port of the receiver: ")
if not port :
    port_receiver = port_sender   #loop back on same port
    print(f"\tusing default Port: {port_receiver:5d}")
else:
    port_receiver = int(port)


name = input("\nEnter your name: ")
if not name :
    name=currentHostName
    print("\tusing default name: "+name)

send = threading.Thread(target=sender)

receive = threading.Thread(target=receiver)

receive.start()   #allow receiver to start first (debug info screws up sender first prompt)
send.start()

send.join()  #Joins wait for threads to terminate
receive.join()

print("\n======\nTerminating Chat....\n======")

sys.exit(0)


