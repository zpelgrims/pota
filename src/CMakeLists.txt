set(SRC lentil.cpp lentil_loader.cpp lentil_camera_tl.cpp lentil_camera_po.cpp lentil_filter.cpp lentil_imager.cpp)
set(SHADER lentil)

add_library(${SHADER} SHARED ${SRC})

target_link_libraries(${SHADER} ai)
set_target_properties(${SHADER} PROPERTIES PREFIX "")

install(TARGETS ${SHADER} DESTINATION ${DSO_INSTALL_DIR})