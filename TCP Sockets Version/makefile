main: client.c server.c
	gcc -std=c99 -o client client.c -pthread
	gcc -std=c99 -o server server.c

clean:
	rm client
	rm server
	
tar:
	tar -cf SempleCarlton_Project1 client.c server.c movie.txt makefile
