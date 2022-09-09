#!/bin/bash
mfilegi="mfile-gi.txt"
mfilete="mfile-te.txt"
mfileib="mfile-ib.txt"
pathib="paths-ib.sh"
pathip="paths-ip.sh"
path="paths.sh"
resultDir="result"
resultDirOsu="$resultDir/result-mpi-txt"
resultDirAggregate="$resultDir/result-aggregate"
maxruns=1000
minruns=10
aggregates=(1 2 8 64 128 512 1000)
sizestop=$((2**24))
gridthreads=8


function getArgs {
	source $path
	echo "$BUILD_DIR/$EXE_NAME -maxruns=$maxruns -minruns=$minruns -size-stop=$sizestop -grid-threads=$gridthreads -time-combined"
}

function getMfile {
	source $path
	echo $MACHINE_FILE
}

function runAggregateBenchmarks {
	dir=$1
	extraArgs="-bench=gbs_ubench_put_solo_single_udir,gbs_ubench_put_aggregate_single_udir"
	if ! [ -d $dir ] ; then
		mkdir -p $dir
	fi
	
	for a in ${aggregates[@]} ; do
		f=$dir/agg-$a.txt
		./launch.sh $(getArgs) $extraArgs -aggregate=$a | tee $f
	done
}

if [ -d $resultDir ] ; then
	rm -rf $resultDir
fi

mkdir -p $resultDir

cp $pathip $path

cp $mfilegi $(getMfile)
./launch.sh $(getArgs) | tee $resultDir/result-gi.txt
runAggregateBenchmarks $resultDirAggregate/gi

cp $mfilete $(getMfile)
./launch.sh $(getArgs) | tee $resultDir/result-te.txt
runAggregateBenchmarks $resultDirAggregate/te

cp $mfileib $(getMfile)
./launch.sh $(getArgs) | tee $resultDir/result-ipoib.txt
runAggregateBenchmarks $resultDirAggregate/ipoib

cp $pathib $path
cp $mfilete $(getMfile)
./launch.sh $(getArgs) | tee $resultDir/result-ib.txt
runAggregateBenchmarks $resultDirAggregate/ib


bash run-mpibench.sh $resultDirOsu $sizestop eno1 gi
bash run-mpibench.sh $resultDirOsu $sizestop eno2 te
bash run-mpibench.sh $resultDirOsu $sizestop ib0 ipoib
bash run-mpibench.sh $resultDirOsu $sizestop
