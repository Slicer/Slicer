/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Slicer includes
#include "vtkSlicerConfigure.h"

#ifdef _WIN32

#include <shobjidl_core.h>
#include <vtksys/Encoding.hxx>
void SetAppUserModelID()
{
  // Set AppUserModelID to group Slicer windows under the same icon on the taskbar.
  // It allows pinning the Slicer application launcher to the taskbar (instead SlicerApp-real
  // which is not suitable for starting the application).
  // The same ID is set in the launcher's shortcut in SlicerCPack.cmake.
  std::wstring aumid = vtksys::Encoding::ToWide(Slicer_WIN32_APP_USER_MODEL_ID);
  SetCurrentProcessExplicitAppUserModelID(aumid.c_str());
}

#endif

#if defined (_WIN32) && !defined (Slicer_BUILD_WIN32_CONSOLE)
# include <windows.h>

int __stdcall WinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine, int nShowCmd)
{
#ifdef _WIN32
  SetAppUserModelID();
#endif
  Q_UNUSED(hInstance);
  Q_UNUSED(hPrevInstance);
  Q_UNUSED(nShowCmd);

  // CommandLineToArgvW has no narrow-character version, so we get the arguments in wide strings
  // and then convert to regular string.
  int argc;
  LPWSTR* argvStringW = CommandLineToArgvW(GetCommandLineW(), &argc);

  std::vector< const char* > argv(argc); // usual const char** array used in main() functions
  std::vector< std::string > argvString(argc); // this stores the strings that the argv pointers point to
  for(int i=0; i<argc; i++)
    {
    argvString[i] = vtksys::Encoding::ToNarrow(argvStringW[i]);
    argv[i] = argvString[i].c_str();
    }

  LocalFree(argvStringW);

  return SlicerAppMain(argc, const_cast< char** >(&argv[0]));
}
#else
int main(int argc, char *argv[])
{
#ifdef _WIN32
  SetAppUserModelID();
#endif
  return SlicerAppMain(argc, argv);
}
#endif
