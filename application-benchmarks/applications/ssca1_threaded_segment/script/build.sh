
source paths.sh
flags="-pipe -lm -ldl -Wall -frename-registers  -std=c99 -D_POSIX_C_SOURCE=199309L -fopenmp"
optLevel="-O3"
debugLevel="-g"
buildDir="${HERE}/build"
gpiDir="$2"
if [ -z $2 ] ; then
	gpiDir=$GPI_DIR
fi
gpiDirInclude="$gpiDir/src/include"
gpiDirSharedLibs="$gpiDir/src/.libs/libGPI2.so"
mpildflags=$MPI_LD_FLAGS
MPIlibflags=$MPI_LIB_FLAGS 
gaspi_mpi_shmem="$1"
if [ -z $1 ] ; then
	gaspi_mpi_shmem=$GASPI_MPI_SHMEM
fi
name="build/ssca_$gaspi_mpi_shmem"
##################################
cd ../

if ! [ -d $buildDir ] ; then
	mkdir -p build
fi

echo "Compiling program"

# Deleting old file
if [ -f $name ] ; then	
	rm $name
fi

#Compiling
if [ $gaspi_mpi_shmem = "gaspi" ] ; then
	echo "GASPI"
	gcc $(ls *.c) $gpiDirSharedLibs $flags -DUSE_GASPI $optLevel $debugLevel -I$gpiDirInclude -o $name
fi
if [ $gaspi_mpi_shmem = "mpi" ] ; then
	echo "MPI"
	mpicc $(ls *.c) $flags -DUSE_MPI3 $optLevel $debugLevel -o $name
fi
if [ $gaspi_mpi_shmem = "shmem" ] ; then
	echo "OpenSHMEM"
	oshcc $(ls *.c) $mpildflags $MPI_LIB_FLAGS $flags -DUSE_SHMEM $optLevel $debugLevel -o $name

fi

echo "Deleting object files"
rm -f $ofiles

if [ -f $name ] ; then
	echo "Build successful"
else
	echo "Build failed"
fi




