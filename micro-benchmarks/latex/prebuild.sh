#!/bin/bash

user=$HEIBOX_USER
pass=$HEIBOX_PASSWORD
zipname="gaspi-benchmarks.zip"
csvmerger="csv-merger"
heiboxPath="${HEIBOX_PATH}/Messdaten/"
resultPath="https://heibox.uni-heidelberg.de/seafdav/$heiboxPath/"

resultgi="result-gi"
resultte="result-te"
resultib="result-ib"
resultipoib="result-ipoib"
result="result-merged"


function addCsvNetworkNames {
	csv=$1
	csvtmp="$csv-tmp"
	echo "network,"$(awk 'NR==1' $csv) > $csvtmp
	echo "1\,GbE,"$(awk 'NR==2' $csv) >> $csvtmp
	echo "10\,GbE,"$(awk 'NR==3' $csv) >> $csvtmp
	echo "IPoIB,"$(awk 'NR==4' $csv) >> $csvtmp
	echo "IB,"$(awk 'NR==5' $csv) >> $csvtmp
	mv $csvtmp $csv
}

function mergeOneLineCsvs {
	csvname=$1
	resultname="$result/$csvname"
	./$csvmerger -c=repetitions -r=0 -o=$resultname $resultgi/$csvname $resultte/$csvname $resultipoib/$csvname $resultib/$csvname
	addCsvNetworkNames $resultname
}

echo $resultPath

#Download Results
curl -o $zipname -u ${user}:${pass} $resultPath/$zipname

unzip -o $zipname
rm -f $zipname

# Download Merger
if ! [ -f $csvmerger ] ; then
	curl -o $csvmerger -u ${user}:${pass} $resultPath/$csvmerger
	chmod +x $csvmerger
fi

if ! [ -d $result ] ; then
	mkdir $result
fi




# Merge Barrier files
mergeOneLineCsvs "gbs_ubench_barrier.csv"


# Merge Atomic FAA
mergeOneLineCsvs "gbs_ubench_atomic_fetch_add_single.csv"
mergeOneLineCsvs "gbs_ubench_atomic_fetch_add_all.csv"

# Merge Atomic CAS
mergeOneLineCsvs "gbs_ubench_atomic_compare_swap_single.csv"
mergeOneLineCsvs "gbs_ubench_atomic_compare_swap_all.csv"


# Merge Notification Rate
mergeOneLineCsvs "gbs_ubench_noti_single_udir.csv"
mergeOneLineCsvs "gbs_ubench_noti_single_bdir.csv"


