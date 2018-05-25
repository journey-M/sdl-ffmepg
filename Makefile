
abc: sdllean.o	
		gcc sdllean.o -o abc -lSDL2 -lSDL2_image

sdllean.o: sdllean.c
		gcc -c sdllean.c 

clean: 
	rm sdllean.o  abc
