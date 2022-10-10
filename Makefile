main: main.cpp Encoder.cpp Encoder.h Writer.cpp Writer.h stb_image.h
	g++ -o encoder -std=c++11 main.cpp Encoder.cpp Writer.cpp

clean:
	rm encoder
