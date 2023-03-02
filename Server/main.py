# OS
import platform
import subprocess
# Network
import socket
# Date and Time
import time
import datetime

#buffer Size
bufferSize = 1024

# getting the hostname by socket.gethostname() method
hostname = socket.gethostname()
# getting the IP address using socket.gethostbyname() method
ip_address = socket.gethostbyname(hostname)
ip_address_main = "192.168.153.13"
# Port we will using to UDP comms.
local_port_1 = 13000 # will use this port for rx/tx of text based Commands
local_port_2 = 13001 # will use this port for rx Image files

# CMD Server
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
UDPServerSocket.bind((ip_address_main, local_port_1))

print(f"UDP CMD server up and listening on {ip_address}:{local_port_1}")

# Image Server
UDPImgServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
UDPImgServerSocket.bind((ip_address_main, local_port_2))

print(f"UDP Image server up and listening on {ip_address}:{local_port_2}")

# network settings for master node
udp_stepper_ip = "192.168.153.20"
udp_cam1_ip = "192.168.153.21"
udp_cam2_ip = "192.168.153.22"
udp_cam3_ip = "192.168.153.23"

max_segments = int(20) # 20 * 3 = 60 photos and (20 * 10 * 1.8) = 360 degree

def ping(host):
    # Option for the number of packets as a function of
    param = '-n' if platform.system().lower()=='windows' else '-c'

    # Building the command. Ex: "ping -c 1 google.com"
    command = ['ping', param, '1', host]

    return subprocess.call(command) == 0

def rev_image_segs(rotation, cam_no):
    now = datetime.datetime.now()
    now = now.strftime("%Y%m%d%H%M%S")
    custom_name = str(now) + '_'+ str(rotation)+'_cam_1.jpg'
    if (cam_no == 2):
        custom_name = str(now) + '_'+ str(rotation)+'_cam_2.jpg'
    if (cam_no == 3):
        custom_name = str(now) + '_'+ str(rotation)+'_cam_3.jpg'
    file = open("images/"+custom_name, 'wb')
    print("Getting Captured img from the camera "+ str(cam_no))
    while(True):
        data_img = UDPImgServerSocket.recvfrom(1600)
        print(len(data_img[0]))
        file.write(data_img[0])
        if(len(data_img[0]) < 1460):
            file.close()
            break

def rev_msg():
    print("Getting Ack from the client")
    data_ack = UDPServerSocket.recvfrom(bufferSize)
    print(data_ack[0].decode("utf-8"))

# Start Time
startTime = time.time()
print("Started")
for rotation in range(0, max_segments):
    print(f"Progress : {int(((rotation + 1)/20) * 100)}%")
    # Move the Motor
    print("Sending CMD to Move the Motor")
    UDPServerSocket.sendto(str.encode("Spin"), (udp_stepper_ip, local_port_1))
    rev_msg()
    # Take photo from Camera 1
    print("Sending CMD to Click A Picture")
    UDPServerSocket.sendto(str.encode("Capture"), (udp_cam1_ip, local_port_1))
    rev_image_segs(rotation, 1)
    rev_msg()
    #end camera 1
    # Take photo from Camera 2
    print("Sending CMD to Click A Picture")
    UDPServerSocket.sendto(str.encode("Capture"), (udp_cam2_ip, local_port_1))
    rev_image_segs(rotation, 2)
    rev_msg()
    #end camera 2
    # Take photo from Camera 3
    print("Sending CMD to Click A Picture")
    UDPServerSocket.sendto(str.encode("Capture"), (udp_cam3_ip, local_port_1))
    rev_image_segs(rotation, 3)
    rev_msg()
    #end camera 3
    time.sleep(1) # sleep for 1 sec

print('Time taken:', time.time() - startTime)
exit()
