add_executable(tdm main.c tdm.c read_yaml.c)
target_include_directories(tdm PRIVATE ${PETSC_INCLUDES} ${JIGSAW_DIR}/inc
                                       ${LIBYAML_INCLUDE_DIRS})
target_link_libraries(tdm ${PETSC_LIBRARIES} jigsaw yaml)

install(TARGETS tdm DESTINATION bin)
