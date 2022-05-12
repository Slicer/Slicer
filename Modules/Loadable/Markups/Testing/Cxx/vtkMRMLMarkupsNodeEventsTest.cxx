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
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkIndent.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

// STL includes
#include <vector>

#include "vtkMRMLCoreTestingMacros.h"

namespace
{

class vtkMRMLMarkupNodeObserver : public vtkCommand
{
public:
  static vtkMRMLMarkupNodeObserver *New()
    {
    return new vtkMRMLMarkupNodeObserver;
    }

  vtkMRMLMarkupNodeObserver()
    {
    }

  void Execute(vtkObject *caller, unsigned long event, void*) override
    {
    vtkMRMLDisplayableNode* dispNode = vtkMRMLDisplayableNode::SafeDownCast(caller);
    if (!dispNode)
      {
      return;
      }
    invokedEvents.push_back(event);
    }

  std::vector<int> invokedEvents;
};

void addEventsToObserver(vtkMRMLMarkupsNode* node, vtkMRMLMarkupNodeObserver* observer)
{
  node->AddObserver(vtkMRMLMarkupsNode::LockModifiedEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::LabelFormatModifiedEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::PointAddedEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::PointRemovedEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::PointPositionDefinedEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::PointPositionUndefinedEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::PointPositionMissingEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::PointPositionNonMissingEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::PointModifiedEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::CenterOfRotationModifiedEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent, observer);
  node->AddObserver(vtkMRMLMarkupsNode::PointAboutToBeRemovedEvent, observer);
}

bool containsEvent(vtkMRMLMarkupNodeObserver* observer, int eventId)
{
  bool found = std::find(observer->invokedEvents.begin(), observer->invokedEvents.end(),
    eventId) != observer->invokedEvents.end();
  observer->invokedEvents.clear();
  return found;
}

}

int vtkMRMLMarkupsNodeEventsTest(int, char* [])
{
  vtkNew<vtkMRMLMarkupsFiducialNode> node;
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
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::PointAddedEvent), true);

  // Test 2: LockModifiedEvent
  node->SetLocked(true);
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::LockModifiedEvent), true);

  // Test 3: LabelFormatModifiedEvent
  node->SetMarkupLabelFormat("the value is: %");
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::LabelFormatModifiedEvent), true);

  // Test 4: PointRemovedEvent
  node->RemoveNthControlPoint(0);
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::PointRemovedEvent), true);

  // Test 5: PointPositionDefinedEvent
  node->AddControlPoint(point1, "first point");
  node->SetNthControlPointPosition(0, point2[0], point2[1], point2[2]);
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::PointPositionDefinedEvent), true);

  // Test 6: PointPositionUndefinedEvent
  node->UnsetNthControlPointPosition(0);
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::PointPositionUndefinedEvent), true);

  // Test 7: PointPositionMissingEvent
  node->SetNthControlPointPositionMissing(0);
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::PointPositionMissingEvent), true);

  // Test 8: PointPositionNonMissingEvent
  node->SetNthControlPointPosition(0, point2[0], point2[1], point2[2]);
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::PointPositionNonMissingEvent), true);

  // Test 9: PointModifiedEvent
  node->SetNthControlPointLabel(0, "new first point");
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::PointModifiedEvent), true);

  // Test 10: CenterPointModifiedEvent
  node->SetCenterOfRotation(point2.GetData());
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::CenterOfRotationModifiedEvent), true);

  // Test 11: FixedNumberOfControlPointsModifiedEvent
  node->SetFixedNumberOfControlPoints(true);
  node->SetFixedNumberOfControlPoints(false);
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent), true);

  // Test 12: PointAboutToBeRemovedEvent
  node->RemoveNthControlPoint(0);
  CHECK_BOOL(containsEvent(observer, vtkMRMLMarkupsNode::PointAboutToBeRemovedEvent), true);

  return EXIT_SUCCESS;
}
