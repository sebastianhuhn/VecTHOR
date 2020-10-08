#!/bin/bash
if [[ $# != 3 && $# != 4 ]]
then
	echo "-- Error: Incorrect Usage" 1>&2
        echo "-- Usage: $0 <test name> <test file to be processed> <vsim call parameters> [<batch mode>]" 1>&2
	exit 1
fi

export testname="$1"
export testfile="$2"
export batchmode=""

if [[ -n $4  ]]
then
    echo "Info: Using batch mode!"
    export batchmode="true"
    cpara="-batch $3"
else
    cpara=$3
fi

echo "Info: Calling 'vsim $cpara'"
vsim $cpara

exit_code=$?
compressed=`echo $3 | grep "run_compressed_resim.do" -c `
if [ $compressed == 1 ]
then
    echo "Info: Compressed mode"
    mv tdr_dump.log dump/$1_compressed.dump
else
    echo "Info: Legacy mode"
    mv tdr_dump.log dump/$1_legacy.dump
fi

if [ $? != 0 ]
then
    echo "Error: Resimulation of '$2' failed!" 1>&2
fi

exit $exit_code

