main: client3.c server3.c
	gcc -std=c99 -o client client3.c -pthread
	gcc -std=c99 -o server server3.c

clean:
	rm client
	rm server
	
tar:
	tar -cf SempleCarlton_Project3 client3.c server3.c movie.txt makefile
