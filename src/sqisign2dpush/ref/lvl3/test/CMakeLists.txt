set(SOURCE_FILES_SQISIGN2DPUSH_GENERIC_REF_TESTS
    ${SQISIGN2DPUSH_LVLX_DIR}/test/doublepath.c
    ${SQISIGN2DPUSH_LVLX_DIR}/test/test_sqisign2dpush.c
)
add_executable(sqisign_test_sqisign2dpush_${SVARIANT_LOWER} ${SOURCE_FILES_SQISIGN2DPUSH_GENERIC_REF_TESTS})
target_link_libraries(sqisign_test_sqisign2dpush_${SVARIANT_LOWER} ${LIB_SQISIGN2DPUSH_${SVARIANT_UPPER}} ${LIB_HD_${SVARIANT_UPPER}} ${LIB_ID2ISO_${SVARIANT_UPPER}} ${LIB_EC} ${LIB_KLPT_${SVARIANT_UPPER}} ${LIB_QUATERNION} ${LIB_PRECOMP_${SVARIANT_UPPER}} ${LIB_EC_${SVARIANT_UPPER}} ${LIB_INTBIG} ${LIB_PUBLIC} ${LIB_GF_${SVARIANT_UPPER}} ${GMP} sqisign_common_sys )
target_include_directories(sqisign_test_sqisign2dpush_${SVARIANT_LOWER} PRIVATE ${INC_PUBLIC} ${INC_INTBIG} ${INC_PRECOMP_${SVARIANT_UPPER}} ${INC_QUATERNION} ${INC_KLPT} ${INC_GF_${SVARIANT_UPPER}} ${INC_EC} ${INC_COMMON} ${INC_ID2ISO} ${INC_SQISIGN2DPUSH} ./include/)

add_test(sqisign_test_sqisign2dpush_${SVARIANT_LOWER} sqisign_test_sqisign2dpush_${SVARIANT_LOWER})
