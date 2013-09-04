#!/bin/ruby

require 'socket'

include Socket::Constants
host='localhost'
port = 40713;

begin
	socket = Socket.new(AF_INET,SOCK_STREAM,0)
	sockaddr = Socket.pack_sockaddr_in(port,host)
	socket.connect(sockaddr)

	# puts 'sending set'
	# socket.write("*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n");
	# puts socket.recvfrom(256)[0].chomp

	# puts 'sending get'
	# socket.write("*2\r\n$3\r\nGET\r\n$5\r\nmykey\r\n");
	# puts socket.recvfrom(256)[0].chomp
	# 
	
	puts 'sending get'
	socket.write("*2\r\n$3\r\nGET\r\n$5\r\nmykey\r\n");
	puts socket.recvfrom(256)[0].chomp
rescue
	puts $!
ensure
	socket.close if socket
end