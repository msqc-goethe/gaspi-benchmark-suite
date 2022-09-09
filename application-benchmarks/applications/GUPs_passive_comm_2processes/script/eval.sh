#gaspi
bash build.sh "gaspi" "/opt/gaspi/diana/gaspi_benchmark/GPI-2_Infiniband/"
for j in {1..2}
do
   for i in 2
   do
         bash run_eval.sh "gaspi" "100cycles_gaspi_mfile_ib0$i idx$j.txt" $i "mfile_ib0.txt"
   done
done

#mpi
bash build.sh "mpi"
for k in  "mfile_ib0"
   do
   for j in {1..2}
   do
      for i in  2
      do
         bash run_eval.sh "mpi" "100cycles_mpi_$k$i idx$j.txt" $i $k".txt"
      done
   done
done


