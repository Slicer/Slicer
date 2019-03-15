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
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/

// vtkAddon includes
#include "vtkAddonTestingUtilities.h"

using namespace vtkAddonTestingUtilities;

//----------------------------------------------------------------------------
bool TestCheckInt();
bool TestCheckNotNull();
bool TestCheckNull();
bool TestCheckPointer();
bool TestCheckString();

//----------------------------------------------------------------------------
int vtkAddonTestingUtilitiesTest1(int , char * [] )
{
  bool res = true;
  res = res && TestCheckInt();
  res = res && TestCheckNotNull();
  res = res && TestCheckNull();
  res = res && TestCheckPointer();
  res = res && TestCheckString();
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
bool TestCheckInt()
{
  if (!CheckInt(__LINE__, "TestCheckInt", 1, 1)
      || CheckInt(__LINE__, "TestCheckInt Expected Failure", 1, -1))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckInt failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestCheckNotNull()
{
  int foo = 1;
  if (!CheckNotNull(__LINE__, "TestCheckNotNull", &foo)
      || CheckNotNull(__LINE__, "TestCheckNotNull Expected Failure", nullptr))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckNotNull failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestCheckNull()
{
  int foo = 1;
  if (!CheckNull(__LINE__, "TestCheckNull", nullptr)
      || CheckNull(__LINE__, "TestCheckNull Expected Failure", &foo))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckNull failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestCheckPointer()
{
  int foo = 1;
  int bar = 1;
  if (!CheckPointer(__LINE__, "TestCheckPointer", &foo, &foo)
      || CheckPointer(__LINE__, "TestCheckPointer Expected Failure", &foo, &bar))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckPointer failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestCheckString()
{
  const char* foo = "foo";
  const char* bar = "bar";
  if (!CheckString(__LINE__, "TestCheckString", nullptr, nullptr)
      ||!CheckString(__LINE__, "TestCheckString", foo, foo)
      || CheckString(__LINE__, "TestCheckString Expected Failure", foo, bar)
      || CheckString(__LINE__, "TestCheckString Expected Failure", foo, nullptr))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckString failed" << std::endl;
    return false;
    }
  return true;
}
