CC = gcc -g


player:	audio_decode.o video_decode.o demuxing.o main.o 
		gcc -g $^  -o player -lSDL2 -lSDL2_image -lavutil -lavformat -lavcodec -lswscale -lswresample -lpthread

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

audio_decode.o: audio_decode.c
	$(CC) -c $^ -lavformat -lavcodec -lavutil -lswresample


play_audio:	play_audio.c
	$(CC) -o $@ $^ -lSDL2


.PHONY: clean
clean: 
	-rm *.o  player

