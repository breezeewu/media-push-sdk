valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./Ppcs_clouddemo -n 1 -c conf/push_iot_dev.conf > val.log 2>&1 &
