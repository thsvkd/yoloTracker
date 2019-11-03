CXX = g++
OPENCV = `pkg-config --libs opencv`

all:video_maker

video_maker:
	$(CXX) -o video_maker video_maker.cpp -g $(OPENCV)

clean:
	rm video_maker