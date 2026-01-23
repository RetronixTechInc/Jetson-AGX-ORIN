#!/bin/bash

TARGET_PATH=$1
ACTION=$2
FILE_LIST=$(find . \( -name ".git" -o -name "deploy_files.sh" -o -name "test_script" \) -prune -o -type f -print)

print_usage() {
	echo "deploy.sh target_path diff : list files which are different with target"
	echo "deploy.sh target_path copy : copy all files into target_path"
}

if [ $# -ne 2 ]; then
	print_usage
	exit 1
fi

if [ ! -e ${TARGET_PATH}/jetson-agx-orin-devkit.conf ]; then
	echo "${TARGET_PATH} is not correct path, because I cannot find jetson-agx-orin.conf under it"
	exit 1
fi

case $ACTION in
	diff)
		diff_flag=0
		echo "below files are different :"
		for file in ${FILE_LIST}
		do
			rst=$(diff ${file} ${TARGET_PATH}/${file})
			if [ $? -ne 0 ]; then
				diff_flag=1
				echo "diff ${file} ${TARGET_PATH}/${file}"
				echo "${rst}"
			fi
		done
		
		if [ ${diff_flag} -eq 0 ]; then
			echo "no different"
		fi
		;;
	copy)
		echo "copy files to corresponding path under ${TARGET_PATH}"
		for file in ${FILE_LIST}
		do
			dir_name=$(dirname ${file})
			cp ${file} ${TARGET_PATH}/${dir_name}
		done
		;;
	*)
		print_usage
		exit 1
esac

exit 0



