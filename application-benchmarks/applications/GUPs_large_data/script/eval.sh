#scale=(1 10 100 1000 10000 100000 1000000)
scale=(1000000)
#gaspi
bash build.sh "gaspi" "/opt/gaspi/diana/gaspi_benchmark/GPI-2_Infiniband/"
for j in {1..3}
do
   for i in 2
   do
      for sc in ${scale[@]}
      do
         bash run_eval.sh "gaspi" "gaspi_mfile_ib0$i$sc idx$j.txt" $i "mfile_ib0.txt" $sc
      done
   done
done

#mpi
bash build.sh "mpi"
for k in  "mfile_ib0"
   do
   for j in {1..3}
   do
      for i in 2
      do
         for sc in ${scale[@]}
         do
            bash run_eval.sh "mpi" "mpi_$k$i$sc idx$j.txt" $i $k".txt" $sc
         done
      done
   done
done

#shmem
bash build.sh "shmem"
for k in "mfile_ib0"
   do
   for j in {1..3}
   do
      for i in 2
      do
         for sc in ${scale[@]}
         do
            bash run_eval.sh "shmem" "shmem_$k$i$sc idx$j.txt" $i $k".txt" $sc
         done
      done
   done
done

