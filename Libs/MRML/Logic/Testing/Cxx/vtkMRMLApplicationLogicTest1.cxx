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

// MRMLLogic includes
#include "vtkMRMLApplicationLogic.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <string>

//-----------------------------------------------------------------------------
int vtkMRMLApplicationLogicTest1(int , char * [])
{
  //-----------------------------------------------------------------------------
  // Test void vtkMRMLApplicationLogic::SetTemporaryPath(const char* path);
  // Test const char* vtkMRMLApplicationLogic::GetTemporaryPath();
  //-----------------------------------------------------------------------------

    vtkNew<vtkMRMLApplicationLogic> MRMLAppLogic;

    MRMLAppLogic->SetTemporaryPath(NULL); // Test NULL
    if (strlen(MRMLAppLogic->GetTemporaryPath()) != 0)
      {
      std::string temporaryEmptyString;
      std::cerr << "Line " << __LINE__ << " - Problem with SetTemporaryPath using NULL" << "\n"
                << "\ttemporaryPath:" << MRMLAppLogic->GetTemporaryPath() << "\n"
                << "\texpected:" << temporaryEmptyString << std::endl;
      return EXIT_FAILURE;
      }

    typedef std::vector<std::string> TestDataType;
    TestDataType data;
    data.push_back("/tmp/slicer");
    data.push_back("/tmp/slicer/test");
    data.push_back("/tmp/slicer/test"); // Test same path as before
    data.push_back("c:\\path\\to\\temp\\slicer");
    data.push_back(""); // Test empty string

    for(TestDataType::size_type rowIdx = 0; rowIdx < data.size(); ++rowIdx)
      {
      std::string temporaryPath(data.at(rowIdx));
      MRMLAppLogic->SetTemporaryPath(temporaryPath.c_str());
      if (MRMLAppLogic->GetTemporaryPath() != temporaryPath)
        {
        std::cerr << "Line " << __LINE__ << " - Problem with SetTemporaryPath using " << temporaryPath << " :\n"
                  << "\ttemporaryPath:" << MRMLAppLogic->GetTemporaryPath() << "\n"
                  << "\texpected:" << temporaryPath << std::endl;
        return EXIT_FAILURE;
        }
      }

    return EXIT_SUCCESS;
}
