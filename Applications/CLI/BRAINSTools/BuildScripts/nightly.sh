#!/bin/sh
#
# Set the library paths such that all locally built shared libraries are found and used
# ahead of system libs
setpaths ()
{
ABI=$1
SAVEPATH=$PATH
current=`pwd`
export PATH="${current}:${current}/bin:${current}/lib:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/VTK-build/bin:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/Insight-build/bin:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/KWWidgets-build/bin:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/tcl-build/lib:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/tcl-build/bin:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/tcl-build/lib:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/tcl-build/lib/bin:/sw/bin:$PATH"
libpath="${current}:${current}/bin:${current}/lib:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/VTK-build/bin:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/Insight-build/bin:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/KWWidgets-build/bin:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/tcl-build/lib:\
${top}/BRAINS-COMPILE/${platform}/${ABI}-lib/tcl-build/lib:$DYLD_LIBRARY_PATH"
if [ $platform = "Darwin" ] ; then
SAVEDYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=$libpath
else
SAVELD_LIBRARY_PATH=$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$libpath
fi
SAVETCLLIBPATH=$SAVETTCLLIBPATH
export TCLLIBPATH=' {${top}/BRAINS-COMPILE/${platform}/DEBUG-lib/VTK-build/Wrapping/Tcl} {${top}/BRAINS-COMPILE/${platform}/DEBUG-lib/KWWidgets-build/Wrapping/Tcl} '$TCLLIBPATH

SAVEPYTHONPATH=$PYTHONPATH
export PYTHONPATH="${top}/BRAINS-COMPILE/${platform}/DEBUG-lib/VTK-build/Wrapping/Tcl/../Python:${top}/BRAINS-COMPILE/${platform}/DEBUG-lib/VTK-build/bin:${top}/BRAINS-COMPILE/${platform}/DEBUG-lib/KWWidgets-build/bin:$PYTHONPATH"
}
#
# restore paths saved in setpaths
restorepaths ()
{

export PATH=$SAVEPATH
if [ $platform = "Darwin" ] ; then
export DYLD_LIBRARY_PATH=$SAVEDYLD_LIBRARY_PATH
else
export LD_LIBRARY_PATH=$SAVELD_LIBRARY_PATH
fi
export TCLLIBPATH=$SAVETTCLLIBPATH
export PYTHONPATH=$SAVEPYTHONPATH

}

#
# when run by cron, the path variable is only /bin:/usr/bin
export PATH="$PATH:/usr/sbin:/opt/cmake/bin"
#
# prefer gcc4
if which gcc4
then
export CC=`which gcc4`
fi
if which g++4
then
export CXX=`which g++4`
fi

#
# make the testing directory, based on current user name
#user=`who -m | sed -e 's/ .*$//'`
user=${LOGNAME}
mkdir -p /scratch/${user}/Testing

cd /scratch/${user}/Testing
top=`pwd`
echo WORKING IN $top

svn co https://www.nitrc.org/svn/brainscut BRAINSCut
svn co https://www.nitrc.org/svn/brainsdemonwarp BRAINSDemonWarp
svn co https://www.nitrc.org/svn/multimodereg BRAINSFit
svn co https://www.nitrc.org/svn/brainstracer BRAINSTracer
svn co https://www.nitrc.org/svn/art/trunk art
export platform=`uname`
#
# for all projects using the BuildScripts build method
for target in BRAINSTracer BRAINSCut BRAINSDemonWarp BRAINSFit art
do
  cd ${top}
  # do debug and fast builds
  ${target}/BuildScripts/getbuildtest.tcl --shared --abi DEBUG --nightly
#  echo NIGHTLYdebug FOR $target finished
  ${target}/BuildScripts/getbuildtest.tcl --shared --abi FAST --release --nightly
#  echo NIGHTLYfast FOR $TARGEt finishd 
#   # do ctest for debug
#   cd ${top}/BRAINS-COMPILE/${platform}/DEBUG-build/${target}-build
#   setpaths DEBUG
#   ctest -D NightlyStart -D NightlyConfigure -D NightlyBuild -D NightlyMemCheck -D NightlySubmit
#   restorepaths
#   # do ctest for fast
#   setpaths FAST
#   cd ${top}/BRAINS-COMPILE/${platform}/FAST-build/${target}-build
#   ctest -D NightlyStart -D NightlyConfigure -D NightlyBuild -D NightlyMemCheck -D NightlySubmit
#   restorepaths
done
