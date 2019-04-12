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

#
#  W A R N I N G
#  -------------
#
# This file is not part of the Slicer API.  It exists purely as an
# implementation detail.  This python module may change from version to
# version without notice, or even be removed.
#
# We mean it.
#

from __future__ import print_function

import os
import pipes
import subprocess
import sys
import time

__all__ = ['EXIT_FAILURE', 'EXIT_SUCCESS', 'run', 'runSlicer', 'runSlicerAndExit', 'timecall']

EXIT_FAILURE=1
EXIT_SUCCESS=0

def dropcache():
  if sys.platform in ["linux", "linux2"]:
    run('/usr/bin/sudo', ['sysctl', 'vm.drop_caches=1'], drop_cache=False)
  else:
    # XXX Implement other platform (Windows: EmptyStandbyList ?, MacOSX: Purge ?)
    raise Exception("--drop-cache is not supported on %s" % sys.platform)

def run(executable, arguments=[], verbose=True, shell=False, drop_cache=False):
  """Run ``executable`` with provided ``arguments``.
  """
  if drop_cache:
    dropcache()
  if verbose:
    print("%s %s" % (os.path.basename(executable), " ".join([pipes.quote(arg) for arg in arguments])))
  arguments.insert(0, executable)
  if shell:
    arguments = " ".join([pipes.quote(arg) for arg in arguments])
  p = subprocess.Popen(args=arguments, stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE, shell=shell)
  stdout, stderr = p.communicate()

  if p.returncode != EXIT_SUCCESS:
    print('STDERR: ' + stderr.decode(), file=sys.stderr)

  return (p.returncode, stdout.decode(), stderr.decode())

def runSlicer(slicer_executable, arguments=[], verbose=True, **kwargs):
  """Run ``slicer_executable`` with provided ``arguments``.
  """
  args = ['--no-splash']
  args.extend(arguments)
  return run(slicer_executable, args, verbose, **kwargs)

def runSlicerAndExit(slicer_executable, arguments=[], verbose=True, **kwargs):
  """Run ``slicer_executable`` with provided ``arguments`` and exit.
  """
  args = ['--exit-after-startup']
  args.extend(arguments)
  return runSlicer(slicer_executable, args, verbose, **kwargs)

def timecall(method, **kwargs):
  """Wrap ``method`` and return its execution time.
  """
  repeat = 1
  if 'repeat' in kwargs:
    repeat = kwargs['repeat']
  def wrapper(*args, **kwargs):
    durations = []
    for iteration in range(1, repeat + 1):
      start = time.time()
      result = method(*args, **kwargs)
      durations.append(time.time() - start)
      print("{:d}/{:d}: {:.2f}s".format(iteration, repeat, durations[-1]))
    average = sum(durations) / len(durations)
    print("Average: {:.2f}s\n".format(average))
    duration = average
    return (duration, result)
  return wrapper
