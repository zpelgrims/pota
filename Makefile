OS := $(shell uname)

ifeq ($(OS), Darwin)
	CXX=clang++
endif
ifeq ($(OS), Linux)
	CXX=g++
endif

# these need to be changed to the envvar
ifeq ($(OS), Darwin)
	ARNOLD_PATH=${LENTIL_ARNOLD_SDKS}/Arnold-5.2.0.0-darwin
endif
ifeq ($(OS), Linux)
	ARNOLD_PATH=${LENTIL_ARNOLD_SDKS}/Arnold-5.2.1.0-linux
endif


LENSES = -DLENS_ID_FREE
# lens list is expected in following format: lens_list=.1001.2001.2002.2003 (first dot is important)
LENSES += $(subst ., -DLENS_ID_, ${lens_list})
$(info    LENSES: $(LENSES))



CXXFLAGS=\
	-Wall\
	-std=c++14\
	-O3\
	-shared\
	-fPIC\
	-Wno-narrowing\
	-I${ARNOLD_PATH}/include\
	-I/../Eigen/Eigen\
	-I/../polynomial-optics/src\
	-DDEBUG_LOG\
	-DFMT_HEADER_ONLY\
	-g

LDFLAGS=-L${ARNOLD_PATH}/bin -lai 

HEADERS=\
	src/lens.h\
	src/pota.h\
	src/tinyexr.h

.PHONY=all clean

all: pota pota_raytraced potabokehAOV

ifeq ($(OS), Darwin)
pota: Makefile src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} src/pota.cpp -o ${user_build_folder}/bin/pota.dylib ${LDFLAGS}

pota_raytraced: Makefile src/pota_raytraced.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} src/pota_raytraced.cpp -o ${user_build_folder}/bin/pota_raytraced.dylib ${LDFLAGS}

potabokehAOV: Makefile src/potabokehAOV.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} src/potabokehAOV.cpp -o ${user_build_folder}/bin/potabokehAOV.dylib ${LDFLAGS}
endif
ifeq ($(OS), Linux)
pota: Makefile src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} src/pota.cpp -o ${user_build_folder}/bin/pota.so ${LDFLAGS}

pota_raytraced: Makefile src/pota_raytraced.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} src/pota_raytraced.cpp -o ${user_build_folder}/bin/pota_raytraced.so ${LDFLAGS}

potabokehAOV: Makefile src/potabokehAOV.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} src/potabokehAOV.cpp -o ${user_build_folder}/bin/potabokehAOV.so ${LDFLAGS}
endif


clean:
	rm -f pota pota_raytraced potabokehAOV
