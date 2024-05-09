#/bin/bash

# 获取脚本的绝对路径
script_path=$(readlink -f "$0")
script_dir_path=$(dirname "$script_path")
load_workloads="/home/db/YCSB-YXLFE/workloads/blockdb_workloada.spec"
run_workloads="/home/db/YCSB-YXLFE/workloads/blockdb_workloada.spec"
exec="/home/db/YCSB-YXLFE/ycsbc" 
log="ycsb-log.txt"
dbpath="/home/db/blockdb_ycsb_test/$(date +%Y%m%d)"  # 指定日期格式为 YYYYMMDD

if [ -d "$dbpath" ]; then  
    rm -f $dbpath/*  
else
    echo "Directory does not exist: $dbpath. Creating directory."  
    mkdir -p $dbpath  
fi

if [ -f "$exec" ];then
    cp -f $exec $script_dir_path
else
    echo "exec ycsbc does not exist."
fi

date >> $log
cmd="./ycsbc -load 1 -threads 10  -db blockdb -dbpath $dbpath -P $load_workloads >> $log"
echo "${cmd}" >> $log
echo "${cmd}"
eval "${cmd}" >> $log
if [ $? -ne 0 ]; then
    echo "Error: Command failed during load phase."
    exit 1
fi

# # ./ycsbc -db leveldb -dbpath $dbpath -threads 8 -P $workload -run true -dboption 1

# for file_name in $workloads; do
#   echo "Running  $file_name"
#   wait
# done





