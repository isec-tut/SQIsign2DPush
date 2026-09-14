set(REPRESENT_TEST sqisign_test_represent_integer_${SVARIANT_LOWER})
add_executable(${REPRESENT_TEST} ${KLPTX_DIR}/test/represent_integer.c)
target_link_libraries(${REPRESENT_TEST} ${LIB_KLPT_${SVARIANT_UPPER}}
    ${LIB_QUATERNION} ${LIB_PRECOMP_${SVARIANT_UPPER}} ${LIB_INTBIG}
    ${LIB_PUBLIC} ${GMP} sqisign_common_test)
target_include_directories(${REPRESENT_TEST} PRIVATE ${INC_PUBLIC} ${INC_COMMON}
    ${INC_INTBIG} ${INC_PRECOMP_${SVARIANT_UPPER}} ${INC_QUATERNION} ${INC_KLPT})
add_test(NAME ${REPRESENT_TEST} COMMAND ${REPRESENT_TEST})
