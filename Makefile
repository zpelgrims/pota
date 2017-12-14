CC=clang
CXX=clang++

ARNOLD_PATH=/Users/zeno/Arnold-5.0.2.0-darwin

CXXFLAGS=-Wall -std=c++11 -O3 -shared -fPIC -Wno-narrowing -I${ARNOLD_PATH}/include
LDFLAGS=-L${ARNOLD_PATH}/bin -lai

HEADERS=\
include/lens.h\
include/init.h\
include/pt_evaluate.h\
include/pt_sample_aperture.h\
include/pt_evaluate_aperture.h\
include/lt_sample_aperture.h


.PHONY=all clean

all: pota pota_thinlens sample_bokeh

pota: Makefile src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} src/pota.cpp -o bin/pota.dylib ${LDFLAGS}

pota_thinlens: Makefile src/pota_thinlens.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} src/pota_thinlens.cpp -o bin/pota_thinlens.dylib ${LDFLAGS}

sample_bokeh: Makefile src/sample_bokeh.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} src/sample_bokeh.cpp -o bin/sample_bokeh.dylib ${LDFLAGS}

clean:
	rm -f pota pota_thinlens sample_bokeh
