thread=(16)

#gaspi
bash build.sh "gaspi" "/opt/gaspi/diana/gaspi_benchmark/GPI-2_Infiniband/"
for j in {1..3}
do
   for i in 2
   do
      for th in ${thread[@]}
      do
         bash run_eval.sh "gaspi" "gaspi_mfile_ib0$i$th idx$j.txt" $i "mfile_ib0.txt" $th
      done
   done
done

#mpi
#bash build.sh "mpi"
#for k in  "mfile_ib0"
#   do
#   for j in {1..3}
#   do
#      for i in 2
#      do
#         for th in thread
#         do
#            bash run_eval.sh "mpi" "mpi_$k$i$th idx$j.txt" $i $k".txt" $th
#         done
#      done
#   done
#done

#shmem
bash build.sh "shmem"
for k in "mfile_ib0"
   do
   for j in {1..3}
   do
      for i in 2
      do
         for th in ${thread[@]}
         do
            bash run_eval.sh "shmem" "shmem_$k$i$th idx$j.txt" $i $k".txt" $th
         done
      done
   done
done

