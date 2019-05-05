import socket
import time
import cv2
import numpy as np
from copy import deepcopy
from math import cos, sin

def bytes2coor(byte_fnc):
	receivedCoor_fnc = [0, 0, 0]

	receivedCoor_fnc[0] = float(((-1)**(byte_fnc[0]>>7)) * ((byte_fnc[1]) | (((byte_fnc[0]&0x7f)<<8))))
	receivedCoor_fnc[1] = float(((-1)**(byte_fnc[2]>>7)) * ((byte_fnc[3]) | (((byte_fnc[2]&0x7f)<<8))))
	receivedCoor_fnc[2] = float(((-1)**(byte_fnc[4]>>7)) * ((byte_fnc[5]) | (((byte_fnc[4]&0x7f)<<8))))

	return receivedCoor_fnc


if __name__ == '__main__':
	# Initial setup

	# create a blank canvas
	img = np.zeros((512, 512, 3), np.uint8)
	img.fill(255)

	# Setup server and client
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	s.bind((socket.gethostname(), 1234)) # bind(ip, port)
	print("Done binding.")
	s.listen(2)

	clientsocket, address = s.accept()
	print(f"Connection from {address} has been established!")

	clientsocket.settimeout(1)

	while True:
		print()
		print("What do you want to do?")
		print("0. Ask for id.")
		print("1. Ask for angle until interrupt.")

		try:
			a = int(input("I choose: "))
		except Exception:
			print("Error.")
			a = -1;


		if (a == 0): # ask for identification
			clientsocket.send(bytes([0]))
			while True:
				try:
					identification = clientsocket.recv(8)
					identification = int.from_bytes(identification, "little")
					print(f"ID: {identification}.")
					break

				except socket.timeout:
					print("Time out. Will try again.")
		
		elif (a == 1): # ask for angle		
			start = time.time()
			while True:
				try:
					elapsedTime = time.time()
					if elapsedTime > 0.5:
						start = time.time()
						try:
							clientsocket.send(bytes([1]))

							bytesReceived = []
							full_msg = []

							while (len(full_msg) < 6):
								bytesReceived = clientsocket.recv(8)
								for x in range(len(bytesReceived)):
									full_msg.append(bytesReceived[x])

							receivedAngle = bytes2coor(full_msg)
							print(f"coordinate received: {receivedAngle}")

							draw_point = (256 + int(100*cos(receivedAngle[2]/180.0*3.1415)),256 - int(100*sin(receivedAngle[2]/180.0*3.1415)))
							print(f"Draw point: {draw_point}")

							img.fill(255)
							cv2.arrowedLine(img, (256, 256), (draw_point[0], draw_point[1]), (0, 0, 0), thickness = 5)
							cv2.imshow("Drone orientation",img)
							cv2.waitKey(1)

						except socket.timeout:
							print("Time out. Will try again.")

				except KeyboardInterrupt:
					print("Done.")
					break
