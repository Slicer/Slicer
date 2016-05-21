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

import argparse
import json
import os
import sys
import tempfile

from SlicerAppTesting import *

"""
Usage:
    MeasureStartupTimes.py /path/to/Slicer
"""

def TemporaryPythonScript(code, *args, **kwargs):
  if 'suffix' not in kwargs:
    kwargs['suffix'] = '.py'
  if 'mode' not in kwargs:
    kwargs['mode'] = 'w'
  script = tempfile.NamedTemporaryFile(*args, **kwargs)
  script.write(code)
  script.flush()
  print("Written script %s [%s]" % (script.name, code))
  return script

def collect_startup_times_overall(outut_file):

  tests = [
    [],
    ['--disable-builtin-cli-modules'],
    ['--disable-builtin-loadable-modules'],
    ['--disable-builtin-scripted-loadable-modules'],
    ['--disable-builtin-cli-modules', '--disable-builtin-scripted-loadable-modules'],
    ['--disable-modules']
  ]

  results= {}

  for test in tests:
    (duration, result) = runSlicerAndExitWithTime(slicer_executable, test)
    results[" ".join(test)] = duration
    print("{:.3f} seconds".format(duration))
    print("")

  for test in tests:
    test.insert(0, '--disable-python')
    (duration, result) = runSlicerAndExitWithTime(slicer_executable, test)
    results[" ".join(test)] = duration
    print("{:.3f} seconds".format(duration))
    print("")

  with open(outut_file, 'w') as file:
    file.write(json.dumps(results, indent=4))


def collect_startup_times_excluding_one_module(output_file):

  # Collect list of all modules and their associated types
  python_script = TemporaryPythonScript("""
import json

modules = {}
moduleManager = slicer.app.moduleManager()
for name in dir(slicer.moduleNames):
  module = moduleManager.module(name)
  if module is None:
    continue
  moduleType = None
  if module.inherits('qSlicerLoadableModule'):
    module_type = "Loadable"
  elif module.inherits('qSlicerCLIModule'):
    module_type = "CLI"
  elif module.inherits('qSlicerScriptedLoadableModule'):
    module_type = "Scripted"
  elif module.inherits('qSlicerCoreModule'):
    module_type = "Core"
  else:
    raise Exception("Failed to module type of %s" % name)
  print("Found module %s of type %s" % (name, module_type))
  modules[name] = module_type
with open("Modules.json", 'w') as output:
  output.write(json.dumps(modules, indent=4))
""")

  (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, ['--python-script', python_script.name])
  assert returnCode == EXIT_SUCCESS
  print("=> ok\n")

  # Read list of modules
  with open('Modules.json') as input:
    modules = json.load(input)
  print("Found %d  modules\n" % len(modules))

  # Collect startup times disabling each module one by one
  moduleTimes = {}
  for (idx, (moduleName, moduleType)) in enumerate(modules.iteritems(), start=1):
    #if moduleType == "CLI":
    #  print("=> Skipping CLI [%s]\n" % moduleName)
    #  continue
    print("[%d/%d]" % (idx, len(modules)))
    (duration, result) = runSlicerAndExitWithTime(slicer_executable, ['--testing', '--modules-to-ignore', moduleName])
    (returnCode, stdout, stderr) = result
    if returnCode != EXIT_SUCCESS:
      # XXX Ignore module with dependencies
      duration = None
      print("=> failed\n")
    else:
      moduleTimes[moduleName] = duration
      print("=> {:.3f} seconds\n".format(duration))

  with open(output_file, 'w') as file:
    file.write(json.dumps(moduleTimes, indent=4))

if __name__ == '__main__':

  parser = argparse.ArgumentParser(description='Measure startup times.')
  parser.add_argument("--overall", action="store_true")
  parser.add_argument("--excluding-one-module", action="store_true")
  parser.add_argument("/path/to/Slicer")
  args = parser.parse_args()

  slicer_executable = os.path.expanduser(getattr(args, "/path/to/Slicer"))
  all = not args.overall and not args.excluding_one_module

  runSlicerAndExitWithTime = timecall(runSlicerAndExit)

  if all or args.overall:
    collect_startup_times_overall("StartupTimes.json")

  if all or args.excluding_one_module:
    collect_startup_times_excluding_one_module("StartupTimesExcludingOneModule.json")
