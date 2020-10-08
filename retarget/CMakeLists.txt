#####################################
##Do not change anything after here##
#####################################
project ( VecTHOR )
cmake_minimum_required(VERSION 2.8)

include ( CTest )
include ( CMakeConfig.txt )


if ( ${CMAKE_SYSTEM} MATCHES "x86_64" )
	set ( SYSTEM_TYPE "m64" )
else()
	set ( SYSTEM_TYPE "m32" )
endif()

#option ( USE_LOCAL_YAML "Build local YAML version" on )
option ( USE_CLASP "Build clasp [req. for formal approach]" on )
option ( USE_PI "Build hardware emitter for RaspPi" off )
option ( USE_STIL "Build stil parser infrastructure" off )
option ( RUN_TESTS "Run test file" on )
option ( FAST_TESTING "Run only fast tests" on )
option ( VSIM_BATCH "Run VSIM in batch mode" on )

find_package (Boost "1.41.0" REQUIRED COMPONENTS filesystem system iostreams regex program_options )

find_library ( YAML_CPP yaml-cpp )

if( ${USE_PI} )
	add_definitions( -DPI=true )
	set ( PI_LIB "wiringPi" )
endif()

if( ${YAML_CPP} )
    message ( WARNING "-- Could NOT find YAML" )
    add_subdirectory( lib/yaml-cpp-master )
    include_directories( lib/yaml-cpp-master/include )
endif()

if( ${USE_CLASP} )
    if( ${CLASP_INCLUDE_DIR} )
            message ( WARNING "-- Could NOT find CLASP" )
    else()
        add_subdirectory( lib/clasp-3.1.4 )
        include_directories( lib/clasp-3.1.4/ )
    endif()
endif()

if( ${USE_PI} )
# TODO
endif()


if( ${Boost_INCLUDE_DIRS} )
	message ( FATAL_ERROR "-- Could NOT find Boost" )
else()
	include_directories (${Boost_INCLUDE_DIRS})
endif()

#find_library ( LIBCLASP clasp REQUIRED )
#find_package ( BISON REQUIRED)
#find_package ( FLEX REQUIRED)

#include_directories (${Boost_INCLUDE_PATH})
include_directories ( src )

add_subdirectory ( src ) 

#Start tests
if(${RUN_TESTS})
        add_subdirectory ( test )
endif()
