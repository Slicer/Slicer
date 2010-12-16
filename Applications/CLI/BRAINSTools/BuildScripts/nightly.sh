#!/bin/sh
#
# Set the library paths such that all locally built shared 
# libraries are found and used
# ahead of system libs
package=BRAINS3

#
# when run by cron, the path variable is only /bin:/usr/bin
export PATH="/usr/local/bin:/usr/sbin:$PATH"

# export CC=/usr/bin/gcc34 CXX=/usr/bin/g++34
#
# make the testing directory, based on current user name
#user=`who -m | sed -e 's/ .*$//'`
user=${LOGNAME}

ThisComputer=`hostname`

#
# the default is to use /brainsdev/kent -- which is
# appropriate on the b2dev VMs.
if [ $# = 0 ] ; then
  startdir=/brainsdev/kent/Testing
else
  startdir=$1
fi

mkdir -p ${startdir}
if [ ! -d ${startdir} ] ; then
echo ${startdir} cannot be created, exiting
exit 1
fi

cd ${startdir}

echo checking out test data in `pwd`

svn co --username kentwilliams https://www.nitrc.org/svn/brainstestdata
if [ $? != 0 ]
then
  echo brainstestdata checkout failed, continuing with old version
fi

mkdir -p ${startdir}/${ThisComputer}

cd ${startdir}/${ThisComputer}

top=`pwd`
echo WORKING IN $top

svn co --username kentwilliams https://www.nitrc.org/svn/brains/BRAINS/trunk ${package}

OsName=`uname`
which gcc > /dev/null 2>&1
if [ $? == 0 ] ; then
    Compiler=gcc-`gcc -dumpversion`-`gcc -dumpmachine`
else
    Compiler=unknown
fi

for BUILD_TYPE in Debug Fast
do
  B3Build=${top}/${BUILD_TYPE}

  mkdir -p ${B3Build}

  cd ${B3Build}

  # build clean
  echo "Deleting old builds"
  rm -fr *

  ldpaths="${B3Build}/src/lib/vtk-5.6:${B3Build}/src/lib/InsightToolkit:${B3Build}/src/lib"
  if [ `uname` = Darwin ]
  then
    if [ ${DYLD_LIBRARY_PATH} ] ; then
      set ldpaths="${ldpaths}:${DYLD_LIBRARY_PATH}"
    fi
    export DYLD_LIBRARY_PATH="${ldpaths}"
  else
    if [ ${LD_LIBRARY_PATH} ] ; then
      set ldpaths="${ldpaths}:${LD_LIBRARY_PATH}"
    fi
    export LD_LIBRARY_PATH="${ldpaths}"
  fi
  echo LD_LIBRARY_PATH=$LD_LIBRARY_PATH

  cmake -DSITE:STRING=${ThisComputer} \
      -DBUILDNAME:STRING="${OsName}-${Compiler}-${BUILD_TYPE}" \
      -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE} ${top}/BRAINS3/SuperBuild

  echo "Building in `pwd`"
  #
  # get rid of the thousands of 'Up-to-date' messages so you can
  # actually see what happens
  make 2>&1 | grep -v 'Up-to-date:'

  cd src/src-build

  make Nightly

done

cd ${top}

mkdir -p Standalone
cd Standalone
svn co --username kentwilliams http://www.nitrc.org/svn/brains/StandAloneApps
mkdir -p SA-build
cd SA-build
cmake -DCMAKE_BUILD_TYPE:STRING=RELEASE ../StandAloneApps
make
make AllTests

