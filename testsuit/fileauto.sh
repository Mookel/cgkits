#!/bin/bash

#Must provide the test name ,or exit.
if [ ! -n "$1" ] ;then
    echo "Please input the test name."
    exit
else
    echo "The test name is $1."
fi

#If test_suit.c not existed, read mtest and create it.
test_suit_name="test_suit.c"
if [ ! -e ./$test_suit_name ]; then
	touch $test_suit_name
	if [ ! -e ./mtest ]; then
		echo "Error."
		exit
	fi
	model_test_string=`cat ./mtest`
	echo "$model_test_string" >> $test_suit_name
fi

test_name=$1
file_name=$1"_test.c"

if [ ! -e ./$file_name ];then
    touch $file_name
else
    rm -i $file_name
fi

###Generate test file.
echo "The file name is $file_name."
echo "/*This file is generated by fileauto.sh used by test_suit.*/" >> $file_name
echo "/*Author: Mookel, all rights reversed.*/" >> $file_name
echo "" >> $file_name
echo "#include <$1"".h>" >> $file_name
echo " "
echo "int __$test_name""_test__()" >> $file_name
echo "{" >> $file_name
echo "  return 0;" >> $file_name
echo "}" >> $file_name

###Add test macro into the test_suit.c file.
flag="fucktab"
test_add_flag="TEST_ADD_FLAG(TRUE)"
addtest="ADD_TEST(\"$test_name\", $test_name);"

find_result=`cat $test_suit_name | grep "$addtest"`;

if [ -n "$find_result" ]; then
    echo "The $test_name has been added to test_suit."
    read -p "Do you want to repalce it ? (y/n): " command
    if [ -n $command ]; then
        if [ "$command" == "y" ]; then
            sed -i "" '/'"$addtest"'/d' $test_suit_name
        else
            echo "Fuck, do nothing...."
            exit
        fi
    else
        echo "What fuck do you want to do! exit...."
    fi
fi

addto="$flag $addtest"
sed -i '' '/'"${test_add_flag}"'/i\
'"${addto}"'
' $test_suit_name

###replace flag with tab or space.
sed -i "" 's/fucktab/   /g' $test_suit_name

