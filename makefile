server: superServer.c
	gcc -c superServer.c
	gcc -o  server superServer.c -lpthread

client: superClient.c
	gcc -c superClient.c
	gcc -o  client superClient.o -lpthread


