find_package(Doxygen REQUIRED)

set(DOXYGEN_CONF ${PROJECT_SOURCE_DIR}/Doxyfile)

add_custom_target(doc
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONF}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Generating API documentation with Doxygen"
  VERBATIM)
