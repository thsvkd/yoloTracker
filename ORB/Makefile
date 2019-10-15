CXX = g++
OPENCV = `pkg-config --libs --cflags opencv`
DEBUG=1
OPTS = -Ofast

ifeq ($(DEBUG), 1) 
OPTS=-O0 -g
endif

tracker: 
	$(CXX) $(OPTS) -o orb orb.cpp $(OPENCV)

clean:
	rm orb
