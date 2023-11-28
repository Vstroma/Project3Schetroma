output: socket.o 
	gcc socket.o -o output

socket.o: socket.c
	gcc -c socket.c

clean: 
	rm *.o output
