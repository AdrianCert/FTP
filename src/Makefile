all:
	make remove
	make server
	make client

mkpath:
	mkdir -p ./../dist

server:
	gcc server.c common.c -o server_ftp
	make mkpath
	mv server_ftp ../dist/server_ftp

client:
	gcc client.c common.c -o client_ftp
	make mkpath
	mv client_ftp ../dist/client_ftp

remove:
	rm -f ../dist/client_ftp
	rm -f ../dist/server_ftp