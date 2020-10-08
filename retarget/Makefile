RMDIR_CMD			=		`which rm` -fr
MKDIR_CMD			=		`which mkdir` -p
ECHO_CMD			=		`which echo`

CTEST_CMD			=		`which ctest`

CLANG++_CMD			= 		`which clang++`
LLVM_LINKER			= 		`which llvm-link`
LLVM_NM				= 		`which llvm-nm`		
LLVM_OBJDUMP                    = 		`which llvm-objdump`
LLVM_RANLIB			= 		`which llvm-ranlib`
RUNLIM_CMD			= 		`which runlim`

ifndef ROOT_DIR
        ROOT_DIR=$(PWD)
endif

ifndef BUILD_DIR
        BUILD_DIR=/build-`hostname -s`
endif


ifdef BUILD
	CMAKE_OPTIONS+=-DCMAKE_BUILD_TYPE:STRING=${BUILD}
else
        CMAKE_OPTIONS+=-DCMAKE_BUILD_TYPE=RELWITHDEBINFO
endif

ifdef TEST
	CMAKE_OPTIONS+=-DTEST:BOOL=ON
endif

ifdef ADV_TESTING
	CMAKE_OPTIONS+=-DTEST:BOOL=ON -DFAST_TESTING:BOOL=OFF
endif

ifdef CLANG
	CMAKE_OPTIONS+=-DCMAKE_CXX_COMPILER=${CLANG++_CMD} 

endif

ifdef LLVM
	CMAKE_OPTIONS+=--DCMAKE_LINKER=${LLVM_LINKER} -DCMAKE_NM=${LLVM_NM} \
				   -DCMAKE_OBJDUMP=${LLVM_OBJDUMP} -DCMAKE_RANLIB=${LLVM_RANLIB} \
				   --no-warn-unused-cli
endif

ifdef RUNLIM
	RUNLIM_CALL=${RUNLIM_CMD} -s ${RUNLIM}
endif

.PHONY: default all clean test install report test test_summary

default: all

install:
		@( \
		if [ ! -d $(ROOT_DIR)/$(BUILD_DIR) ] ; then \
			$(MAKE) prepare ; \
		fi ; \
		)
		$(MAKE) all -C $(ROOT_DIR)/$(BUILD_DIR)

install/strip_%: $(ROOT_DIR)/platforms/%
		@( \
		if [ ! -d $(ROOT_DIR)/platforms/$* ] ; then \
			$(MAKE) prepare ; \
		fi ; \
		)

clean:  
		@$(ECHO_CMD) "** cleaning $(ROOT_DIR)/$(BUILD_DIR)"
		@$(RMDIR_CMD) $(ROOT_DIR)/$(BUILD_DIR)


prepare:
		$(MAKE) report
		@( \
		if [ ! -d $(ROOT_DIR)/$(BUILD_DIR) ] ; then \
			$(ECHO_CMD) "** Preparing $(BUILD_DIR) tree"; \
			$(MKDIR_CMD) $(ROOT_DIR)/$(BUILD_DIR); \
		else \
			$(ECHO_CMD) "** Updating $(BUILD_DIR) tree"; \
		fi; \
		if [ "${VERBOSE}" = "1" ] ; then \
			$(ECHO_CMD) "** using CMAKE_OPTIONS=${CMAKE_OPTIONS}"; \
		fi ; \
		cd $(ROOT_DIR)/$(BUILD_DIR); \
		cmake ${CMAKE_OPTIONS} $(ROOT_DIR); \
		)

all: 	
		@( \
		if [ ! -d $(ROOT_DIR)/$(BUILD_DIR) ] ; then \
			$(MAKE) prepare ; \
		fi ; \
		)
		$(MAKE) install
report:
	@$(ECHO_CMD) "root directory : 		$(ROOT_DIR)"
	@$(ECHO_CMD) "build directory: 		$(BUILD_DIR)"
	@$(ECHO_CMD) "rmdir call :			$(RMDIR_CMD)"
	@$(ECHO_CMD) "mkdir call:			$(MKDIR_CMD)"
	@$(ECHO_CMD) "echo call:			$(ECHO_CMD)"
	@$(ECHO_CMD) "ctest call:			$(CTEST_CMD)"
	@$(ECHO_CMD) "clang++ exec.:			$(CLANG++_CMD)"
	@$(ECHO_CMD) "llvm_link exec.:		$(LLVM_LINKER)"
	@$(ECHO_CMD) "llvm-nm exec.:			$(LLVM_NM)"
	@$(ECHO_CMD) "llvm-objdump exec.:		$(LLVM_OBJDUMP)"
	@$(ECHO_CMD) "llvm-ranlib exec.:		$(LLVM_RANLIB)"
	@$(ECHO_CMD) "runlim:				$(RUNLIM_CMD)"
test: KEEP_GOING=1
test:
	@${ECHO_CMD} "***************************************************"
	@${ECHO_CMD} "** test summary"	
	@( \
		if [ ! -d $(ROOT_DIR)/$(BUILD_DIR) ] ; then \
			$(MAKE) prepare ; \
		fi ; \
		if [ -f $(ROOT_DIR)/$(BUILD_DIR)/Testing/Temporary/LastTestsFailed.log ] ; then \
			/bin/rm -f $(ROOT_DIR)/$(BUILD_DIR)/Testing/Temporary/LastTestsFailed.log ; \
		fi ; \
		cd $(ROOT_DIR)/$(BUILD_DIR) ; \
		$(RUNLIM_CALL) $(CTEST_CMD) ; \
		ctest_result=$$? ; \
		if [ $$ctest_result -ne 0 ] && [ "x$(KEEP_GOING)" != "x1" ] ; then \
			exit $(ctest_result) ; \
		fi  )
		
test_summary: @total_test_failed_errors=`cat $(ROOT_DIR)/$(BUILD_DIR)/Testing/Temporary/LastTestsFailed.log | wc -l 2>/dev/null`
			  total_test_failed_errors?=0
test_summary:
	@${ECHO_CMD} "***************************************************"
	@${ECHO_CMD} **Test summary: $(total_test_failed_errors) failed tests
	@${ECHO_CMD} "***************************************************"
	@exit $(total_test_failed_errors)
	 
help:
	@$(ECHO_CMD) " targets:"
	@$(ECHO_CMD) "	- clean    		 clean"
	@$(ECHO_CMD) "	- all    		 build all"
	@$(ECHO_CMD) "	- test   	  	 build and launch the tests"
	@$(ECHO_CMD) "	- test_summary   prints the test summary"
	@$(ECHO_CMD) "	- install    	 build and install"
	@$(ECHO_CMD) ""
	@$(ECHO_CMD) " options:"
	@$(ECHO_CMD) "	- BUILD			 type of build: Debug, Release,..."
	@$(ECHO_CMD) "	- TEST			 enable unit tests if defined."
	#@$(ECHO_CMD) "	- BENCHMARK		 enable benchmark tests if defined."
	@$(ECHO_CMD) ""

