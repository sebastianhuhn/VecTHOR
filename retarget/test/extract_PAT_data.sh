#!/bin/bash

if [ $# != 1 ] 
then
	echo "[e] Wrong number of arguments! Usage: extract_PI_data.sh <read-in path>" 
	exit 1
fi

#CBD=${CMAKE_CURRENT_BINARY_DIR}
CBD=.

INPUT_DIR=$1/org_edt
TMP_DIR=$CBD/tmp
OUTPUT_DIR=$CBD/out

mkdir -pv $TMP_DIR
mkdir -pv $OUTPUT_DIR

echo "[i] Cleaning up directories: $TMP_DIR ; $OUTPUT_DIR"
rm -f $TMP_DIR/*
rm -f $OUTPUT_DIR/*

for f in $INPUT_DIR/*.pat
do
	TEST_FILENAME=`basename $f`
	PRE_OUT_FILE=$TMP_DIR/$TEST_FILENAME
	cat $f |  sed -n '/^SCAN_TEST/,$p' | sed -n "/[[:space:]]pattern/,/end;/p" | tr -d "\\" | tr -d "=" | tr -d "\"" | tr -d ";" | sed 's/chain[0-9]*//g' | sed 's/X/0/g' | sed 's/end/;/g' | sed 's/apply.*$//g' | sed 's/pattern.*$//g' | tr -d [:space:] | sed 's/;/\n/g' > $PRE_OUT_FILE
	#split -d -l 128 $PRE_OUT_FILE chunk_
	echo "[i] Pre-Processing file $f - written to $PRE_OUT_FILE"
done

for ff in $TMP_DIR/*
do
    CTR=0
    TEST_FILENAME=`basename $ff`
    POST_OUT_FILE=$OUTPUT_DIR/$TEST_FILENAME
    while IFS= read -ra d 
    do 
    	echo "[i] Processing file $ff - written to $POST_OUT_FILE.$CTR"
	echo "$d[$CTR]" >  $POST_OUT_FILE.$CTR	
	let "CTR++"
    done < $ff  
done
exit 0
