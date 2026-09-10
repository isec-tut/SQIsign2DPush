
set(SOURCE_FILES_GF_${SVARIANT_UPPER}_REF
    fp_backend.c fp.c fp_exp.c fp2.c
)

add_library(${LIB_GF_${SVARIANT_UPPER}} ${SOURCE_FILES_GF_${SVARIANT_UPPER}_REF})
target_include_directories(${LIB_GF_${SVARIANT_UPPER}} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/precomp/ref/${SVARIANT_LOWER}/include
    ${PROJECT_SOURCE_DIR}/src/common/generic/include
    ${PROJECT_SOURCE_DIR}/include
)
target_compile_definitions(${LIB_GF_${SVARIANT_UPPER}} PUBLIC SQISIGN_VARIANT=${SVARIANT_LOWER})
target_compile_options(${LIB_GF_${SVARIANT_UPPER}} PRIVATE ${C_OPT_FLAGS})

add_subdirectory(test)
