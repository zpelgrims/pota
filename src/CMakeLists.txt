set(SRC lentil.cpp lentil_loader.cpp lentil_camera.cpp lentil_filter.cpp lentil_imager.cpp)
set(SHADER lentil)
set(UI ${CMAKE_CURRENT_SOURCE_DIR}/lentil_camera.ui)
set(MTD ${CMAKE_CURRENT_BINARY_DIR}/lentil_camera.mtd)
# set(MTDEXTRA ${CMAKE_CURRENT_SOURCE_DIR}/lentil.mtd)
set(AE ${CMAKE_CURRENT_BINARY_DIR}/lentilTemplate.py)
#set(AEXML ${CMAKE_CURRENT_BINARY_DIR}/AElentilTemplate.xml)
#set(NEXML ${CMAKE_CURRENT_BINARY_DIR}/NElentilTemplate.xml)
#set(SPDL ${CMAKE_CURRENT_BINARY_DIR}/lentil_camera.spdl)
set(KARGS ${CMAKE_CURRENT_BINARY_DIR}/lentil_camera.args)
set(C4DRES ${CMAKE_CURRENT_BINARY_DIR}/C4DtoA)
set(PRESET_SCENES ${CMAKE_SOURCE_DIR}/preset_scenes)


add_library(${SHADER} SHARED ${SRC})

target_link_libraries(${SHADER} ai)
set_target_properties(${SHADER} PROPERTIES PREFIX "")

add_custom_command(OUTPUT ${MTD} COMMAND python3 ARGS ${CMAKE_SOURCE_DIR}/src/uigen.py ${UI} ${MTD} ${AE} ${KARGS} ${CMAKE_CURRENT_BINARY_DIR} ${HTML} DEPENDS ${UI})
add_custom_target(${SHADER}UI ALL DEPENDS ${MTD})

install(TARGETS ${SHADER} DESTINATION ${DSO_INSTALL_DIR})
# install(FILES ${MTD} DESTINATION ${MTD_INSTALL_DIR})
# install(FILES ${MTDEXTRA} DESTINATION ${MTD_INSTALL_DIR})
install(FILES ${AE} DESTINATION ${AE_INSTALL_DIR})
#install(FILES ${AEXML} DESTINATION ${AEXML_INSTALL_DIR})
#install(FILES ${NEXML} DESTINATION ${NEXML_INSTALL_DIR})
#install(FILES ${SPDL} DESTINATION ${SPDL_INSTALL_DIR})
install(FILES ${KARGS} DESTINATION ${ARGS_INSTALL_DIR})
install(DIRECTORY ${C4DRES} DESTINATION ${INSTALL_DIR})
install(DIRECTORY ${PRESET_SCENES} DESTINATION ${SCENES_INSTALL_DIR})







# concat mtd files
set(MTD_FILES ${CMAKE_CURRENT_BINARY_DIR}/lentil_camera.mtd ${CMAKE_CURRENT_SOURCE_DIR}/lentil_extra.mtd)

function(cat IN_FILE OUT_FILE)
  file(READ ${IN_FILE} CONTENTS)
  file(APPEND ${OUT_FILE} "${CONTENTS}")
endfunction()

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/lentil.mtd "")
foreach(MTD_FILE ${MTD_FILES})
  cat(${MTD_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/lentil.mtd)
endforeach()

set(MTD_LENTIL ${CMAKE_CURRENT_SOURCE_DIR}/lentil.mtd)
install(FILES ${MTD_LENTIL} DESTINATION ${MTD_INSTALL_DIR})