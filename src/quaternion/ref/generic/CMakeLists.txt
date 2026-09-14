set(SOURCE_FILES_QUATERNION_GENERIC_REF
    algebra.c
    ideal.c
    dim4.c
    dim2.c
    sum_of_two_squares.c
    integers.c
    lattice.c
    finit.c
    printer.c
    lll.c
    lll/l2.c
    matkermod.c
)

add_library(${LIB_QUATERNION} ${SOURCE_FILES_QUATERNION_GENERIC_REF})
target_include_directories(${LIB_QUATERNION} PRIVATE common ${CMAKE_CURRENT_SOURCE_DIR} ${INC_PUBLIC} ${INC_COMMON} ${INC_INTBIG} ${INC_QUATERNION} internal_quaternion_headers)
target_compile_options(${LIB_QUATERNION} PRIVATE ${C_OPT_FLAGS})
if(NOT MSVC)
    target_link_libraries(${LIB_QUATERNION} PUBLIC m)
endif()

add_subdirectory(test)
