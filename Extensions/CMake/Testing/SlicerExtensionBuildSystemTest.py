from __future__ import print_function
import hashlib
import io
import json
import os
import unittest
import random
import re
import string
import subprocess
import sys
import textwrap
import threading

import http.server
import socketserver as socketserver

from urllib.parse import urlparse, parse_qs

import SlicerExtensionBuildSystemTestConfig as config

_server = None
_midas_token = 'TestTokenWithExpectedLengthxxxxxxxxxxxxx'
_requests = []

_midas_upload_query_data = {}
"""Keep track of Midas upload query parameters for each submitted Extension.
"""

def get_cmakecache_values(file_path, variables):
  result = dict.fromkeys(variables)
  with open(file_path) as cmakecache:
    for line in cmakecache:
      line = line.strip()
      if not line or line.startswith('//'):
        continue
      for variable in list(variables):
        if line.startswith(variable):
          result[variable] = str.split(line, sep='=', maxsplit=1)[1]
          variables.remove(variable)
          break
      if not len(variables):
        break
  return result

def save_request(request, response_code):
  global _requests
  _requests.append((request, response_code))
  print("do_" + request.split(' ')[0] + '[%s]' % response_code)
  sys.stdout.flush()

def get_open_port():
  """Sources:
  * http://stackoverflow.com/questions/2838244/get-open-tcp-port-in-python/2838309#2838309
  * https://www.dnorth.net/2012/03/17/the-port-0-trick/
  * https://pypi.python.org/pypi/port-for/
  """
  import socket
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.bind(("",0))
  port = s.getsockname()[1]
  return port

class Handler(http.server.BaseHTTPRequestHandler):

  def send_response_with_message(self,
      code=200, message="OK", response_type="text/html"):
    self.send_response(code)
    self.send_header('Content-Type', response_type)
    self.send_header('Content-Length', str(len(message)))
    self.send_header('Connection', 'close')
    self.end_headers()
    self.wfile.write(message.encode())

  def do_GET(self):
    if self.headers.get("Expect") == "100-continue":
      self.send_response(100)
      self.end_headers()

    response_type = "text/html"
    message="OK"

    is_midas = self.path.startswith('/midas3/api/json')
    if is_midas:
      response_type = "application/json"
      query_data = parse_qs(urlparse(self.path).query)
      if query_data['method'][0] == 'midas.login':
        # Send back token
        response_data = {
          'stat': 'ok',
          'code': '0',
          'data': {
            'token': _midas_token
          }
        }
        message = json.dumps(response_data)

    self.send_response_with_message(response_type=response_type, message=message)


  def do_PUT(self):

    if self.headers.get("Expect") == "100-continue":
      self.send_response(100)
      self.end_headers()

    length = int(self.headers['Content-Length'])
    content = None
    with io.BytesIO() as file:
      blockSize = 1024 * 1024
      bytesLeft = length
      while bytesLeft > 0:
        bytesToRead = min(blockSize, bytesLeft)
        file.write(self.rfile.read(bytesToRead))
        bytesLeft -= bytesToRead
      content = file.getvalue()

    response_type = "text/html"
    message= "OK"

    is_cdash = self.path.startswith('/submit.php')
    if is_cdash:
      hash = hashlib.md5()
      hash.update(content)
      md5sum = hash.hexdigest()

      response_type = "text/xml"
      message = """<cdash version="2.3.0">
        <status>OK</status>
        <message></message>
        <md5>%s</md5>
      </cdash>
      """ % md5sum

    is_midas = self.path.startswith('/midas3/api/json')
    if is_midas:
      response_type = "application/json"
      query_data = parse_qs(urlparse(self.path).query)
      if query_data['method'][0] == 'midas.slicerpackages.extension.upload':

        extension_name = query_data['productname'][0]

        # Keep track of method parameters
        global _midas_upload_query_data
        _midas_upload_query_data[extension_name] = query_data

        # Send back extension data
        response_data = {
          'stat': 'ok',
          'code': '0',
          'message': '',
          'data': {
            'extension': {
              'productname': extension_name
              # XXX Real server sends back all properties of extension DAO
            }
          }
        }
        message = json.dumps(response_data)

    self.send_response_with_message(response_type=response_type, message=message)

  def log_message(self, *args, **kwargs):
    request = args[1]
    response_code = args[2]
    if response_code == '100': # Do not keep track of continuation request
      return
    save_request(request, response_code)


class ServerThread(object):
  def __init__(self, port):
    self.port = port
    self.httpd = socketserver.TCPServer(('127.0.0.1', self.port), Handler)
    self.thread = threading.Thread(target=self.start, args=(self.httpd,))
    print("ServerThread: Starting using port %d" % port)
    self.thread.start()

  def start(self, httpd):
    httpd.serve_forever()

  def stop(self):
    #
    self.httpd.shutdown()
    self.httpd.server_close()
    self.thread.join(timeout=10)


class SlicerExtensionBuildSystemTest(unittest.TestCase):

  def setUp(self):
    # Set CMAKE_CFG_INTDIR
    cmake_cfg_intdir = sys.argv[1]
    if cmake_cfg_intdir == '.':
      if 'CMAKE_CONFIG_TYPE' in os.environ:
        self.assertEqual(os.environ['CMAKE_CONFIG_TYPE'], config.CMAKE_BUILD_TYPE)
      self.assertIn(config.CMAKE_BUILD_TYPE, ['Debug', 'MinSizeRel', 'Release', 'RelWithDebInfo'])
      self.assertTrue(config.CMAKE_CONFIGURATION_TYPES == "")
    else:
      self.assertIn('CMAKE_CONFIG_TYPE', os.environ)
      cmake_cfg_intdir = os.environ['CMAKE_CONFIG_TYPE']
      self.assertTrue(config.CMAKE_BUILD_TYPE == "")
      self.assertFalse(config.CMAKE_CONFIGURATION_TYPES == "")
    setattr(config, 'CMAKE_CFG_INTDIR', cmake_cfg_intdir)

    # Get an open port
    self.port = get_open_port()

    # Set URLs
    self.ctest_drop_site = "127.0.0.1:%d" % self.port
    self.midas_package_url = "http://127.0.0.1:%d/midas3" % self.port

    # Clear request lists
    global _requests
    _requests = []

    # Start mock server
    global _server
    _server = ServerThread(self.port)

    # Filter out paths starting with config.Slicer_SUPERBUILD_DIR
    # This is needed to ensure executables like svn or git installed
    # on the system resolved their expected dependencies (e.g system OpenSSL
    # libraries instead of Slicer OpenSSL libraries)
    env = dict(os.environ)
    for varname in ['LD_LIBRARY_PATH', 'PATH', 'PYTHONPATH']:
      if varname not in env:
        continue
      paths = env[varname].split(os.pathsep)
      filtered_paths = [path for path in paths if not path.startswith(config.Slicer_SUPERBUILD_DIR)]
      env[varname] = os.pathsep.join(filtered_paths)

    self.env = env

  def tearDown(self):
    if _server:
      _server.stop()

  def _remove_matching_lines(self, filepath, patterns=[]):
    """Given an input ``filepath`` and a list of ``patterns``, this function
    will update the file in place removing all lines matching any of the
    listed ``patterns``.
    """
    updated_content = []
    with open(filepath) as content:
      for line in content:
        if any([pattern in line for pattern in patterns]):
          continue
        updated_content.append(line)
    with open(filepath, 'w') as output:
      output.write("".join(updated_content))

  def _prepare_test_binary_dir(self, test_binary_dir):

    # Remove binary directory
    subprocess.check_call(
      [config.CMAKE_COMMAND, '-E', 'remove_directory', test_binary_dir],
      cwd=config.CMAKE_CURRENT_BINARY_DIR,
      env=self.env
    )

    # Create binary directory
    os.mkdir(test_binary_dir)

    # Create directory
    extension_description_dir = test_binary_dir + '/TestIndex'
    os.mkdir(extension_description_dir)

    sys.path.append(config.Slicer_SOURCE_DIR + '/Utilities/Scripts')
    from SlicerWizard import ExtensionProject

    for suffix in ['A', 'B', 'C']:
      # Generate extension sources
      subprocess.check_call(
        [sys.executable,
        config.Slicer_SOURCE_DIR + '/Utilities/Scripts/ExtensionWizard.py',
        '--create', 'TestExt%s' % suffix,
        '--addModule', 'scripted:Mod%s' % suffix,
        test_binary_dir
        ],
        cwd=config.CMAKE_CURRENT_BINARY_DIR,
        )
      extension_dir = test_binary_dir + '/TestExt%s' % suffix

      if suffix == 'A':
        # Add a failing test to ExtensionA. This allows to check that
        # dependers of ExtensionsA can still configure/build/test without
        # any issues. See #4247
        module_dir = extension_dir + '/Mod%s' % suffix
        module_cmakelists = module_dir + '/Testing/Python/CMakeLists.txt'
        with open(module_cmakelists, 'a') as content:
          content.write("add_test(NAME FailingTest COMMAND invalid_test)\n")

        display_error_script = module_dir + '/Testing/Python/slicerDisplayErrors.cmake'
        with open(display_error_script, 'w') as content:
          content.write(textwrap.dedent("""
          message("ERROR: In /path/to/Slicer/qSlicerCoreApplication.cxx, line 212
          vtkObject (0x25f2b30): This is an an error message from VTK")
          """))
        with open(module_cmakelists, 'a') as content:
          content.write("add_test(NAME TestDisplayingError COMMAND ${CMAKE_COMMAND} -P \"%s\")\n" % display_error_script)

      if suffix == 'B':
        project = ExtensionProject(extension_dir)
        project.setValue('EXTENSION_DEPENDS', 'TestExtA')
        project.save()

      if suffix == 'C':
        project = ExtensionProject(extension_dir)
        project.setValue('EXTENSION_DEPENDS', 'TestExtA TestExtB')
        project.save()

      # Generate extension description file
      description = subprocess.check_output(
        [sys.executable,
        config.Slicer_SOURCE_DIR + '/Utilities/Scripts/ExtensionWizard.py',
        '--localExtensionsDir=%s' % test_binary_dir,
        '--describe', extension_dir,
        ],
        cwd=config.CMAKE_CURRENT_BINARY_DIR,
        ).decode()
      with open(extension_description_dir + '/TestExt%s.s4ext' % suffix, 'w') as description_file:
        description_file.write(description)

      if suffix == 'A':
        #
        # Strip all optional metadata from Extension A. This allows to check
        # that the full process can effectively succeed without them.
        # See issue #4276
        #
        # Extension A is chosen because it has no dependency. This mean we
        # can even strip the DEPENDS option.
        #
        # Finally, note this is done after generating the description file
        # because the ExtensionWizard currently expects them to be set.
        #
        # The list of optional metadata has been copied from
        # SlicerExtensionDescriptionSpec.cmake CMake module.
        #
        patterns = ['EXTENSION_' + pattern for pattern in [
          'SVNUSERNAME', 'SVNPASSWORD', 'DEPENDS', 'BUILD_SUBDIRECTORY',
          'HOMEPAGE', 'CONTRIBUTORS', 'CATEGORY', 'ICONURL',
          'DESCRIPTION', 'SCREENSHOTURLS', 'ENABLED', 'STATUS'
          ]]
        self._remove_matching_lines(extension_dir + '/CMakeLists.txt', patterns)

    # Add extension description file with invalid repository
    description_file = extension_description_dir + '/TestExtInvalidSCM.s4ext'
    print("Writing %s" % description_file)
    with open(description_file, 'w') as description_file:
      description_file.write(textwrap.dedent(
        """
        scm git
        scmurl git://github.com/Slicer/ExtensionThatDoNotExists
        scmrevision abcdefg
        depends TestExtA TestExtB TestExtC
        """))


  def test_index_build_with_upload(self):
    self._test_index_build('build_with_upload', True)

  def test_index_build_with_upload_using_ctest(self):
    self._test_index_build_using_ctest('build_with_upload_using_ctest', True)

  def test_index_build_without_upload(self):
    self._test_index_build('build_without_upload', False)

  def test_index_build_without_upload_using_ctest(self):
    self._test_index_build_using_ctest('build_without_upload_using_ctest', False)

  def _test_index_build_using_ctest(self, test_name, test_upload):
    test_binary_dir = config.CMAKE_CURRENT_BINARY_DIR + '/' + test_name + '-build'

    self._prepare_test_binary_dir(test_binary_dir)

    build_config = config.CMAKE_BUILD_TYPE
    if config.CMAKE_CONFIGURATION_TYPES:
      build_config = config.CMAKE_CFG_INTDIR

    # XXX This will return CL, GCC or CLANG. Ideally the version should
    #     be included.
    compiler_name = os.path.splitext(os.path.basename(config.CMAKE_CXX_COMPILER))[0].upper()

    cmake_osx_architectures = (
      'CMAKE_OSX_ARCHITECTURES=' + config.CMAKE_OSX_ARCHITECTURES
      if config.CMAKE_OSX_ARCHITECTURES.strip() else ''
    )

    content = \
"""
set(MY_OPERATING_SYSTEM   "{operating_system}")
set(MY_COMPILER           "{compiler_name}")
set(MY_QT_VERSION         "{qt_version}")
set(CTEST_SITE            "testscript.kitware")
set(CTEST_DASHBOARD_ROOT  "{test_binary_dir}")
set(CTEST_CMAKE_GENERATOR "{cmake_generator}")
set(CTEST_CMAKE_GENERATOR_PLATFORM "{cmake_generator_platform}")
set(CTEST_CMAKE_GENERATOR_TOOLSET "{cmake_generator_toolset}")
set(MY_BITNESS            "64")
set(EXTENSIONS_BUILDSYSTEM_TESTING TRUE)
set(CTEST_BUILD_CONFIGURATION "{ctest_build_configuration}")
set(CTEST_TEST_TIMEOUT 500)
set(CTEST_BUILD_FLAGS "")
set(CTEST_PARALLEL_LEVEL 8)
set(CTEST_DROP_SITE "{ctest_drop_site}")
set(MIDAS_PACKAGE_URL "{midas_package_url}")

set(Slicer_UPLOAD_EXTENSIONS "{slicer_upload_extensions}")

set(SCRIPT_MODE "experimental")

set(ADDITIONAL_CMAKECACHE_OPTION "
CTEST_DROP_SITE:STRING=${{CTEST_DROP_SITE}}
Slicer_EXTENSION_DESCRIPTION_DIR:PATH={extension_description_dir}
Slicer_LOCAL_EXTENSIONS_DIR:PATH={local_extensions_dir}
CMAKE_C_COMPILER:PATH={cmake_c_compiler}
CMAKE_CXX_COMPILER:PATH={cmake_cxx_compiler}
CMAKE_OSX_DEPLOYMENT_TARGET={cmake_osx_deployment_target}
{cmake_osx_architectures}
CMAKE_OSX_SYSROOT={cmake_osx_sysroot}
")

set(CTEST_GIT_COMMAND "{git_executable}")
set(CTEST_SVN_COMMAND "{svn_executable}")

set(EXTENSIONS_TRACK_QUALIFIER "master") # "master", 4.4, ...
set(EXTENSIONS_INDEX_GIT_TAG "origin/${{EXTENSIONS_TRACK_QUALIFIER}}") # origin/master, origin/4.4, ...

set(EXTENSIONS_INDEX_GIT_REPOSITORY git://github.com/jcfr/ExtensionsIndex.git)

set(BUILD_OPTIONS_STRING "${{MY_BITNESS}}bits-Qt${{MY_QT_VERSION}}-PythonQt-With-Tcl-CLI")

set(dir_suffix ${{BUILD_OPTIONS_STRING}}-${{CTEST_BUILD_CONFIGURATION}}-${{SCRIPT_MODE}})

set(Slicer_DIR "{slicer_dir}")

set(testing_suffix "")
if(EXTENSIONS_BUILDSYSTEM_TESTING)
  set(testing_suffix "-Testing")
endif()
#set(CTEST_BINARY_DIRECTORY "${{CTEST_DASHBOARD_ROOT}}/SlicerExtensions-build-${{dir_suffix}}${{testing_suffix}}-${{EXTENSIONS_TRACK_QUALIFIER}}")
set(CTEST_BINARY_DIRECTORY "S-E")

set(EXTENSIONS_BUILDSYSTEM_SOURCE_DIRECTORY "{slicer_source_dir}/Extensions/CMake")

set(CTEST_NOTES_FILES "${{CTEST_SCRIPT_DIRECTORY}}/${{CTEST_SCRIPT_NAME}}")

set(CTEST_PROJECT_NAME "SlicerPreview")
set(CTEST_BUILD_NAME "${{MY_OPERATING_SYSTEM}}-${{MY_COMPILER}}-${{BUILD_OPTIONS_STRING}}-${{EXTENSIONS_INDEX_GIT_TAG}}-${{CTEST_BUILD_CONFIGURATION}}")

include({slicer_source_dir}/Extensions/CMake/SlicerExtensionsDashboardDriverScript.cmake)
""".format(
      cmake_generator=config.CMAKE_GENERATOR,
      cmake_generator_platform=config.CMAKE_GENERATOR_PLATFORM,
      cmake_generator_toolset=config.CMAKE_GENERATOR_TOOLSET,
      cmake_osx_deployment_target=config.CMAKE_OSX_DEPLOYMENT_TARGET,
      cmake_osx_architectures=cmake_osx_architectures,
      cmake_osx_sysroot=config.CMAKE_OSX_SYSROOT,
      compiler_name=compiler_name,
      ctest_drop_site=self.ctest_drop_site,
      ctest_build_configuration=build_config,
      extension_description_dir=test_binary_dir + '/TestIndex',
      git_executable=config.GIT_EXECUTABLE,
      local_extensions_dir=test_binary_dir,
      cmake_c_compiler=config.CMAKE_C_COMPILER,
      cmake_cxx_compiler=config.CMAKE_CXX_COMPILER,
      midas_package_url=self.midas_package_url,
      slicer_upload_extensions='1' if test_upload else '0',
      operating_system=config.Slicer_OS,
      qt_version=config.QT_VERSION,
      slicer_dir=config.Slicer_BINARY_DIR,
      slicer_source_dir=config.Slicer_SOURCE_DIR,
      svn_executable=config.Subversion_SVN_EXECUTABLE,
      test_binary_dir=test_binary_dir
    )

    script_file_path = test_binary_dir + '/build_index.cmake'
    with open(script_file_path, 'w') as file:
      file.write(content)

    # Run CTest
    cmd = [config.CMAKE_CTEST_COMMAND, '-C', build_config,'-S', script_file_path, '-VV']
    subprocess.check_call(
      cmd, cwd=test_binary_dir, env=self.env
    )

    # Check
    self._check_caches(test_binary_dir + '/S-E')
    self._check_queries(test_upload, with_ctest=True)
    self._check_midas_upload_query_parameters(test_upload, with_ctest=True)

  def _test_index_build(self, test_name, test_upload):

    test_binary_dir = config.CMAKE_CURRENT_BINARY_DIR + '/' + test_name + '-build'

    self._prepare_test_binary_dir(test_binary_dir)

    cmake_osx_architectures = (
      '-DCMAKE_OSX_ARCHITECTURES:STRING=' + config.CMAKE_OSX_ARCHITECTURES
      if config.CMAKE_OSX_ARCHITECTURES.strip() else ''
    )

    # Prepare configure command
    cmd = [
      config.CMAKE_COMMAND,
      '-G', config.CMAKE_GENERATOR,
      '-DSlicer_DIR:PATH=' + config.Slicer_BINARY_DIR,
      '-DSlicer_EXTENSION_DESCRIPTION_DIR:PATH=' + test_binary_dir + '/TestIndex',
      '-DSlicer_LOCAL_EXTENSIONS_DIR:PATH=' + test_binary_dir,
     ' -DBUILD_TESTING:BOOL=0',
      '-DCMAKE_C_COMPILER:PATH=' + config.CMAKE_C_COMPILER,
      '-DCMAKE_CXX_COMPILER:PATH=' + config.CMAKE_CXX_COMPILER,
      '-DGIT_EXECUTABLE:PATH=' + config.GIT_EXECUTABLE,
      '-DSubversion_SVN_EXECUTABLE:PATH=' + config.Subversion_SVN_EXECUTABLE,
      '-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=' + config.CMAKE_OSX_DEPLOYMENT_TARGET,
      cmake_osx_architectures,
      '-DCMAKE_OSX_SYSROOT:PATH=' + config.CMAKE_OSX_SYSROOT
      ]

    if not config.CMAKE_CONFIGURATION_TYPES:
      cmd.append('-DCMAKE_BUILD_TYPE:STRING=' + config.CMAKE_BUILD_TYPE)

    if config.CMAKE_GENERATOR_PLATFORM:
      cmd.extend(['-A', config.CMAKE_GENERATOR_PLATFORM])

    if config.CMAKE_GENERATOR_TOOLSET:
      cmd.extend(['-T', config.CMAKE_GENERATOR_TOOLSET])

    cmd.append('-DSlicer_UPLOAD_EXTENSIONS:BOOL=%s' % ('1' if test_upload else '0'))
    cmd.append('-DCTEST_DROP_SITE:STRING=%s' % self.ctest_drop_site)

    if test_upload:
      cmd.append('-DMIDAS_PACKAGE_URL:STRING=%s' % self.midas_package_url)

    cmd.append(config.Slicer_SOURCE_DIR + '/Extensions/CMake')

    # Configure
    subprocess.check_call(
      cmd,
      cwd=test_binary_dir,
      env=self.env
    )

    # Build
    cmd = [config.CMAKE_COMMAND, '--build', test_binary_dir]
    if config.CMAKE_CONFIGURATION_TYPES:
      cmd.extend(['--config', config.CMAKE_CFG_INTDIR])
    subprocess.check_call(
      cmd, cwd=test_binary_dir, env=self.env
    )

    # Check
    self._check_caches(test_binary_dir)
    self._check_queries(test_upload, with_ctest=False)
    self._check_midas_upload_query_parameters(test_upload, with_ctest=False)

  def _check_caches(self, test_binary_dir):

    # Check extension caches
    for extensionName in ['TestExtA', 'TestExtB', 'TestExtC']:

      # Read cache
      varname = '%s_BUILD_SLICER_EXTENSION' % extensionName
      cmakecache = test_binary_dir + '/' + extensionName + '-build/CMakeCache.txt'
      cache_values = get_cmakecache_values(cmakecache, [varname])

      # Check <ExtensionName>_BUILD_SLICER_EXTENSION variable
      self.assertIn(cache_values[varname], ['1', 'ON', 'TRUE'])

    # Read TestExtC cache
    cmakecache = test_binary_dir + '/TestExtC-build/CMakeCache.txt'
    cache_values = get_cmakecache_values(cmakecache, [
      'EXTENSION_DEPENDS',
      'TestExtA_DIR',
      'TestExtB_DIR',
      ])

    # Check EXTENSION_DEPENDS variable
    self.assertEqual(cache_values['EXTENSION_DEPENDS'], 'TestExtA;TestExtB')

    # Check <extensionName>_DIR variables
    self.assertIsNotNone(cache_values['TestExtA_DIR'])
    self.assertTrue(os.path.exists(cache_values['TestExtA_DIR']))
    self.assertIsNotNone(cache_values['TestExtB_DIR'])
    self.assertTrue(os.path.exists(cache_values['TestExtB_DIR']))

    # Check TestExtC generated description file
    cmd = [
      config.CMAKE_COMMAND,
      '-DSlicer_EXTENSIONS_CMAKE_DIR:PATH=' + config.Slicer_SOURCE_DIR + '/Extensions/CMake',
      '-DTestExtC_BUILD_DIR:PATH=' + test_binary_dir + '/TestExtC-build/',
      '-P', config.CMAKE_CURRENT_SOURCE_DIR + '/CheckGeneratedDescriptionFiles.cmake'
      ]
    subprocess.check_call(
      cmd, cwd=test_binary_dir, env=self.env
    )

  def _check_queries(self, test_upload, with_ctest=False):

    def parse_request(result):
      (request, response_code) = result
      self.assertEqual(response_code, '200')
      (method, raw_query, _) = request.split(' ')
      query_data = parse_qs(urlparse(raw_query).query)
      return (method, query_data)

    def check_cdash_request(parsed_request, expected_http_method, expected_filename_regex):
      http_method = parsed_request[0]
      query_data = parsed_request[1]
      self.assertEqual(http_method, expected_http_method)
      self.assertEqual(query_data['project'][0], 'SlicerPreview')
      self.assertIsNotNone(re.match(expected_filename_regex, query_data['FileName'][0]))

    def check_midas_request(parsed_request, expected_http_method, expected_midas_method, expected_params={}):
      http_method = parsed_request[0]
      query_data = parsed_request[1]
      self.assertEqual(http_method, expected_http_method)
      self.assertEqual(query_data['method'][0], expected_midas_method)
      for expected_key, expected_value in expected_params.items():
        self.assertEqual(query_data[expected_key][0], expected_value)

    requests = iter(_requests)

    # Check number of requests
    expected_request_count = 0

    # Top-level configure/build/notes
    if with_ctest:
      expected_request_count += 3

    # Configure/Build/Test/Packaging for TestExtA, TestExtB and TestExtC
    expected_request_count += 4 * 3

    # Configure for TestExtInvalidSCM
    expected_request_count += 1

    # Upload to Midas and CDash for TestExtA, TestExtB and TestExtC
    if test_upload:
      expected_request_count += 3 * 3

    self.assertEqual(len(_requests), expected_request_count)

    # Check CDash and Midas queries
    if with_ctest:
      # Upload top-level configure results to CDash
      check_cdash_request(parse_request(next(requests)), 'PUT', r'.+Configure\.xml')

    for extensionName in ['TestExtA', 'TestExtB', 'TestExtC', 'TestExtInvalidSCM']:

      # Upload configure/build/test results to CDash
      check_cdash_request(parse_request(next(requests)), 'PUT', r'.+' + extensionName + r'.+Configure\.xml')
      if extensionName == 'TestExtInvalidSCM':
        continue
      check_cdash_request(parse_request(next(requests)), 'PUT', r'.+' + extensionName + r'.+Build\.xml')
      check_cdash_request(parse_request(next(requests)), 'PUT', r'.+' + extensionName + r'.+Test\.xml')

      if test_upload:
        # Upload package to midas
        check_midas_request(parse_request(next(requests)), 'GET', 'midas.login')
        check_midas_request(parse_request(next(requests)), 'PUT', 'midas.slicerpackages.extension.upload', {'productname': extensionName})

      # Upload packaging result to CDash
      check_cdash_request(parse_request(next(requests)), 'PUT', r'.+' + extensionName + r'.+Build\.xml')

      if test_upload:
        # Upload url to CDash
        check_cdash_request(parse_request(next(requests)), 'PUT', r'.+' + extensionName + r'.+Upload\.xml')

    if with_ctest:
      # Upload top-level build results and notes to CDash
      check_cdash_request(parse_request(next(requests)), 'PUT', r'.+Build\.xml')
      check_cdash_request(parse_request(next(requests)), 'PUT', r'.+Notes\.xml')

  def _check_midas_upload_query_parameters(self, test_upload, with_ctest=False):
    if not test_upload:
      for extensionName in ['TestExtA', 'TestExtB', 'TestExtC']:
        self.assertTrue(extensionName not in _midas_upload_query_data)

    else:

      def _expected_parameters(extensionName):
        expected = {
          'arch': ['amd64'],
          'codebase': ['Slicer4'],
          'method': ['midas.slicerpackages.extension.upload'],
          'name': None, # ['25430-linux-amd64-TestExtB-local0-0000-00-00.tar.gz'],
          'os': None, # ['linux'],
          'packagetype': ['archive'],
          'productname': [extensionName],
          'repository_type': ['local'],
          'repository_url': ['NA'],
          'revision': ['0'],
          'slicer_revision': None, #['25430'],
          'submissiontype': ['experimental'],
          'token': ['TestTokenWithExpectedLengthxxxxxxxxxxxxx']
          }
        if extensionName != 'TestExtA':
          expected.update({
            'category': ['Examples'],
            'contributors': ['John Doe (AnyWare Corp.)'],
            'description': ['This is an example of a simple extension'],
            'enabled': ['1'],
            'homepage': ['https://www.slicer.org/wiki/Documentation/Nightly/Extensions/%s' % extensionName],
            'icon_url': ['http://www.example.com/Slicer/Extensions/%s.png' % extensionName],
            'screenshots': ['http://www.example.com/Slicer/Extensions/%s/Screenshots/1.png' % extensionName],
          })
        else:
          # XXX We should have a way to get the default value from
          #     SlicerExtensionDescriptionSpec.cmake
          # XXX Empty parameter are not sent to the server
          expected.update({
            #'category': [''],
            #'contributors': [''],
            #'description': [''],
            'enabled': ['1'],
            #'homepage': [''],
            #'icon_url': [''],
            #'screenshots': [''],
          })
        return expected

      for extensionName in ['TestExtA', 'TestExtB', 'TestExtC']:
        query_data = _midas_upload_query_data[extensionName]
        expected_query_data = _expected_parameters(extensionName)

        self.assertEqual(sorted(query_data.keys()), sorted(expected_query_data.keys()))

        for key, expected_value in expected_query_data.items():
          current_value = query_data[key]
          if expected_value is None:
            # XXX For now only consider values that do not change with platform
            # and Slicer revision.
            continue
          self.assertEqual(current_value, expected_value)
