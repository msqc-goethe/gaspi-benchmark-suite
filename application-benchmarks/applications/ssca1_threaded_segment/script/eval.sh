block="nonblocking"
scale="26"

bash build.sh "gaspi" "/opt/gaspi/diana/gaspi_benchmark/GPI-2_Ethernet/"
k="mfile_eno2_10GB"
for j in {1..10}
do
   for i in 2
   do
      for l in 1 
      do
         bash run_eval.sh "gaspi" "gaspi_$k$i idx$j$scale$block$l.txt" $i $k".txt" $l
      done
   done
done

#bash build.sh "gaspi" "/opt/gaspi/diana/gaspi_benchmark/GPI-2_Ethernet/"
#for k in "mfile_eno2_10GB" #"mfile_eno2_10GB"
#   do
#   for j in {1..10}
#   do
#      for i in 1 2 3 4 5 6
#      do
#      for l in 1 2 4 8
#         do
#            bash run_eval.sh "gaspi" "gaspi_$k$i idx$j$scale$block$l.txt" $i $k".txt" $l
#         done
#      done
#      echo "Gaspi$k $j"
#   done
#done
#bash build.sh "gaspi" "/opt/gaspi/diana/gaspi_benchmark/GPI-2_Ethernet/"
#for k in "mfile_eno1_1GB" "mfile_eno2_10GB"
#   do
#   for j in {1..10}
#   do
#      for i in 1 2 3 4 5 6
#      do
#         for l in 1 2 4 8
#         do
#            bash run_eval.sh "gaspi" "gaspi_$k$i idx$j$scale$block$l.txt" $i $k".txt" $l
#         done
#      done
#      echo "Gaspi$k $j"
#   done
#done


#mpi
#bash build.sh "mpi"
#for k in "mfile_eno1_1GB" "mfile_eno2_10GB" "mfile_ib0"
#   do
#   for j in {1..10}
#   do
#      for i in  1 2 3 4 5 6
#      do
#         for l in 1 2 4 8 
#         do
#            bash run_eval.sh "mpi" "mpi_$k$i idx$j$scale$block$l.txt" $i $k".txt" $l
#         done
#      done
#   done
#done

#shmem
#bash build.sh "shmem"
#for k in "mfile_eno1_1GB" "mfile_eno2_10GB" "mfile_ib0"
#   do
#   for j in {1..10}
#   do
#      for i in  1 2 3 4 5 6
#      do
#         for l in 1 2 4 8
#         do
#            bash run_eval.sh "shmem" "shmem_$k$i idx$j$scale$block$l.txt" $i $k".txt" $l
#         done
#      done
#   done
#done

#bash build.sh "gaspi" "/opt/gaspi/diana/gaspi_benchmark/GPI-2_Infiniband/"
#for j in {1..10}
#do
#   for i in 1 2 3 4 5 6
#   do
#      for l in 1 2 4 8
#      do
#         bash run_eval.sh "gaspi" "gaspi_mfile_ib0$i idx$j$scale$block$l.txt" $i "mfile_ib0.txt" $l
#      done
#   done
#done