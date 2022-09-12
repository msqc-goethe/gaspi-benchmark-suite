#!/bin/bash
source paths.sh
gpiDir=$GPI_DIR
exePath="$gpiDir/bin/gaspi_run"
gaspiProg=$1
runfile=$GASPI_RUN_FILE
gaspiProgFull="$(readlink -f $gaspiProg)"


if [ -f "$gaspiProgFull" ] ; then
	$exePath -N $runfile $gpiDir $gaspiProgFull "${@:2}"
else
	echo "Executable $gaspiProg not found"
fi



