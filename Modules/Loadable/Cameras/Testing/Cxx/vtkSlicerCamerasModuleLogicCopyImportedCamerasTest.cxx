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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Cameras includes
#include <vtkSlicerCamerasModuleLogic.h>

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
bool TestCopyImportedCameras(bool clear, bool legacy);

//----------------------------------------------------------------------------
int vtkSlicerCamerasModuleLogicCopyImportedCamerasTest(int vtkNotUsed(argc),
                                                       char* vtkNotUsed(argv)[])
{
  bool res = true;

  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  res = TestCopyImportedCameras(false, true) && res;
  res = TestCopyImportedCameras(true, true) && res;
  TESTING_OUTPUT_ASSERT_WARNINGS_END();

  res = TestCopyImportedCameras(false, false) && res;
  res = TestCopyImportedCameras(true, false) && res;

  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
bool TestCopyImportedCameras(bool clear, bool legacy)
{
  double camera1Pos[3] = {10., 10., 10.};
  double camera2Pos[3] = {10., 10., 10.};
  double importedCamera1Pos[3] = {5., 5., 5.};
  double importedCamera2Pos[3] = {0., 0., 0.};
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLCameraNode> camera1;
  camera1->SetName("Default Scene Camera");
  if(!legacy)
    {
    camera1->SetSingletonTag("1");
    }
  camera1->SetPosition(camera1Pos);
  scene->AddNode(camera1.GetPointer());

  vtkNew<vtkMRMLCameraNode> camera2;
  if(!legacy)
    {
    camera2->SetSingletonTag("2");
    }
  camera2->SetName("Camera2");
  camera2->SetPosition(camera2Pos);
  scene->AddNode(camera2.GetPointer());

  vtkNew<vtkSlicerCamerasModuleLogic> logic;
  logic->SetMRMLScene(scene.GetPointer());

  std::string sceneXML =
"<MRML  version=\"Slicer4.4.0\" userTags=\"\">\n"
"<View id=\"vtkMRMLViewNode1\" name=\"View\" "
"singletonTag=\"1\" layoutName=\"1\" "
"attributes=\"MappedInLayout:1\" layoutLabel=\"1\" active=\"false\" "
"visibility=\"true\"  fieldOfView=\"200\" letterSize=\"0.05\" "
"boxVisible=\"true\" fiducialsVisible=\"true\" fiducialLabelsVisible=\"true\" "
"axisLabelsVisible=\"true\" axisLabelsCameraDependent=\"true\" "
"backgroundColor=\"0.756863 0.764706 0.909804\" "
"backgroundColor2=\"0.454902 0.470588 0.745098\" animationMode=\"Off\" "
"viewAxisMode=\"LookFrom\" spinDegrees=\"2\" spinMs=\"5\" "
"spinDirection=\"YawLeft\" rotateDegrees=\"5\" rockLength=\"200\" "
"rockCount=\"0\" stereoType=\"NoStereo\" renderMode=\"Perspective\">"
"</View>\n"
"<Camera id=\"vtkMRMLCameraNode1\" name=\"Camera\" description=\"Default Scene Camera\" "
"position=\"5. 5. 5.\" focalPoint=\"0 0 0\" "
"viewUp=\"-0.41918 -0.433183 0.797898\" parallelProjection=\"false\" "
"parallelScale=\"1\" singletonTag=\"1\" "
"appliedTransform=\"1 0 0 0  0 1 0 0  0 0 1 0  0 0 0 1\"  ></Camera>\n"
"</MRML>";

  if (legacy)
    {
    sceneXML =
"<MRML  version=\"Slicer4\" userTags=\"\">\n"
"<View id=\"vtkMRMLViewNode1\" name=\"View\" "
"attributes=\"MappedInLayout:1\" layoutLabel=\"1\" active=\"false\" "
"visibility=\"true\"  fieldOfView=\"200\" letterSize=\"0.05\" "
"boxVisible=\"true\" fiducialsVisible=\"true\" fiducialLabelsVisible=\"true\" "
"axisLabelsVisible=\"true\" axisLabelsCameraDependent=\"true\" "
"backgroundColor=\"0.756863 0.764706 0.909804\" "
"backgroundColor2=\"0.454902 0.470588 0.745098\" animationMode=\"Off\" "
"viewAxisMode=\"LookFrom\" spinDegrees=\"2\" spinMs=\"5\" "
"spinDirection=\"YawLeft\" rotateDegrees=\"5\" rockLength=\"200\" "
"rockCount=\"0\" stereoType=\"NoStereo\" renderMode=\"Perspective\">"
"</View>\n"
"<Camera id=\"vtkMRMLCameraNode1\" name=\"Default Scene Camera\" "
"position=\"5. 5. 5.\" focalPoint=\"0 0 0\" "
"viewUp=\"-0.41918 -0.433183 0.797898\" parallelProjection=\"false\" "
"parallelScale=\"1\" activetag=\"vtkMRMLViewNode1\" "
"appliedTransform=\"1 0 0 0  0 1 0 0  0 0 1 0  0 0 0 1\"  ></Camera>\n"
"</MRML>";
    }

  scene->SetSceneXMLString(sceneXML);
  scene->SetLoadFromXMLString(1);
  if (clear)
    {
    scene->Connect();
    }
  else
    {
    scene->Import();
    }

  double* expectedCamera1Pos = camera1Pos;
  double* expectedCamera2Pos = camera2Pos;
  double* expectedFirstCamera = camera1Pos;
  if (clear)
    {
    expectedFirstCamera = importedCamera1Pos;
    if (!legacy)
      {
      expectedCamera1Pos = importedCamera1Pos;
      expectedCamera2Pos = importedCamera2Pos;
      }
    }
  else
    {
    expectedCamera1Pos = importedCamera1Pos;
    expectedFirstCamera = importedCamera1Pos;
    }

  vtkMRMLCameraNode* firstCamera = vtkMRMLCameraNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkMRMLCameraNode"));

  if (camera1->GetPosition()[0] != expectedCamera1Pos[0] ||
      camera2->GetPosition()[0] != expectedCamera2Pos[0] ||
      firstCamera->GetPosition()[0] != expectedFirstCamera[0])
    {
    std::cout << "vtkSlicerCamerasModuleLogic::CopyImportedCameras failed.\n"
              << "Clear: " << clear << "\n"
              << "Camera 1\n"
              << "   current: " << camera1->GetPosition()[0] << "\n"
              << "  expected: " << expectedCamera1Pos[0] << "\n"
              << "Camera 2\n"
              << "   current: " << camera2->GetPosition()[0] << "\n"
              << "  expected: " << expectedCamera2Pos[0] << "\n"
              << "1st Camera\n"
              << "   current: " << firstCamera->GetPosition()[0] << "\n"
              << "  expected: " << expectedFirstCamera[0]
              << std::endl;
    return false;
    }
  return true;
}
