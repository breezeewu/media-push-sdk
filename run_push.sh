cur_dir="$(cd "$(dirname "$0")"; pwd)"
cd $cur_dir
#./svpush $1
./Ppcs_clouddemo -n $1 -c conf/push_aws_test.conf >log.txt 2>&1 &
