#!/usr/bin/python
from socket import *
from sys import *
import struct
import random


class server():

	def __init__(self):
		#set up the port		
		port = int(argv[1])
		host = "localhost"
		self.sock = socket(AF_INET,SOCK_DGRAM)
		self.sock.bind((host,port))
		
		#initialize client table and map		
		self.clients = set([])
		self.x=''

		print "Server listening on ", port
	
	#method to generate the map	
	def cx(self):
		self.x='';
		for i in range(1250):
			self.x += chr(random.randint(0,255))	
	
	def listen(self):
		while(1):
			buf = 1250

			#Add client to client table when message recieved			
			data,addr = self.sock.recvfrom(buf,0)
			self.clients.add(addr)
			print addr
	
			print data
	
			print "We have " + str(len(self.clients)) + " clients connected"	
			
			#send map to all of the cients
			self.cx();
			for client in self.clients:
				self.sock.sendto(self.x,client)
							
j = [str(x)[0] for x in range(1250)]
x= ''
for i in j:
	x += i	

#start the server
serv = server()
serv.listen()
