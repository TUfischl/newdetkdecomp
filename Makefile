appname := detkdecomp

CXX := g++
CXXFLAGS := -std=c++14 -O2 -I../Cbc-2.9-bin/include 
LDFLAGS := -L../Cbc-2.9-bin/lib 
LDLIBS := -lCoinUtils -lClp -lCbc
#LDLIBS := -lOsiClp -lOsi -lCoinUtils -lClp -lCgl -lCbc

srcfiles := $(shell find . -name "*.cpp")
objects  := $(patsubst %.cpp, %.o, $(srcfiles))

all: $(appname)

$(appname): $(objects)
	    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(appname) $(objects) $(LDLIBS)

depend: .depend

.depend: $(srcfiles)
	    rm -f ./.depend
		    $(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	    rm -f $(objects)

dist-clean: clean
	    rm -f *~ .depend

include .depend
