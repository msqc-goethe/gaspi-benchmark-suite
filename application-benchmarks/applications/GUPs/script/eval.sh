#gaspi
#bash build.sh "gaspi" "/opt/gaspi/diana/gaspi_benchmark/GPI-2_Infiniband/"
#for j in {1..3}
#do
#   for i in 1 2 3 4 5 6
#   do
#         bash run_eval.sh "gaspi" "gaspi_mfile_ib0$i idx$j.txt" $i "mfile_ib0.txt"
#   done
#done

#mpi
bash build.sh "mpi"
for k in  "mfile_ib0"
   do
   for j in {1..3}
   do
      for i in  1 2 3 4 5 6
      do
         bash run_eval.sh "mpi" "mpi_$k$i idx$j.txt" $i $k".txt"
      done
   done
done

#shmem
#bash build.sh "shmem"
#for k in "mfile_ib0"
#   do
#   for j in {1..3}
#   do
#      for i in  1 2 3 4 5 6
#      do
#         bash run_eval.sh "shmem" "shmem_$k$i idx$j.txt" $i $k".txt"
#      done
#   done
#done

