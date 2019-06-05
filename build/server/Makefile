ifeq (${os}, Darwin)
	CXX=clang++
endif
ifeq (${os}, Linux)
	CXX=g++
endif
ifeq (${os}, Windows)
	CXX=x86_64-w64-mingw32-g++
endif

# these need to be changed to the envvar
ifeq (${os}, Darwin)
	ARNOLD_PATH=/Users/xcloud/lentil-build/arnold/Arnold-5.2.0.0-darwin
endif
ifeq (${os}, Linux)
	ARNOLD_PATH=/root/lentil-build/arnold/Arnold-5.2.0.0-linux
endif
ifeq (${os}, Windows)
	ARNOLD_PATH=/root/lentil-build/arnold/Arnold-5.2.0.0-windows
endif


LENSES = -DLENS_ID_FREE
# lens list is expected in following format: lens_list=.1001.2001.2002.2003 (first dot is important)
LENSES += $(subst ., -DLENS_ID_, ${lens_list})
$(info    LENSES: $(LENSES))

# is there an issue when I specify the same lens twice? once in lens_id_free and once in the user string?



CXXFLAGS=\
	-Wall\
	-std=c++17\
	-shared\
	-O3\
	-fPIC\
	-Wno-narrowing\
	-I${ARNOLD_PATH}/include\
	-I/../../../Eigen/Eigen\
	-I/../../../fmt/include/fmt\
	-I/../../../polynomial-optics/src\
	-DDEBUG_LOG\
	-DFMT_HEADER_ONLY


ifeq (${os}, Darwin)
	LDFLAGS=-L${ARNOLD_PATH}/bin -lai
endif
ifeq (${os}, Linux)
	LDFLAGS=-L${ARNOLD_PATH}/bin -lai
endif
ifeq (${os}, Windows)
	LDFLAGS=-L${ARNOLD_PATH}/lib -lai
endif


HEADERS=\
	../../src/lens.h\
	../../src/pota.h

.PHONY=all clean

all: lentil


ifeq (${os}, Darwin)
lentil: Makefile ../../src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} ../../src/pota.cpp -o ${user_build_folder}/bin/lentil.dylib ${LDFLAGS}
endif
ifeq (${os}, Linux)
lentil: Makefile ../../src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} ../../src/pota.cpp -o ${user_build_folder}/bin/lentil.so ${LDFLAGS}
endif
ifeq (${os}, Windows)
lentil: Makefile ../../src/pota.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -static -static-libgcc -static-libstdc++ ${LENSES} ../../src/pota.cpp -o ${user_build_folder}/bin/lentil.dll ${LDFLAGS}
endif


clean:
	rm -f lentil
