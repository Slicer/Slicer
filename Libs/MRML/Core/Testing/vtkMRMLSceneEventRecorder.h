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

// VTK includes
#include <vtkCommand.h>

// STD includes
#include <map>

//---------------------------------------------------------------------------
// Utility class to observe the MRML scene events
// vtkNew<vtkMRMLSceneViewEventRecorder> callback;
//  scene->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());
// // action(s) on the scene
// if (callback->CalledEvents.size() != 1 ||
//     callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] != 1 ||
//     callback->LastEventMTime[vtkMRMLScene::StartBatchProcessEvent] > mtime)
//   {
//   return EXIT_FAILURE;
//   }
// callback->CalledEvents.clear();
class vtkMRMLSceneEventRecorder : public vtkCommand
{
public:
  vtkTypeMacro(vtkMRMLSceneEventRecorder, vtkCommand);
  static vtkMRMLSceneEventRecorder *New();
  void Execute(vtkObject *caller, unsigned long eventId,
                       void *callData) override;
  // List of node that should be updated when NodeAddedEvent is caught
  std::map<unsigned long, unsigned int> CalledEvents;
  std::map<unsigned long, vtkMTimeType> LastEventMTime;
protected:
  vtkMRMLSceneEventRecorder();
  ~vtkMRMLSceneEventRecorder() override;
};
