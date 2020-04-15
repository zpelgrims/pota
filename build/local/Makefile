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
	ARNOLD_PATH_5.3.0.2=${LENTIL_ARNOLD_SDKS}/Arnold-5.3.0.2-darwin
	ARNOLD_PATH_6.0.0.0=${LENTIL_ARNOLD_SDKS}/Arnold-6.0.0.0-darwin
endif
ifeq ($(OS), Linux)
	ARNOLD_PATH_5.3.0.2=${LENTIL_ARNOLD_SDKS}/Arnold-5.3.0.2-linux
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



all: lentil_operator\
	 lentil_filter_debug_operator\
	 lentil_debug_filter\
	 lentil_thin_lens_bokeh_driver\
	 lentil_bokeh_driver\
	 lentil\
	 lentil_thinlens
	


lentil_bokeh_driver: ../../src/lentil_bokeh_driver.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.3.0.2}/include ${LENSES} ../../src/lentil_bokeh_driver.cpp -o ${user_build_folder}/bin/5.3.0.2/lentil_bokeh_driver.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.3.0.2}/bin -lai 
	#${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ${LENSES} ../../src/lentil_bokeh_driver.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_bokeh_driver.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil_thin_lens_bokeh_driver: ../../src/lentil_thin_lens_bokeh_driver.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.3.0.2}/include ${LENSES} ../../src/lentil_thin_lens_bokeh_driver.cpp -o ${user_build_folder}/bin/5.3.0.2/lentil_thin_lens_bokeh_driver.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.3.0.2}/bin -lai 
	#${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ${LENSES} ../../src/lentil_thin_lens_bokeh_driver.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_thin_lens_bokeh_driver.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil: ../../src/lentil.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.3.0.2}/include ${LENSES} ../../src/lentil.cpp -o ${user_build_folder}/bin/5.3.0.2/lentil.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.3.0.2}/bin -lai 
	#${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ${LENSES} ../../src/lentil.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil_thinlens: ../../src/lentil_thinlens.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.3.0.2}/include ../../src/lentil_thinlens.cpp -o ${user_build_folder}/bin/5.3.0.2/lentil_thinlens.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.3.0.2}/bin -lai 
	#${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ../../src/lentil_thinlens.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_thinlens.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil_debug_filter: ../../src/lentil_debug_filter.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.3.0.2}/include ../../src/lentil_debug_filter.cpp -o ${user_build_folder}/bin/5.3.0.2/lentil_debug_filter.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.3.0.2}/bin -lai 
	#${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ../../src/lentil_debug_filter.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_debug_filter.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil_operator: ../../src/lentil_operator.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.3.0.2}/include ../../src/lentil_operator.cpp -o ${user_build_folder}/bin/5.3.0.2/lentil_operator.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.3.0.2}/bin -lai 
	#${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ../../src/lentil_operator.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_operator.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 

lentil_filter_debug_operator: ../../src/lentil_filter_debug_operator.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.3.0.2}/include ../../src/lentil_filter_debug_operator.cpp -o ${user_build_folder}/bin/5.3.0.2/lentil_filter_debug_operator.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.3.0.2}/bin -lai 
	#${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ../../src/lentil_filter_debug_operator.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_filter_debug_operator.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 


# lentil_transmitted_z: ../../src/transmitted_z.cpp ${HEADERS}
# 	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_5.3.0.2}/include ../../src/transmitted_z.cpp -o ${user_build_folder}/bin/5.3.0.2/lentil_transmitted_z.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_5.3.0.2}/bin -lai 
# 	${CXX} ${CXXFLAGS} -I${ARNOLD_PATH_6.0.0.0}/include ../../src/transmitted_z.cpp -o ${user_build_folder}/bin/6.0.0.0/lentil_transmitted_z.${SHARED_LIB_FORMAT} -L${ARNOLD_PATH_6.0.0.0}/bin -lai 


# lentil_raytraced: ../../src/lentil_raytraced.cpp ${HEADERS}
# 	${CXX} ${CXXFLAGS} ${LENSES} ../../src/lentil_raytraced.cpp -o ${user_build_folder}/bin/lentil_raytraced.${SHARED_LIB_FORMAT} ${LDFLAGS}

