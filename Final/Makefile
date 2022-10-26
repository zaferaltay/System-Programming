all :
	gcc -c -g server.c -Wall -Wextra -pedantic
	gcc -o server server.o -lm -pthread 

	gcc -c -g client.c -Wall -Wextra -pedantic
	gcc -o client client.o -lm -pthread 
clean:
	rm -f server *.o
	rm -f client *.o

