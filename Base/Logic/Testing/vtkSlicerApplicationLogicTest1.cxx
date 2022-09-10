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
#include "vtkSlicerConfigure.h"

// Slicer MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLModelHierarchyNode.h"

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
  std::string inputModulePath3("c:\\VolumeRendering\\qSlicerVolumeRenderingModule.dll");
  std::string expectedModuleSlicerXYShareDirectory1("/path/to/Slicer-Superbuild/Slicer-build/share/Slicer-X.Y");
  std::string expectedModuleSlicerXYShareDirectory2("c:/path/to/Slicer-Superbuild/Slicer-build/share/Slicer-X.Y");
  std::string expectedModuleSlicerXYShareDirectory3("");
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

    expectedModuleShareDirectory = expectedModuleSlicerXYShareDirectory3;
    currentModuleShareDirectory = vtkSlicerApplicationLogic::GetModuleShareDirectory("VolumeRendering", inputModulePath3);
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
    //   - slicerRevision
    //   - isEmbeddedExpected
    typedef std::vector<std::string> TestRowType;
    typedef std::vector< TestRowType > TestDataType;
    TestDataType data;
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/MRIChangeDetector-build/lib/Slicer-X.Y/qt-scripted-modules/MRIChangeDetector.pyc");
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build");
      row.push_back("4810");
      row.push_back("0");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build/lib/Slicer-X.Y/qt-scripted-modules/MRIChangeDetector.pyc");
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build");
      row.push_back("4810");
      row.push_back("1");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/Slicer.app/Contents/lib/Slicer-4.1/qt-loadable-modules/libqSlicerMarkupsModule.dylib");
      row.push_back("/home/jchris/Projects/Slicer.app/Contents");
      row.push_back("4810");
      row.push_back("1");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/Something.app/Contents/lib/Slicer-4.1/qt-loadable-modules/libqSlicerMarkupsModule.dylib");
      row.push_back("/home/jchris/Projects/Something.app/Contents");
      row.push_back("4810");
      row.push_back("1");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/" Slicer_BUNDLE_LOCATION "/lib/Slicer-4.1/qt-loadable-modules/libqSlicerMarkupsModule.dylib");
      row.push_back("/home/jchris/Projects/" Slicer_BUNDLE_LOCATION);
      row.push_back("4810");
      row.push_back("1");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build/bin/Slicer.app/Contents/Extensions-4810/Reporting/lib/Slicer-4.1/qt-loadable-modules/Python/vtkSlicerReportingModuleLogic.py");
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build");
      row.push_back("4810");
      row.push_back("0");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build/bin/" Slicer_BUNDLE_LOCATION "/Extensions-4810/Reporting/lib/Slicer-4.1/qt-loadable-modules/Python/vtkSlicerReportingModuleLogic.py");
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build");
      row.push_back("4810");
      row.push_back("0");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build/bin/Foo.app/Contents/Extensions-4810/Reporting/lib/Slicer-4.1/qt-loadable-modules/Python/vtkSlicerReportingModuleLogic.py");
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build");
      row.push_back("4810");
      row.push_back("0");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build/bin/Slicer.app/Contents/Extensions-4810/Reporting/lib/Slicer-4.1/qt-loadable-modules/Python/vtkSlicerReportingModuleLogic.py");
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build");
      row.push_back("4811");
      row.push_back("1");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build/bin/" Slicer_BUNDLE_LOCATION "/Extensions-4810/Reporting/lib/Slicer-4.1/qt-loadable-modules/Python/vtkSlicerReportingModuleLogic.py");
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build");
      row.push_back("4811");
      row.push_back("1");
      data.push_back(row);
    }
    {
      TestRowType row;
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build/bin/Foo.app/Contents/Extensions-4810/Reporting/lib/Slicer-4.1/qt-loadable-modules/Python/vtkSlicerReportingModuleLogic.py");
      row.push_back("/home/jchris/Projects/Slicer4-Superbuild-Debug/Slicer-build");
      row.push_back("4811");
      row.push_back("1");
      data.push_back(row);
    }
    for(TestDataType::size_type rowIdx = 0; rowIdx < data.size(); ++rowIdx)
      {
      std::string filePath(data.at(rowIdx).at(0));
      std::string applicationHomeDir(data.at(rowIdx).at(1));
      std::string slicerRevision(data.at(rowIdx).at(2));
      std::string isEmbeddedExpectedAsStr(data.at(rowIdx).at(3));
      bool isEmbeddedExpected = (isEmbeddedExpectedAsStr == "1");

      bool isEmbedded = vtkSlicerApplicationLogic::IsEmbeddedModule(filePath, applicationHomeDir, slicerRevision);
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

  //-----------------------------------------------------------------------------
  // Test ProcessReadSceneData(ReadDataRequest& req)
  //-----------------------------------------------------------------------------
  {
  vtkNew<vtkSlicerApplicationLogic> appLogic;
  // create a scene with a model hierarchy that will clash with the imported scene
  vtkNew<vtkMRMLScene> mrmlScene;
  appLogic->SetMRMLScene(mrmlScene.GetPointer());
  vtkNew<vtkMRMLModelHierarchyNode> originalModelHierarchy1;
  originalModelHierarchy1->SetName("originalTop");
  mrmlScene->AddNode(originalModelHierarchy1.GetPointer());
  vtkNew<vtkMRMLModelHierarchyNode> originalModelHierarchy2;
  originalModelHierarchy2->SetName("originalSecond");
  mrmlScene->AddNode(originalModelHierarchy2.GetPointer());
  originalModelHierarchy2->SetParentNodeID(originalModelHierarchy1->GetID());
  // set up the importing
  std::vector<std::string> targetIDs;
  targetIDs.emplace_back(originalModelHierarchy1->GetID());
  std::vector<std::string> sourceIDs;
  sourceIDs.emplace_back(originalModelHierarchy1->GetID());
  // now create a scene to import that has a hierarchy
  vtkNew<vtkMRMLScene> importScene;
  std::string filename = "applicationLogicModelHierarchyImportTestScene.mrml";
  importScene->SetURL(filename.c_str());
  // make a few deep model hierarchy tree
  for (int i = 0; i < 5; i++)
    {
    vtkNew<vtkMRMLModelHierarchyNode> mhn;
    importScene->AddNode(mhn.GetPointer());
    std::string idNumberString;
    std::stringstream ss;
    ss << i;
    ss >> idNumberString;
    mhn->SetName(idNumberString.c_str());
    if (i > 0)
      {
      std::string parentNodeID = std::string("vtkMRMLModelHierarchyNode") + idNumberString;
      std::cout << "Setting parent node id on node " << mhn->GetID() << " to " << parentNodeID.c_str() << std::endl;
      mhn->SetParentNodeID(parentNodeID.c_str());
      }
    }
  importScene->Commit();
  // set up to read the file
  appLogic->CreateProcessingThread();
  int retval = appLogic->RequestReadScene(filename, targetIDs, sourceIDs, 0, 1);
  if (retval == 0)
    {
    std::cerr << "Unable to process request read scene" << std::endl;
    return EXIT_FAILURE;
    }
  appLogic->ProcessReadData();
  // test that the app Logic's scene has the proper hierarchy
  int numNodes = appLogic->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode");
  std::cout << "After processing read data, app logic scene has " << numNodes << " model hierarchy nodes" << std::endl;
  // the five nodes that were imported over wrote one
  if (numNodes != 6)
    {
    std::cerr << "Expected to have 6 nodes!" << std::endl;
    return EXIT_FAILURE;
    }
  for (int i = 0; i < numNodes; i++)
    {
    vtkMRMLNode *mrmlNode = appLogic->GetMRMLScene()->GetNthNodeByClass(i, "vtkMRMLModelHierarchyNode");
    if (mrmlNode && mrmlNode->IsA("vtkMRMLModelHierarchyNode"))
      {
      vtkMRMLModelHierarchyNode *hnode = vtkMRMLModelHierarchyNode::SafeDownCast(mrmlNode);
      std::cout << i << ": Model Hierarchy node named " << hnode->GetName() << " with id " << hnode->GetID() << " has parent node id of " << (hnode->GetParentNodeID() ? hnode->GetParentNodeID() : "null") << std::endl;
      // the second level clashed with the original hierarchy second level node, so below that, the parent node ids have been shifted
      if (strcmp(hnode->GetName(),"1") == 0 &&
          strcmp(hnode->GetParentNodeID(), "vtkMRMLModelHierarchyNode1") != 0)
        {
        std::cerr << "Hierarchy node has incorrect parent node id, expected vtkMRMLModelHierarchyNode1" << std::endl;
        return EXIT_FAILURE;
        }
      if (strcmp(hnode->GetName(),"2") == 0 &&
          strcmp(hnode->GetParentNodeID(), "vtkMRMLModelHierarchyNode3") != 0)
        {
        std::cerr << "Hierarchy node has incorrect parent node id, expected vtkMRMLModelHierarchyNode3" << std::endl;
        return EXIT_FAILURE;
        }
      if (strcmp(hnode->GetName(),"3") == 0 &&
          strcmp(hnode->GetParentNodeID(), "vtkMRMLModelHierarchyNode4") != 0)
        {
        std::cerr << "Hierarchy node has incorrect parent node id, expected vtkMRMLModelHierarchyNode4" << std::endl;
        return EXIT_FAILURE;
        }
      if (strcmp(hnode->GetName(),"4") == 0 &&
          strcmp(hnode->GetParentNodeID(), "vtkMRMLModelHierarchyNode5") != 0)
        {
        std::cerr << "Hierarchy node has incorrect parent node id, expected vtkMRMLModelHierarchyNode5" << std::endl;
        return EXIT_FAILURE;
        }
      }
    }
  }

  return EXIT_SUCCESS;
}
