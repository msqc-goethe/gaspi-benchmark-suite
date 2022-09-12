#!/bin/bash

resultDir="result"
resultMpiIn="result-mpi-txt"
resultOmbOut="result-omb"
resultImbOut="result-imb"
resultAggregate="result-aggregate"
csvParsePath="../osu-imb-csv-parser"
csvParsePathFull=$(readlink -f $csvParsePath)
csvParser="$csvParsePathFull/build/csv-parser"
uploadName="gaspi-benchmarks"
uploadUser="CHANGEME"
uploadPass="CHANGEME"

curDir=$(pwd)

if ! [ -f $csvParser ] ; then
	echo "CSV parser not found - building"
	cd $csvParsePathFull
	make
	cd $curDir
fi

if ! [ -f $csvParser ] ; then
	echo "CSV Parser not available"
	return
fi

cd $resultDir

for f in *.txt ; do
	prefix=$(basename "$f" .txt)
	$csvParser -i=$f -o=$prefix -sep=, -imb
done

# Parse Aggregate Results
cd $resultAggregate

for d in */ ; do
	cd $d
	# For every interconnect parse the files for various aggregate modes
	for f in *.txt ; do
		prefix=$(basename "$f" .txt)
		$csvParser -i=$f -o=$prefix -sep=, -imb
	done
	cd ..
done

cd ..

# Parse OMB

if [ -d $resultOmbOut ] ; then
	rm -rf $resultOmbOut
fi
mkdir -p $resultOmbOut

for f in $(find $resultMpiIn -maxdepth 1 -name "osu*.txt") ; do
        prefix=$(basename "$f" .txt)
        $csvParser -i=$f -o=${resultOmbOut}/${prefix}.csv -sep=, -osu
done


# Parse IMB

if [ -d $resultImbOut ] ; then
	rm -rf $resultImbOut
fi
mkdir -p $resultImbOut

for f in $(find $resultMpiIn -maxdepth 1 -name "imb*.txt") ; do
        prefix=$(basename "$f" .txt)
        $csvParser -i=$f -o=${resultImbOut}/${prefix} -sep=, -imb
done





zipfile="../${uploadName}.zip"

if [ -f $zipfile ] ; then
	rm -f $zipfile
fi

zip -9 -r $zipfile .
curl -T $zipfile -u ${uploadUser}:${uploadPass} https://heibox.uni-heidelberg.de/seafdav/Projektpraktikum/Messdaten/
