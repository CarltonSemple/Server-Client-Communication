main: client.c server.c
	gcc -std=c99 -o client client2.c -pthread
	gcc -std=c99 -o server server2.c

clean:
	rm client
	rm server
	
tar:
	tar -cf SempleCarlton_Project2 client2.c server2.c movie.txt makefile
