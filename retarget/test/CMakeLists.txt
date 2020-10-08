configure_file( extract_PI_data.sh.in ${CMAKE_CURRENT_BINARY_DIR}/extract_PI_data.sh )
configure_file( eval_simulation_result.sh.in ${CMAKE_CURRENT_BINARY_DIR}/eval_simulation_result.sh )
configure_file( get_binary_data_FV.sh.in ${CMAKE_CURRENT_BINARY_DIR}/get_binary_data_FV.sh )


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vsim_wrapper.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set ( VSIM_BIN ${CMAKE_CURRENT_BINARY_DIR}/vsim_wrapper.sh )
set ( EVAL_BIN ${CMAKE_CURRENT_BINARY_DIR}/eval_simulation_result.sh ) 

# Add test generating the extracted pattern
add_test ( NAME GENERATE_TDR COMMAND extract_PI_data.sh ${CMAKE_CURRENT_SOURCE_DIR} )
add_test ( NAME GENERATE_FV COMMAND get_binary_data_FV.sh ${FV_PATH} )
set ( TESTFILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SUB_DIR} )
set ( TESTFILE_FV_PATH ${CMAKE_CURRENT_BINARY_DIR}/${TEST_FV_SUB_DIR} )

# Use batch mode
if(${VSIM_BATCH})
       set ( VSIM_BATCH_PARAM "-batch" )
endif()

option ( RUN_VALIDATION "Run validation for retargetting" on)
option ( SC_TESTING_ENABLE  "Run tests for SC" off )
option ( SCM_TESTING_ENABLE "Run tests for SCM" off )
option ( SDCM_TESTING_ENABLE "Run tests for SDCM" off )
option ( SBDCM_TESTING_ENABLE "Run tests for SBDCM" off )
option ( SSC_TESTING_ENABLE "Run tests for SSC" off )
option ( SSCPP_TESTING_ENABLE "Run tests for SSCPP" off )
option ( SPC_TESTING_ENABLE "Run tests for SPC" on )

option ( RUN_RTDR_TESTS "Run RTDR tests" on )

# TODO: Implement test fixture
function( add_testclass_fixture TESTCLASS TESTDIR )
    message( STATUS "Adding fixture for class \"${TESTCLASS}\"" )
endfunction( add_testclass_fixture )

# Add RETARGETING random test
function( add_retarget_rtdr_test PREFIX RTDR TESTDIR )
    #get_filename_component ( TESTFILE_WE ${TESTFILE} NAME_WE )
    set ( TESTNAME ${PREFIX}RETARGET_RTDR_TEST_${RTDR} )
    message (STATUS "Adding Retargetting test for \"${TESTNAME}\"" )
    set ( RESIM_LEGACY_FILE ${TEST_RESULT_DIR}/RTDR_${RTDR}_legacy.out  )
    set ( RESIM_COMPRESSED_FILE ${TEST_RESULT_DIR}/RTDR_${RTDR}_compressed.out  )
    set ( RESIM_GOLDEN_FILE ${TEST_RESULT_DIR}/RTDR_${RTDR}.golden )
    set ( VECTHOR_PARAMETERS --CompressedJTAG ${RESIM_COMPRESSED_FILE} --LegacyJTAG ${RESIM_LEGACY_FILE} --NumRTDR ${RTDR} --WriteGolden ${RESIM_GOLDEN_FILE} --ConfigFile ${VECTHOR_CONFIG_FILE} )
    add_test ( NAME ${TESTNAME} COMMAND $<TARGET_FILE:VecTHOR> ${VECTHOR_PARAMETERS} )
    set_tests_properties( ${TESTNAME} PROPERTIES TIMEOUT 86400 )
endfunction( add_retarget_rtdr_test )

# Add RETARGETING test
function( add_retarget_test PREFIX TESTFILE TESTFILE_BASENAME TESTDIR TESTPARAM )
    #get_filename_component ( TESTFILE_WE ${TESTFILE} NAME_WE )
    set ( TESTNAME ${PREFIX}RETARGET_TEST_${TESTFILE_BASENAME} )
    message (STATUS "Adding Retargetting test for \"${TESTNAME}\"" )
    set ( RESIM_LEGACY_FILE ${TESTDIR}/${TEST_RESULT_DIR}/${TESTFILE_BASENAME}_legacy.out )
    set ( RESIM_COMPRESSED_FILE ${TESTDIR}/${TEST_RESULT_DIR}/${TESTFILE_BASENAME}_compressed.out )
    set ( RESIM_GOLDEN_FILE ${TESTDIR}/${TEST_RESULT_DIR}/${TESTFILE_BASENAME}.golden )
    set ( VECTHOR_PARAMETERS --Stats --CompressedJTAG ${RESIM_COMPRESSED_FILE} --LegacyJTAG ${RESIM_LEGACY_FILE} --ReadTDR ${TESTFILE} --WriteGolden ${RESIM_GOLDEN_FILE} --ConfigFile ${VECTHOR_CONFIG_FILE} ${TESTPARAM} )
    add_test ( NAME ${TESTNAME} COMMAND $<TARGET_FILE:VecTHOR> ${VECTHOR_PARAMETERS} )
    set_tests_properties( ${TESTNAME} PROPERTIES TIMEOUT 86400 )
endfunction( add_retarget_test )

# Add RESIM test
function( add_resim_test PREFIX TESTFILE_BASENAME TESTDIR )
    if(${RUN_VALIDATION})
        message (STATUS "Adding Resimulation test for \"${TESTPREFIX}${TESTFILE_BASENAME}\"" )
        set ( RESIM_LEGACY_FILE ${TESTDIR}/${TEST_RESULT_DIR}/${TESTFILE_BASENAME}_legacy.out )
        set ( RESIM_LEGACY_DUMP ${TESTDIR}/${TEST_DUMP_DIR}/${TESTFILE_BASENAME}_legacy.dump )
        set ( RESIM_COMPRESSED_FILE ${TESTDIR}/${TEST_RESULT_DIR}/${TESTFILE_BASENAME}_compressed.out )
        set ( RESIM_COMPRESSED_DUMP ${TESTDIR}/${TEST_DUMP_DIR}/${TESTFILE_BASENAME}_compressed.dump )
        set ( RESIM_GOLDEN_FILE ${TESTDIR}/${TEST_RESULT_DIR}/${TESTFILE_BASENAME}.golden )
        set ( RESIM_LEGACY_PARAMETERS "-64 -do ${VSIM_LEGACY_DOFILE}" )
        set ( RESIM_COMPRESSED_PARAMETERS "-64 -do ${VSIM_COMPRESSED_DOFILE}" )

        set ( LEGACY_TESTNAME ${PREFIX}RESIM_LEGACY_TEST_${TESTFILE_BASENAME} )
        add_test ( NAME ${LEGACY_TESTNAME} COMMAND ${VSIM_BIN} ${TESTFILE_BASENAME} ${RESIM_LEGACY_FILE} ${RESIM_LEGACY_PARAMETERS} ${VSIM_BATCH_PARAM} )
        set_property( TEST ${LEGACY_TESTNAME} PROPERTY FAIL_REGULAR_EXPRESSION "${failRegex}" )

        set ( COMPRESSED_TESTNAME ${PREFIX}RESIM_COMPRESSED_TEST_${TESTFILE_BASENAME} )
        add_test ( NAME ${COMPRESSED_TESTNAME}  COMMAND ${VSIM_BIN} ${TESTFILE_BASENAME} ${RESIM_COMPRESSED_FILE} ${RESIM_COMPRESSED_PARAMETERS} ${VSIM_BATCH_PARAM} )
        set_property( TEST ${COMPRESSED_TESTNAME} PROPERTY FAIL_REGULAR_EXPRESSION "${failRegex}" )

        set ( EVAL_TESTNAME ${PREFIX}RESIM_EVAL_${TESTFILE_BASENAME} )
        add_test ( NAME ${EVAL_TESTNAME} COMMAND ${EVAL_BIN} ${RESIM_GOLDEN_FILE} ${RESIM_LEGACY_DUMP} ${RESIM_COMPRESSED_DUMP} )
        set_property( TEST ${EVAL_TESTNAME} PROPERTY DEPENDS ${LEGACY_TESTNAME} ${COMPRESSED_TESTNAME} )
    endif()

endfunction( add_resim_test )

if(${SC_TESTING_ENABLE})
    add_subdirectory ( struct_compressed )
endif()

if(${SCM_TESTING_ENABLE})
    add_subdirectory ( struct_mergecompressed )
endif()

if(${SDCM_TESTING_ENABLE})
    add_subdirectory ( struct_dynmergecompressed )
endif()

if(${SBDCM_TESTING_ENABLE})
    add_subdirectory ( struct_bufferdynmergecompressed )
endif()

if(${SSC_TESTING_ENABLE})
    add_subdirectory ( struct_satcompressed )
endif()

if(${SSCPP_TESTING_ENABLE})
    add_subdirectory ( struct_satppcompressed )
endif()

if(${SPC_TESTING_ENABLE})
    add_subdirectory ( struct_partcompressed )
endif()


