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

// MRML includes
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLCoreTestingMacros.h"
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

// STD includes
#include <string>


//-----------------------------------------------------------------------------
int SliceLogicsTest();
int SliceOrientationPresetInitializationTest();
int TemporaryPathTest();

//-----------------------------------------------------------------------------
int vtkMRMLApplicationLogicTest1(int , char * [])
{
  CHECK_INT(SliceLogicsTest(), EXIT_SUCCESS);
  CHECK_INT(SliceOrientationPresetInitializationTest(), EXIT_SUCCESS);
  CHECK_INT(TemporaryPathTest(), EXIT_SUCCESS);
  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int SliceLogicsTest()
{
  vtkNew<vtkMRMLApplicationLogic> appLogic;

  // By default, a null collection is expected
  {
    vtkMTimeType mtime = appLogic->GetMTime();
    CHECK_NULL(appLogic->GetSliceLogics());
    CHECK_BOOL(appLogic->GetMTime() == mtime, true);
  }

  // Set a null string
  {
    vtkMTimeType mtime = appLogic->GetMTime();
    appLogic->SetSliceLogics(0);
    CHECK_NULL(appLogic->GetSliceLogics());
    CHECK_BOOL(appLogic->GetMTime() > mtime, false);
  }

  // Set a non-empty collection should work
  {
    vtkMTimeType mtime = appLogic->GetMTime();
    vtkNew<vtkCollection> logics;
    logics->AddItem(vtkSmartPointer<vtkObject>::New());
    appLogic->SetSliceLogics(logics.GetPointer());
    CHECK_POINTER(appLogic->GetSliceLogics(), logics.GetPointer());
    CHECK_INT(appLogic->GetSliceLogics()->GetNumberOfItems(), 1);
    CHECK_BOOL(appLogic->GetMTime() > mtime, true);
  }

  // Set a null collection.
  {
    vtkMTimeType mtime = appLogic->GetMTime();
    appLogic->SetSliceLogics(0);
    CHECK_NULL(appLogic->GetSliceLogics());
    CHECK_BOOL(appLogic->GetMTime() > mtime, true);
  }

  {
    vtkNew<vtkCollection> logics;
    logics->AddItem(vtkSmartPointer<vtkObject>::New());
    appLogic->SetSliceLogics(logics.GetPointer());
  }

  // Set an empty collection.
  {
    vtkMTimeType mtime = appLogic->GetMTime();
    vtkNew<vtkCollection> logics;
    appLogic->SetSliceLogics(logics.GetPointer());
    CHECK_NOT_NULL(appLogic->GetSliceLogics());
    CHECK_INT(appLogic->GetSliceLogics()->GetNumberOfItems(), 0);
    CHECK_BOOL(appLogic->GetMTime() > mtime, true);
  }

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int SliceOrientationPresetInitializationTest()
{
  {
    vtkNew<vtkMRMLSliceNode> sliceNode;
    CHECK_INT(sliceNode->GetNumberOfSliceOrientationPresets(), 0);
  }

  {
    vtkNew<vtkMRMLScene> scene;
    vtkMRMLSliceNode * defaultSliceNode =
        vtkMRMLSliceNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkMRMLSliceNode"));
    CHECK_NULL(defaultSliceNode);
  }

  {
    vtkNew<vtkMRMLScene> scene;
    vtkNew<vtkMRMLApplicationLogic> appLogic;
    appLogic->SetMRMLScene(scene.GetPointer());
    vtkMRMLSliceNode * defaultSliceNode =
        vtkMRMLSliceNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkMRMLSliceNode"));
    CHECK_INT(defaultSliceNode->GetNumberOfSliceOrientationPresets(), 3);
  }

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int TemporaryPathTest()
{
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
