for file_name in `find ./ -name *.cpp  -type f`
	do
		file_type=`file -i $file_name`
		type=`echo $file_type|grep "utf-16le"`
		if [ -n "$type" ];then
			echo "123456"|sudo -S iconv -f UTF-16LE -t UTF-8 $file_name -o $file_name
		fi
	done
