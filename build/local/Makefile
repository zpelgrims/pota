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
	ARNOLD_PATH=${LENTIL_ARNOLD_SDKS}/Arnold-6.0.0.0-darwin
endif
ifeq ($(OS), Linux)
	ARNOLD_PATH=${LENTIL_ARNOLD_SDKS}/Arnold-5.2.2.0-linux
endif


LENSES = -DLENS_ID_FREE
LENSES += -DLENS_ID_COMMERCIAL
$(info    LENSES: $(LENSES))


CXXFLAGS=\
	-Wall\
	-std=c++17\
	-shared\
	-fPIC\
	-Wno-narrowing\
	-I${ARNOLD_PATH}/include\
	-I/../../../Eigen/Eigen\
	-I/../../../fmt/include/fmt\
	-I/../../../polynomial-optics/src\
	-DDEBUG_LOG\
	-DFMT_HEADER_ONLY\
	-O3\
	#-g

	


LDFLAGS=-L${ARNOLD_PATH}/bin -lai 

HEADERS=\
	../../src/lens.h\
	../../src/lentil.h\
	../../src/imagebokeh.h\
	../../src/tinyexr.h

.PHONY=all clean

all: lentil_thinlens_heatmapaov lentil_thin_lens_bokeh_driver lentil_bokeh_driver lentil #lentil_thinlens

lentil_bokeh_driver: Makefile ../../src/lentil_bokeh_driver.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} ../../src/lentil_bokeh_driver.cpp -o ${user_build_folder}/bin/lentil_bokeh_driver.${SHARED_LIB_FORMAT} ${LDFLAGS}

lentil_thin_lens_bokeh_driver: Makefile ../../src/lentil_thin_lens_bokeh_driver.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} ../../src/lentil_thin_lens_bokeh_driver.cpp -o ${user_build_folder}/bin/lentil_thin_lens_bokeh_driver.${SHARED_LIB_FORMAT} ${LDFLAGS}

lentil: Makefile ../../src/lentil.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ${LENSES} ../../src/lentil.cpp -o ${user_build_folder}/bin/lentil.${SHARED_LIB_FORMAT} ${LDFLAGS}

lentil_thinlens: Makefile ../../src/lentil_thinlens.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} ../../src/lentil_thinlens.cpp -o ${user_build_folder}/bin/lentil_thinlens.${SHARED_LIB_FORMAT} ${LDFLAGS}

# lentil_thinlens_heatmapaov: Makefile ../../src/lentil_thinlens_aovheatmap.cpp ${HEADERS}
# 	${CXX} ${CXXFLAGS} ../../src/lentil_thinlens_aovheatmap.cpp -o ${user_build_folder}/bin/lentil_thinlens_aovheatmap.${SHARED_LIB_FORMAT} ${LDFLAGS}

# lentil_raytraced: Makefile ../../src/lentil_raytraced.cpp ${HEADERS}
# 	${CXX} ${CXXFLAGS} ${LENSES} ../../src/lentil_raytraced.cpp -o ${user_build_folder}/bin/lentil_raytraced.${SHARED_LIB_FORMAT} ${LDFLAGS}


clean:
	rm -f lentil_thin_lens_bokeh_driver lentil_bokeh_driver lentil lentil_thinlens #lentil_thinlens_heatmapaov
