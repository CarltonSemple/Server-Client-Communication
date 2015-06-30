main: client4.c server4.c
	gcc -std=c99 -o client client4.c -pthread
	gcc -std=c99 -o server server4.c

clean:
	rm client
	rm server
	
tar:
	tar -cf SempleCarlton_Project4 client4.c server4.c movie.txt makefile
