set(SRC lentil.cpp lentil_loader.cpp lentil_camera.cpp lentil_filter_debug.cpp lentil_operator_debug.cpp lentil_imager_debug.cpp)
set(SHADER lentil)

add_library(${SHADER} SHARED ${SRC})

target_link_libraries(${SHADER} ai)
set_target_properties(${SHADER} PROPERTIES PREFIX "")

install(TARGETS ${SHADER} DESTINATION ${DSO_INSTALL_DIR})