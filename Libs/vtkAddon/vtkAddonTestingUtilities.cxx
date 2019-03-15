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

#include "vtkAddonTestingUtilities.h"

// STD includes
#include <iostream>

namespace vtkAddonTestingUtilities
{

//----------------------------------------------------------------------------
bool CheckInt(int line, const std::string& description,
              int current, int expected)
{
  return Check<int>(line, description, current, expected, "CheckInt");
}

//----------------------------------------------------------------------------
bool CheckDoubleTolerance(int line, const std::string& description,
  double current, double expected, double tolerance)
{
  if (fabs(current - expected) > tolerance)
    {
    std::cerr << "\nLine " << line << " - " << description.c_str()
      << " :  CheckDoubleTolerance failed"
      << "\n\tcurrent:  " << current
      << "\n\texpected: " << expected
      << "\n\tdifference: " << current-expected
      << "\n\ttolerance: " << tolerance
      << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckNotNull(int line, const std::string& description,
                  const void* pointer)
{
  if(!pointer)
    {
    std::cerr << "\nLine " << line << " - " << description.c_str()
              << " : CheckNotNull failed"
              << "\n\tpointer:" << pointer
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckNull(int line, const std::string& description, const void* pointer)
{
  if(pointer)
    {
    std::cerr << "\nLine " << line << " - " << description.c_str()
              << " : CheckNull failed"
              << "\n\tpointer:" << pointer
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckPointer(int line, const std::string& description,
                  void* current, void* expected, bool errorIfDifferent /* = true */)
{
  return Check<void*>(line, description, current, expected, "CheckPointer", errorIfDifferent);
}

//----------------------------------------------------------------------------
bool CheckString(int line, const std::string& description,
                 const char* current, const char* expected, bool errorIfDifferent /* = true */)
{
  std::string testName = "CheckString";

  bool different = true;
  if (current == nullptr || expected == nullptr)
    {
    different = !(current == nullptr && expected == nullptr);
    }
  else if(strcmp(current, expected) == 0)
    {
    different = false;
    }
  if(different == errorIfDifferent)
    {
    std::cerr << "\nLine " << line << " - " << description.c_str()
              << " : " << testName.c_str() << "  failed"
              << "\n\tcurrent :" << (current ? current : "<null>")
              << "\n\texpected:" << (expected ? expected : "<null>")
              << std::endl;
    return false;
    }
  return true;
}

} // namespace vtkAddonTestingUtilities
