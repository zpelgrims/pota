OS := $(shell uname)

ifeq ($(OS), Darwin)
	CXX=clang++
endif
ifeq ($(OS), Linux)
	CXX=g++
endif

# these need to be changed to the envvar
ifeq ($(OS), Darwin)
	ARNOLD_PATH=/root/lentil-build/arnold/Arnold-5.2.0.0-darwin
endif
ifeq ($(OS), Linux)
	ARNOLD_PATH=/root/lentil-build/arnold/Arnold-5.2.0.0-linux
endif


LENSES = -DLENS_ID_FREE
# lens list is expected in following format: lens_list=.1001.2001.2002.2003 (first dot is important)
LENSES += $(subst ., -DLENS_ID_, ${lens_list})
$(info    LENSES: $(LENSES))

# is there an issue when I specify the same lens twice? once in lens_id_free and once in the user string?



CXXFLAGS=\
	-Wall\
	-std=c++17\
	-O3\
	-shared\
	-fPIC\
	-Wno-narrowing\
	-I${ARNOLD_PATH}/include\
	-I/../../Eigen/Eigen\
	-I/../../fmt/include/fmt\
	-I/../../polynomial-optics/src\
	-DDEBUG_LOG\
	-DFMT_HEADER_ONLY

LDFLAGS=-L${ARNOLD_PATH}/bin -lai 

HEADERS=\
	../src/lens.h\
	../src/pota.h

.PHONY=all clean

all: lentil


ifeq ($(OS), Darwin)
lentil: Makefile ../src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} ../src/pota.cpp -o ${user_build_folder}/bin/lentil.dylib ${LDFLAGS}
endif
ifeq ($(OS), Linux)
lentil: Makefile ../src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} ../src/pota.cpp -o ${user_build_folder}/bin/lentil.so ${LDFLAGS}
endif


clean:
	rm -f lentil
