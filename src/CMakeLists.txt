add_executable(tdm main.c tdm.c)
target_include_directories(tdm PRIVATE ${PETSC_INCLUDES} ${JIGSAW_DIR}/inc)
target_link_libraries(tdm ${PETSC_LIBRARIES} jigsaw)

install(TARGETS tdm DESTINATION bin)
