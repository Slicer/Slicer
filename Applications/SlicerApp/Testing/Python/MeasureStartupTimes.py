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

from __future__ import print_function
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

def collect_startup_times_normal(output_file, drop_cache=False, display_output=False):
  results= {}
  test = []
  (duration, result) = runSlicerAndExitWithTime(slicer_executable, test, drop_cache=drop_cache)
  (returnCode, stdout, stderr) = result
  if display_output:
    if stdout: print("STDOUT [%s]\n" % stdout)
    if stderr and returnCode == EXIT_SUCCESS: print("STDERR [%s]\n" % stderr)
  results[" ".join(test)] = duration
  with open(output_file, 'w') as file:
    file.write(json.dumps(results, indent=4))

def collect_startup_times_overall(output_file, drop_cache=False, display_output=False):

  results= {}

  test = ["--help"]
  (duration, result) = runSlicerAndExitWithTime(slicer_executable, test, drop_cache=drop_cache)
  results[" ".join(test)] = duration

  tests = [
    [],
    ['--disable-builtin-cli-modules'],
    ['--disable-builtin-loadable-modules'],
    ['--disable-builtin-scripted-loadable-modules'],
    ['--disable-builtin-cli-modules', '--disable-builtin-scripted-loadable-modules'],
    ['--disable-modules']
  ]

  for test in tests:
    (duration, result) = runSlicerAndExitWithTime(slicer_executable, test, drop_cache=drop_cache)
    results[" ".join(test)] = duration

  for test in tests:
    test.insert(0, '--disable-python')
    (duration, result) = runSlicerAndExitWithTime(slicer_executable, test, drop_cache=drop_cache)
    results[" ".join(test)] = duration

    (returnCode, stdout, stderr) = result
    if display_output:
      if stdout: print("STDOUT [%s]\n" % stdout)
      if stderr and returnCode == EXIT_SUCCESS: print("STDERR [%s]\n" % stderr)

  with open(output_file, 'w') as file:
    file.write(json.dumps(results, indent=4))

def read_modules(input_file):
  # Read list of modules
  with open(input_file) as input:
    modules = json.load(input)
  print("Found %d modules reading %s\n" % (len(modules), input_file))
  return modules

def collect_modules(output_file):
  # Collect list of all modules and their associated types
  python_script = TemporaryPythonScript("""
import json

modules = {{}}
moduleManager = slicer.app.moduleManager()
for name in dir(slicer.moduleNames):
  module = moduleManager.module(name)
  if module is None:
    continue
  moduleType = None
  if module.inherits('qSlicerCLIModule'):
    module_type = "CLI"
  elif module.inherits('qSlicerScriptedLoadableModule'):
    module_type = "Scripted"
  elif module.inherits('qSlicerLoadableModule'):
    module_type = "Loadable"
  elif module.inherits('qSlicerCoreModule'):
    module_type = "Core"
  else:
    raise Exception("Failed to module type of %s" % name)
  print("Found module %s of type %s" % (name, module_type))
  modules[name] = module_type
with open("{0}", 'w') as output:
  output.write(json.dumps(modules, indent=4))
""".format(output_file))

  (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, ['--python-script', python_script.name])
  assert returnCode == EXIT_SUCCESS
  print("=> ok\n")

  return read_modules(output_file)

def slicerRevision():
  (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, [
    '--no-main-window', '--ignore-slicerrc', '--disable-modules',
    '--python-code', 'print(slicer.app.repositoryRevision)'
  ])
  assert returnCode == EXIT_SUCCESS
  return stdout.split()[0]

def collect_startup_times_including_one_module(output_file, module_list, drop_cache=False, display_output=False):
  modules = collect_modules(module_list)
  # Collect startup times disabling each module one by one
  moduleTimes = {}
  for (idx, (moduleName, moduleType)) in enumerate(modules.iteritems(), start=1):
    modules_minus_one = list(modules.keys())
    del modules_minus_one[modules_minus_one.index(moduleName)]
    print("[%d/%d] including %s" % (idx, len(modules), moduleName))
    test = ['--testing', '--modules-to-ignore', ",".join(modules_minus_one)]
    (duration, result) = runSlicerAndExitWithTime(slicer_executable, test, drop_cache=drop_cache)
    (returnCode, stdout, stderr) = result
    if display_output:
      if stdout: print("STDOUT [%s]\n" % stdout)
      if stderr and returnCode == EXIT_SUCCESS: print("STDERR [%s]\n" % stderr)
    if returnCode != EXIT_SUCCESS:
      # XXX Ignore module with dependencies
      duration = None
      print("=> failed\n")
    else:
      moduleTimes[moduleName] = duration

  with open(output_file, 'w') as file:
    file.write(json.dumps(moduleTimes, indent=4))

def collect_startup_times_excluding_one_module(output_file, module_list, drop_cache=False, display_output=False):
  modules = collect_modules(module_list)
  # Collect startup times disabling each module one by one
  moduleTimes = {}
  for (idx, (moduleName, moduleType)) in enumerate(modules.iteritems(), start=1):
    #if moduleType == "CLI":
    #  print("=> Skipping CLI [%s]\n" % moduleName)
    #  continue
    print("[%d/%d]" % (idx, len(modules)))
    (duration, result) = runSlicerAndExitWithTime(slicer_executable, ['--testing', '--modules-to-ignore', moduleName], drop_cache=drop_cache)
    (returnCode, stdout, stderr) = result
    if display_output:
      if stdout: print("STDOUT [%s]\n" % stdout)
      if stderr and returnCode == EXIT_SUCCESS: print("STDERR [%s]\n" % stderr)
    if returnCode != EXIT_SUCCESS:
      # XXX Ignore module with dependencies
      duration = None
      print("=> failed\n")
    else:
      moduleTimes[moduleName] = duration

  with open(output_file, 'w') as file:
    file.write(json.dumps(moduleTimes, indent=4))

def collect_startup_times_modules_to_load(output_file, modules_to_load, module_list, drop_cache=False, display_output=False):
  modules = collect_modules(module_list)
  modulesToIgnore = list(modules.keys())
  for moduleName in modules_to_load.split(","):
    print("Including %s" % moduleName)
    del modulesToIgnore[modulesToIgnore.index(moduleName)]

  test = ['--testing', '--modules-to-ignore', ",".join(modulesToIgnore)]
  (duration, result) = runSlicerAndExitWithTime(slicer_executable, test, drop_cache=drop_cache)
  (returnCode, stdout, stderr) = result
  if display_output:
    if stdout: print("STDOUT [%s]\n" % stdout)
    if stderr and returnCode == EXIT_SUCCESS: print("STDERR [%s]\n" % stderr)

  results= {}
  results[" ".join(modulesToIgnore)] = duration
  with open(output_file, 'w') as file:
    file.write(json.dumps(results, indent=4))

if __name__ == '__main__':

  parser = argparse.ArgumentParser(description='Measure startup times.')
  # Experiments
  parser.add_argument("--normal", action="store_true")
  parser.add_argument("--modules-to-load")
  parser.add_argument("--overall", action="store_true")
  parser.add_argument("--excluding-one-module", action="store_true")
  parser.add_argument("--including-one-module", action="store_true")
  # Common options
  parser.add_argument("-n", "--repeat",  default=1, type=int)
  parser.add_argument("--drop-cache", action="store_true")
  parser.add_argument("--reuse-module-list", action="store_true")
  parser.add_argument("--display-slicer-output", action="store_true")
  parser.add_argument("/path/to/Slicer")
  args = parser.parse_args()

  slicer_executable = os.path.expanduser(getattr(args, "/path/to/Slicer"))
  all = (not args.normal
    and not args.modules_to_load
    and not args.overall
    and not args.excluding_one_module
    and not args.including_one_module)

  runSlicerAndExitWithTime = timecall(runSlicerAndExit, repeat=args.repeat)

  sliver_revision = slicerRevision()
  module_list = "Modules-r%s.json" % sliver_revision

  if args.reuse_module_list:
    print("Loading existing module listing")
    collect_modules = read_modules

  common_kwargs = {
    'display_output': args.display_slicer_output,
    'drop_cache': args.drop_cache
    }

  # Since the "normal" experiment is included in the "overall" one,
  # it is not executed by default.
  if args.normal:
    collect_startup_times_normal("StartupTimesNormal-r%s.json" % sliver_revision, **common_kwargs)

  # Since the "modules-to-load" experiment requires user input and is provided
  # for convenience, it is not executed by default.
  if args.modules_to_load:
    collect_startup_times_modules_to_load(
      "StartupTimesSelectedModules.json", args.modules_to_load, module_list, **common_kwargs)

  if all or args.overall:
    collect_startup_times_overall("StartupTimes-r%s.json" % sliver_revision, **common_kwargs)

  if all or args.excluding_one_module:
    collect_startup_times_excluding_one_module(
      "StartupTimesExcludingOneModule-r%s.json" % sliver_revision, module_list, **common_kwargs)

  if all or args.including_one_module:
    collect_startup_times_including_one_module(
      "StartupTimesIncludingOneModule-r%s.json" % sliver_revision, module_list, **common_kwargs)
