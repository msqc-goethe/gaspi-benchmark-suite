scale="scale12"
block="blocking"
#gaspi
bash build.sh "gaspi" "/opt/gaspi/diana/gaspi_benchmark/GPI-2_Ethernet/"
for k in "mfile_eno1_1GB"
   do
   for j in {1..10}
   do
      for i in 1 2 3 4 5 6
      do
         bash run_eval.sh "gaspi" "gaspi_$k$i idx$j$scale$block.txt" $i $k".txt"
      done
      echo "Gaspi$k $j"
   done
done


