#!/bin/bash
source ../scripts/paths.sh

flags="-Wall -Wextra -pedantic -std=c99 -D_POSIX_C_SOURCE=199309L -pthread"
optLevel="-O0"
debugLevel="-g"

buildDir=$BUILD_DIR
name="$buildDir/$EXE_NAME"
includeDir=$INCLUDE_DIR
gpiDir=$GPI_DIR
gpiDirInclude="$gpiDir/src/include"
gpiDirSharedLibs="$gpiDir/src/.libs/libGPI2.so"

##################################

if ! [ -d $buildDir ] ; then
	mkdir -p build
fi

curDir=$(pwd)

# Compile Includes
cd $includeDir
echo "Compiling Include Directory"
gcc $(ls *.c) $flags $optLevel $debugLevel -c 


cd $curDir
ofiles=$(ls $includeDir/*.o)

echo "Compiling program"

# Deleting old file
if [ -f $name ] ; then	
	rm $name
fi

gcc $(ls *.c) $ofiles $gpiDirSharedLibs $flags $optLevel $debugLevel -I$includeDir -I$gpiDirInclude -o "$name"

echo "Deleting object files"
rm -f $ofiles

if [ -f $name ] ; then
	echo "Build successful"
else
	echo "Build failed"
fi

