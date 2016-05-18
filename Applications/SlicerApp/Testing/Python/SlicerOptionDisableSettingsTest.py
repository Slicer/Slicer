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
    SlicerOptionDisableSettingsTest.py /path/to/Slicer
"""

def checkUserSettings(slicer_executable, common_args):
  # Add a user setting
  args = list(common_args)
  args.extend(['--python-code',
      'slicer.app.userSettings().setValue("foo", "bar"); print("foo: %s" % slicer.app.userSettings().value("foo"))'])
  assert(runSlicerAndExit(slicer_executable, args) == EXIT_SUCCESS)

  # User settings previously added should NOT be set
  args = list(common_args)
  args.extend(['--python-code',
      'if slicer.app.userSettings().value("foo") is not None: raise Exception("Setting \'foo\' shoult NOT be set.")'])
  assert(runSlicerAndExit(slicer_executable, args) == EXIT_SUCCESS)

def checkRevisionUserSettings(slicer_executable, common_args):
  # Add a user revision setting
  args = list(common_args)
  args.extend(['--python-code',
      'slicer.app.revisionUserSettings().setValue("foo", "bar"); print("foo: %s" % slicer.app.revisionUserSettings().value("foo"))'])
  assert(runSlicerAndExit(slicer_executable, args) == EXIT_SUCCESS)

  # User revision settings previously added should NOT be set
  args = list(common_args)
  args.extend(['--python-code',
      'if slicer.app.revisionUserSettings().value("foo") is not None: raise Exception("Setting \'foo\' shoult NOT be set.")'])
  assert(runSlicerAndExit(slicer_executable, args) == EXIT_SUCCESS)

if __name__ == '__main__':

  if len(sys.argv) != 2:
    print(os.path.basename(sys.argv[0]) +" /path/to/Slicer")
    exit(EXIT_FAILURE)

  slicer_executable = os.path.expanduser(sys.argv[1])
  common_args = ['--disable-settings', '--disable-modules', '--no-main-window']

  checkUserSettings(slicer_executable, common_args)
  checkRevisionUserSettings(slicer_executable, common_args)
