

CXXFLAGS := -std=c++14 -O2 -I../Cbc-2.9-bin/include 
LDFLAGS := -L../Cbc-2.9-bin/lib 
LDLIBS := -lCoinUtils -lClp -lCbc

srcfiles := $(shell  find . -name "*.cpp")
localfiles := $(shell  find . -maxdepth 1 -name "*.cpp")
objects  := $(patsubst %.cpp, %.o, $(localfiles))
allobjects := $(patsubst %.cpp, %.o, $(srcfiles))

all: bin/detkdecomp bin/localbipkdecomp bin/globalbipkdecomp bin/balsepkdecomp bin/hg-stats bin/fracimprovedecomp bin/rankfhdecomp

bin/detkdecomp: $(objects) mains/DetKDecompMain.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o bin/detkdecomp ./mains/DetKDecompMain.o $(objects) $(LDLIBS)

bin/localbipkdecomp: $(objects) mains/LocalBipKDecompMain.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o bin/localbipkdecomp ./mains/LocalBipKDecompMain.o $(objects) $(LDLIBS)
		
bin/globalbipkdecomp: $(objects) mains/GlobalBipKDecompMain.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o bin/globalbipkdecomp ./mains/GlobalBipKDecompMain.o $(objects) $(LDLIBS)
		
bin/balsepkdecomp: $(objects) mains/BalSepKDecompMain.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o bin/balsepkdecomp ./mains/BalSepKDecompMain.o $(objects) $(LDLIBS)
		
bin/hg-stats: $(objects) mains/HypergraphStats.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o bin/hg-stats ./mains/HypergraphStats.o $(objects) $(LDLIBS)
		
bin/fracimprovedecomp: $(objects) mains/FracImproveHDMain.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o bin/hg-stats ./mains/FracImproveHDMain.o $(objects) $(LDLIBS)
		
bin/rankfhdecomp: $(objects) mains/RankFHDDecompMain.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o bin/hg-stats ./mains/RankFHDDecompMain.o $(objects) $(LDLIBS)

depend: .depend

.depend: $(srcfiles)
	    rm -f ./.depend
		    $(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	    rm -f $(allobjects)
	    rm -f bin/*

dist-clean: clean
	    rm -f *~ .depend

include .depend
