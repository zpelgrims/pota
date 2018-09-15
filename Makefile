OS := $(shell uname)

ifeq ($(OS), Darwin)
	CXX=clang++
endif
ifeq ($(OS), Linux)
	CXX=g++
endif

# make sure to modify the path to the arnold SDK
ifeq ($(OS), Darwin)
	ARNOLD_PATH=/Users/zeno/Arnold-5.1.1.2-darwin
endif
ifeq ($(OS), Linux)
	ARNOLD_PATH=/home/users/zenop/Arnold-5.1.1.1-linux
endif


LENSES = -DLENS_ID_FREE
#for i in lens list, LENSES += LENS_ID_X


CXXFLAGS=-Wall -std=c++11 -O3 -shared -fPIC -Wno-narrowing -I${ARNOLD_PATH}/include -I/../Eigen/Eigen -DDEBUG_LOG
LDFLAGS=-L${ARNOLD_PATH}/bin -lai 

HEADERS= src/lens.h src/pota.h src/tinyexr.h src/common.h
.PHONY=all clean

all: pota potabokehAOV

ifeq ($(OS), Darwin)
pota: Makefile src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} src/pota.cpp -o bin/pota.dylib ${LDFLAGS}

potabokehAOV: Makefile src/potabokehAOV.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} src/potabokehAOV.cpp -o bin/potabokehAOV.dylib ${LDFLAGS}
endif
ifeq ($(OS), Linux)
pota: Makefile src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} src/pota.cpp -o bin/pota.so ${LDFLAGS}

potabokehAOV: Makefile src/potabokehAOV.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} src/potabokehAOV.cpp -o bin/potabokehAOV.so ${LDFLAGS}
endif


clean:
	rm -f pota potabokehAOV
