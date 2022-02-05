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

#include "vtkMRMLMarkupsNode.h"

// MRML includes
#include "vtkCurveGenerator.h"
#include <vtkEventBroker.h>
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsStorageNode.h"
#include "vtkMRMLStaticMeasurement.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLUnitNode.h"
#include "vtkMRMLScene.h"

// vtkAddon includes
#include "vtkAddonMathUtilities.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkBitArray.h>
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCellLocator.h>
#include <vtkCollection.h>
#include <vtkParallelTransportFrame.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTrivialProducer.h>
#include "vtk_eigen.h"
#include VTK_EIGEN(Dense)

// STD includes
#include <sstream>
#include <algorithm>

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode::vtkMRMLMarkupsNode()
{
  this->TextList = vtkSmartPointer<vtkStringArray>::New();

  this->CenterOfRotation.Set(0,0,0);

  this->CurveInputPoly = vtkSmartPointer<vtkPolyData>::New();
  vtkNew<vtkPoints> curveInputPoints;
  this->CurveInputPoly->SetPoints(curveInputPoints);

  this->CurveGenerator = vtkSmartPointer<vtkCurveGenerator>::New();
  this->CurveGenerator->SetInputData(this->CurveInputPoly);
  this->CurveGenerator->SetCurveTypeToLinearSpline();
  this->CurveGenerator->SetNumberOfPointsPerInterpolatingSegment(1);
  this->CurveGenerator->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);

  this->CurvePolyToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  this->CurvePolyToWorldTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->CurvePolyToWorldTransformer->SetInputConnection(this->CurveGenerator->GetOutputPort());
  this->CurvePolyToWorldTransformer->SetTransform(this->CurvePolyToWorldTransform);

  this->CurveCoordinateSystemGeneratorWorld = vtkSmartPointer<vtkParallelTransportFrame>::New();
  // Curve coordinate system is computed at the very end of the pipeline so that it is only computed
  // if needed (it is not recomputed when a control point or the world transformation is modified).
  this->CurveCoordinateSystemGeneratorWorld->SetInputConnection(this->CurvePolyToWorldTransformer->GetOutputPort());

  this->TransformedCurvePolyLocator = vtkSmartPointer<vtkPointLocator>::New();
  this->InteractionHandleToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->ContentModifiedEvents->InsertNextValue(vtkMRMLMarkupsNode::PointModifiedEvent);

  this->Measurements = vtkCollection::New();
  this->Measurements->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode::~vtkMRMLMarkupsNode()
{
  this->CurveGenerator->RemoveObserver(this->MRMLCallbackCommand);
  this->SetFixedNumberOfControlPoints(false); // remove the control points instead of just unsetting them
  this->RemoveAllControlPoints();

  if (this->Measurements)
    {
    this->Measurements->RemoveObserver(this->MRMLCallbackCommand);
    this->Measurements->Delete();
    this->Measurements = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(locked, Locked);
  vtkMRMLWriteXMLStdStringMacro(controlPointLabelFormat, ControlPointLabelFormat);
  vtkMRMLWriteXMLMatrix4x4Macro(interactionHandleToWorldMatrix, InteractionHandleToWorldMatrix);
  vtkMRMLWriteXMLEndMacro();

  int textLength = static_cast<int>(this->TextList->GetNumberOfValues());
  for (int i = 0 ; i < textLength; i++)
    {
    of << " textList" << i << "=\"" << this->TextList->GetValue(i) << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);

  this->RemoveAllControlPoints();
  this->ClearValueForAllMeasurements();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(locked, Locked);
  vtkMRMLReadXMLStdStringMacro(markupLabelFormat, MarkupLabelFormat);
  vtkMRMLReadXMLStdStringMacro(controlPointLabelFormat, ControlPointLabelFormat);
  vtkMRMLReadXMLOwnedMatrix4x4Macro(interactionHandleToWorldMatrix, InteractionHandleToWorldMatrix);
  vtkMRMLReadXMLEndMacro();

  /* TODO: read measurements
  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strncmp(attName, "textList", 9))
      {
      this->AddText(attValue);
      }
    }
  */
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLMarkupsNode* node = vtkMRMLMarkupsNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  // TODO: For now, we always deep-copy. We could improve copy performance
  // by implementing shallow-copy.

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(Locked);
  vtkMRMLCopyStdStringMacro(ControlPointLabelFormat);
  vtkMRMLCopyOwnedMatrix4x4Macro(InteractionHandleToWorldMatrix);
  vtkMRMLCopyEndMacro();

  this->TextList->DeepCopy(node->TextList);

  // set max number of markups after adding the new ones
  this->LastUsedControlPointNumber = node->LastUsedControlPointNumber;

  this->CurveClosed = node->CurveClosed;

  // BUG: When fiducial nodes appear in scene views as of Slicer 4.1 the per
  // fiducial information (visibility, position etc) is saved to the file on
  // disk and not read, so the scene view copy of a fiducial node doesn't have
  // any fiducials in it. This work around prevents the main scene fiducial
  // list from being cleared of points and then not repopulated.
  // TBD: if scene view node reading xml triggers reading the data from
  // storage nodes, this should no longer be necessary.
  if (this->Scene &&
      this->Scene->IsRestoring())
    {
    if (this->GetNumberOfControlPoints() != 0 &&
        node->GetNumberOfControlPoints() == 0)
      {
      // just return for now
      vtkWarningMacro("MarkupsNode Copy: Scene view is restoring and list to restore is empty, skipping copy of points");
      return;
      }
    }

  this->CurveInputPoly->GetPoints()->Reset();
  this->RemoveAllControlPoints();
  int numMarkups = node->GetNumberOfControlPoints();
  for (int n = 0; n < numMarkups; n++)
    {
    ControlPoint* controlPoint = node->GetNthControlPoint(n);
    ControlPoint* controlPointCopy = new ControlPoint;
    (*controlPointCopy) = (*controlPoint);
    this->AddControlPoint(controlPointCopy, false);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ProcessMRMLEvents(vtkObject *caller,
                                           unsigned long event,
                                           void *callData)
{
  if (caller != nullptr && event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    vtkMRMLTransformNode::GetTransformBetweenNodes(this->GetParentTransformNode(), nullptr, this->CurvePolyToWorldTransform);
    this->UpdateInteractionHandleToWorldMatrix();
    this->UpdateAllMeasurements();
    }
  else if (caller == this->CurveGenerator.GetPointer())
    {
    this->UpdateAllMeasurements();
    int n = -1;
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
    this->StorableModifiedTime.Modified();
    this->Modified();
    }
  else if (caller == this->Measurements)
    {
    vtkEventBroker* broker = vtkEventBroker::GetInstance();
    vtkCollectionSimpleIterator it;
    vtkObject* measurementObject = nullptr;
    for (this->Measurements->InitTraversal(it); (measurementObject = this->Measurements->GetNextItemAsObject(it)) ;)
      {
      if (!broker->GetObservationExist(measurementObject, vtkMRMLMeasurement::InputDataModifiedEvent, this, this->MRMLCallbackCommand))
        {
        broker->AddObservation(measurementObject, vtkMRMLMeasurement::InputDataModifiedEvent, this, this->MRMLCallbackCommand);
        }
      }
    }
  else if (caller->IsA("vtkMRMLMeasurement") && event == vtkMRMLMeasurement::InputDataModifiedEvent)
    {
    this->UpdateAllMeasurements();
    }
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::EndModify(int previousDisableModifiedEventState)
{
  // Event PointAboutToBeRemovedEvent is not listed below because the event does
  // not indicate an actual modification yet.
  bool processPendingPointModifiedEvents = !previousDisableModifiedEventState &&
    (this->GetModifiedEventPending() > 0
    || this->GetCustomModifiedEventPending(vtkMRMLMarkupsNode::PointModifiedEvent) > 0
    || this->GetCustomModifiedEventPending(vtkMRMLMarkupsNode::PointAddedEvent) > 0
    || this->GetCustomModifiedEventPending(vtkMRMLMarkupsNode::PointRemovedEvent) > 0
    || this->GetCustomModifiedEventPending(vtkMRMLMarkupsNode::PointPositionDefinedEvent) > 0
    || this->GetCustomModifiedEventPending(vtkMRMLMarkupsNode::PointPositionUndefinedEvent) > 0
    || this->GetCustomModifiedEventPending(vtkMRMLMarkupsNode::PointPositionMissingEvent) > 0
    || this->GetCustomModifiedEventPending(vtkMRMLMarkupsNode::PointPositionNonMissingEvent) > 0);
  if (processPendingPointModifiedEvents)
    {
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }

  int wasModified = Superclass::EndModify(previousDisableModifiedEventState);

  if (processPendingPointModifiedEvents)
    {
    this->UpdateAllMeasurements();
    }
  return wasModified;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(Locked);
  vtkMRMLPrintStdStringMacro(ControlPointLabelFormat);
  vtkMRMLPrintMatrix4x4Macro(InteractionHandleToWorldMatrix)
  vtkMRMLPrintEndMacro();

  os << indent << "Control point number locked: ";
  os << this->GetFixedNumberOfControlPoints() << "\n";

  os << indent << "MaximumNumberOfControlPoints: ";
  if (this->MaximumNumberOfControlPoints >= 0)
    {
    os << this->MaximumNumberOfControlPoints << "\n";
    }
  else
    {
    os << "unlimited\n";
    }
  os << indent << "RequiredNumberOfControlPoints: ";
  if (this->RequiredNumberOfControlPoints>0)
    {
    os << this->RequiredNumberOfControlPoints << "\n";
    }
  else
    {
    os << "unlimited\n";
    }
  os << indent << "NumberOfControlPoints: " << this->GetNumberOfControlPoints() << "\n";

  for (int controlPointIndex = 0; controlPointIndex < this->GetNumberOfControlPoints(); controlPointIndex++)
    {
    ControlPoint* controlPoint = this->GetNthControlPoint(controlPointIndex);
    if (!controlPoint)
      {
      continue;
      }
    os << indent << "Control Point " << controlPointIndex << ":\n";
    os << indent << indent << "ID: " << controlPoint->ID.c_str() << "\n";
    os << indent << indent << "Label: " << controlPoint->Label.c_str() << "\n";
    os << indent << indent << "Description: " << controlPoint->Description.c_str() << "\n";
    os << indent << indent << "Associated node id: " << controlPoint->AssociatedNodeID.c_str() << "\n";
    os << indent << indent << "Selected: " << controlPoint->Selected << "\n";
    os << indent << indent << "Locked: " << controlPoint->Locked << "\n";
    os << indent << indent << "Visibility: " << controlPoint->Visibility << "\n";
    os << indent << indent << "PositionStatus: " << controlPoint->PositionStatus << "\n";
    os << indent << indent << "Position: " << controlPoint->Position[0] << ", " <<
          controlPoint->Position[1] << ", " << controlPoint->Position[2] << "\n";
    os << indent << indent << "Orientation: ";
    for (int i = 0; i < 9; i++)
      {
      if (i > 0)
        {
        os << ",  ";
        }
      os << controlPoint->OrientationMatrix[i];
      }
    os << "\n";
    }

  if  (this->GetNumberOfMeasurements()>0)
    {
    os << indent << "Measurements:\n";
    for (int measurementIndex = 0; measurementIndex < this->GetNumberOfMeasurements(); measurementIndex++)
      {
      vtkMRMLMeasurement* m = this->GetNthMeasurement(measurementIndex);
      os << indent << indent << m->GetName() << ": " << m->GetValueWithUnitsAsPrintableString() << std::endl;
      }
    }

}
//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::UnsetAllControlPoints()
{
  if (this->ControlPoints.empty())
    {
    // no control points to remove
    return;
    }
  bool definedPointsExisted = false;
  bool missingPointsExisted = false;
  for (unsigned int i = 0; i < this->ControlPoints.size(); i++)
    {
    if (this->ControlPoints[i]->PositionStatus == vtkMRMLMarkupsNode::PositionDefined)
      {
      definedPointsExisted = true;
      }
    if (this->ControlPoints[i]->PositionStatus == vtkMRMLMarkupsNode::PositionMissing)
      {
      missingPointsExisted = true;
      }
    this->UnsetNthControlPointPosition(i);
    }
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointRemovedEvent);
  if (definedPointsExisted)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionUndefinedEvent);
    }
  if (missingPointsExisted)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionNonMissingEvent);
    }

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::RemoveAllControlPoints()
{
  if (this->ControlPoints.empty())
    {
    // no control points to remove
    return;
    }

  if (this->GetFixedNumberOfControlPoints())
    {
    vtkErrorMacro("RemoveAllControlPoints: Markup node control point number locked.");
    return;
    }

  bool definedPointsExisted = false;
  bool missingPointsExisted = false;
  for(unsigned int i = 0; i < this->ControlPoints.size(); i++)
    {
    if (this->ControlPoints[i]->PositionStatus == vtkMRMLMarkupsNode::PositionDefined)
      {
      definedPointsExisted = true;
      }
    if (this->ControlPoints[i]->PositionStatus == vtkMRMLMarkupsNode::PositionMissing)
      {
      missingPointsExisted = true;
      }
    delete this->ControlPoints[i];
    }

  this->ControlPoints.clear();

  if (!this->GetDisableModifiedEvent())
    {
    this->CurveInputPoly->GetPoints()->Reset();
    this->CurveInputPoly->GetPoints()->Squeeze();
    this->UpdateInteractionHandleToWorldMatrix();
    }

  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointRemovedEvent);
  if (definedPointsExisted)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionUndefinedEvent);
    }
  if (missingPointsExisted)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionNonMissingEvent);
    }

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
}

//-------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLMarkupsNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLMarkupsJsonStorageNode"));
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsNode::CreateDefaultDisplayNodes()
{
  if (this->GetDisplayNode() != nullptr &&
    vtkMRMLMarkupsDisplayNode::SafeDownCast(this->GetDisplayNode()) != nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene() == nullptr)
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkMRMLMarkupsDisplayNode* dispNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkMRMLMarkupsDisplayNode"));
  if (!dispNode)
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::CreateDefaultDisplayNodes failed: scene failed to instantiate a vtkMRMLMarkupsDisplayNode node");
    return;
    }
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetLocked(int locked)
{
  if (this->Locked == locked)
    {
    return;
    }
  this->Locked = locked;

  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::LockModifiedEvent);
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode *vtkMRMLMarkupsNode::GetMarkupsDisplayNode()
{
  vtkMRMLDisplayNode *displayNode = this->GetDisplayNode();
  if (displayNode &&
      displayNode->IsA("vtkMRMLMarkupsDisplayNode"))
    {
    return vtkMRMLMarkupsDisplayNode::SafeDownCast(displayNode);
    }
  return nullptr;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::ControlPointExists(int n)
{
  if (n < 0 || n >= this->GetNumberOfControlPoints())
    {
    return false;
    }
  return (this->ControlPoints[static_cast<size_t>(n)] != nullptr);
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsNode::ControlPoint* vtkMRMLMarkupsNode::GetNthControlPointCustomLog(int n, const char* failedMethodName)
{
  if (n < 0 || n >= this->GetNumberOfControlPoints())
    {
      vtkErrorMacro("vtkMRMLMarkupsNode::" << failedMethodName << " failed: control point " <<
        n << " does not exist");
    return nullptr;
    }

  ControlPoint* controlPoint = this->ControlPoints[static_cast<size_t>(n)];
  if (!controlPoint)
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::" << failedMethodName << " failed: control point " <<
      n << " is invalid");
    }

  return controlPoint;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNumberOfControlPoints()
{
  return static_cast<int> (this->ControlPoints.size());
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsNode::ControlPoint* vtkMRMLMarkupsNode::GetNthControlPoint(int n)
{
  if (n < 0 || n >= this->GetNumberOfControlPoints())
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::GetNthControlPoint failed: control point " <<
      n << " does not exist");
    return nullptr;
    }

  ControlPoint* controlPoint = this->ControlPoints[static_cast<size_t>(n)];
  if (!controlPoint)
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::GetNthControlPoint failed: control point " <<
      n << " is invalid");
    }

  return controlPoint;
}

//-----------------------------------------------------------
std::vector< vtkMRMLMarkupsNode::ControlPoint* > * vtkMRMLMarkupsNode::GetControlPoints()
{
  return &this->ControlPoints;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddControlPoint(ControlPoint *controlPoint, bool autoLabel/*=true*/)
{
  if (this->MaximumNumberOfControlPoints >= 0 &&
      this->GetNumberOfControlPoints() + 1 > this->MaximumNumberOfControlPoints)
    {
    vtkErrorMacro("AddNControlPoints: number of points major than maximum number of control points allowed.");
    return -1;
    }

  if (this->GetFixedNumberOfControlPoints())
      {
      vtkErrorMacro("AddNControlPoints: Markup node control point number is locked.");
      return -1;
      }
  // generate a unique id based on list policy
  if (controlPoint->ID.empty())
    {
    controlPoint->ID = this->GenerateUniqueControlPointID();
    }
  if (controlPoint->Label.empty() && autoLabel)
    {
    controlPoint->Label = this->GenerateControlPointLabel(this->LastUsedControlPointNumber);
    }

  this->ControlPoints.push_back(controlPoint);

  if (!this->GetDisableModifiedEvent())
    {
    // Add point to CurveInputPoly
    // TODO: set point mask based on PositionStatus
    this->CurveInputPoly->GetPoints()->InsertNextPoint(controlPoint->Position);
    this->CurveInputPoly->GetPoints()->Modified();
    this->UpdateInteractionHandleToWorldMatrix();
    }

  int controlPointIndex = this->GetNumberOfControlPoints() - 1;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointAddedEvent, static_cast<void*>(&controlPointIndex));
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&controlPointIndex));
  if (controlPoint->PositionStatus == vtkMRMLMarkupsNode::PositionDefined)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionDefinedEvent, static_cast<void*>(&controlPointIndex));
    }
  if (controlPoint->PositionStatus == PositionMissing)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionMissingEvent, static_cast<void*>(&controlPointIndex));
    }

  this->StorableModifiedTime.Modified();
  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
  return controlPointIndex;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddNControlPoints(int n, std::string label, double point[3])
{
  if (point)
    {
    vtkVector3d pointVector(point);
    return this->AddNControlPoints(n, label, &pointVector);
    }
  else
    {
    return this->AddNControlPoints(n, label);
    }
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddNControlPoints(int n, std::string label /*=std::string()*/, vtkVector3d* point /*=nullptr*/)
{
  if (n < 0)
    {
    vtkErrorMacro("AddNControlPoints: invalid number of points " << n);
    return -1;
    }

  if (this->MaximumNumberOfControlPoints >= 0 && this->GetNumberOfControlPoints() + n > this->MaximumNumberOfControlPoints)
    {
    vtkErrorMacro("AddNControlPoints: number of existing points (" << this->GetNumberOfControlPoints()
      << ") plus requested number of new points (" << n << ") are more than maximum number of control points allowed ("
      << this->MaximumNumberOfControlPoints << ")");
    return -1;
    }

  if (this->GetFixedNumberOfControlPoints())
    {
    vtkErrorMacro("AddNControlPoints: Markup node control point number is locked.");
    return -1;
    }

  int controlPointIndex = -1;
  for (int i = 0; i < n; i++)
    {
    ControlPoint *controlPoint = new ControlPoint;
    controlPoint->Label = label;
    if (point != nullptr)
      {
      controlPoint->Position[0] = point->GetX();
      controlPoint->Position[1] = point->GetY();
      controlPoint->Position[2] = point->GetZ();
      controlPoint->PositionStatus = PositionDefined;
      }
    else
      {
      controlPoint->PositionStatus = PositionUndefined;
      }
    controlPointIndex = this->AddControlPoint(controlPoint);
    }

  return controlPointIndex;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddControlPointWorld(double x, double y, double z, std::string label /*=std::string()*/)
{
  return this->AddControlPointWorld(vtkVector3d(x, y, z), label);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddControlPointWorld(double pointWorld[3], std::string label /*=std::string()*/)
{
  return this->AddControlPointWorld(vtkVector3d(pointWorld), label);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddControlPointWorld(vtkVector3d pointWorld, std::string label /*=std::string()*/)
{
  vtkVector3d point;
  this->TransformPointFromWorld(pointWorld, point);
  return this->AddNControlPoints(1, label, &point);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddControlPoint(double x, double y, double z, std::string label /*=std::string()*/)
{
  return this->AddControlPoint(vtkVector3d(x, y, z), label);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddControlPoint(double point[3], std::string label /*=std::string()*/)
{
  return this->AddControlPoint(vtkVector3d(point), label);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddControlPoint(vtkVector3d point, std::string label /*=std::string()*/)
{
  return this->AddNControlPoints(1, label, &point);
}

//-----------------------------------------------------------
vtkVector3d vtkMRMLMarkupsNode::GetNthControlPointPositionVector(int pointIndex)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(pointIndex, "GetNthControlPointPositionVector");
  if (!controlPoint)
    {
    return vtkVector3d(0, 0, 0);
    }
  return vtkVector3d(controlPoint->Position);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthControlPointPosition(int pointIndex, double point[3])
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(pointIndex, "GetNthControlPointPosition");
  if (!controlPoint)
    {
    point[0] = 0.0;
    point[1] = 0.0;
    point[2] = 0.0;
    return;
    }

  double* position = controlPoint->Position;
  point[0] = position[0];
  point[1] = position[1];
  point[2] = position[2];
}

//-----------------------------------------------------------
double* vtkMRMLMarkupsNode::GetNthControlPointPosition(int pointIndex)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(pointIndex, "GetNthControlPointPosition");
  if (!controlPoint)
    {
    return nullptr;
    }

  return controlPoint->Position;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::GetNthControlPointPositionWorld(int pointIndex, double worldxyz[3])
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(pointIndex, "GetNthControlPointPositionWorld");
  if (!controlPoint)
    {
    return 0;
    }
  this->TransformPointToWorld(controlPoint->Position, worldxyz);
  return 1;
}

//-----------------------------------------------------------
vtkVector3d vtkMRMLMarkupsNode::GetNthControlPointPositionWorld(int pointIndex)
{
  vtkVector3d worldxyz(0.0, 0.0, 0.0);
  this->GetNthControlPointPositionWorld(pointIndex, worldxyz.GetData());
  return worldxyz;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::RemoveNthControlPoint(int pointIndex)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(pointIndex, "RemoveNthControlPoint");
  if (!controlPoint)
    {
    return;
    }
  if (this->GetFixedNumberOfControlPoints())
    {
    vtkErrorMacro("RemoveNthControlPoint: Markup node control point number locked.");
    return;
    }
  // Allow reusing last control point number (to prevent continuously
  // incrementing the number in the control point's name when adding/removing preview points).
  std::string lastAutoGeneratedLabel = this->GenerateControlPointLabel(this->LastUsedControlPointNumber);
  if (lastAutoGeneratedLabel == this->GetNthControlPointLabel(pointIndex))
    {
    this->LastUsedControlPointNumber--;
    }

  bool positionWasDefined = (this->ControlPoints[static_cast<unsigned int>(pointIndex)]->PositionStatus == vtkMRMLMarkupsNode::PositionDefined);
  bool positionWasMissing = (this->ControlPoints[static_cast<unsigned int>(pointIndex)]->PositionStatus == vtkMRMLMarkupsNode::PositionMissing);

  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointAboutToBeRemovedEvent, static_cast<void*>(&pointIndex));

  delete this->ControlPoints[static_cast<unsigned int> (pointIndex)];
  this->ControlPoints.erase(this->ControlPoints.begin() + pointIndex);

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }
  if (positionWasDefined)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionUndefinedEvent, static_cast<void*>(&pointIndex));
    }
  if (positionWasMissing)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionNonMissingEvent, static_cast<void*>(&pointIndex));
    }
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointRemovedEvent, static_cast<void*>(&pointIndex));
  this->StorableModifiedTime.Modified();
  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
}

//-----------------------------------------------------------
bool vtkMRMLMarkupsNode::InsertControlPoint(ControlPoint *controlPoint, int targetIndex)
{
  // generate a unique id based on list policy
  if (controlPoint->ID.empty())
    {
    controlPoint->ID = this->GenerateUniqueControlPointID();
    }

  int listSize = this->GetNumberOfControlPoints();
  int destIndex = targetIndex;
  if (targetIndex < 0)
    {
    destIndex = 0;
    }
  else if (targetIndex > listSize)
    {
    destIndex = listSize;
    }

  std::vector < ControlPoint* >::iterator pos = this->ControlPoints.begin() + destIndex;
  this->ControlPoints.insert(pos, controlPoint);

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }
  // let observers know that a markup was added
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointAddedEvent, static_cast<void*>(&targetIndex));
  if (controlPoint->PositionStatus == vtkMRMLMarkupsNode::PositionDefined)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionDefinedEvent, static_cast<void*>(&targetIndex));
    }
  if (controlPoint->PositionStatus == vtkMRMLMarkupsNode::PositionMissing)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionMissingEvent, static_cast<void*>(&targetIndex));
    }
  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
  return true;
}

//-----------------------------------------------------------
bool vtkMRMLMarkupsNode::InsertControlPointWorld(int n, double pointWorld[3], std::string label)
{
  return this->InsertControlPointWorld(n, vtkVector3d(pointWorld), label);
}

//-----------------------------------------------------------
bool vtkMRMLMarkupsNode::InsertControlPointWorld(int n, vtkVector3d pointWorld, std::string label)
{
  vtkVector3d point;
  this->TransformPointFromWorld(pointWorld, point);
  return this->InsertControlPoint(n, point, label);
}

//-----------------------------------------------------------
bool vtkMRMLMarkupsNode::InsertControlPoint(int n, double point[3], std::string label)
{
  return this->InsertControlPoint(n, vtkVector3d(point), label);
}

//-----------------------------------------------------------
bool vtkMRMLMarkupsNode::InsertControlPoint(int n, vtkVector3d point, std::string label)
{
  ControlPoint *controlPoint = new ControlPoint;
  controlPoint->Label = label;
  controlPoint->Position[0] = point.GetX();
  controlPoint->Position[1] = point.GetY();
  controlPoint->Position[2] = point.GetZ();
  controlPoint->PositionStatus = PositionDefined;
  return this->InsertControlPoint(controlPoint, n);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::UpdateCurvePolyFromControlPoints()
{
  // Add points
  vtkPoints* points = this->CurveInputPoly->GetPoints();
  points->Reset();
  bool includePreview = true;
  int numberOfControlPoints = this->GetNumberOfControlPoints();
  int numberOfDefinedControlPoints = this->GetNumberOfDefinedControlPoints(includePreview);
  if (numberOfDefinedControlPoints == 0)
    {
    points->Squeeze();
    }

  for (int i = 0; i < numberOfControlPoints; i++)
    {
    if (this->ControlPoints[i]->PositionStatus == PositionDefined ||
      this->ControlPoints[i]->PositionStatus == PositionPreview)
      {
      points->InsertNextPoint(this->ControlPoints[i]->Position);
      }
    }
  points->Modified();

}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SwapControlPoints(int m1, int m2)
{
  ControlPoint *controlPoint1 = this->GetNthControlPointCustomLog(m1, "SwapControlPoints");
  ControlPoint *controlPoint2 = this->GetNthControlPointCustomLog(m2, "SwapControlPoints");
  if (!controlPoint1 || !controlPoint2)
    {
    return;
    }

  // make a copy of the first control point
  ControlPoint controlPoint1Backup = *controlPoint1;
  // copy the second control point into the first
  *controlPoint1 = *controlPoint2;
  // and copy the backup of the first one into the second
  *controlPoint2 = controlPoint1Backup;

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }

  // and let listeners know that two control points have changed
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&m1));
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&m2));
  this->StorableModifiedTime.Modified();
  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPosition(const int pointIndex,
    const double position[3], int positionStatus/*=PositionDefined*/)
{
  this->SetNthControlPointPosition(pointIndex, position[0], position[1], position[2], positionStatus);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPosition(const int pointIndex,
  const double x, const double y, const double z, int positionStatus/*=PositionDefined*/)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(pointIndex, "SetNthControlPointPosition");
  if (!controlPoint)
    {
    return;
    }

  // TODO: return if no modification
  double* controlPointPosition = controlPoint->Position;

  controlPointPosition[0] = x;
  controlPointPosition[1] = y;
  controlPointPosition[2] = z;
  int oldPositionStatus = controlPoint->PositionStatus;
  controlPoint->PositionStatus = positionStatus;

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }

  // throw an event to let listeners know the position has changed
  int n = pointIndex;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  if (oldPositionStatus != PositionDefined && positionStatus == PositionDefined)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionDefinedEvent, static_cast<void*>(&n));
    }
  else if (oldPositionStatus == PositionDefined && positionStatus != PositionDefined)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionUndefinedEvent, static_cast<void*>(&n));
    }
  if (oldPositionStatus != PositionMissing && positionStatus == PositionMissing)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionMissingEvent, static_cast<void*>(&n));
    }
  else if (oldPositionStatus == PositionMissing && positionStatus != PositionMissing)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionNonMissingEvent, static_cast<void*>(&n));
    }
  this->StorableModifiedTime.Modified();

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
  if (this->GetDisplayNode())
    {
    this->GetDisplayNode()->UpdateScalarRange();
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionWorld(const int pointIndex,
    const double positionWorld[3], int positionStatus/*=PositionDefined*/)
{
  this->SetNthControlPointPositionWorld(pointIndex,
    positionWorld[0], positionWorld[1], positionWorld[2], positionStatus);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionWorld(const int pointIndex,
  const double x, const double y, const double z, int positionStatus/*=PositionDefined*/)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(pointIndex, "SetNthControlPointPositionWorld");
  if (!controlPoint)
    {
    return;
    }
  vtkVector3d markupxyz;
  TransformPointFromWorld(vtkVector3d(x,y,z), markupxyz);
  this->SetNthControlPointPosition(pointIndex, markupxyz[0], markupxyz[1], markupxyz[2], positionStatus);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::TransformOrientationMatrixFromNodeToWorld(
  const double position_Node[3], const double orientationMatrix_Node[9], double orientationMatrix_World[9])
{
  double xAxis_World[3] = { orientationMatrix_Node[0], orientationMatrix_Node[3], orientationMatrix_Node[6] };
  double yAxis_World[3] = { orientationMatrix_Node[1], orientationMatrix_Node[4], orientationMatrix_Node[7] };
  double zAxis_World[3] = { orientationMatrix_Node[2], orientationMatrix_Node[5], orientationMatrix_Node[8] };
  if (this->GetParentTransformNode())
    {
    // Transform orientation matrix from world
    vtkNew<vtkGeneralTransform> nodeToWorldTransform;
    this->GetParentTransformNode()->GetTransformToWorld(nodeToWorldTransform);

    double xAxis_Node[3] = { orientationMatrix_Node[0], orientationMatrix_Node[3], orientationMatrix_Node[6] };
    double yAxis_Node[3] = { orientationMatrix_Node[1], orientationMatrix_Node[4], orientationMatrix_Node[7] };
    double zAxis_Node[3] = { orientationMatrix_Node[2], orientationMatrix_Node[5], orientationMatrix_Node[8] };

    nodeToWorldTransform->TransformVectorAtPoint(position_Node, xAxis_Node, xAxis_World);
    nodeToWorldTransform->TransformVectorAtPoint(position_Node, yAxis_Node, yAxis_World);
    nodeToWorldTransform->TransformVectorAtPoint(position_Node, zAxis_Node, zAxis_World);
    }

  for (int i = 0; i < 3; ++i)
    {
    orientationMatrix_World[3*i]   = xAxis_World[i];
    orientationMatrix_World[3*i+1] = yAxis_World[i];
    orientationMatrix_World[3*i+2] = zAxis_World[i];
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::TransformOrientationMatrixFromWorldToNode(
  const double position_World[3], const double orientationMatrix_World[9], double orientationMatrix_Node[9])
{
  double xAxis_Node[3] = { orientationMatrix_World[0], orientationMatrix_World[3], orientationMatrix_World[6] };
  double yAxis_Node[3] = { orientationMatrix_World[1], orientationMatrix_World[4], orientationMatrix_World[7] };
  double zAxis_Node[3] = { orientationMatrix_World[2], orientationMatrix_World[5], orientationMatrix_World[8] };
  if (this->GetParentTransformNode())
    {
    // Transform orientation matrix from world
    vtkNew<vtkGeneralTransform> worldToNodeTransform;
    this->GetParentTransformNode()->GetTransformFromWorld(worldToNodeTransform);

    double xAxis_World[3] = { orientationMatrix_Node[0], orientationMatrix_Node[3], orientationMatrix_Node[6] };
    double yAxis_World[3] = { orientationMatrix_Node[1], orientationMatrix_Node[4], orientationMatrix_Node[7] };
    double zAxis_World[3] = { orientationMatrix_Node[2], orientationMatrix_Node[5], orientationMatrix_Node[8] };

    worldToNodeTransform->TransformVectorAtPoint(position_World, xAxis_World, xAxis_Node);
    worldToNodeTransform->TransformVectorAtPoint(position_World, yAxis_World, yAxis_Node);
    worldToNodeTransform->TransformVectorAtPoint(position_World, zAxis_World, zAxis_Node);
    }
  for (int i = 0; i < 3; ++i)
    {
    orientationMatrix_Node[3 * i] = xAxis_Node[i];
    orientationMatrix_Node[3 * i + 1] = yAxis_Node[i];
    orientationMatrix_Node[3 * i + 2] = zAxis_Node[i];
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionOrientationWorld(
  const int pointIndex, const double pos[3], const double orientationMatrix_World[9],
  const char* associatedNodeID, int positionStatus/*=PositionDefined*/)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(pointIndex, "SetNthControlPointPositionOrientationWorld");
  if (!controlPoint)
    {
    return;
    }
  // TODO: return if no modification
  this->TransformPointFromWorld(pos, controlPoint->Position);
  int oldPositionStatus = controlPoint->PositionStatus;
  controlPoint->PositionStatus = positionStatus;

  if (positionStatus != vtkMRMLMarkupsNode::PositionPreview)
    {
    controlPoint->AutoCreated = false;
    }
  // Transform orientation matrix to world
  this->TransformOrientationMatrixFromWorldToNode(pos, orientationMatrix_World, controlPoint->OrientationMatrix);
  if (associatedNodeID)
    {
    controlPoint->AssociatedNodeID = associatedNodeID;
    }
  else
    {
    controlPoint->AssociatedNodeID.clear();
    }

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }

  // throw an event to let listeners know the position has changed
  int n = pointIndex;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  if (oldPositionStatus != PositionDefined && positionStatus == PositionDefined)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionDefinedEvent, static_cast<void*>(&n));
    }
  else if (oldPositionStatus == PositionDefined && positionStatus != PositionDefined)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionUndefinedEvent, static_cast<void*>(&n));
    }
  if (oldPositionStatus != PositionMissing && positionStatus == PositionMissing)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionMissingEvent, static_cast<void*>(&n));
    }
  else if (oldPositionStatus == PositionMissing && positionStatus != PositionMissing)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionNonMissingEvent, static_cast<void*>(&n));
    }
  this->StorableModifiedTime.Modified();

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }
}

//-----------------------------------------------------------
vtkVector3d vtkMRMLMarkupsNode::GetCenterOfRotation()
{
  return this->CenterOfRotation;
}

//-----------------------------------------------------------
bool vtkMRMLMarkupsNode::GetCenterOfRotation(double point[3])
{
  point[0] = this->CenterOfRotation.GetX();
  point[1] = this->CenterOfRotation.GetY();
  point[2] = this->CenterOfRotation.GetZ();
  return 1;
}

//-----------------------------------------------------------
bool vtkMRMLMarkupsNode::GetCenterOfRotationWorld(double worldxyz[3])
{
  vtkVector3d world;
  this->TransformPointToWorld(this->GetCenterOfRotation(), world);
  worldxyz[0] = world[0];
  worldxyz[1] = world[1];
  worldxyz[2] = world[2];
  return true;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetCenterOfRotation(const double pos[3])
{
  this->CenterOfRotation = vtkVector3d(pos);
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::CenterOfRotationModifiedEvent);
  this->StorableModifiedTime.Modified();
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetCenterOfRotation(const double x, const double y, const double z)
{
  double pos[3] = { x, y, z };
  this->SetCenterOfRotation(pos);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetCenterOfRotationWorld(const double x, const double y, const double z)
{
  vtkVector3d centerxyz;
  TransformPointFromWorld(vtkVector3d(x,y,z), centerxyz);
  this->SetCenterOfRotation(centerxyz[0], centerxyz[1], centerxyz[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetCenterOfRotationWorld(const double positionWorld[3])
{
  double centerxyz[3]={0.0, 0.0, 0.0};
  TransformPointFromWorld(positionWorld, centerxyz);
  this->SetCenterOfRotation(centerxyz);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientation(int n, double w, double x, double y, double z)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointOrientation");
  if (!controlPoint)
    {
    return;
    }
  // TODO: return if no modification

  double wxyz[] = { w, x, y, z };
  vtkMRMLMarkupsNode::ConvertOrientationWXYZToMatrix(wxyz, controlPoint->OrientationMatrix);

  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();
  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientation(int n, const double wxyz[4])
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointOrientation");
  if (!controlPoint)
    {
    return;
    }
  // TODO: return if no modification

  vtkMRMLMarkupsNode::ConvertOrientationWXYZToMatrix(wxyz, controlPoint->OrientationMatrix);

  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();
  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthControlPointOrientation(int n, double orientation[4])
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointOrientation");
  if (!controlPoint)
    {
    return;
    }
  vtkMRMLMarkupsNode::ConvertOrientationMatrixToWXYZ(controlPoint->OrientationMatrix, orientation);
}

//-----------------------------------------------------------
double* vtkMRMLMarkupsNode::GetNthControlPointOrientationMatrix(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointOrientationMatrix");
  if (!controlPoint)
    {
    static double identity[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    return identity;
    }
  return controlPoint->OrientationMatrix;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthControlPointOrientationMatrix(int n, vtkMatrix3x3* orientationMatrix)
{
  if (!orientationMatrix)
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::GetNthControlPointOrientationMatrix failed: invalid output matrix.");
    return;
    }
  std::copy_n(this->GetNthControlPointOrientationMatrix(n), 9, orientationMatrix->GetData());
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientationMatrix(int n, double orientationMatrix[9])
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointOrientationMatrix");
  if (!controlPoint)
    {
    return;
    }
  std::copy_n(orientationMatrix, 9, controlPoint->OrientationMatrix);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientationMatrix(int n, vtkMatrix3x3* orientationMatrix)
{
  if (!orientationMatrix)
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::SetNthControlPointOrientationMatrix failed: invalid input matrix.");
    return;
    }

  this->SetNthControlPointOrientationMatrix(n, orientationMatrix->GetData());
}

//-----------------------------------------------------------
vtkVector<double, 9> vtkMRMLMarkupsNode::GetNthControlPointOrientationMatrixWorld(int n)
{
  ControlPoint* controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointOrientationMatrixWorld");
  vtkVector<double, 9> orientationMatrix_World;
  if (!controlPoint)
    {
    static double identity[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    std::copy_n(identity, 9, orientationMatrix_World.GetData());
    return orientationMatrix_World;
    }

  this->TransformOrientationMatrixFromNodeToWorld(controlPoint->Position, controlPoint->OrientationMatrix, orientationMatrix_World.GetData());
  return orientationMatrix_World;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthControlPointOrientationMatrixWorld(int n, double orientationMatrix_World[9])
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointOrientationMatrixWorld");
  if (!controlPoint)
    {
    static double identity[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    std::copy_n(identity, 9, orientationMatrix_World);
    return;
    }

  this->TransformOrientationMatrixFromNodeToWorld(controlPoint->Position, controlPoint->OrientationMatrix, orientationMatrix_World);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthControlPointOrientationMatrixWorld(int n, vtkMatrix3x3* orientationMatrix_World)
{
  if (!orientationMatrix_World)
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::GetNthControlPointOrientationMatrixWorld failed: invalid output matrix.");
    return;
    }
  this->GetNthControlPointOrientationMatrixWorld(n, orientationMatrix_World->GetData());
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientationMatrixWorld(int n, const double orientationMatrix[9])
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointOrientationMatrixWorld");
  if (!controlPoint)
    {
    return;
    }

  double orientationMatrix_Node[9] = { 1.0, 0.0, 0.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0 };
  double controlPointPosition_World[3] = { 0.0, 0.0, 0.0 };
  this->TransformPointToWorld(controlPoint->Position, controlPointPosition_World);
  this->TransformOrientationMatrixFromWorldToNode(controlPointPosition_World, orientationMatrix, orientationMatrix_Node);
  this->SetNthControlPointOrientationMatrix(n, orientationMatrix_Node);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientationMatrixWorld(int n, vtkMatrix3x3* orientationMatrix_World)
{
  if (!orientationMatrix_World)
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::SetNthControlPointOrientationMatrixWorld failed: invalid input matrix.");
    return;
    }
  this->SetNthControlPointOrientationMatrixWorld(n, orientationMatrix_World->GetData());
}

//-----------------------------------------------------------
vtkVector3d vtkMRMLMarkupsNode::GetNthControlPointNormal(int n)
{
  vtkVector3d normal;
  this->GetNthControlPointNormal(n, normal.GetData());
  return normal;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthControlPointNormal(int n, double normal[3])
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointNormal");
  if (!controlPoint)
    {
    return;
    }
  double* orientationMatrix = this->GetNthControlPoint(n)->OrientationMatrix;
  normal[0] = orientationMatrix[2];
  normal[1] = orientationMatrix[5];
  normal[2] = orientationMatrix[8];
}

//-----------------------------------------------------------
vtkVector3d vtkMRMLMarkupsNode::GetNthControlPointNormalWorld(int n)
{
  vtkVector3d normalWorld;
  this->GetNthControlPointNormalWorld(n, normalWorld.GetData());
  return normalWorld;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthControlPointNormalWorld(int n, double normalWorld[3])
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointNormalWorld");
  if (!controlPoint)
    {
    return;
    }

  double normalNode[3] = { 0.0, 0.0, 1.0 };
  this->GetNthControlPointNormal(n, normalNode);
  this->CurvePolyToWorldTransform->TransformVectorAtPoint(
    &(controlPoint->Position[0]),
    normalNode,
    normalWorld);
}

//-----------------------------------------------------------
vtkVector4d vtkMRMLMarkupsNode::GetNthControlPointOrientationVector(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointOrientationVector");
  if (!controlPoint)
    {
    return vtkVector4d(0, 0, 0, 0);
    }

  double orientationWXYZ[4] = { 1.0, 0.0, 0.0, 0.0 };
  vtkMRMLMarkupsNode::ConvertOrientationMatrixToWXYZ(controlPoint->OrientationMatrix, orientationWXYZ);

  vtkVector4d orientationXYZW;
  // Note the order difference: vtkVector4d stores orientation as xyzw
  orientationXYZW.SetX(orientationWXYZ[1]);
  orientationXYZW.SetY(orientationWXYZ[2]);
  orientationXYZW.SetZ(orientationWXYZ[3]);
  orientationXYZW.SetW(orientationWXYZ[0]);
  return orientationXYZW;
}

//-----------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthControlPointAssociatedNodeID(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointAssociatedNodeID");
  if (!controlPoint)
    {
    return std::string("");
    }
  return controlPoint->AssociatedNodeID;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointAssociatedNodeID(int n, std::string id)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointAssociatedNodeID");
  if (!controlPoint)
    {
    return;
    }
  if (controlPoint->AssociatedNodeID == id)
    {
    // no change
    return;
    }
  controlPoint->AssociatedNodeID = std::string(id.c_str());
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();
  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
}

//-----------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthControlPointID(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointID");
  if (!controlPoint)
    {
    return std::string("");
    }
  return controlPoint->ID;
}

//-------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNthControlPointIndexByID(const char* controlPointID)
{
  if (!controlPointID)
    {
    return -1;
    }
  for (int controlPointIndex = 0; controlPointIndex < this->GetNumberOfControlPoints(); controlPointIndex++)
    {
    ControlPoint *compareControlPoint = this->ControlPoints[controlPointIndex];
    if (compareControlPoint &&
        strcmp(compareControlPoint->ID.c_str(), controlPointID) == 0)
      {
      return controlPointIndex;
      }
    }
  return -1;
}

//-------------------------------------------------------------------------
vtkMRMLMarkupsNode::ControlPoint* vtkMRMLMarkupsNode::GetNthControlPointByID(const char* controlPointID)
{
  if (!controlPointID)
    {
    return nullptr;
    }
  int controlPointIndex = this->GetNthControlPointIndexByID(controlPointID);
  if (controlPointIndex < 0 || controlPointIndex >= this->GetNumberOfControlPoints())
    {
    return nullptr;
    }
  return this->GetNthControlPoint(controlPointIndex);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointID(int n, std::string id)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointID");
  if (!controlPoint)
    {
    return;
    }
  if (controlPoint->ID.compare(id) == 0)
    {
    // no change
    return;
    }
  controlPoint->ID = id;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthControlPointSelected(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointSelected");
  if (!controlPoint)
   {
   return false;
   }
 return controlPoint->Selected;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointSelected(int n, bool flag)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointSelected");
  if (!controlPoint)
    {
    return;
    }
  if (controlPoint->Selected == flag)
    {
    // no change
    return;
    }
  controlPoint->Selected = flag;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();
  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthControlPointLocked(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointLocked");
  if (!controlPoint)
    {
    return false;
    }
  return controlPoint->Locked;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointLocked(int n, bool flag)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointLocked");
  if (!controlPoint)
    {
    return;
    }
  if (controlPoint->Locked == flag)
    {
    // no change
    return;
    }
  controlPoint->Locked = flag;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthControlPointVisibility(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointVisibility");
  if (!controlPoint)
    {
    return false;
    }
  return controlPoint->Visibility;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthControlPointPositionVisibility(int n)
{
  ControlPoint* controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointPositionVisibility");
  if (!controlPoint)
    {
    return false;
    }
  bool positionStatusDefined = (controlPoint->PositionStatus == PositionDefined) || (controlPoint->PositionStatus == PositionPreview);
  return  positionStatusDefined;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointVisibility(int n, bool flag)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointVisibility");
  if (!controlPoint)
    {
    return;
    }
  if (controlPoint->Visibility == flag)
    {
    // no change
    return;
    }
  controlPoint->Visibility = flag;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthControlPointLabel(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointLabel");
  if (!controlPoint)
    {
    return std::string("");
    }
  return controlPoint->Label;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointLabel(int n, std::string label)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointLabel");
  if (!controlPoint)
    {
    return;
    }
  if (!controlPoint->Label.compare(label))
    {
    // no change
    return;
    }
  controlPoint->Label = label;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthControlPointDescription(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointDescription");
  if (!controlPoint)
    {
    return std::string("");
    }
  return controlPoint->Description;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointDescription(int n, std::string description)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointDescription");
  if (!controlPoint)
    {
    return;
    }
  if (!controlPoint->Description.compare(description))
    {
    // no change
    return;
    }
  controlPoint->Description = description;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ApplyTransform(vtkAbstractTransform* transform)
{
  MRMLNodeModifyBlocker blocker(this);

  vtkLinearTransform* linearTransform = vtkLinearTransform::SafeDownCast(transform);
  if (linearTransform)
    {
    // The orientation of some markup types are not fully defined by their control points (line, etc.).
    // For these cases, we need to manually apply a rotation to the interaction handles.
    vtkNew<vtkTransform> handleToWorldTransform;
    handleToWorldTransform->PostMultiply();
    handleToWorldTransform->Concatenate(this->InteractionHandleToWorldMatrix);
    handleToWorldTransform->Concatenate(linearTransform);
    this->InteractionHandleToWorldMatrix->DeepCopy(handleToWorldTransform->GetMatrix());
    }

  int numControlPoints = this->GetNumberOfControlPoints();
  double xyzIn[3];
  double xyzOut[3];
  for (int controlPointIndex = 0; controlPointIndex < numControlPoints; controlPointIndex++)
    {
    this->GetNthControlPointPosition(controlPointIndex, xyzIn);
    transform->TransformPoint(xyzIn,xyzOut);
    int status = this->GetNthControlPointPositionStatus(controlPointIndex);
    this->SetNthControlPointPosition(controlPointIndex, xyzOut, status);
    }
  this->StorableModifiedTime.Modified();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::
WriteCLI(std::vector<std::string>& commandLine, std::string prefix,
         int coordinateSystem, int multipleFlag)
{
  int numControlPoints = this->GetNumberOfControlPoints();

  // check if the coordinate system flag is set to LPS, otherwise assume RAS
  bool useLPS = (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS);

  // loop over the control points
  for (int m = 0; m < numControlPoints; m++)
    {
    // only use selected markups
    if (this->GetNthControlPointSelected(m))
      {
      std::stringstream ss;
      double point[3];
      this->GetNthControlPointPosition(m, point);
      if (useLPS)
        {
        point[0] = -point[0];
        point[1] = -point[1];
        }
      // write
      if (prefix.compare("") != 0)
        {
        commandLine.push_back(prefix);
        }
      // avoid scientific notation
      //ss.precision(5);
      //ss << std::fixed << point[0] << "," <<  point[1] << "," <<  point[2] ;
      ss << point[0] << "," <<  point[1] << "," <<  point[2];
      commandLine.push_back(ss.str());
      if (multipleFlag == 0)
        {
        // only print out one markup, but print out all the points in that markup
        // (if have a ruler, need to do 2 points)
        break;
        }
      }
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetModifiedSinceRead()
{
  if (this->Superclass::GetModifiedSinceRead())
    {
    return true;
    }
  vtkPoints* points = this->CurveInputPoly->GetPoints();
  if (points != nullptr && points->GetMTime() > this->GetStoredTime())
    {
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::ResetNthControlPointID(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "ResetNthControlPointID");
  if (!controlPoint)
    {
    return false;
    }

  this->SetNthControlPointID(n, this->GenerateUniqueControlPointID());
  return true;
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GenerateUniqueControlPointID()
{
  this->LastUsedControlPointNumber++;
  return std::to_string(this->LastUsedControlPointNumber);
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GenerateControlPointLabel(int controlPointIndex)
{
  std::string formatString = this->ReplaceListNameInControlPointLabelFormat();
  char buf[128];
  buf[sizeof(buf) - 1] = 0; // make sure the string is zero-terminated
  snprintf(buf, sizeof(buf) - 1, formatString.c_str(), controlPointIndex);
  return std::string(buf);
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetControlPointLabelFormat()
{
  return this->ControlPointLabelFormat;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetControlPointLabelFormat(std::string format)
{
  if (this->ControlPointLabelFormat.compare(format) == 0)
    {
    return;
    }
  this->ControlPointLabelFormat = format;

  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::LabelFormatModifiedEvent);
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::ReplaceListNameInControlPointLabelFormat()
{
  std::string newFormatString = this->ControlPointLabelFormat;

  // Long-name replacement
  size_t replacePos = newFormatString.find("%N");
  if (replacePos != std::string::npos)
    {
    // replace the special character with the list name, or an empty string if
    // no list name is set
    std::string name;
    if (this->GetName() != nullptr)
      {
      name = std::string(this->GetName());
      }
    newFormatString.replace(replacePos, 2, name);
    }

  // Short-name replacement
  replacePos = newFormatString.find("%S");
  if (replacePos != std::string::npos)
    {
    // replace the special character with the list name, or an empty string if
    // no list name is set
    std::string name;
    if (this->GetDefaultNodeNamePrefix() != nullptr)
      {
      name = std::string(this->GetDefaultNodeNamePrefix());
      }
    newFormatString.replace(replacePos, 2, name);
    }

  return newFormatString;
}

//----------------------------------------------------------------------
void vtkMRMLMarkupsNode::ConvertOrientationMatrixToWXYZ(const double orientationMatrix[9], double wxyz[4])
{
  if (!orientationMatrix || !wxyz)
    {
    return;
    }

  // adapted from vtkTransform::GetOrientationWXYZ

  double ortho[3][3];
  ortho[0][0] = orientationMatrix[0];
  ortho[0][1] = orientationMatrix[1];
  ortho[0][2] = orientationMatrix[2];
  ortho[1][0] = orientationMatrix[3];
  ortho[1][1] = orientationMatrix[4];
  ortho[1][2] = orientationMatrix[5];
  ortho[2][0] = orientationMatrix[6];
  ortho[2][1] = orientationMatrix[7];
  ortho[2][2] = orientationMatrix[8];

  if (vtkMath::Determinant3x3(ortho) < 0)
    {
    ortho[0][2] = -ortho[0][2];
    ortho[1][2] = -ortho[1][2];
    ortho[2][2] = -ortho[2][2];
    }

  vtkMath::Matrix3x3ToQuaternion(ortho, wxyz);

  // calc the return value wxyz
  double mag = sqrt(wxyz[1] * wxyz[1] + wxyz[2] * wxyz[2] + wxyz[3] * wxyz[3]);

  if (mag != 0.0)
    {
    wxyz[0] = 2.0 * vtkMath::DegreesFromRadians(atan2(mag, wxyz[0]));
    wxyz[1] /= mag;
    wxyz[2] /= mag;
    wxyz[3] /= mag;
    }
  else
    {
    wxyz[0] = 0.0;
    wxyz[1] = 0.0;
    wxyz[2] = 0.0;
    wxyz[3] = 1.0;
    }
}

//----------------------------------------------------------------------
void vtkMRMLMarkupsNode::ConvertOrientationWXYZToMatrix(const double orientationWXYZ[4], double orientationMatrix[9])
{
  if (!orientationWXYZ || !orientationMatrix)
    {
    return;
    }

  // adapted from vtkTransformConcatenation::Rotate(double angle, double x, double y, double z)

  double angle = orientationWXYZ[0];
  double x = orientationWXYZ[1];
  double y = orientationWXYZ[2];
  double z = orientationWXYZ[3];

  if (angle == 0.0 || (x == 0.0 && y == 0.0 && z == 0.0))
    {
    orientationMatrix[0] = 1.0;
    orientationMatrix[1] = 0.0;
    orientationMatrix[2] = 0.0;
    orientationMatrix[3] = 0.0;
    orientationMatrix[4] = 1.0;
    orientationMatrix[5] = 0.0;
    orientationMatrix[6] = 0.0;
    orientationMatrix[7] = 0.0;
    orientationMatrix[8] = 1.0;
    return;
    }

  // convert to radians
  angle = vtkMath::RadiansFromDegrees(angle);

  // make a normalized quaternion
  double w = cos(0.5*angle);
  double f = sin(0.5*angle) / sqrt(x*x + y * y + z * z);
  x *= f;
  y *= f;
  z *= f;

  double ww = w * w;
  double wx = w * x;
  double wy = w * y;
  double wz = w * z;

  double xx = x * x;
  double yy = y * y;
  double zz = z * z;

  double xy = x * y;
  double xz = x * z;
  double yz = y * z;

  double s = ww - xx - yy - zz;

  orientationMatrix[0] = xx * 2 + s;    // (0,0)
  orientationMatrix[3] = (xy + wz) * 2; // (1,0)
  orientationMatrix[6] = (xz - wy) * 2; // (2,0)

  orientationMatrix[1] = (xy - wz) * 2; // (0,1)
  orientationMatrix[4] = yy * 2 + s;    // (1,1)
  orientationMatrix[7] = (yz + wx) * 2; // (2,1)

  orientationMatrix[2] = (xz + wy) * 2; // (0,2)
  orientationMatrix[5] = (yz - wx) * 2; // (1,2)
  orientationMatrix[8] = zz * 2 + s;    // (2,2)
}

//----------------------------------------------------------------------
vtkPoints* vtkMRMLMarkupsNode::GetCurvePoints()
{
  this->CurveGenerator->Update();
  if (!this->CurveGenerator->GetOutput())
    {
    return nullptr;
    }
  return this->CurveGenerator->GetOutput()->GetPoints();
}

//----------------------------------------------------------------------
vtkPoints* vtkMRMLMarkupsNode::GetCurvePointsWorld()
{
  vtkPolyData* curvePolyDataWorld = this->GetCurveWorld();
  if (!curvePolyDataWorld)
    {
    return nullptr;
    }
  return curvePolyDataWorld->GetPoints();
}

//----------------------------------------------------------------------
vtkPolyData* vtkMRMLMarkupsNode::GetCurve()
{
  this->CurveGenerator->Update();
  return this->CurveGenerator->GetOutput();
}

//----------------------------------------------------------------------
vtkPolyData* vtkMRMLMarkupsNode::GetCurveWorld()
{
  if (this->GetNumberOfControlPoints() < 1)
    {
    return nullptr;
    }
  this->CurvePolyToWorldTransformer->Update();
  vtkPolyData* curvePolyDataWorld = this->CurvePolyToWorldTransformer->GetOutput();
  this->TransformedCurvePolyLocator->SetDataSet(curvePolyDataWorld);
  return curvePolyDataWorld;
}

//----------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLMarkupsNode::GetCurveWorldConnection()
{
  return this->CurvePolyToWorldTransformer->GetOutputPort();
}

//----------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetControlPointIndexFromInterpolatedPointIndex(vtkIdType interpolatedPointIndex)
{
  vtkIdType controlPointId = this->CurveGenerator->GetControlPointIdFromInterpolatedPointId(interpolatedPointIndex);
  if (controlPointId < 0)
    {
    controlPointId = this->GetNumberOfControlPoints();
    }
  return controlPointId;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::GetRASBounds(double bounds[6])
{
  vtkBoundingBox box;
  box.GetBounds(bounds);

  int numberOfControlPoints = this->GetNumberOfControlPoints();
  if (numberOfControlPoints == 0)
    {
    return;
    }
  double markup_RAS[4] = { 0, 0, 0, 1 };

  for (int i = 0; i < numberOfControlPoints; i++)
    {
    this->GetNthControlPointPositionWorld(i, markup_RAS);
    box.AddPoint(markup_RAS);
    }
  box.GetBounds(bounds);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::GetBounds(double bounds[6])
{
  vtkBoundingBox box;
  box.GetBounds(bounds);

  int numberOfControlPoints = this->GetNumberOfControlPoints();
  if (numberOfControlPoints == 0)
    {
    return;
    }
  double markupPos[4] = { 0, 0, 0 };

  for (int i = 0; i < numberOfControlPoints; i++)
    {
    this->GetNthControlPointPosition(i, markupPos);
    box.AddPoint(markupPos);
    }
  box.GetBounds(bounds);
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNthControlPointPositionStatus(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointPositionStatus");
  if (!controlPoint)
    {
    return PositionUndefined;
    }
  return controlPoint->PositionStatus;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::UnsetNthControlPointPosition(int n)
{
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(n, "UnsetNthControlPointPosition");
  if (!controlPoint)
    {
    return;
    }
  if (controlPoint->PositionStatus == PositionUndefined)
    {
    // no change
    return;
    }
  int oldPositionStatus = controlPoint->PositionStatus;
  controlPoint->PositionStatus = PositionUndefined;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionUndefinedEvent, static_cast<void*>(&n));
  if (oldPositionStatus == PositionMissing)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionNonMissingEvent, static_cast<void*>(&n));
    }

  this->StorableModifiedTime.Modified();

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateAllMeasurements();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionMissing(int n)
{
  ControlPoint* controlPoint = this->GetNthControlPointCustomLog(n, "MissingNthControlPointPosition");
  if (!controlPoint)
    {
    return;
    }
  if (controlPoint->PositionStatus == PositionMissing)
    {
    // no change
    return;
    }
  controlPoint->PositionStatus = PositionMissing;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointPositionMissingEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }

  this->UpdateMeasurementsInternal();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ResetNthControlPointPosition(int n)
{
  ControlPoint* controlPoint = this->GetNthControlPointCustomLog(n, "ResetNthControlPointPosition");
  if (!controlPoint)
    {
    return;
    }
  if (controlPoint->PositionStatus == PositionPreview)
    {
    // no change
    return;
    }
  int numberOfControlPoints = this->GetNumberOfControlPoints();
  for (int i = 0; i < numberOfControlPoints; i++)
    {
    int pointStatus = this->GetNthControlPointPositionStatus(i);
    if (pointStatus == vtkMRMLMarkupsNode::PositionPreview)
      {
      this->UnsetNthControlPointPosition(i);
      break;
      }
  }
  controlPoint->PositionStatus = PositionPreview;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }

  this->UpdateMeasurementsInternal();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::RestoreNthControlPointPosition(int n)
{
  ControlPoint* controlPoint = this->GetNthControlPointCustomLog(n, "RevertNthControlPointPosition");
  if (!controlPoint)
    {
    return;
    }
  if (controlPoint->PositionStatus == PositionDefined)
    {
    // no change
     return;
    }
  controlPoint->PositionStatus = PositionDefined;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();

  if (!this->GetDisableModifiedEvent())
    {
    this->UpdateCurvePolyFromControlPoints();
    this->UpdateInteractionHandleToWorldMatrix();
    }

  this->UpdateMeasurementsInternal();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointAutoCreated(int n, bool flag)
{
  ControlPoint* controlPoint = this->GetNthControlPointCustomLog(n, "SetNthControlPointAutoCreated");
  if (!controlPoint)
    {
    return;
    }
  if (controlPoint->AutoCreated == flag)
    {
    // no change
    return;
    }
  controlPoint->Locked = flag;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
  this->StorableModifiedTime.Modified();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthControlPointAutoCreated(int n)
{
  ControlPoint* controlPoint = this->GetNthControlPointCustomLog(n, "GetNthControlPointAutoCreated");
  if (!controlPoint)
    {
    return false;
    }
  else
    {
    return controlPoint->AutoCreated;
    }
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNumberOfDefinedControlPoints(bool includePreview/*=false*/)
{
  int numberOfDefinedControlPoints = 0;
  for (ControlPointsListType::iterator controlPointIt = this->ControlPoints.begin();
    controlPointIt != this->ControlPoints.end(); ++controlPointIt)
    {
    if ((*controlPointIt)->PositionStatus == PositionDefined)
      {
      numberOfDefinedControlPoints++;
      }
    else if (includePreview && (*controlPointIt)->PositionStatus == PositionPreview)
      {
      numberOfDefinedControlPoints++;
      }
    }
  return numberOfDefinedControlPoints;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNumberOfUndefinedControlPoints(bool includePreview/*=false*/)
{
  int numberOfUndefinedControlPoints = 0;
  for (ControlPointsListType::iterator controlPointIt = this->ControlPoints.begin();
    controlPointIt != this->ControlPoints.end(); ++controlPointIt)
    {
    if ((*controlPointIt)->PositionStatus == PositionUndefined)
      {
      numberOfUndefinedControlPoints++;
      }
    else if (includePreview && (*controlPointIt)->PositionStatus == PositionPreview)
      {
      numberOfUndefinedControlPoints++;
      }
    }
  return numberOfUndefinedControlPoints;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode)
{
  vtkMRMLTransformNode::GetTransformBetweenNodes(this->GetParentTransformNode(), nullptr, this->CurvePolyToWorldTransform);
  Superclass::OnTransformNodeReferenceChanged(transformNode);
  this->UpdateInteractionHandleToWorldMatrix();
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetClosestControlPointIndexToPositionWorld(double pos[3], bool visibleOnly/*=false*/)
{
  int numberOfControlPoints = this->GetNumberOfControlPoints();
  if (numberOfControlPoints <= 0)
    {
    return -1;
    }
  if (numberOfControlPoints == 1)
    {
    // there is one control point, so the closest one is the only one
    return 0;
    }
  vtkIdType indexOfClosestMarkup = -1;
  double closestDistanceSquare = 0;
  for (vtkIdType pointIndex = 0; pointIndex < numberOfControlPoints; pointIndex++)
    {
    if (visibleOnly && !this->GetNthControlPointVisibility(pointIndex))
      {
      // we need to find closest visible point but this one is not visible
      continue;
      }
    double currentPos[4] = { 0.0, 0.0, 0.0, 1.0 };
    this->GetNthControlPointPositionWorld(pointIndex, currentPos);
    double distanceSquare = vtkMath::Distance2BetweenPoints(pos, currentPos);
    if (distanceSquare < closestDistanceSquare || indexOfClosestMarkup < 0)
      {
      indexOfClosestMarkup = pointIndex;
      closestDistanceSquare = distanceSquare;
      }
    }
  return indexOfClosestMarkup;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::GetControlPointLabels(vtkStringArray* labels)
{
  if (!labels)
    {
    vtkErrorMacro("GetControlPointLabels failed: invalid labels");
    return;
    }
  int numberOfControlPoints = this->GetNumberOfControlPoints();
  labels->SetNumberOfValues(numberOfControlPoints);
  for (vtkIdType pointIndex = 0; pointIndex < numberOfControlPoints; pointIndex++)
    {
    labels->SetValue(pointIndex, this->GetNthControlPointLabel(pointIndex));
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetControlPointPositionsWorld(vtkPoints* points)
{
  if (!points)
    {
    this->RemoveAllControlPoints();
    return;
    }

  int wasModified = this->StartModify();
  this->IsUpdatingPoints = true;

  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  for (vtkIdType pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
    {
    double* posWorld = points->GetPoint(pointIndex);
    if (pointIndex < this->GetNumberOfControlPoints())
      {
      // point already exists, just update it
      this->SetNthControlPointPositionWorld(pointIndex, posWorld);
      }
    else
      {
      // need to add a new point
      vtkMRMLMarkupsNode::AddControlPointWorld(vtkVector3d(posWorld));
      }
    }
  while (this->GetNumberOfControlPoints() > numberOfPoints)
    {
    this->RemoveNthControlPoint(this->GetNumberOfControlPoints() - 1);
    }

  this->IsUpdatingPoints = false;
  // No need to call UpdateAllMeasurements(), because it is automatically
  // called in EndModify().
  this->EndModify(wasModified);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::GetControlPointPositionsWorld(vtkPoints* points)
{
  if (!points)
    {
    return;
    }
  int numberOfControlPoints = this->GetNumberOfControlPoints();
  points->SetNumberOfPoints(numberOfControlPoints);
  double posWorld[3] = { 0.0, 0.0, 0.0 };
  for (int controlPointIndex = 0; controlPointIndex < numberOfControlPoints; controlPointIndex++)
    {
    this->GetNthControlPointPositionWorld(controlPointIndex, posWorld);
    points->SetPoint(controlPointIndex, posWorld);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::SetControlPointLabelsWorld(vtkStringArray* labels, vtkPoints* points, std::string separator /*=""*/)
{
  if (!labels || !points || labels->GetNumberOfValues() != points->GetNumberOfPoints())
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::SetControlPointLabelsWorld failed: invalid inputs");
    return false;
    }

  int wasModified = this->StartModify();

  // Erase all previous labels
  int numberOfControlPoints = this->GetNumberOfControlPoints();
  for (int n = 0; n < numberOfControlPoints; n++)
    {
    this->SetNthControlPointLabel(n, "");
    }

  // Set each label at the closest control point
  vtkIdType numberOfLabels = labels->GetNumberOfValues();
  for (vtkIdType labelIndex = 0; labelIndex < numberOfLabels; ++labelIndex)
    {
    int markupIndex = this->GetClosestControlPointIndexToPositionWorld(points->GetPoint(labelIndex));
    if (markupIndex >= 0)
      {
      std::string oldLabel = this->GetNthControlPointLabel(markupIndex);
      if (oldLabel.empty())
        {
        this->SetNthControlPointLabel(markupIndex, labels->GetValue(labelIndex));
        }
      else
        {
        this->SetNthControlPointLabel(markupIndex, oldLabel + separator + labels->GetValue(labelIndex));
        }
      }
    }

  this->EndModify(wasModified);
  return true;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNumberOfMeasurements()
{
  return static_cast<int>(this->Measurements->GetNumberOfItems());
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNumberOfEnabledMeasurements()
{
  int numberOfEnabledMeasurements = 0;
  vtkMRMLMeasurement* currentMeasurement = nullptr;
  vtkCollectionSimpleIterator it;
  for (this->Measurements->InitTraversal(it);
      (currentMeasurement=vtkMRMLMeasurement::SafeDownCast(this->Measurements->GetNextItemAsObject(it)));)
    {
    if (currentMeasurement->GetEnabled())
      {
      numberOfEnabledMeasurements++;
      }
    }

  return numberOfEnabledMeasurements;
}

//---------------------------------------------------------------------------
vtkMRMLMeasurement* vtkMRMLMarkupsNode::GetNthMeasurement(int id)
{
  if (id < 0 || id >= this->GetNumberOfMeasurements())
    {
    return nullptr;
    }
  return vtkMRMLMeasurement::SafeDownCast(this->Measurements->GetItemAsObject(id));
}

//---------------------------------------------------------------------------
vtkMRMLMeasurement* vtkMRMLMarkupsNode::GetMeasurement(const char* name)
{
  if (!name)
    {
    return nullptr;
    }
  for (int measurementIndex = 0; measurementIndex < this->GetNumberOfMeasurements(); measurementIndex++)
    {
    vtkMRMLMeasurement* measurement = this->GetNthMeasurement(measurementIndex);
    if (measurement->GetName() == name)
      {
      // found
      return measurement;
      }
    }
  return nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMeasurement(int id, vtkMRMLMeasurement* measurement)
{
  if (id < 0 || id > this->GetNumberOfMeasurements())
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::SetNthMeasurement failed: id out of range");
    return;
    }
  if (id >= this->GetNumberOfMeasurements())
    {
    this->Measurements->AddItem(measurement);
    }
  else
    {
    this->Measurements->ReplaceItem(id, measurement);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::AddMeasurement(vtkMRMLMeasurement* measurement)
{
  this->Measurements->AddItem(measurement);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMeasurement(int id,
  const std::string& name, double value, const std::string &units,
  std::string printFormat/*=""*/, std::string description/*=""*/,
  vtkCodedEntry* quantityCode/*=nullptr*/, vtkCodedEntry* derivationCode/*=nullptr*/,
  vtkCodedEntry* unitsCode/*=nullptr*/, vtkCodedEntry* methodCode/*=nullptr*/)
{
  if (id < 0 || id > this->GetNumberOfMeasurements())
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::SetNthMeasurement failed: id out of range");
    return;
    }
  vtkSmartPointer<vtkMRMLStaticMeasurement> measurement;
  if (id >= this->GetNumberOfMeasurements())
    {
    measurement = vtkSmartPointer<vtkMRMLStaticMeasurement>::New();
    this->Measurements->AddItem(measurement);
    }
  else
    {
    measurement = vtkMRMLStaticMeasurement::SafeDownCast(this->Measurements->GetItemAsObject(id));
    if (measurement == nullptr)
      {
      vtkErrorMacro("SetNthMeasurement: Cannot set non-constant measurement manually (ID: " << id << ")");
      return;
      }
    }
  measurement->SetName(name.c_str());
  measurement->SetDisplayValue(value, units.c_str());
  if (!printFormat.empty())
    {
    measurement->SetPrintFormat(printFormat.c_str());
    }
  measurement->SetDescription(description.c_str());
  measurement->SetQuantityCode(quantityCode);
  measurement->SetDerivationCode(derivationCode);
  measurement->SetUnitsCode(unitsCode);
  measurement->SetMethodCode(methodCode);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::RemoveNthMeasurement(int id)
{
  if (id < 0 || id >= this->GetNumberOfMeasurements())
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::RemoveNthMeasurement failed: id out of range");
    }
  this->Measurements->RemoveItem(id);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::RemoveAllMeasurements()
{
  this->Measurements->RemoveAllItems();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ClearValueForAllMeasurements()
{
  for (int index=0; index<this->Measurements->GetNumberOfItems(); ++index)
    {
    vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(
      this->Measurements->GetItemAsObject(index) );
    if (currentMeasurement)
      {
      currentMeasurement->ClearValue();
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::UpdateAllMeasurements()
{
  if (this->IsUpdatingPoints)
    {
    return;
    }

  this->UpdateMeasurementsInternal();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::UpdateMeasurementsInternal()
{
  // Calculate enabled measurements
  bool wasModify = this->StartModify();
  for (int index=0; index<this->Measurements->GetNumberOfItems(); ++index)
    {
    vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(this->Measurements->GetItemAsObject(index));
    if (currentMeasurement && currentMeasurement->GetEnabled() && !currentMeasurement->IsA("vtkMRMLStaticMeasurement"))
      {
      currentMeasurement->ClearValue();
      currentMeasurement->Compute();
      }
    }

  this->WriteMeasurementsToDescription();
  this->EndModify(wasModify);
}

//---------------------------------------------------------------------------
vtkMRMLUnitNode* vtkMRMLMarkupsNode::GetUnitNode(const char* quantity)
{
  if (!quantity)
    {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetUnitNode failed: invalid quantity");
    return nullptr;
    }
  if (!this->GetScene())
    {
    return nullptr;
    }

  vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast(
    this->GetScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
  if (!selectionNode)
    {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetUnitNode failed: selection node not found");
    return nullptr;
    }
  vtkMRMLUnitNode* unitNode = vtkMRMLUnitNode::SafeDownCast(this->GetScene()->GetNodeByID(
    selectionNode->GetUnitNodeID(quantity)));

  // Do not log warning if null, because for example there is no 'angle' unit node, and in
  // that case hundreds of warnings would be thrown in a non erroneous situation.
  return unitNode;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::WriteMeasurementsToDescription()
{
  int numberOfValidMeasurements = 0;
  std::string properties;
  std::string description;
  vtkMRMLMeasurement* currentMeasurement = nullptr;
  vtkCollectionSimpleIterator it;
  std::string measurementText;
  for (this->Measurements->InitTraversal(it);
      (currentMeasurement=vtkMRMLMeasurement::SafeDownCast(this->Measurements->GetNextItemAsObject(it)));)
    {
    if (!currentMeasurement->GetEnabled() || currentMeasurement->GetName().empty() || !currentMeasurement->GetValueDefined())
      {
      continue;
      }

    std::string measurementValue = currentMeasurement->GetValueWithUnitsAsPrintableString();
    if (measurementValue.empty())
      {
      continue;
      }
    numberOfValidMeasurements++;

    // properties label special cases
    if (numberOfValidMeasurements == 1)
      {
      // if there is only one measurement then show it in the same line
      // and don't include the measurement to make display more compact
      properties = " " + currentMeasurement->GetValueWithUnitsAsPrintableString();
      }
    else if (numberOfValidMeasurements == 2)
      {
      // rewrite first measurement
      // we still have the full text of the last measurement, use it
      properties = "\n" + measurementText;
      }

    measurementText = currentMeasurement->GetName() + std::string(": ") + measurementValue;

    // description
    if (!description.empty())
      {
      // not the first measurement, add a separator
      description += "; ";
      }
    description += measurementText;

    // properties label
    if (numberOfValidMeasurements > 1)
      {
      // rewrite first measurement
      // we still have the full text of the last measurement, use it
      properties += "\n" + measurementText;
      }
    }

  bool wasModify = this->StartModify();
  this->SetDescription(description.c_str());
  if (properties != this->PropertiesLabelText)
    {
    this->PropertiesLabelText = properties;
    this->Modified();
    }
  this->EndModify(wasModify);
}

//---------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLMarkupsNode::GetInteractionHandleToWorldMatrix()
{
  return this->InteractionHandleToWorldMatrix;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::UpdateInteractionHandleToWorldMatrix()
{
  // The origin of the coordinate system is at the center of mass of the control points
  double origin_World[3] = { 0 };
  int numberOfControlPoints = this->GetNumberOfControlPoints();
  vtkNew<vtkPoints> controlPoints_World;
  for (int i = 0; i < numberOfControlPoints; ++i)
    {
    double controlPointPosition_World[3] = { 0.0, 0.0, 0.0 };
    this->GetNthControlPointPositionWorld(i, controlPointPosition_World);

    origin_World[0] += controlPointPosition_World[0] / numberOfControlPoints;
    origin_World[1] += controlPointPosition_World[1] / numberOfControlPoints;
    origin_World[2] += controlPointPosition_World[2] / numberOfControlPoints;

    controlPoints_World->InsertNextPoint(controlPointPosition_World);
    }

  for (int i = 0; i < 3; ++i)
    {
    this->InteractionHandleToWorldMatrix->SetElement(i, 3, origin_World[i]);
    }

  if (this->GetNumberOfControlPoints() < 3)
    {
    return;
    }

  // The orientation of the coordinate system is adjusted so that the z axis aligns with the normal of the
  // best fit plane defined by the control points.

  vtkNew<vtkPlane> bestFitPlane_World;
  vtkAddonMathUtilities::FitPlaneToPoints(controlPoints_World, bestFitPlane_World);

  double normal_World[3] = { 0.0, 0.0, 1.0 };
  bestFitPlane_World->GetNormal(normal_World);

  double handleZ_World[4] = { 0.0, 0.0, 1.0, 0.0 };
  this->InteractionHandleToWorldMatrix->MultiplyPoint(handleZ_World, handleZ_World);

  if (vtkMath::Dot(handleZ_World, normal_World) < 0.0)
    {
    handleZ_World[0] = -handleZ_World[0];
    handleZ_World[1] = -handleZ_World[1];
    handleZ_World[2] = -handleZ_World[2];
    }

  double rotationVector_World[3] = { 0.0, 0.0, 1.0 };
  double angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(handleZ_World, normal_World));
  double epsilon = 0.001;
  if (angle < epsilon)
    {
    return;
    }
  vtkMath::Cross(handleZ_World, normal_World, rotationVector_World);
  vtkMath::Normalize(rotationVector_World);

  vtkNew<vtkTransform> handleToWorldMatrix;
  handleToWorldMatrix->PostMultiply();
  handleToWorldMatrix->Concatenate(this->InteractionHandleToWorldMatrix);
  handleToWorldMatrix->Translate(-origin_World[0], -origin_World[1], -origin_World[2]);
  handleToWorldMatrix->RotateWXYZ(angle, rotationVector_World);
  handleToWorldMatrix->Translate(origin_World);
  this->InteractionHandleToWorldMatrix->DeepCopy(handleToWorldMatrix->GetMatrix());
}

//---------------------------------------------------------------------------
const char* vtkMRMLMarkupsNode::GetPositionStatusAsString(int id)
{
  switch (id)
    {
    case vtkMRMLMarkupsNode::PositionUndefined:
      {
      return "undefined";
      }
    case vtkMRMLMarkupsNode::PositionPreview:
      {
      return "preview";
      }
    case vtkMRMLMarkupsNode::PositionDefined:
      {
      return "defined";
      }
    case vtkMRMLMarkupsNode::PositionMissing:
      {
      return "missing";
      }
    default:
      {
      vtkGenericWarningMacro("Unknown position status: " << id);
      return "";
      }
    }
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetPositionStatusFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkMRMLMarkupsNode::PositionStatus_Last; i++)
    {
    if (strcmp(name, vtkMRMLMarkupsNode::GetPositionStatusAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  vtkGenericWarningMacro("Unknown position status: " << name);
  return -1;
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetPropertiesLabelText()
{
  return this->PropertiesLabelText;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNthControlPointIndexByPositionStatus(int pointIndex, int positionStatus)
{
  int foundControlPoints = 0;
  for (ControlPointsListType::iterator controlPointIt = this->ControlPoints.begin();
    controlPointIt != this->ControlPoints.end(); ++controlPointIt)
    {
    if ((*controlPointIt)->PositionStatus == positionStatus)
      {
      if (foundControlPoints == pointIndex)
        {
        return controlPointIt - this->ControlPoints.begin();
        }
      foundControlPoints++;
      }
    }
  return -1;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetFixedNumberOfControlPoints()
{
  return this->FixedNumberOfControlPoints;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetFixedNumberOfControlPoints(bool fixed)
{
  if (this->FixedNumberOfControlPoints == fixed)
    {
    // no change
    return;
    }
  this->FixedNumberOfControlPoints = fixed;

  // Update MaximumNumberOfControlPoints
  int fixedPointNumber = fixed ? this->GetNumberOfControlPoints() : this->GetRequiredNumberOfControlPoints();
  if (fixedPointNumber >= 0)
    {
    this->MaximumNumberOfControlPoints = fixedPointNumber;
    }

  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent);
  this->StorableModifiedTime.Modified();
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetControlPointPlacementStartIndex()
{
  return this->ControlPointPlacementStartIndex;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetControlPointPlacementStartIndex(int row)
{
  // no modified event is invoked, as this property is currently not stored persistently
  this->ControlPointPlacementStartIndex = row;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetControlPointPlacementComplete()
{
  bool hasRequiredPoints = this->GetRequiredNumberOfControlPoints() > 0;
  bool hasRequiredPointNumber = this->GetNumberOfControlPoints() >= this->GetRequiredNumberOfControlPoints();
  bool requiredPointsReached = hasRequiredPoints && hasRequiredPointNumber && !(this->GetNumberOfUndefinedControlPoints() > 0);
  bool lockedPointsReached = this->GetFixedNumberOfControlPoints() && !(this->GetNumberOfUndefinedControlPoints() > 0);
  if (requiredPointsReached || lockedPointsReached)
    {
    return true;
    }
  return false;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionWorldFromArray(
  const int pointIndex, const double pos[3], int positionStatus/*=PositionDefined*/)
{
  vtkWarningMacro("vtkMRMLMarkupsNode::SetNthControlPointPositionWorldFromArray method is deprecated, please use SetNthControlPointPositionWorld instead");
  ControlPoint *controlPoint = this->GetNthControlPointCustomLog(pointIndex, "SetNthControlPointPositionWorldFromArray");
  if (!controlPoint)
    {
    return;
    }
  double markupxyz[3] = { 0.0, 0.0, 0.0 };
  this->TransformPointFromWorld(pos, markupxyz);
  this->SetNthControlPointPositionFromArray(pointIndex, markupxyz, positionStatus);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionFromPointer(const int pointIndex,
                                                               const double * pos)
{
  vtkWarningMacro("vtkMRMLMarkupsNode::SetNthControlPointPositionFromPointer method is deprecated, please use SetNthControlPointPosition instead");
  if (!pos)
    {
    vtkErrorMacro("SetNthControlPointFromPointer: invalid position pointer!");
    return;
    }

  this->SetNthControlPointPosition(pointIndex, pos[0], pos[1], pos[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetCenterOfRotationFromPointer(const double *pos)
{
  vtkWarningMacro("vtkMRMLMarkupsNode::SetCenterOfRotationFromPointer method is deprecated, please use SetCenterOfRotation instead");
  if (!pos)
    {
    vtkErrorMacro("SetCenterOfRotationFromPointer: invalid position pointer!");
    return;
    }

  this->SetCenterOfRotation(pos[0], pos[1], pos[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientationFromPointer(int n, const double *orientation)
{
  vtkWarningMacro("vtkMRMLMarkupsNode::SetNthControlPointOrientationFromPointer method is deprecated, please use SetNthControlPointOrientation instead");
  if (!orientation)
    {
    vtkErrorMacro("Invalid orientation pointer!");
    return;
    }
  this->SetNthControlPointOrientation(n, orientation[0], orientation[1], orientation[2], orientation[3]);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::GetMarkupPoint(int markupIndex, int pointIndex, double point[3])
{
  vtkWarningMacro("vtkMRMLMarkupsNode::GetMarkupPoint method is deprecated, please use GetNthControlPointPosition instead");
  if (markupIndex == 0)
    {
    this->GetNthControlPointPosition(pointIndex, point);
    }
  else if (pointIndex == 0)
    {
    this->GetNthControlPointPosition(markupIndex, point);
    }
  else
    {
    vtkErrorMacro("vtkMRMLMarkupsNode::GetMarkupPoint failed: only one markup with multiple control points is supported.");
    }
}
