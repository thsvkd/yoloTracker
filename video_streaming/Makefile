CXX = g++
OPENCV = `pkg-config --libs --cflags opencv`
PT = -lpthread
OPENMP = -fopenmp

all:videoserver videoclient videoclient_4cam

videoserver:
	$(CXX) -o videoserver videoserver.cpp $(PT) $(OPENCV)

videoclient:
	$(CXX) -o videoclient videoclient.cpp $(OPENCV)

videoclient_4cam:
	$(CXX) -o videoclient_4cam videoclient_4cam.cpp $(PT) $(OPENCV) $(OPENMP)

clean:
	rm videoserver videoclient videoclient_4cam
