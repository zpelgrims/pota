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
	ARNOLD_PATH_6.1.0.0=${LENTIL_ARNOLD_SDKS}/Arnold-6.1.0.0-darwin
endif
ifeq ($(OS), Linux)
	ARNOLD_PATH_6.1.0.0=${LENTIL_ARNOLD_SDKS}/Arnold-6.1.0.0-linux
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
	-static-libstdc++\
	-static-libgcc\
	-O3
	

HEADERS=\
	../../src/lens.h\
	../../src/lentil.h\
	../../src/imagebokeh.h



all: lentil_operator\
	 lentil_debug_operator\
	 lentil_debug_filter\
	 lentil_thin_lens_bokeh_filter\
	 lentil\
	 lentil_thinlens\
	 lentil_imager\
	 lentil_bokeh_filter
	#  lentil_glare_imager
	


lentil_bokeh_filter: ../../src/lentil_bokeh_filter.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.1.0.0}/include ${LENSES} ../../src/lentil_bokeh_filter.cpp -o ${user_build_folder}/bin/6.1.0.0/lentil_bokeh_filter.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.1.0.0}/bin -lai 

lentil_thin_lens_bokeh_filter: ../../src/lentil_thin_lens_bokeh_filter.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.1.0.0}/include ${LENSES} ../../src/lentil_thin_lens_bokeh_filter.cpp -o ${user_build_folder}/bin/6.1.0.0/lentil_thin_lens_bokeh_filter.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.1.0.0}/bin -lai 

lentil: ../../src/lentil.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.1.0.0}/include ${LENSES} ../../src/lentil.cpp -o ${user_build_folder}/bin/6.1.0.0/lentil.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.1.0.0}/bin -lai 

lentil_thinlens: ../../src/lentil_thinlens.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.1.0.0}/include ../../src/lentil_thinlens.cpp -o ${user_build_folder}/bin/6.1.0.0/lentil_thinlens.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.1.0.0}/bin -lai 

lentil_debug_filter: ../../src/lentil_debug_filter.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.1.0.0}/include ../../src/lentil_debug_filter.cpp -o ${user_build_folder}/bin/6.1.0.0/lentil_debug_filter.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.1.0.0}/bin -lai 

lentil_operator: ../../src/lentil_operator.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.1.0.0}/include ../../src/lentil_operator.cpp -o ${user_build_folder}/bin/6.1.0.0/lentil_operator.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.1.0.0}/bin -lai 

lentil_debug_operator: ../../src/lentil_debug_operator.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.1.0.0}/include ../../src/lentil_debug_operator.cpp -o ${user_build_folder}/bin/6.1.0.0/lentil_debug_operator.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.1.0.0}/bin -lai 

lentil_imager: ../../src/lentil_imager.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.1.0.0}/include ${LENSES} ../../src/lentil_imager.cpp -o ${user_build_folder}/bin/6.1.0.0/lentil_imager.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.1.0.0}/bin -lai 

# lentil_glare_imager: ../../src/lentil_glare_imager.cpp ${HEADERS}
# 	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.1.0.0}/include ${LENSES} ../../src/lentil_glare_imager.cpp -o ${user_build_folder}/bin/6.1.0.0/lentil_glare_imager.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.1.0.0}/bin -lai 


# lentil_transmitted_z: ../../src/transmitted_z.cpp ${HEADERS}
# 	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.1.0.0}/include ../../src/transmitted_z.cpp -o ${user_build_folder}/bin/6.1.0.0/lentil_transmitted_z.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.1.0.0}/bin -lai 
# 	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ../../src/transmitted_z.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_transmitted_z.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 


# lentil_raytraced: ../../src/lentil_raytraced.cpp ${HEADERS}
# 	${CXX} ${CXXFLAGS} ${LENSES} ../../src/lentil_raytraced.cpp -o ${user_build_folder}/bin/lentil_raytraced.${SHARED_LIB_FORMAT} ${LDFLAGS}

