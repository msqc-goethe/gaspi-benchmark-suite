source paths.sh
block="nonblocking"
scale="26"

#gaspi
bash build.sh "gaspi"
for i in ${ranks[@]}
   do
       bash run_eval.sh "gaspi" "gaspi_$i idx1$scale$block$l.txt" $i $MACHINE_FILE "1"
done


#mpi
bash build.sh "mpi"
for i in ${ranks[@]}
do
   bash run_eval.sh "mpi" "mpi_$i idx1$scale$block$l.txt" $i $MACHINE_FILE "1"
done


#shmem
bash build.sh "shmem"
for i in ${ranks[@]}
do
   bash run_eval.sh "shmem" "shmem_$i idx1$scale$block$l.txt" $i $MACHINE_FILE "1"
done

