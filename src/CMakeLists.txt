add_executable(tdm main.c)
target_include_directories(tdm PRIVATE ${JIGSAW_DIR}/inc)
target_link_libraries(tdm ${PETSC_LIBRARIES} jigsaw)

install(TARGETS tdm DESTINATION bin)
