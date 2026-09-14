set(CCSD_NAME gf)
set(CCSD_NAME_UPPER GF)
if(SQISIGN_BUILD_TYPE STREQUAL "ref")
    set(GF_IMPLEMENTATION ref)
else()
    set(GF_IMPLEMENTATION fiat_crypto)
endif()

foreach(SVARIANT ${SVARIANT_S})
    string(TOUPPER ${SVARIANT} SVARIANT_UPPER)
    string(TOLOWER ${SVARIANT} SVARIANT_LOWER)
    set(LIB_GF_${SVARIANT_UPPER} sqisign_gf_${SVARIANT_LOWER} CACHE INTERNAL "GF library")
    set(INC_GF_${SVARIANT_UPPER}
        ${CMAKE_CURRENT_SOURCE_DIR}/${GF_IMPLEMENTATION}/${SVARIANT_LOWER}/include
        CACHE INTERNAL "GF include directory")
    add_subdirectory(${GF_IMPLEMENTATION}/${SVARIANT_LOWER})
endforeach()
