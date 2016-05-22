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

#ifndef __vtkAddonTestingUtilities_txx
#define __vtkAddonTestingUtilities_txx

#include "vtkAddonTestingUtilities.h"

// STD includes
#include <iostream>

namespace vtkAddonTestingUtilities
{

//----------------------------------------------------------------------------
template<typename TYPE>
bool Check(int line, const std::string& description,
           TYPE current, TYPE expected,
           const std::string& _testName,
           bool errorIfDifferent = true)
{
  std::string testName = _testName.empty() ? "Check" : _testName;
  if (errorIfDifferent)
    {
    if(current != expected)
      {
      std::cerr << "\nLine " << line << " - " << description.c_str()
                << " : " << testName.c_str() << " failed"
                << "\n\tcurrent :" << current
                << "\n\texpected:" << expected
                << std::endl;
      return false;
      }
    }
  else
    {
    if(current == expected)
      {
      std::cerr << "\nLine " << line << " - " << description.c_str()
                << " : " << testName.c_str() << " failed"
                << "\n\tcurrent :" << current
                << "\n\texpected to be different from:" << expected
                << std::endl;
      return false;
      }
    }
  return true;
}

} // namespace vtkAddonTestingUtilities

#endif
