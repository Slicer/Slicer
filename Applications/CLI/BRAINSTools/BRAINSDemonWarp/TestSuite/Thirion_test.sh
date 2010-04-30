#!/bin/bash

if [ $# -ne 3 ]; then
  echo "USAGE: $0 <path_to_Thirion> <path to outputdir> <path to gauss output >  "
  exit
fi

TD=$1/bin/ThirionRegistration.exe
TestDir=$2
gaussoutput=$3
mkdir -p ${TestDir}
cd ${TestDir}
export LD_LIBRARY_PATH=$1/lib:$1/lib/InsightToolkit
export DYLD_LIBRARY_PATH=${LD_LIBRARY_PATH}

which ldd >/dev/null 2>&1
if [ $? -eq 0 ]; then
   ldd ${TD}
fi
which otool >/dev/null 2>&1
if [ $? -eq 0 ]; then
   otool -L ${TD}
fi

image1=${gaussoutput}/TestImages/Image1.nii.gz
image2=${gaussoutput}/TestImages/Image2.nii.gz
if [ -f ${1}/bin/ThirionRegistration.exe ]; then
${TD} -input $image1 -target $image2  -p ${gaussoutput}/ThirionDemons/param_test.txt -output ${TestDir}/test.nii.gz
else
 echo ${1}/bin/ThirionRegistration.exe does not exists
fi
${1}/bin/SqErr.exe ${gaussoutput}/ThirionDemons/standard.nii.gz ${TestDir}/test.nii.gz ${TestDir}/testvar.nii.gz |grep SQERR | tee ${TestDir}/test

#GetValue function returns the required values from a given file 
function GetValue ()
{
FILE=$1
LINE=$2
COLUMN=$3
GetValue_Result=$(cat  ${FILE} | head -${LINE} | tail -1 | awk '{print $'${COLUMN}'}')
}

#Isclose function tests whether the test values ar within 5% of the standard result. 
function IsClose ()
{
TEST_VALUE=$1
EPSILON=$2
TEST_VALUE=$(printf "%.9f" ${TEST_VALUE})
ispositive=$(echo "${TEST_VALUE} < 0" | bc -l)

if [ ! -z "${ispositive}" ]; then
  if  [ ${ispositive} -eq 1 ] ;then
   TEST_VALUE=$(echo "${TEST_VALUE} * -1" | bc -l)
  fi
fi
IsClose_Result=$(echo "${TEST_VALUE} > ${EPSILON}" | bc -l)
}

GetValue ${TestDir}/test 2 4
test_value=${GetValue_Result}
IsClose ${test_value} 0.0005
cd ${1}
rm -r ${TestDir}
 #if the results are close enough the test data is correct.
if [ ! -z "${IsClose_Result}" ] ; then
  if [ ${IsClose_Result} -eq 0 ]; then
    echo '"success: Thirion test Passed"'
    exit 0
  fi
fi
echo '"false: Thirion test failed " because not close enough: ${test_value}'
exit 1


