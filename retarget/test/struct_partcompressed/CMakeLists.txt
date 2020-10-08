# Catch do files
configure_file( run_legacy_resim.do.in ${CMAKE_CURRENT_BINARY_DIR}/run_legacy_resim.do )
set ( VSIM_LEGACY_DOFILE "${CMAKE_CURRENT_BINARY_DIR}/run_legacy_resim.do" )

configure_file( run_partcompressed_resim.do.in ${CMAKE_CURRENT_BINARY_DIR}/run_compressed_resim.do )
set ( VSIM_COMPRESSED_DOFILE "${CMAKE_CURRENT_BINARY_DIR}/run_compressed_resim.do" )

# Build directory structure
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dump)

# Setup test environment
set ( TESTPREFIX "PDMC_" )
file(COPY ${VECTHOR_CONFIG_FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy verilog files
file(GLOB DESIGN_FILES ${DESIGN_INC_DIR_SBSC}/*.v )
file(COPY ${DESIGN_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file( GLOB FV_PATTERN ${TESTFILE_FV_PATH}/* )
foreach( FV_TESTFILE ${FV_PATTERN} )
    get_filename_component( FV_TESTFILE_BASENAME ${FV_TESTFILE} NAME )
    add_retarget_test( ${TESTPREFIX} ${FV_TESTFILE} ${FV_TESTFILE_BASENAME} ${CMAKE_CURRENT_BINARY_DIR} "" )
    add_resim_test( ${TESTPREFIX} ${FV_TESTFILE_BASENAME} ${CMAKE_CURRENT_BINARY_DIR} )
endforeach( FV_TESTFILE )

if ( NOT ${FAST_TESTING} )
    file( GLOB TDR_PATTERN ${TESTFILE_PATH}/* )
    foreach( TESTFILE ${TDR_PATTERN} )
        get_filename_component( TESTFILE_BASENAME ${TESTFILE} NAME )
        add_retarget_test( ${TESTPREFIX} ${TESTFILE} ${TESTFILE_BASENAME} ${CMAKE_CURRENT_BINARY_DIR} "" )
        add_resim_test( ${TESTPREFIX} ${TESTFILE_BASENAME} ${CMAKE_CURRENT_BINARY_DIR} )
    endforeach( TESTFILE )
    if( ${RUN_RTDR_TESTS} )
        foreach( RTDR RANGE 1 18432 )
             add_retarget_rtdr_test(  ${TESTPREFIX} ${RTDR} ${CMAKE_CURRENT_BINARY_DIR} )
             add_resim_test( ${TESTPREFIX} RTDR_${RTDR} ${CMAKE_CURRENT_BINARY_DIR} )
        endforeach( RTDR )
    endif()
else()
    foreach( TESTFILE ${FAST_TDR_PATTERN} )
        get_filename_component( TESTFILE_BASENAME ${TESTFILE} NAME )
        add_retarget_test( ${TESTPREFIX} ${TESTFILE} ${TESTFILE_BASENAME} ${CMAKE_CURRENT_BINARY_DIR} "" )
        add_resim_test( ${TESTPREFIX} ${TESTFILE_BASENAME} ${CMAKE_CURRENT_BINARY_DIR} )
    endforeach( TESTFILE )
    if( ${RUN_RTDR_TESTS} )
        foreach( RTDR ${FAST_RTDR_PATTERN} )
             add_retarget_rtdr_test(  ${TESTPREFIX} ${RTDR} ${CMAKE_CURRENT_BINARY_DIR} )
             add_resim_test( ${TESTPREFIX} RTDR_${RTDR} ${CMAKE_CURRENT_BINARY_DIR} )
        endforeach( RTDR )
    endif()
endif()
