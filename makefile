client: superClient.c
	gcc -o  client superClient.c -lpthread

server: superServer.c
	gcc -o  server superServer.c -lpthread
