#!/bin/bash

echo "Using autotools versions:"  
if [[ -e /usr/bin/autoconf-2.5x && -e /usr/bin/automake-1.7 ]]
then
  autoconf-2.5x --version | grep autoconf
  automake-1.7 --version | grep automake
  aclocal-1.7 -I config  
  autoconf-2.5x
  autoheader-2.5x
  automake-1.7 -a
else
  autoconf --version | grep autoconf
  automake --version | grep automake
  aclocal -I config  
  autoconf
  autoheader
  automake -a
fi
