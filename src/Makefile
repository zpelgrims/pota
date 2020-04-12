CXXFLAGS=\
        -Wall\
        -std=c++17\
        -shared\
        -fPIC\
        -Wno-narrowing\
        -I/home/cactus/lentil/Eigen/Eigen\
        -I/home/cactus/lentil/polynomial-optics/src\
        -DDEBUG_LOG\
        -DFMT_HEADER_ONLY\
        -O3
	

# arnoldplugin : cuda_dummy_driver.cpp link.o
# 	g++ ${CXXFLAGS} -c -fPIC -D_LINUX -I/home/cactus/Arnold-5.3.0.2-linux/include cuda_dummy_driver.cpp -o dummy_driver_arnold.o
# 	g++ -shared  -L/usr/local/cuda/lib64 -lcudadevrt -lcudart dummy_driver_arnold.o link.o -lai -L /home/cactus/Arnold-5.3.0.2-linux/bin -o cuda_dummy_driver_arnold.so

# # cuda_dummy_driver_lib.so : link.o
# # 	g++  -shared -Wl,-soname,cuda_dummy_driver_lib.so  cuda_dummy_driver.o  link.o  -L/usr/local/cuda/lib64 -lcudart -o cuda_dummy_driver_lib.so

# link.o : cuda_dummy_driver.cu cuda_dummy_driver.h
# 	nvcc -Xcompiler '-fPIC' -dc  cuda_dummy_driver.cu
# 	nvcc -Xcompiler '-fPIC' -dlink  cuda_dummy_driver.o -o link.o

# clean :
# 	rm -f arnoldplugin cuda_dummy_driver.o  link.o cuda_dummy_driver_lib.so main.o


# g++ -std=c++11 -c -Wall -fPIC -fpermissive -D_LINUX -I $ARNOLD_PATH/include \
# quantize.cpp
# g++ -lai -shared -Wall -fpermissive -L $ARNOLD_PATH/bin -o "quantize.so" \
# quantize.o

# nvcc -Xcompiler '-fPIC' -dc cuda_dummy_driver.cu
# nvcc -Xcompiler '-fPIC' -dlink cuda_dummy_driver.o -o link.o
# g++ -shared -o cuda_dummy_driver.so cuda_dummy_driver.o link.o -L/usr/local/cuda/lib64 -lcudart



# // this is how you'd normally compile an arnold shader, first to object, then to shared library

# g++ ${CXXFLAGS} -c -fPIC -D_LINUX -I$ARNOLD_PATH/include cuda_dummy_driver.cpp -o cuda_dummy_driver_arnold.o
# g++ -o cuda_dummy_driver.so -shared cuda_dummy_driver_arnold.o -L$ARNOLD_PATH/bin -lai




arnoldplugin : cuda_dummy_driver.cpp cuda_dummy_driver.o
	g++ ${CXXFLAGS} -c -fPIC -D_LINUX -I/home/cactus/Arnold-5.3.0.2-linux/include cuda_dummy_driver.cpp -o dummy_driver_arnold.o
	
	# seems like i need to link using nvcc instead of g++
	# g++ -shared  -fPIC -L/usr/local/cuda/lib64 -lcudadevrt -lcudart dummy_driver_arnold.o cuda_dummy_driver.o -lai -L /home/cactus/Arnold-5.3.0.2-linux/bin -o cuda_dummy_driver_arnold.so
	nvcc -Xcompiler '-fPIC' -shared  -L/usr/local/cuda/lib64 -lcudadevrt -lcudart dummy_driver_arnold.o cuda_dummy_driver.o -lai -L /home/cactus/Arnold-5.3.0.2-linux/bin -o cuda_dummy_driver_arnold.so


cuda_dummy_driver.o : cuda_dummy_driver.cu cuda_dummy_driver.h
	nvcc -Xcompiler '-fPIC' -dc  cuda_dummy_driver.cu -o cuda_dummy_driver.o
