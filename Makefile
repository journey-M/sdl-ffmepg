CC = gcc -g


player: video_decode.o demuxing.o main.o 
		gcc -g video_decode.o demuxing.o main.o  -o player -lSDL2 -lSDL2_image -lavformat -lavutil -lavcodec -lswscale -lswresample

# player: decVideo.o main.o 
# 		gcc -g decVideo.o  main.o  -o player -lSDL2 -lSDL2_image -lavformat -lavutil -lavcodec -lswscale -lswresample

main.o:	main.c
	gcc -g -c $^ -lSDL2

decVideo.o: decVideo.c
	gcc -g -c $^ -lavformat -lavutil -lavcode -lswscale -lswresample

demuxing.o:	demuxing.c
	$(CC) -c $^ -lavformat -lavcodec 

video_decode.o: video_decode.c
	$(CC) -c $^ -lavformat -lavcodec 

.PHONY: clean
clean: 
	-rm *.o  player

