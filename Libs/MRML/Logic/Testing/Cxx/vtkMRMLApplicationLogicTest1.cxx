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
#include <vtksys/SystemTools.hxx>

// STD includes
#include <sstream>
#include <string>

//-----------------------------------------------------------------------------
int SliceLogicsTest();
int SliceOrientationPresetInitializationTest();
int TemporaryPathTest();
int CreateUniqueFileNameTest(std::string tempDir);
int AddModuleLogicTest();

//-----------------------------------------------------------------------------
int vtkMRMLApplicationLogicTest1(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Line " << __LINE__ << " - Missing parameters!\n"
              << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }
  const char* tempDir = argv[1];

  CHECK_EXIT_SUCCESS(SliceLogicsTest());
  CHECK_EXIT_SUCCESS(SliceOrientationPresetInitializationTest());
  CHECK_EXIT_SUCCESS(TemporaryPathTest());
  CHECK_EXIT_SUCCESS(CreateUniqueFileNameTest(tempDir));
  CHECK_EXIT_SUCCESS(AddModuleLogicTest());
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
    appLogic->SetSliceLogics(nullptr);
    CHECK_NULL(appLogic->GetSliceLogics());
    CHECK_BOOL(appLogic->GetMTime() > mtime, false);
  }

  // Set a non-empty collection should work
  {
    vtkMTimeType mtime = appLogic->GetMTime();
    vtkNew<vtkCollection> logics;
    logics->AddItem(vtkSmartPointer<vtkObject>::New());
    appLogic->SetSliceLogics(logics);
    CHECK_POINTER(appLogic->GetSliceLogics(), logics);
    CHECK_INT(appLogic->GetSliceLogics()->GetNumberOfItems(), 1);
    CHECK_BOOL(appLogic->GetMTime() > mtime, true);
  }

  // Set a null collection.
  {
    vtkMTimeType mtime = appLogic->GetMTime();
    appLogic->SetSliceLogics(nullptr);
    CHECK_NULL(appLogic->GetSliceLogics());
    CHECK_BOOL(appLogic->GetMTime() > mtime, true);
  }

  {
    vtkNew<vtkCollection> logics;
    logics->AddItem(vtkSmartPointer<vtkObject>::New());
    appLogic->SetSliceLogics(logics);
  }

  // Set an empty collection.
  {
    vtkMTimeType mtime = appLogic->GetMTime();
    vtkNew<vtkCollection> logics;
    appLogic->SetSliceLogics(logics);
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
    vtkMRMLSliceNode* defaultSliceNode =
      vtkMRMLSliceNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkMRMLSliceNode"));
    CHECK_NULL(defaultSliceNode);
  }

  {
    vtkNew<vtkMRMLScene> scene;
    vtkNew<vtkMRMLApplicationLogic> appLogic;
    appLogic->SetMRMLScene(scene);
    vtkMRMLSliceNode* defaultSliceNode =
      vtkMRMLSliceNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkMRMLSliceNode"));
    CHECK_INT(defaultSliceNode->GetNumberOfSliceOrientationPresets(), 3);
  }

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int TemporaryPathTest()
{
  vtkNew<vtkMRMLApplicationLogic> MRMLAppLogic;

  MRMLAppLogic->SetTemporaryPath(nullptr); // Test nullptr
  if (strlen(MRMLAppLogic->GetTemporaryPath()) != 0)
  {
    std::string temporaryEmptyString;
    std::cerr << "Line " << __LINE__ << " - Problem with SetTemporaryPath using NULL"
              << "\n"
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

  for (TestDataType::size_type rowIdx = 0; rowIdx < data.size(); ++rowIdx)
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

//-----------------------------------------------------------------------------
int CreateUniqueFileNameTest(std::string tempDir)
{
  vtkNew<vtkMRMLApplicationLogic> appLogic;

  // Clean up all earlier test files
  vtksys::SystemTools::RemoveFile(tempDir + "/CreateUniqueFileNameTest.txt");
  vtksys::SystemTools::RemoveFile(tempDir + "/CreateUniqueFileNameTest_1.txt");
  vtksys::SystemTools::RemoveFile(tempDir + "/CreateUniqueFileNameTest.nii.gz");
  vtksys::SystemTools::RemoveFile(tempDir + "/CreateUniqueFileNameTest_1.nii.gz");

  // Check if we get the same file name if already unique
  // existing file: (none)
  CHECK_STD_STRING(appLogic->CreateUniqueFileName(tempDir + "/CreateUniqueFileNameTest.txt"),
                   tempDir + "/CreateUniqueFileNameTest.txt");

  // Check if we get a suffixed filename if the file exists already
  vtksys::SystemTools::Touch(tempDir + "/CreateUniqueFileNameTest.txt", true);
  // existing files: CreateUniqueFileNameTest.txt
  CHECK_STD_STRING(appLogic->CreateUniqueFileName(tempDir + "/CreateUniqueFileNameTest.txt"),
                   tempDir + "/CreateUniqueFileNameTest_1.txt");

  // Check if a suffix is incremented if the file is already suffixed
  vtksys::SystemTools::Touch(tempDir + "/CreateUniqueFileNameTest_1.txt", true);
  // existing files: CreateUniqueFileNameTest.txt, CreateUniqueFileNameTest_1.txt
  CHECK_STD_STRING(appLogic->CreateUniqueFileName(tempDir + "/CreateUniqueFileNameTest.txt"),
                   tempDir + "/CreateUniqueFileNameTest_2.txt");
  CHECK_STD_STRING(appLogic->CreateUniqueFileName(tempDir + "/CreateUniqueFileNameTest_1.txt"),
                   tempDir + "/CreateUniqueFileNameTest_2.txt");

  // Check if a suffix is incremented if a composite file extension is used

  // Check if we get a suffixed filename if the file exists already
  vtksys::SystemTools::Touch(tempDir + "/CreateUniqueFileNameTest.nii.gz", true);
  // existing files: CreateUniqueFileNameTest.txt
  CHECK_STD_STRING(appLogic->CreateUniqueFileName(tempDir + "/CreateUniqueFileNameTest.nii.gz", ".nii.gz"),
                   tempDir + "/CreateUniqueFileNameTest_1.nii.gz");

  // Check if a suffix is incremented if the file is already suffixed
  vtksys::SystemTools::Touch(tempDir + "/CreateUniqueFileNameTest_1.nii.gz", true);
  // existing files: CreateUniqueFileNameTest.txt, CreateUniqueFileNameTest_1.txt
  CHECK_STD_STRING(appLogic->CreateUniqueFileName(tempDir + "/CreateUniqueFileNameTest.nii.gz", ".nii.gz"),
                   tempDir + "/CreateUniqueFileNameTest_2.nii.gz");
  CHECK_STD_STRING(appLogic->CreateUniqueFileName(tempDir + "/CreateUniqueFileNameTest_1.nii.gz", ".nii.gz"),
                   tempDir + "/CreateUniqueFileNameTest_2.nii.gz");

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int AddModuleLogicTest()
{
  vtkNew<vtkMRMLApplicationLogic> appLogic;
  std::string module_name = "TestModule";

  // Registration of a module logic should work
  {
    vtkNew<vtkMRMLAbstractLogic> moduleLogic;
    appLogic->SetModuleLogic(module_name.c_str(), moduleLogic);

    CHECK_POINTER(appLogic->GetModuleLogic(module_name.c_str()), moduleLogic);
  }

  // Getting a pointer to a logic that it is gone should give nullptr.
  // Note that moduleLogic from last scope is gone. GetModuleLogic should return nullptr
  {
    CHECK_NULL(appLogic->GetModuleLogic(module_name.c_str()));
  }

  // Updating module logic with a new object can be useful for dynamic reloading of modules.
  {
    vtkNew<vtkMRMLAbstractLogic> moduleLogic;
    appLogic->SetModuleLogic(module_name.c_str(), moduleLogic);
    CHECK_POINTER(appLogic->GetModuleLogic(module_name.c_str()), moduleLogic);
  }

  // Trying to get a logic that has not been registered should return nullptr
  {
    const vtkMRMLAbstractLogic* retval = appLogic->GetModuleLogic((module_name + "a").c_str());

    // return value should be nullptr
    CHECK_NULL(retval);
  }

  // Passing nullptr to GetModuleLogic should trigger an error and return nullptr
  {
    // Triggers an error
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    const vtkMRMLAbstractLogic* retval = appLogic->GetModuleLogic(nullptr);
    TESTING_OUTPUT_ASSERT_ERRORS(1);
    TESTING_OUTPUT_ASSERT_ERRORS_END();

    // return value should be nullptr
    CHECK_NULL(retval);
  }

  // Trying to unregister a non-registered module logic
  {
    appLogic->SetModuleLogic((module_name + "a").c_str(), nullptr);
  }

  // Removing an already registered module logic should succeed
  {
    // Successfully removes module logic association
    appLogic->SetModuleLogic(module_name.c_str(), nullptr);

    const vtkMRMLAbstractLogic* retval = appLogic->GetModuleLogic((module_name).c_str());
    CHECK_NULL(retval);
  }

  return EXIT_SUCCESS;
}
