CXX = g++
OPENCV = `pkg-config --libs --cflags opencv`
PT = -lpthread

all:videoserver videoclient

videoserver:
	$(CXX) -o videoserver videoserver.cpp $(PT) $(OPENCV)

videoclient:
	$(CXX) -o videoclient videoclient.cpp $(OPENCV)

clean:
	rm videoserver videoclient
