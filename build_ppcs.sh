cd ./librtmp
make env=$1 clean
make env=$1
cd ../sdk
make env=$1 clean
make env=$1
make env=$1 install
cd ../demo
make env=$1 clean
make env=$1