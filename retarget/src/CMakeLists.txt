set ( SRC
        TypeDefs.h
        Config.C
        Config.h
        CompressorCore.C
        CompressorCore.h
        Compressor.C
        Compressor.h
        Decompressor.C
        Decompressor.h
        DynDecompressor.C
        DynDecompressor.h
        FormalDecompressor.C
        FormalDecompressor.h
        Emitter.C
        Emitter.h
#To be merged in branch
#        HardwareEmitter.C
#        HardwareEmitter.h
        P2SBuffer.h
        P2SBuffer.C
        Plotter.C
        Plotter.h
        Stats.C
        Stats.h
        TDRGen.C
        TDRGen.h
        TDRReader.C
        TDRReader.h
        TypeDefs.h
        Utils.C
        Utils.h
        Validator.C
        Validator.h
        VecTHOR.C
        VecTHOR.h
)
  
#For required header files for compiling bison generated files
#include_directories ( verilogparser )

#add_subdirectory ( solver )
#add_subdirectory ( utils )
#add_subdirectory ( verilogparser )

add_library ( ${CMAKE_PROJECT_NAME}_LIB STATIC ${SRC} )

add_executable ( VecTHOR VecTHOR.C  )
set ( REQ_LIB
    ${CMAKE_PROJECT_NAME}_LIB 
    libclasp
    pthread
    yaml-cpp
    ${PI_LIB}
    #${YAML_LIB}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)
set_target_properties ( VecTHOR PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin" )
target_link_libraries ( VecTHOR ${REQ_LIB} )

