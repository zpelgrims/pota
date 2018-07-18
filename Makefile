OS := $(shell uname)

ifeq ($(OS), Darwin)
	CXX=clang++
endif
ifeq ($(OS), Linux)
	CXX=g++
endif

# make sure to modify the path to the arnold SDK
ifeq ($(OS), Darwin)
	ARNOLD_PATH=/Users/zeno/Arnold-5.0.2.0-darwin
endif
ifeq ($(OS), Linux)
	ARNOLD_PATH=/home/users/zenop/Arnold-5.1.1.1-linux
endif


CXXFLAGS=-Wall -std=c++11 -O3 -shared -fPIC -Wno-narrowing -I${ARNOLD_PATH}/include -I/../Eigen/Eigen
LDFLAGS=-L${ARNOLD_PATH}/bin -lai 

HEADERS= src/lens.h src/pota.h src/tinyexr.h 

.PHONY=all clean

all: pota pota_bokehAOV

ifeq ($(OS), Darwin)
pota: Makefile src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} src/pota.cpp -o bin/pota.dylib ${LDFLAGS}

pota_bokehAOV: Makefile src/pota_bokehAOV.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} src/pota_bokehAOV.cpp -o bin/pota_bokehAOV.dylib ${LDFLAGS}
endif
ifeq ($(OS), Linux)
pota: Makefile src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} src/pota.cpp -o bin/pota.so ${LDFLAGS}

pota_bokehAOV: Makefile src/pota_bokehAOV.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} src/pota_bokehAOV.cpp -o bin/pota_bokehAOV.so ${LDFLAGS}
endif


clean:
	rm -f pota pota_bokehAOV
