#!/bin/bash
source ../scripts/paths.sh

buildDir="build"
name="$buildDir/csv-parser"
flags="-Wall -Wextra -pedantic -ansi"
includeDir="$INCLUDE_DIR"
optLevel="-O0"
debugLevel="-g"
if ! [ -d $buildDir ] ; then
	mkdir -p build
fi

curDir=$(pwd)

# Compile Includes
cd $includeDir
echo "Compiling Include Directory"
gcc "utility.c" "arraylist.c" $flags $optLevel $debugLevel -c 


cd $curDir
ofiles=$(ls $includeDir/*.o)

echo "Compiling program"

# Deleting old file
if [ -f $name ] ; then	
	rm $name
fi

gcc $(ls *.c) $ofiles $flags $optLevel $debugLevel -I$includeDir -o "$name"

echo "Deleting object files"
rm -f $ofiles

if [ -f $name ] ; then
	echo "Build successful"
else
	echo "Build failed"
fi

