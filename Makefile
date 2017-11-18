CC=clang
CXX=clang++

ARNOLD_PATH=/Users/zeno/Arnold-5.0.2.0-darwin

CXXFLAGS=-Wall -O3 -shared -fPIC -I${ARNOLD_PATH}/include
LDFLAGS=-L${ARNOLD_PATH}/bin -lai

HEADERS=\
polynomialOptics/render/lens.h\
polynomialOptics/render/init.h\
polynomialOptics/render/pt_evaluate.h\
polynomialOptics/render/pt_sample_aperture.h\
polynomialOptics/render/pt_evaluate_aperture.h\
polynomialOptics/render/lt_sample_aperture.h


.PHONY=all clean

all: pota

pota: Makefile src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} src/pota.cpp -o bin/pota.dylib ${LDFLAGS}

clean:
	rm -f pota
