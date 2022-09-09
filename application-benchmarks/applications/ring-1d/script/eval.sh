thread=2
#scale=(16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288)
scale=(64)
#gaspi
bash build.sh "gaspi" "/opt/gaspi/diana/gaspi_benchmark/GPI-2_Infiniband/"
for j in {1..1}
do
   for sc in ${scale[@]}
   do
         bash run_eval.sh "gaspi" "scale_gaspi_mfile_ib0$sc idx$j$thread.txt" "4" "mfile_ib0.txt" $sc
   done
done

#mpi
bash build.sh "mpi"
for k in  "mfile_ib0"
   do
   for j in {1..1}
   do
      for sc in ${scale[@]}
      do
         bash run_eval.sh "mpi" "scale_mpi_$k$sc idx$j$thread.txt" "4" $k".txt" $sc
      done
   done
done


#shmem
bash build.sh "shmem"
for k in  "mfile_ib0"
   do
   for j in {1..1}
   do
      for sc in ${scale[@]}
      do
         bash run_eval.sh "shmem" "scale_shmem_$k$sc idx$j$thread.txt" "4" $k".txt" $sc
      done
   done
done

