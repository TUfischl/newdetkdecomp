

CXX := g++
CXXFLAGS := -std=c++14 -O2 -I../Cbc-2.9-bin/include 
LDFLAGS := -L../Cbc-2.9-bin/lib 
#LDLIBS := -lCoinUtils -lClp -lCbc
#LDLIBS := -lOsiClp -lOsi -lCoinUtils -lClp -lCgl -lCbc

srcfiles := $(shell  find . -name "*.cpp")
localfiles := $(shell  find . -maxdepth 1 -name "*.cpp")
objects  := $(patsubst %.cpp, %.o, $(localfiles))
allobjects := $(patsubst %.cpp, %.o, $(srcfiles))

all: detkdecomp

detkdecomp: $(objects) mains/DetKDecompMain.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o detkdecomp ./mains/DetKDecompMain.o $(objects) $(LDLIBS)

localbipkdecomp: $(objects) mains/LocalBipKDecompMain.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o localbipkdecomp ./mains/LocalBipKDecompMain.o $(objects) $(LDLIBS)
		
globalbipkdecomp: $(objects) mains/GlobalBipDecompMain.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o globalbipkdecomp ./mains/GlobalBipDecompMain.o $(objects) $(LDLIBS)
		
balsepkdecomp: $(objects) mains/BalSepKDecompMain.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o balsepkdecomp ./mains/BalSepKDecompMain.o $(objects) $(LDLIBS)
		
hg-stats: $(objects) mains/HypergraphStats.o
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o hg-stats ./mains/HypergraphStats.o $(objects) $(LDLIBS)

depend: .depend

.depend: $(srcfiles)
	    rm -f ./.depend
		    $(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	    rm -f $(allobjects)

dist-clean: clean
	    rm -f *~ .depend

include .depend
