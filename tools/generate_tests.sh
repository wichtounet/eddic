#!/bin/bash

rm -f tests.mk
rm -f test_list

while read line
do
    if [[ $line == "Entering test suite"* ]]
    then
        test_suite=`echo $line | cut -b 22- | sed 's/.$//'`

        if [[ $test_suite == "eddic_test_suite" ]]
        then
            test_suite=""
        fi
    fi
    
    if [[ $line == "Leaving test suite"* ]]
    then
        test_suite=""
    fi
    
    if [[ $line == "Entering test case"* ]]
    then
        test_case=`echo $line | cut -b 21- | sed 's/.$//'`

        if [[ $test_suite == "" ]]
        then
            echo $test_case >> test_list
        else
            echo $test_suite/$test_case >> test_list
        fi
    fi
done < tests.tmp.log

while read line
do
    escaped_line=`echo $line | sed 's/\//-/'`
    echo "debug_test_$line" ': $(DEBUG_TEST_EXE)' >> tests.mk
    echo -e "\t @ echo \"Run $line\" > test_reports/test_$escaped_line.log" >> tests.mk
    echo -e "\t @" './$(DEBUG_TEST_EXE)' "--run_test=$line --report_sin=stdout >> test_reports/test_$escaped_line.log" >> tests.mk
    echo >> tests.mk
    
    echo "release_test_$line" ': $(RELEASE_TEST_EXE)' >> tests.mk
    echo -e "\t @ echo \"Run $line\" > test_reports/test_$escaped_line.log" >> tests.mk
    echo -e "\t @" './$(RELEASE_TEST_EXE)' "--run_test=$line --report_sin=stdout >> test_reports/test_$escaped_line.log" >> tests.mk
    echo >> tests.mk
done < test_list

echo -n "debug_test_all: " >> tests.mk

while read line
do
    echo -n "debug_test_$line " >> tests.mk
done < test_list
echo "" >> tests.mk

echo -e "\t @ bash ./tools/test_report.sh" >> tests.mk
echo "" >> tests.mk

echo -n "release_test_all: " >> tests.mk

while read line
do
    echo -n "release_test_$line " >> tests.mk
done < test_list
echo "" >> tests.mk

echo -e "\t @ bash ./tools/test_report.sh" >> tests.mk
echo "" >> tests.mk

echo -n ".PHONY: release_test_all debug_test_all" >> tests.mk

while read line
do
    echo -n "debug_test_$line release_test_$line " >> tests.mk
done < test_list
echo "" >> tests.mk
