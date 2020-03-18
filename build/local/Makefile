OS := $(shell uname)

ifeq ($(OS), Darwin)
	CXX=clang++
	SHARED_LIB_FORMAT=dylib
endif
ifeq ($(OS), Linux)
	CXX=g++
	SHARED_LIB_FORMAT=so
endif


ifeq ($(OS), Darwin)
	ARNOLD_PATH_5.2.0.0=${LENTIL_ARNOLD_SDKS}/Arnold-5.2.2.1-darwin
	ARNOLD_PATH_6.0.0.0=${LENTIL_ARNOLD_SDKS}/Arnold-6.0.0.0-darwin
endif
ifeq ($(OS), Linux)
	ARNOLD_PATH_5.2.0.0=${LENTIL_ARNOLD_SDKS}/Arnold-5.2.0.0-linux
	ARNOLD_PATH_6.0.0.0=${LENTIL_ARNOLD_SDKS}/Arnold-6.0.0.0-linux
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
	-I/../../../Eigen/Eigen\
	-I/../../../fmt/include/fmt\
	-I/../../../polynomial-optics/src\
	-DDEBUG_LOG\
	-DFMT_HEADER_ONLY\
	-O3
	# -g\
	
	

	



HEADERS=\
	../../src/lens.h\
	../../src/lentil.h\
	../../src/imagebokeh.h\
	../../src/tinyexr.h

.PHONY=all clean

all: lentil_debug_filter_5.2.0.0\
	 lentil_thin_lens_bokeh_driver_5.2.0.0\
	 lentil_bokeh_driver_5.2.0.0\
	 lentil_5.2.0.0\
	 lentil_thinlens_5.2.0.0\
	 lentil_transmitted_z_5.2.0.0\
	 lentil_debug_filter_6.0.0.0\
	 lentil_thin_lens_bokeh_driver_6.0.0.0\
	 lentil_bokeh_driver_6.0.0.0\
	 lentil_6.0.0.0\
	 lentil_thinlens_6.0.0.0\
	 lentil_transmitted_z_6.0.0.0


lentil_bokeh_driver_5.2.0.0: Makefile ../../src/lentil_bokeh_driver.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.2.0.0}/include ${LENSES} ../../src/lentil_bokeh_driver.cpp -o ${user_build_folder}/bin/5.2.0.0/lentil_bokeh_driver.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.2.0.0}/bin -lai 

lentil_thin_lens_bokeh_driver_5.2.0.0: Makefile ../../src/lentil_thin_lens_bokeh_driver.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.2.0.0}/include ${LENSES} ../../src/lentil_thin_lens_bokeh_driver.cpp -o ${user_build_folder}/bin/5.2.0.0/lentil_thin_lens_bokeh_driver.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.2.0.0}/bin -lai 

lentil_5.2.0.0: Makefile ../../src/lentil.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.2.0.0}/include ${LENSES} ../../src/lentil.cpp -o ${user_build_folder}/bin/5.2.0.0/lentil.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.2.0.0}/bin -lai 

lentil_thinlens_5.2.0.0: Makefile ../../src/lentil_thinlens.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.2.0.0}/include ../../src/lentil_thinlens.cpp -o ${user_build_folder}/bin/5.2.0.0/lentil_thinlens.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.2.0.0}/bin -lai 

lentil_debug_filter_5.2.0.0: Makefile ../../src/lentil_debug_filter.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.2.0.0}/include ../../src/lentil_debug_filter.cpp -o ${user_build_folder}/bin/5.2.0.0/lentil_debug_filter.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.2.0.0}/bin -lai 

lentil_transmitted_z_5.2.0.0: Makefile ../../src/transmitted_z.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.2.0.0}/include ../../src/transmitted_z.cpp -o ${user_build_folder}/bin/5.2.0.0/lentil_transmitted_z.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.2.0.0}/bin -lai 


lentil_bokeh_driver_6.0.0.0: Makefile ../../src/lentil_bokeh_driver.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ${LENSES} ../../src/lentil_bokeh_driver.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_bokeh_driver.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil_thin_lens_bokeh_driver_6.0.0.0: Makefile ../../src/lentil_thin_lens_bokeh_driver.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ${LENSES} ../../src/lentil_thin_lens_bokeh_driver.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_thin_lens_bokeh_driver.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil_6.0.0.0: Makefile ../../src/lentil.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ${LENSES} ../../src/lentil.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil_thinlens_6.0.0.0: Makefile ../../src/lentil_thinlens.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ../../src/lentil_thinlens.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_thinlens.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil_debug_filter_6.0.0.0: Makefile ../../src/lentil_debug_filter.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ../../src/lentil_debug_filter.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_debug_filter.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil_transmitted_z_6.0.0.0: Makefile ../../src/transmitted_z.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ../../src/transmitted_z.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_transmitted_z.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

# lentil_raytraced: Makefile ../../src/lentil_raytraced.cpp ${HEADERS}
# 	${CXX} ${CXXFLAGS} ${LENSES} ../../src/lentil_raytraced.cpp -o ${user_build_folder}/bin/lentil_raytraced.${SHARED_LIB_FORMAT} ${LDFLAGS}

