#!/usr/bin/env python

#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 1U24CA194354-01
#

import os
import sys

from SlicerAppTesting import *

"""
Usage:
    MeasureStartupTimes.py /path/to/Slicer
"""

if __name__ == '__main__':

  if len(sys.argv) != 2:
    print(os.path.basename(sys.argv[0]) +" /path/to/Slicer")
    exit(EXIT_FAILURE)

  slicer_executable = os.path.expanduser(sys.argv[1])

  tests = [
    [],
    ['--disable-builtin-cli-modules'],
    ['--disable-builtin-loadable-modules'],
    ['--disable-builtin-scripted-loadable-modules'],
    ['--disable-builtin-cli-modules', '--disable-builtin-scripted-loadable-modules'],
    ['--disable-modules']
  ]

  runSlicerAndExit = timecall(runSlicerAndExit)

  for test in tests:
    runSlicerAndExit(slicer_executable, test)

  for test in tests:
    test.insert(0, '--disable-python')
    runSlicerAndExit(slicer_executable, test)
