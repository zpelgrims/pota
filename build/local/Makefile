OS := $(shell uname)

ifeq ($(OS), Darwin)
	CXX=clang++
	SHARED_LIB_FORMAT=dylib
endif
ifeq ($(OS), Linux)
	CXX=g++
	SHARED_LIB_FORMAT=so
endif

# these need to be changed to the envvar
ifeq ($(OS), Darwin)
	ARNOLD_PATH=${LENTIL_ARNOLD_SDKS}/Arnold-5.2.0.0-darwin
endif
ifeq ($(OS), Linux)
	ARNOLD_PATH=${LENTIL_ARNOLD_SDKS}/Arnold-5.2.0.0-linux
endif


LENSES = -DLENS_ID_FREE
LENSES += -DLENS_ID_COMMERCIAL
$(info    LENSES: $(LENSES))


CXXFLAGS=\
	-Wall\
	-std=c++17\
	-O3\
	-shared\
	-fPIC\
	-Wno-narrowing\
	-I${ARNOLD_PATH}/include\
	-I/../../../Eigen/Eigen\
	-I/../../../fmt/include/fmt\
	-I/../../../polynomial-optics/src\
	-DDEBUG_LOG\
	-DFMT_HEADER_ONLY\
#	-g

LDFLAGS=-L${ARNOLD_PATH}/bin -lai 

HEADERS=\
	../../src/lens.h\
	../../src/pota.h\
	../../src/imagebokeh.h\
	../../src/tinyexr.h

.PHONY=all clean

all: pota pota_thinlens pota_raytraced potabokehAOV

pota: Makefile ../../src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} ../../src/pota.cpp -o ${user_build_folder}/bin/pota.${SHARED_LIB_FORMAT} ${LDFLAGS}

pota_thinlens: Makefile ../../src/pota_thinlens.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ../../src/pota_thinlens.cpp -o ${user_build_folder}/bin/pota_thinlens.${SHARED_LIB_FORMAT} ${LDFLAGS}

pota_raytraced: Makefile ../../src/pota_raytraced.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} ../../src/pota_raytraced.cpp -o ${user_build_folder}/bin/pota_raytraced.${SHARED_LIB_FORMAT} ${LDFLAGS}

potabokehAOV: Makefile ../../src/potabokehAOV.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} ../../src/potabokehAOV.cpp -o ${user_build_folder}/bin/potabokehAOV.${SHARED_LIB_FORMAT} ${LDFLAGS}


clean:
	rm -f pota pota_thinlens pota_raytraced potabokehAOV
