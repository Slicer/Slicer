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
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLCoreTestingMacros.h"

// VTK includes
#include <vtkNew.h>

//-----------------------------------------------------------------------------
int vtkSlicerApplicationLogicTest1(int , char * [])
{
  //-----------------------------------------------------------------------------
  // Test GetModuleShareDirectory(const std::string& moduleName, const std::string& filePath);
  //-----------------------------------------------------------------------------
  std::string inputModulePath1("/path/to/Slicer-Superbuild/Slicer-build/lib/Slicer-X.Y/qt-loadable-modules/libqSlicerVolumeRenderingModule.so");
  std::string inputModulePath2("c:\\path\\to\\Slicer-Superbuild\\Slicer-build\\lib\\Slicer-X.Y\\qt-loadable-modules\\Release\\qSlicerVolumeRenderingModule.dll");
  std::string expectedModuleSlicerXYShareDirectory1("/path/to/Slicer-Superbuild/Slicer-build/share/Slicer-X.Y");
  std::string expectedModuleSlicerXYShareDirectory2("c:/path/to/Slicer-Superbuild/Slicer-build/share/Slicer-X.Y");
  {
    std::string expectedModuleShareDirectory = expectedModuleSlicerXYShareDirectory1 + "/qt-loadable-modules/VolumeRendering";
    std::string currentModuleShareDirectory = vtkSlicerApplicationLogic::GetModuleShareDirectory("VolumeRendering", inputModulePath1);
    if (currentModuleShareDirectory != expectedModuleShareDirectory)
      {
      std::cerr << "Line " << __LINE__ << " - Failed to compute module share directory !\n"
                << "\texpected:" << expectedModuleShareDirectory << "\n"
                << "\tcurrent:" << currentModuleShareDirectory << std::endl;
      return EXIT_FAILURE;
      }

    expectedModuleShareDirectory = expectedModuleSlicerXYShareDirectory2 + "/qt-loadable-modules/VolumeRendering";
    currentModuleShareDirectory = vtkSlicerApplicationLogic::GetModuleShareDirectory("VolumeRendering", inputModulePath2);
    if (currentModuleShareDirectory != expectedModuleShareDirectory)
      {
      std::cerr << "Line " << __LINE__ << " - Failed to compute module share directory !\n"
                << "\texpected:" << expectedModuleShareDirectory << "\n"
                << "\tcurrent:" << currentModuleShareDirectory << std::endl;
      return EXIT_FAILURE;
      }
  }

  //-----------------------------------------------------------------------------
  // Test GetModuleSlicerXYShareDirectory(const std::string& filePath);
  //-----------------------------------------------------------------------------
  {
    std::string expectedModuleShareDirectory = expectedModuleSlicerXYShareDirectory1;
    std::string currentModuleShareDirectory = vtkSlicerApplicationLogic::GetModuleSlicerXYShareDirectory(inputModulePath1);
    if (currentModuleShareDirectory != expectedModuleShareDirectory)
      {
      std::cerr << "Line " << __LINE__ << " - Failed to compute module SlicerXY share directory !\n"
                << "\texpected:" << expectedModuleShareDirectory << "\n"
                << "\tcurrent:" << currentModuleShareDirectory << std::endl;
      return EXIT_FAILURE;
      }

    expectedModuleShareDirectory = expectedModuleSlicerXYShareDirectory2;
    currentModuleShareDirectory = vtkSlicerApplicationLogic::GetModuleSlicerXYShareDirectory(inputModulePath2);
    if (currentModuleShareDirectory != expectedModuleShareDirectory)
      {
      std::cerr << "Line " << __LINE__ << " - Failed to compute module SlicerXY share directory !\n"
                << "\texpected:" << expectedModuleShareDirectory << "\n"
                << "\tcurrent:" << currentModuleShareDirectory << std::endl;
      return EXIT_FAILURE;
      }
  }

  //-----------------------------------------------------------------------------
  // Test GetModuleSlicerXYLibDirectory(const std::string& filePath);
  //-----------------------------------------------------------------------------
  {
    std::string expectedModuleSlicerXYLibDirectory1("/path/to/Slicer-Superbuild/Slicer-build/lib/Slicer-X.Y");
    std::string expectedModuleSlicerXYLibDirectory2("c:/path/to/Slicer-Superbuild/Slicer-build/lib/Slicer-X.Y");

    std::string expectedModuleLibDirectory = expectedModuleSlicerXYLibDirectory1;
    std::string currentModuleLibDirectory = vtkSlicerApplicationLogic::GetModuleSlicerXYLibDirectory(inputModulePath1);
    if (currentModuleLibDirectory != expectedModuleLibDirectory)
      {
      std::cerr << "Line " << __LINE__ << " - Failed to compute module SlicerXY lib directory !\n"
                << "\texpected:" << expectedModuleLibDirectory << "\n"
                << "\tcurrent:" << currentModuleLibDirectory << std::endl;
      return EXIT_FAILURE;
      }

    expectedModuleLibDirectory = expectedModuleSlicerXYLibDirectory2;
    currentModuleLibDirectory = vtkSlicerApplicationLogic::GetModuleSlicerXYLibDirectory(inputModulePath2);
    if (currentModuleLibDirectory != expectedModuleLibDirectory)
      {
      std::cerr << "Line " << __LINE__ << " - Failed to compute module SlicerXY lib directory !\n"
                << "\texpected:" << expectedModuleLibDirectory << "\n"
                << "\tcurrent:" << currentModuleLibDirectory << std::endl;
      return EXIT_FAILURE;
      }
  }

  //-----------------------------------------------------------------------------
  // Test IsEmbeddedModule(const std::string& filePath, const std::string& applicationHomeDir);
  //-----------------------------------------------------------------------------
  {
    // The following vector contains rows where each row has three items:
    //   - filePath
    //   - applicationHomeDir
    //   - isEmbeddedExpected
    typedef std::vector<std::string> TestRowType;
    typedef std::vector< TestRowType > TestDataType;
    TestDataType data;
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/MRIChangeDetector-build/lib/Slicer-X.Y/qt-scripted-modules/MRIChangeDetector.pyc");
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build");
      row.push_back("0");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build/lib/Slicer-X.Y/qt-scripted-modules/MRIChangeDetector.pyc");
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build");
      row.push_back("1");
      data.push_back(row);
    }
    for(TestDataType::size_type rowIdx = 0; rowIdx < data.size(); ++rowIdx)
      {
      std::string filePath(data.at(rowIdx).at(0));
      std::string applicationHomeDir(data.at(rowIdx).at(1));
      std::string isEmbeddedExpectedAsStr(data.at(rowIdx).at(2));
      bool isEmbeddedExpected = (isEmbeddedExpectedAsStr == "1");

      bool isEmbedded = vtkSlicerApplicationLogic::IsEmbeddedModule(filePath, applicationHomeDir);
      if (isEmbeddedExpected != isEmbedded)
        {
        std::cerr << "Line " << __LINE__ << " - Problem with isEmbedded ! - Row:" << rowIdx << "\n"
                  << "\tfilePath:" << filePath << ", applicationHomeDir: " << applicationHomeDir << "\n"
                  << "\texpected:" << isEmbeddedExpected << "\n"
                  << "\tcurrent:" << isEmbedded << std::endl;
        return EXIT_FAILURE;
        }
      }
  }

  return EXIT_SUCCESS;
}

