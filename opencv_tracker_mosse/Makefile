CXX = g++
OPENCV = `pkg-config --libs --cflags opencv`
DEBUG=1
OPTS = -Ofast

ifeq ($(DEBUG), 1) 
OPTS=-O0 -g
endif

tracker: 
	$(CXX) $(OPTS) -o tracker tracker_main.cpp orb.cpp tracking_dot.cpp tracker.h $(OPENCV)

clean:
	rm tracker
