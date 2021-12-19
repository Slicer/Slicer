/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRML includes
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkIndent.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

// STL includes
#include <vector>

class vtkMRMLMarkupNodeObserver
    : public vtkCommand
{
public:
  static vtkMRMLMarkupNodeObserver *New(){
    return new vtkMRMLMarkupNodeObserver;
  }

  vtkMRMLMarkupNodeObserver(){
  }

  void Execute(vtkObject *caller, unsigned long event, void*) override{
    vtkMRMLDisplayableNode* dispNode =
        vtkMRMLDisplayableNode::SafeDownCast(caller);

    if (!dispNode)
      return;

    if (event == vtkMRMLMarkupsNode::LockModifiedEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::LockModifiedEvent);
    } else if (event == vtkMRMLMarkupsNode::LabelFormatModifiedEvent){
       invokedEvents.push_back(vtkMRMLMarkupsNode::LabelFormatModifiedEvent);
    } else if (event == vtkMRMLMarkupsNode::PointAddedEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::PointAddedEvent);
    } else if (event == vtkMRMLMarkupsNode::PointRemovedEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::PointRemovedEvent);
    } else if (event == vtkMRMLMarkupsNode::PointPositionDefinedEvent){
       invokedEvents.push_back(vtkMRMLMarkupsNode::PointPositionDefinedEvent);
    } else if (event == vtkMRMLMarkupsNode::PointPositionUndefinedEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::PointPositionUndefinedEvent);
    } else if (event == vtkMRMLMarkupsNode::PointPositionMissingEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::PointPositionMissingEvent);
    } else if (event == vtkMRMLMarkupsNode::PointPositionNonMissingEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::PointPositionNonMissingEvent);
    } else if (event == vtkMRMLMarkupsNode::PointModifiedEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::PointModifiedEvent);
    } else if (event == vtkMRMLMarkupsNode::PointStartInteractionEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::PointStartInteractionEvent);
    } else if (event == vtkMRMLMarkupsNode::PointEndInteractionEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::PointEndInteractionEvent);
    } else if (event == vtkMRMLMarkupsNode::CenterPointModifiedEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::CenterPointModifiedEvent);
    } else if (event == vtkMRMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent);
    } else if (event == vtkMRMLMarkupsNode::PointAboutToBeRemovedEvent){
      invokedEvents.push_back(vtkMRMLMarkupsNode::PointAboutToBeRemovedEvent);
    }
  }

  std::vector<int> invokedEvents;
};

void addEventsToObserver(
    vtkMRMLMarkupsNode* node,
    vtkMRMLMarkupNodeObserver* observer)
{
  node->AddObserver(
        vtkMRMLMarkupsNode::LockModifiedEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::LabelFormatModifiedEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::PointAddedEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::PointRemovedEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::PointPositionDefinedEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::PointPositionUndefinedEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::PointPositionMissingEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::PointPositionNonMissingEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::PointModifiedEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::PointStartInteractionEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::PointEndInteractionEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::CenterPointModifiedEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent,
        observer);
  node->AddObserver(
        vtkMRMLMarkupsNode::PointAboutToBeRemovedEvent,
        observer);
}

int vtkMRMLMarkupsNodeEventsTest(int , char * [] )
{
  vtkNew<vtkMRMLMarkupsNode> node;
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLMarkupNodeObserver> observer;

  addEventsToObserver(node, observer);

  scene->AddNode(node.GetPointer());

  // Default settings
  vtkVector3d point1, point2;
  point1[0] = 10;
  point1[1] = 20;
  point1[2] = 30;


  // Test 1: PointAddedEvent
  node->AddControlPoint(point1, "first point");
  if (std::find(
        observer->invokedEvents.begin(),
        observer->invokedEvents.end(),
        vtkMRMLMarkupsNode::PointAddedEvent) == observer->invokedEvents.end()){
    std::cerr << "ERROR: PointAddedEvent was not invoked" << std::endl;
    return EXIT_FAILURE;
  }
  observer->invokedEvents.clear();

  // Test 2: LockModifiedEvent
  node->SetLocked(true);
  if (std::find(
        observer->invokedEvents.begin(),
        observer->invokedEvents.end(),
        vtkMRMLMarkupsNode::LockModifiedEvent) == observer->invokedEvents.end()){
    std::cerr << "ERROR: LockModifiedEvent was not invoked" << std::endl;
    return EXIT_FAILURE;
  }
  observer->invokedEvents.clear();

  // Test 3: LabelFormatModifiedEvent
  node->SetMarkupLabelFormat("the value is: %");
  if (std::find(
        observer->invokedEvents.begin(),
        observer->invokedEvents.end(),
        vtkMRMLMarkupsNode::LabelFormatModifiedEvent) == observer->invokedEvents.end()){
    std::cerr << "ERROR: LabelFormatModifiedEvent was not invoked" << std::endl;
    return EXIT_FAILURE;
  }
  observer->invokedEvents.clear();

  // Test 4: PointRemovedEvent
  node->RemoveNthControlPoint(0);
  if (std::find(
        observer->invokedEvents.begin(),
        observer->invokedEvents.end(),
        vtkMRMLMarkupsNode::PointRemovedEvent) == observer->invokedEvents.end()){
    std::cerr << "ERROR: PointRemovedEvent was not invoked" << std::endl;
    return EXIT_FAILURE;
  }
  observer->invokedEvents.clear();

  // Test 5: PointPositionDefinedEvent
  node->AddControlPoint(point1, "first point");
  node->SetNthControlPointPosition(0, point2[0], point2[1], point2[2]);
  if (std::find(
        observer->invokedEvents.begin(),
        observer->invokedEvents.end(),
        vtkMRMLMarkupsNode::PointPositionDefinedEvent) == observer->invokedEvents.end()){
    std::cerr << "ERROR: PointPositionDefinedEvent was not invoked" << std::endl;
    return EXIT_FAILURE;
  }
  observer->invokedEvents.clear();

  // Test 6: PointPositionUndefinedEvent
  node->UnsetNthControlPointPosition(0);
  if (std::find(
        observer->invokedEvents.begin(),
        observer->invokedEvents.end(),
        vtkMRMLMarkupsNode::PointPositionUndefinedEvent) == observer->invokedEvents.end()){
    std::cerr << "ERROR: PointPositionUndefinedEvent was not invoked" << std::endl;
    return EXIT_FAILURE;
  }
  observer->invokedEvents.clear();

//  // Test 7: PointPositionMissingEvent
//  node->SetNthControlPointPositionMissing(0);
//  if (std::find(
//        observer->invokedEvents.begin(),
//        observer->invokedEvents.end(),
//        vtkMRMLMarkupsNode::PointPositionMissingEvent) == observer->invokedEvents.end()){
//    std::cerr << "ERROR: PointPositionMissingEvent was not invoked" << std::endl;
//    return EXIT_FAILURE;
//  }
//  observer->invokedEvents.clear();

//  // Test 8: PointPositionNonMissingEvent
//  node->SetNthControlPointPositionMissing(0);
//  if (std::find(
//        observer->invokedEvents.begin(),
//        observer->invokedEvents.end(),
//        vtkMRMLMarkupsNode::PointPositionNonMissingEvent) == observer->invokedEvents.end()){
//    std::cerr << "ERROR: PointPositionNonMissingEvent was not invoked" << std::endl;
//    return EXIT_FAILURE;
//  }
//  observer->invokedEvents.clear();

  // Test 9: PointModifiedEvent
  node->SetNthControlPointLabel(0, "new first point");
  if (std::find(
        observer->invokedEvents.begin(),
        observer->invokedEvents.end(),
        vtkMRMLMarkupsNode::PointModifiedEvent) == observer->invokedEvents.end()){
    std::cerr << "ERROR: PointModifiedEvent was not invoked" << std::endl;
    return EXIT_FAILURE;
  }
  observer->invokedEvents.clear();

  // Test 10: CenterPointModifiedEvent
  node->SetCenterPosition(point2[0], point2[1], point2[2]);
  if (std::find(
        observer->invokedEvents.begin(),
        observer->invokedEvents.end(),
        vtkMRMLMarkupsNode::CenterPointModifiedEvent) == observer->invokedEvents.end()){
    std::cerr << "ERROR: CenterPointModifiedEvent was not invoked" << std::endl;
    return EXIT_FAILURE;
  }
  observer->invokedEvents.clear();

  // Test 11: FixedNumberOfControlPointsModifiedEvent
  node->SetFixedNumberOfControlPoints(true);
  node->SetFixedNumberOfControlPoints(false);
  if (std::find(
        observer->invokedEvents.begin(),
        observer->invokedEvents.end(),
        vtkMRMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent) == observer->invokedEvents.end()){
    std::cerr << "ERROR: FixedNumberOfControlPointsModifiedEvent was not invoked" << std::endl;
    return EXIT_FAILURE;
  }
  observer->invokedEvents.clear();

  // Test 12: PointAboutToBeRemovedEvent
  node->RemoveNthControlPoint(0);
  if (std::find(
        observer->invokedEvents.begin(),
        observer->invokedEvents.end(),
        vtkMRMLMarkupsNode::PointAboutToBeRemovedEvent) == observer->invokedEvents.end()){
    std::cerr << "ERROR: PointAboutToBeRemovedEvent was not invoked" << std::endl;
    return EXIT_FAILURE;
  }
  observer->invokedEvents.clear();


  return EXIT_SUCCESS;
}
