/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Daniel Haehn, UPenn
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLSceneViewLogic_h
#define __vtkMRMLSceneViewLogic_h


#include "vtkSlicerLogic.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLSceneViewNode.h>

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerSceneViewLogic :
  public vtkSlicerLogic
{
public:

  static vtkSlicerSceneViewLogic *New();
  vtkTypeRevisionMacro(vtkSlicerSceneViewLogic,vtkSlicerLogic);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // MRML events
  void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData );

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();

  /// Create a sceneView..
  void CreateSceneView(const char* name, const char* description, int screenshotType, vtkImageData* screenshot);

  /// Modify an existing sceneView.
  void ModifySceneView(vtkStdString id, const char* name, const char* description, int screenshotType, vtkImageData* screenshot);

  /// Return the name of an existing sceneView.
  vtkStdString GetSceneViewName(const char* id);

  /// Return the description of an existing sceneView.
  vtkStdString GetSceneViewDescription(const char* id);

  /// Return the screenshotType of an existing sceneView.
  int GetSceneViewScreenshotType(const char* id);

  /// Return the screenshot of an existing sceneView.
  vtkImageData* GetSceneViewScreenshot(const char* id);

  /// Restore an sceneView.
  void RestoreSceneView(const char* id);

  /// Move sceneView up
  void MoveSceneViewUp(const char* id);

  /// Move sceneView up
  void MoveSceneViewDown(const char* id);



protected:

  vtkSlicerSceneViewLogic();

  virtual ~vtkSlicerSceneViewLogic();

private:

};

#endif
