#!/bin/bash

if [ $# == 0 ]
then
	srcdir=`pwd`
elif [ $# == 1 ]
then
 	srcdir=$1
fi

echo "Reformatting source code for $srcdir"
cd $srcdir
find \( -name '*.h' -or -name '*.C' \) -print0 | xargs -0 clang-format -i

exit 0
