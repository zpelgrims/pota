set(SRC lentil.cpp lentil_loader.cpp lentil_camera.cpp lentil_filter.cpp lentil_operator.cpp lentil_imager.cpp)
# set(SRC lentil.cpp lentil_loader.cpp lentil_camera.cpp lentil_filter.cpp)
set(SHADER lentil)

add_library(${SHADER} SHARED ${SRC})

target_link_libraries(${SHADER} ai)
set_target_properties(${SHADER} PROPERTIES PREFIX "")

install(TARGETS ${SHADER} DESTINATION ${DSO_INSTALL_DIR})