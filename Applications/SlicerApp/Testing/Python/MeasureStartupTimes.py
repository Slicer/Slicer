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
import subprocess
import sys
import time

"""
Usage:
    MeasureStartupTimes.py /path/to/Slicer
"""

EXIT_FAILURE=1
EXIT_SUCCESS=0

def run(slicer, command):
  start = time.time()
  args = ['--no-splash', '--exit-after-startup']
  args.extend(command)
  print("%s %s" % (os.path.basename(slicer), " ".join(args)))
  args.insert(0, slicer)
  p = subprocess.Popen(args=args, stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
  stdout, stderr = p.communicate()

  if p.returncode != EXIT_SUCCESS:
    print('STDOUT: ' + stdout)
    print('STDERR: ' + stderr)

  print("{:.3f} seconds".format(time.time() - start))
  print("")

if __name__ == '__main__':

  if len(sys.argv) != 2:
    print(os.path.basename(sys.argv[0]) +" /path/to/Slicer")
    exit(EXIT_FAILURE)

  slicer = os.path.expanduser(sys.argv[1])

  tests = [
    [],
    ['--disable-builtin-cli-modules'],
    ['--disable-builtin-loadable-modules'],
    ['--disable-builtin-scripted-loadable-modules'],
    ['--disable-builtin-cli-modules', '--disable-builtin-scripted-loadable-modules'],
    ['--disable-modules']
  ]

  for test in tests:
    run(slicer, test)

  for test in tests:
    test.insert(0, '--disable-python')
    run(slicer, test)

