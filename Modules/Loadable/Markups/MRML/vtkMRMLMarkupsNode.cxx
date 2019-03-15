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
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsStorageNode.h"
#include "vtkMRMLTransformNode.h"

// Slicer MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkBitArray.h>
#include <vtkBoundingBox.h>
#include <vtkCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStringArray.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTrivialProducer.h>

// STD includes
#include <sstream>
#include <algorithm>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode::vtkMRMLMarkupsNode()
{
  this->TextList = vtkSmartPointer<vtkStringArray>::New();
  this->Locked = 0;

  this->CurveClosed = false;

  this->RequiredNumberOfControlPoints = 0;
  this->MaximumNumberOfControlPoints = 0;
  this->MarkupLabelFormat = std::string("%N-%d");
  this->LastUsedControlPointNumber = 0;
  this->CenterPos.Set(0,0,0);

  this->CurveInputPoly = vtkSmartPointer<vtkPolyData>::New();
  vtkNew<vtkPoints> curveInputPoints;
  this->CurveInputPoly->SetPoints(curveInputPoints);

  this->CurvePoly = vtkSmartPointer<vtkPolyData>::New();
  vtkNew<vtkPoints> curvePoints;
  this->CurvePoly->SetPoints(curvePoints);
  vtkNew<vtkCellArray> lineCellArray;
  this->CurvePoly->SetLines(lineCellArray);

  this->CurveGenerator = vtkSmartPointer<vtkCurveGenerator>::New();
  this->CurveGenerator->SetInputPoints(curveInputPoints);
  this->CurveGenerator->SetOutputPoints(curvePoints);
  this->CurveGenerator->SetCurveTypeToLinearSpline();
  this->CurveGenerator->SetNumberOfPointsPerInterpolatingSegment(1);

  vtkNew<vtkTrivialProducer> curvePointConnector; // allows connecting a data object to pipeline input
  curvePointConnector->SetOutput(this->CurvePoly);

  this->CurvePolyToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();

  this->CurvePolyToWorldTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->CurvePolyToWorldTransformer->SetInputConnection(curvePointConnector->GetOutputPort());
  this->CurvePolyToWorldTransformer->SetTransform(this->CurvePolyToWorldTransform);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode::~vtkMRMLMarkupsNode()
{
  this->RemoveAllControlPoints();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(locked, Locked);
  vtkMRMLWriteXMLStdStringMacro(markupLabelFormat, MarkupLabelFormat);
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
  int disabledModify = this->StartModify();

  this->RemoveAllControlPoints();
  this->RemoveAllTexts();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(locked, Locked);
  vtkMRMLReadXMLStdStringMacro(markupLabelFormat, MarkupLabelFormat);
  vtkMRMLReadXMLEndMacro();

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

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLMarkupsNode *node = vtkMRMLMarkupsNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(Locked);
  vtkMRMLCopyStdStringMacro(MarkupLabelFormat);
  vtkMRMLCopyEndMacro();

  this->TextList->DeepCopy(node->TextList);

  this->CurveInputPoly->GetPoints()->DeepCopy(node->CurveInputPoly->GetPoints());
  this->UpdateCurvePolyFromCurveInputPoly();

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

  this->RemoveAllControlPoints();
  int numMarkups = node->GetNumberOfControlPoints();
  for (int n = 0; n < numMarkups; n++)
    {
    ControlPoint* controlPoint = node->GetNthControlPoint(n);
    ControlPoint* controlPointCopy = new ControlPoint;
    (*controlPointCopy) = (*controlPoint);
    int controlPointIndex = this->AddControlPoint(controlPointCopy);
    }

  this->EndModify(disabledModify);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ProcessMRMLEvents(vtkObject *caller,
                                           unsigned long event,
                                           void *callData)
{
  if (caller != nullptr && event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    vtkMRMLTransformNode::GetTransformBetweenNodes(this->GetParentTransformNode(), nullptr, CurvePolyToWorldTransform);
    }
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(Locked);
  vtkMRMLPrintStdStringMacro(MarkupLabelFormat);
  vtkMRMLPrintEndMacro();

  os << indent << "MaximumNumberOfControlPoints: ";
  if (this->MaximumNumberOfControlPoints>0)
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
    os << indent << "ID = " << controlPoint->ID.c_str() << "\n";
    os << indent << "Label = " << controlPoint->Label.c_str() << "\n";
    os << indent << "Description = " << controlPoint->Description.c_str() << "\n";
    os << indent << "Associated node id = " << controlPoint->AssociatedNodeID.c_str() << "\n";
    os << indent << "Selected = " << controlPoint->Selected << "\n";
    os << indent << "Locked = " << controlPoint->Locked << "\n";
    os << indent << "Visibility = " << controlPoint->Visibility << "\n";
    os << indent << "Position : " << controlPoint->Position[0] << ", " <<
          controlPoint->Position[1] << ", " << controlPoint->Position[2] << "\n";
    os << indent << "Orientation = ";
    for (int i = 0; i < 9; i++)
      {
      if (i > 0)
        {
        os << ",  ";
        }
      os << controlPoint->OrientationMatrix[i];
      }
    }

  os << indent << "textList: ";
  if  (!this->TextList || !this->GetNumberOfTexts())
    {
    os << indent << "None"  << endl;
    }
  else
    {
    os << endl;
    for (int i = 0 ; i < this->GetNumberOfTexts() ; i++)
      {
      os << indent << "  " << i <<": " <<  (TextList->GetValue(i) ? TextList->GetValue(i) : "(none)") << endl;
      }
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

  for(unsigned int i = 0; i < this->ControlPoints.size(); i++)
    {
    delete this->ControlPoints[i];
    }

  this->ControlPoints.clear();

  this->CurveInputPoly->GetPoints()->Reset();
  this->CurveInputPoly->GetPoints()->Squeeze();
  this->CurvePoly->GetPoints()->Reset();
  this->CurvePoly->GetPoints()->Squeeze();
  this->CurvePoly->GetLines()->Reset();
  this->CurvePoly->GetLines()->Squeeze();

  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::AllPointsRemovedEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetText(int id, const char *newText)
{
  if (id < 0)
    {
    vtkErrorMacro("SetText: Invalid ID");
    return;
    }
  if (!this->TextList)
    {
    vtkErrorMacro("SetText: TextList is NULL");
    return;
    }

  vtkStdString newString;
  if (newText)
    {
    newString = vtkStdString(newText);
    }

  // check if the same as before
  if (((this->TextList->GetNumberOfValues() == 0) && (newText == nullptr || newString == "")) ||
      ((this->TextList->GetNumberOfValues() > id) &&
       (this->TextList->GetValue(id) == newString)))
    {
    return;
    }

  this->TextList->InsertValue(id,newString);

  // invoke a modified event
  this->Modified();
}

//-------------------------------------------------------------------------
int vtkMRMLMarkupsNode::AddText(const char *newText)
{
  if (!this->TextList)
    {
    vtkErrorMacro("Markups: For node " << this->GetName() << " text is not defined");
    return -1 ;
    }

  int n = this->GetNumberOfTexts();
  this->SetText(n,newText);

  return n;
}

//-------------------------------------------------------------------------
vtkStdString vtkMRMLMarkupsNode::GetText(int n)
{
  if ((this->GetNumberOfTexts() <= n) || n < 0)
    {
    return vtkStdString();
    }

  return this->TextList->GetValue(n);
}

//-------------------------------------------------------------------------
int  vtkMRMLMarkupsNode::DeleteText(int id)
{
  if (!this->TextList)
    {
    return -1;
    }

  int n = this->GetNumberOfTexts();
  if (id < 0 || id >= n)
    {
    return -1;
    }

  for (int i = id; i < n-1; i++)
    {
    this->TextList->SetValue(i, this->GetText(i+1));
    }

  this->TextList->Resize(n-1);

  return 1;
}


//-------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNumberOfTexts()
{
  if (!this->TextList)
    {
    return -1;
    }
  return static_cast<int>(this->TextList->GetNumberOfValues());
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsNode::RemoveAllTexts()
{
  this->TextList->Initialize();
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
  // By default we could store points in the scene (especially for lines
  // and angles), but for now we always store in an fcsv file.
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLMarkupsFiducialStorageNode"));
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
bool vtkMRMLMarkupsNode::ControlPointExists(int n, const char* failedMethodName)
{
  if (n < 0 || n >= this->GetNumberOfControlPoints())
    {
    if (failedMethodName)
      {
      vtkErrorMacro("vtkMRMLMarkupsNode::" << failedMethodName << " failed: control point " <<
        n << " does not exist");
      }
    return false;
    }

  if (this->ControlPoints[static_cast<size_t>(n)] != nullptr)
    {
    return true;
    }

  if (failedMethodName)
    {
    vtkErrorMacro(<< failedMethodName << " failed: control point " <<
      n << " is invalid");
    }

  return false;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNumberOfControlPoints()
{
  return static_cast<int> (this->ControlPoints.size());
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsNode::ControlPoint* vtkMRMLMarkupsNode::GetNthControlPoint(int n)
{
  if (!this->ControlPointExists(n, "GetNthControlPoint"))
    {
    return nullptr;
    }

  return this->ControlPoints[static_cast<unsigned int> (n)];
}

//-----------------------------------------------------------
std::vector< vtkMRMLMarkupsNode::ControlPoint* > * vtkMRMLMarkupsNode::GetControlPoints()
{
  return &this->ControlPoints;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddControlPoint(ControlPoint *controlPoint)
{
  if (this->MaximumNumberOfControlPoints != 0 &&
      this->GetNumberOfControlPoints() + 1 > this->MaximumNumberOfControlPoints)
    {
    vtkErrorMacro("AddNControlPoints: number of points major than maximum number of control points allowed.");
    return -1;
    }

  // generate a unique id based on list policy
  if (controlPoint->ID.empty())
    {
    controlPoint->ID = this->GenerateUniqueControlPointID();
    }
  this->LastUsedControlPointNumber++;
  if (controlPoint->Label.empty())
    {
    controlPoint->Label = this->GenerateControlPointLabel(this->LastUsedControlPointNumber);
    }

  this->ControlPoints.push_back(controlPoint);

  // Add point to CurveInputPoly
  this->CurveInputPoly->GetPoints()->InsertNextPoint(controlPoint->Position);
  this->CurveInputPoly->GetPoints()->Modified();
  this->UpdateCurvePolyFromCurveInputPoly();

  this->Modified();
  int controlPointIndex = this->GetNumberOfControlPoints() - 1;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointAddedEvent,  static_cast<void*>(&controlPointIndex));
  return controlPointIndex;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddNControlPoints(int n, std::string label /*=std::string()*/, vtkVector3d* point /*=nullptr*/)
{
  int controlPointIndex = -1;
  if (n < 0)
    {
    vtkErrorMacro("AddNControlPoints: invalid number of points " << n);
    return controlPointIndex;
    }

  if (this->MaximumNumberOfControlPoints != 0 &&  n > this->MaximumNumberOfControlPoints)
    {
    vtkErrorMacro("AddNControlPoints: number of points " << n <<
                  " major than maximum number of control points allowed : " << this->MaximumNumberOfControlPoints);
    return controlPointIndex;
    }

  for (int i = 0; i < n; i++)
    {
    ControlPoint *controlPoint = new ControlPoint;
    controlPoint->Label = label;
    if (point != nullptr)
      {
      controlPoint->Position[0] = point->GetX();
      controlPoint->Position[1] = point->GetY();
      controlPoint->Position[2] = point->GetZ();
      }
    controlPointIndex = this->AddControlPoint(controlPoint);
    }

  return controlPointIndex;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddControlPointWorld(vtkVector3d pointWorld, std::string label /*=std::string()*/)
{
  vtkVector3d point;
  this->TransformPointFromWorld(pointWorld, point);
  return this->AddNControlPoints(1, label, &point);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddControlPoint(vtkVector3d point, std::string label /*=std::string()*/)
{
  return this->AddNControlPoints(1, label, &point);
}

//-----------------------------------------------------------
vtkVector3d vtkMRMLMarkupsNode::GetNthControlPointPositionVector(int pointIndex)
{
  if (!this->ControlPointExists(pointIndex, "GetNthControlPointPositionVector"))
    {
    return vtkVector3d(0, 0, 0);
    }

  return vtkVector3d(this->GetNthControlPoint(pointIndex)->Position);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthControlPointPosition(int pointIndex, double point[3])
{
  if (!this->ControlPointExists(pointIndex, "GetNthControlPointPosition"))
    {
    point[0] = 0.0;
    point[1] = 0.0;
    point[2] = 0.0;
    return;
    }

  double* position = this->GetNthControlPoint(pointIndex)->Position;
  point[0] = position[0];
  point[1] = position[1];
  point[2] = position[2];
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::GetNthControlPointPositionWorld(int pointIndex, double worldxyz[3])
{
  if (!this->ControlPointExists(pointIndex, "GetNthControlPointPositionWorld"))
    {
    return 0;
    }
/*
  this->CurvePolyToWorldTransformer->Update();
  vtkPoints* pointsWorld = this->CurvePolyToWorldTransformer->GetOutput()->GetPoints();
  pointsWorld->GetPoint(pointIndex, worldxyz);
*/
  this->TransformPointToWorld(this->GetNthControlPoint(pointIndex)->Position, worldxyz);
  return 1;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::RemoveNthControlPoint(int pointIndex)
{
  if (!this->ControlPointExists(pointIndex, "RemoveNthControlPoint"))
    {
    return;
    }

  // Allow reusing last control point number (to prevent continuously
  // incrementing the number in the control point's name when adding/removing preview points).
  std::string lastAutoGeneratedLabel = this->GenerateControlPointLabel(this->LastUsedControlPointNumber);
  if (lastAutoGeneratedLabel == this->GetNthControlPointLabel(pointIndex))
    {
    this->LastUsedControlPointNumber--;
    }

  delete this->ControlPoints[static_cast<unsigned int> (pointIndex)];
  this->ControlPoints.erase(this->ControlPoints.begin() + pointIndex);

  this->UpdateCurvePolyFromControlPoints();

  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointRemovedEvent, static_cast<void*>(&pointIndex));
}

//-----------------------------------------------------------
bool vtkMRMLMarkupsNode::InsertControlPoint(ControlPoint *controlPoint, int targetIndex)
{
  // generate a unique id based on list policy
  if (controlPoint->ID.empty())
    {
    controlPoint->ID = this->GenerateUniqueControlPointID();
    }

  /* do not generate labels for inserted points
  if (controlPoint->Label.empty())
    {
    controlPoint->Label = this->GenerateControlPointLabel(targetIndex);
    }
    */

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
  std::vector < ControlPoint* >::iterator result = this->ControlPoints.insert(pos, controlPoint);

  this->UpdateCurvePolyFromControlPoints();

  // let observers know that a markup was added
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointAddedEvent, static_cast<void*>(&targetIndex));

  return true;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::UpdateCurvePolyFromControlPoints()
{
  // Add points
  vtkPoints* points = this->CurveInputPoly->GetPoints();
  points->Reset();
  int numberOfControlPoints = this->GetNumberOfControlPoints();
  for (int i = 0; i < numberOfControlPoints; i++)
    {
    points->InsertNextPoint(this->ControlPoints[i]->Position);
    }
  points->Modified();

  this->UpdateCurvePolyFromCurveInputPoly();
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::UpdateCurvePolyFromCurveInputPoly()
{
  // curve generator is not a filter, it needs manual update
  this->CurveGenerator->Update();

  // Update lines: a single cell containing a line with point
  // indices: 0, 1, ..., last point (and an extra 0 if closed curve).
  vtkIdType numberOfPoints = this->CurvePoly->GetNumberOfPoints();
  vtkCellArray* line = this->CurvePoly->GetLines();
  if (numberOfPoints > 1)
    {
    bool loop = (numberOfPoints > 2 && this->CurveClosed);
    vtkIdType numberOfCellPoints = (loop ? numberOfPoints + 1 : numberOfPoints);

    // Only regenerate indices if necessary
    bool needToUpdateLines = true;
    if (line->GetNumberOfCells() == 1)
      {
      vtkIdType currentNumberOfCellPoints = 0;
      vtkIdType* currentCellPoints = nullptr;
      line->GetCell(0, currentNumberOfCellPoints, currentCellPoints);
      if (currentNumberOfCellPoints == numberOfCellPoints)
        {
        needToUpdateLines = false;
        }
      }

    if (needToUpdateLines)
      {
      line->Reset();
      line->InsertNextCell(numberOfCellPoints);
      for (int i = 0; i < numberOfPoints; i++)
        {
        line->InsertCellPoint(i);
        }
      if (loop)
        {
        line->InsertCellPoint(0);
        }
      line->Modified();
      }
    }
  else
    {
    line->Reset();
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SwapControlPoints(int m1, int m2)
{
  if (!this->ControlPointExists(m1, "SwapControlPoints"))
    {
    return;
    }
  if (!this->ControlPointExists(m2, "SwapControlPoints"))
    {
    return;
    }

  ControlPoint *m1Markup = this->GetNthControlPoint(m1);
  ControlPoint m1MarkupBackup;
  // make a copy of the first control point
  m1MarkupBackup = *m1Markup;
  // copy the second control point into the first
  *m1Markup = *this->GetNthControlPoint(m2);
  // and copy the backup of the first one into the second
  *this->GetNthControlPoint(m2) = m1MarkupBackup;

  this->UpdateCurvePolyFromControlPoints();

  // and let listeners know that two control points have changed
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&m1));
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&m2));
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionFromPointer(const int pointIndex,
                                                               const double * pos)
{
  if (!pos)
    {
    vtkErrorMacro("SetNthControlPointFromPointer: invalid position pointer!");
    return;
    }

  this->SetNthControlPointPosition(pointIndex, pos[0], pos[1], pos[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionFromArray(const int pointIndex,
                                                             const double pos[3])
{
  this->SetNthControlPointPosition(pointIndex, pos[0], pos[1], pos[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPosition(const int pointIndex,
                                                    const double x, const double y, const double z)
{
  if (!this->ControlPointExists(pointIndex, "SetNthControlPointPosition"))
    {
    return;
    }

  // TODO: return if no modification
  double* controlPointPosition = this->GetNthControlPoint(pointIndex)->Position;
  controlPointPosition[0] = x;
  controlPointPosition[1] = y;
  controlPointPosition[2] = z;

  vtkPoints* points = this->CurveInputPoly->GetPoints();
  points->SetPoint(pointIndex, x, y, z);
  points->Modified();
  this->UpdateCurvePolyFromCurveInputPoly();

  // throw an event to let listeners know the position has changed
  this->Modified();
  int n = pointIndex;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionWorld(const int pointIndex,
                                                         const double x, const double y, const double z)
{
  if (!this->ControlPointExists(pointIndex, "SetNthControlPointPositionWorld"))
    {
    return;
    }

  vtkVector3d markupxyz;
  TransformPointFromWorld(vtkVector3d(x,y,z), markupxyz);
  this->SetNthControlPointPosition(pointIndex, markupxyz[0], markupxyz[1], markupxyz[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionWorldFromArray(const int pointIndex, const double pos[3])
{
  if (!this->ControlPointExists(pointIndex, "SetNthControlPointPositionWorldFromArray"))
    {
    return;
    }

  double markupxyz[3] = { 0.0 };
  TransformPointFromWorld(pos, markupxyz);
  this->SetNthControlPointPositionFromArray(pointIndex, markupxyz);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointPositionOrientationWorldFromArray(
  const int pointIndex, const double pos[3], const double orientationMatrix[9],
  const char* associatedNodeID)
{
  if (!this->ControlPointExists(pointIndex, "SetNthControlPointPositionOrientationWorldFromArray"))
    {
    return;
    }
  ControlPoint* controlPoint = this->GetNthControlPoint(pointIndex);

  // TODO: return if no modification
  std::copy_n(pos, 3, controlPoint->Position);
  std::copy_n(orientationMatrix, 3, controlPoint->OrientationMatrix);
  if (associatedNodeID)
    {
    controlPoint->AssociatedNodeID = associatedNodeID;
    }
  else
    {
    controlPoint->AssociatedNodeID.empty();
    }

  vtkPoints* points = this->CurveInputPoly->GetPoints();
  points->SetPoint(pointIndex, pos);
  points->Modified();
  this->UpdateCurvePolyFromCurveInputPoly();

  // throw an event to let listeners know the position has changed
  this->Modified();
  int n = pointIndex;
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
}

//-----------------------------------------------------------
vtkVector3d vtkMRMLMarkupsNode::GetCenterPositionVector()
{
  return this->CenterPos;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetCenterPosition(double point[3])
{
  point[0] = this->CenterPos.GetX();
  point[1] = this->CenterPos.GetY();
  point[2] = this->CenterPos.GetZ();
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::GetCenterPositionWorld(double worldxyz[3])
{
  vtkVector3d world;
  this->TransformPointToWorld(this->GetCenterPositionVector(), world);
  worldxyz[0] = world[0];
  worldxyz[1] = world[1];
  worldxyz[2] = world[2];
  return 1;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetCenterPositionFromPointer(const double *pos)
{
  if (!pos)
    {
    vtkErrorMacro("SetCenterPositionFromPointer: invalid position pointer!");
    return;
    }

  this->SetCenterPosition(pos[0], pos[1], pos[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetCenterPositionFromArray(const double pos[3])
{
  this->SetCenterPosition(pos[0], pos[1], pos[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetCenterPosition(const double x, const double y, const double z)
{
  this->CenterPos.Set(x,y,z);
  this->Modified();
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetCenterPositionWorld(const double x, const double y, const double z)
{
  vtkVector3d centerxyz;
  TransformPointFromWorld(vtkVector3d(x,y,z), centerxyz);
  this->SetCenterPosition(centerxyz[0], centerxyz[1], centerxyz[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientationFromPointer(int n, const double *orientation)
{
  if (!orientation)
    {
    vtkErrorMacro("Invalid orientation pointer!");
    return;
    }
  this->SetNthControlPointOrientation(n, orientation[0], orientation[1], orientation[2], orientation[3]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientationFromArray(int n, const double orientation[4])
{
  this->SetNthControlPointOrientation(n, orientation[0], orientation[1], orientation[2], orientation[3]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientation(int n, double w, double x, double y, double z)
{
  if (!this->ControlPointExists(n, "SetNthControlPointOrientation"))
    {
    return;
    }

  // TODO: return if no modification

  double wxyz[] = { w, x, y, z };
  vtkMRMLMarkupsNode::ConvertOrientationWXYZToMatrix(wxyz, this->GetNthControlPoint(n)->OrientationMatrix);

  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthControlPointOrientation(int n, double orientation[4])
{
  if (!this->ControlPointExists(n, "GetNthControlPointOrientation"))
    {
    return;
    }

  vtkMRMLMarkupsNode::ConvertOrientationMatrixToWXYZ(this->GetNthControlPoint(n)->OrientationMatrix, orientation);
}

//-----------------------------------------------------------
double* vtkMRMLMarkupsNode::GetNthControlPointOrientationMatrix(int n)
{
  if (!this->ControlPointExists(n, "GetNthControlPointOrientationMatrix"))
  {
    static double identity[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    return identity;
  }

  return this->GetNthControlPoint(n)->OrientationMatrix;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientationMatrix(int n, double orientationMatrix[9])
{
  if (!this->ControlPointExists(n, "SetNthControlPointOrientationMatrix"))
    {
    return;
    }
  double* controlPointOrientationMatrix = this->GetNthControlPoint(n)->OrientationMatrix;
  for (int i = 0; i < 9; i++)
    {
    controlPointOrientationMatrix[i] = orientationMatrix[i];
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointOrientationMatrixWorld(int n, double orientationMatrix[9])
{
  this->SetNthControlPointOrientationMatrix(n, orientationMatrix);
}

//-----------------------------------------------------------
double* vtkMRMLMarkupsNode::GetNthControlPointNormal(int n)
{
  if (!this->ControlPointExists(n, "GetNthControlPointNormal"))
    {
    static double identity[3] = { 0.0, 0.0, 1.0 };
    return identity;
    }

  double* orientationMatrix = this->GetNthControlPoint(n)->OrientationMatrix;

  // OrientationMatrix contains z axis direction from index 6
  return orientationMatrix + 6;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthControlPointNormalWorld(int n, double normalWorld[3])
{
  if (!this->ControlPointExists(n, "GetNthControlPointNormalWorld"))
    {
    return;
    }
  this->CurvePolyToWorldTransform->TransformNormalAtPoint(
    &(this->ControlPoints[n]->Position[0]),
    &(this->ControlPoints[n]->OrientationMatrix[0])+6,
    normalWorld);
}

//-----------------------------------------------------------
vtkVector4d vtkMRMLMarkupsNode::GetNthControlPointOrientationVector(int pointIndex)
{
  if (!this->ControlPointExists(pointIndex, "GetNthControlPointOrientationVector"))
    {
    return vtkVector4d(0, 0, 0, 0);
    }

  double orientationWXYZ[4] = { 0.0, 0.0, 0.0, 1.0 };
  vtkMRMLMarkupsNode::ConvertOrientationMatrixToWXYZ(this->GetNthControlPoint(pointIndex)->OrientationMatrix, orientationWXYZ);

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
  if (!this->ControlPointExists(n, "GetNthControlPointAssociatedNodeID"))
    {
    return std::string("");
    }

  return this->GetNthControlPoint(n)->AssociatedNodeID;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointAssociatedNodeID(int n, std::string id)
{
  if (!this->ControlPointExists(n, "SetNthControlPointAssociatedNodeID"))
    {
    return;
    }

  if (this->GetNthControlPoint(n)->AssociatedNodeID == id)
    {
    return;
    }

  vtkDebugMacro("Changing markup " << n << " associated node id from " <<
                this->GetNthControlPoint(n)->AssociatedNodeID.c_str() << " to " << id.c_str());
  this->GetNthControlPoint(n)->AssociatedNodeID = std::string(id.c_str());

  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
}

//-----------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthControlPointID(int n)
{
  if (!this->ControlPointExists(n, "GetNthControlPointID"))
    {
    return std::string("");
    }

  return this->GetNthControlPoint(n)->ID;
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
    ControlPoint *compareControlPoint = this->GetNthControlPoint(controlPointIndex);
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
  if (controlPointIndex >= 0 && controlPointIndex < this->GetNumberOfControlPoints())
    {
    return this->GetNthControlPoint(controlPointIndex);
    }
  return nullptr;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointID(int n, std::string id)
{
  vtkDebugMacro("SetNthControlPointID: n = " << n << ", id = '" << id.c_str() << "'");
  if (!this->ControlPointExists(n, "SetNthControlPointID"))
    {
    return;
    }

  ControlPoint *controlPoint = this->GetNthControlPoint(n);
  if (controlPoint->ID.compare(id) != 0)
    {
    vtkDebugMacro("Changing control point " << n << " associated node id from " <<
                  controlPoint->ID.c_str() << " to " << id.c_str());
    controlPoint->ID = std::string(id.c_str());
    }
  else
    {
    vtkDebugMacro("SetNthControlPointID: not changing, was the same: " << controlPoint->ID);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthControlPointSelected(int n)
{
 if (!this->ControlPointExists(n, "GetNthControlPointSelected"))
   {
   return false;
   }

 return this->GetNthControlPoint(n)->Selected;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointSelected(int n, bool flag)
{
  if (!this->ControlPointExists(n, "SetNthControlPointSelected"))
    {
    return;
    }

  bool wasModified = false;
  ControlPoint *controlPoint = this->GetNthControlPoint(n);
  if (controlPoint->Selected != flag)
    {
    controlPoint->Selected = flag;
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
    wasModified = true;
    }
  if (wasModified)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthControlPointLocked(int n)
{
  if (!this->ControlPointExists(n, "GetNthControlPointLocked"))
    {
    return false;
    }

  return this->GetNthControlPoint(n)->Locked;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointLocked(int n, bool flag)
{
  if (!this->ControlPointExists(n, "SetNthControlPointLocked"))
    {
    return;
    }

  bool wasModified = false;
  ControlPoint *controlPoint = this->GetNthControlPoint(n);
  if (controlPoint->Locked != flag)
    {
    controlPoint->Locked = flag;
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
    wasModified = true;
    }
  if (wasModified)
    {
    this->Modified();
    }

}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthControlPointVisibility(int n)
{
  if (!this->ControlPointExists(n, "GetNthControlPointVisibility"))
    {
    return false;
    }

  return this->GetNthControlPoint(n)->Visibility;
}


//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointVisibility(int n, bool flag)
{
  if (!this->ControlPointExists(n, "SetNthControlPointVisibility"))
    {
    return;
    }

  bool wasModified = false;
  ControlPoint *controlPoint = this->GetNthControlPoint(n);
  if (controlPoint->Visibility != flag)
    {
    controlPoint->Visibility = flag;
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
    wasModified = true;
    }
  if (wasModified)
    {
    this->Modified();
    }

}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthControlPointLabel(int n)
{
  if (!this->ControlPointExists(n, "GetNthControlPointLabel"))
    {
    return std::string("");
    }

  return this->GetNthControlPoint(n)->Label;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointLabel(int n, std::string label)
{
  if (!this->ControlPointExists(n, "SetNthControlPointLabel"))
    {
    return ;
    }

  bool wasModified = false;
  ControlPoint *controlPoint = this->GetNthControlPoint(n);
  if (controlPoint->Label.compare(label))
    {
    controlPoint->Label = label;
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
    wasModified = true;
    }
  if (wasModified)
    {
    this->Modified();
    }

}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthControlPointDescription(int n)
{
  if (!this->ControlPointExists(n, "GetNthControlPointDescription"))
    {
    return std::string("");
    }

  return this->GetNthControlPoint(n)->Description;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthControlPointDescription(int n, std::string description)
{
  if (!this->ControlPointExists(n, "SetNthControlPointDescription"))
    {
    return ;
    }

  bool wasModified = false;
  ControlPoint *controlPoint = this->GetNthControlPoint(n);
  if (controlPoint->Description.compare(description))
    {
    controlPoint->Description = description;
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
    wasModified = true;
    }
  if (wasModified)
    {
    this->Modified();
    }

}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ApplyTransform(vtkAbstractTransform* transform)
{
  int numControlPoints = this->GetNumberOfControlPoints();
  double xyzIn[3];
  double xyzOut[3];
  for (int controlpointIndex = 0; controlpointIndex < numControlPoints; controlpointIndex++)
    {
    this->GetNthControlPointPosition(controlpointIndex, xyzIn);
    transform->TransformPoint(xyzIn,xyzOut);
    this->SetNthControlPointPositionFromArray(controlpointIndex, xyzOut);
    }
  this->StorableModifiedTime.Modified();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::
WriteCLI(std::vector<std::string>& commandLine, std::string prefix,
         int coordinateSystem, int multipleFlag)
{
  Superclass::WriteCLI(commandLine, prefix, coordinateSystem, multipleFlag);

  int numControlPoints = this->GetNumberOfControlPoints();

  // check if the coordinate system flag is set to LPS, otherwise assume RAS
  bool useLPS = false;
  if (coordinateSystem == 1)
    {
    useLPS = true;
    }

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
  return this->Superclass::GetModifiedSinceRead() ||
    (this->GetMTime() > this->GetStoredTime());
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::ResetNthControlPointID(int n)
{
  if (!this->ControlPointExists(n, "ResetNthControlPointID"))
    {
    return false;
    }

  this->SetNthControlPointID(n, this->GenerateUniqueControlPointID());

  return true;
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GenerateUniqueControlPointID()
{
  std::string id;
  int controlPointNumber = this->LastUsedControlPointNumber;
  // increment by one so as not to start with 0
  controlPointNumber++;
  // put the number in a string
  return std::to_string(controlPointNumber);
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GenerateControlPointLabel(int controlPointIndex)
{
  std::string formatString = this->ReplaceListNameInMarkupLabelFormat();
  char buf[128];
  buf[sizeof(buf) - 1] = 0; // make sure the string is zero-terminated
  snprintf(buf, sizeof(buf) - 1, formatString.c_str(), controlPointIndex);
  return std::string(buf);
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetMarkupLabelFormat()
{
  return this->MarkupLabelFormat;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetMarkupLabelFormat(std::string format)
{
  if (this->MarkupLabelFormat.compare(format) == 0)
    {
    return;
    }
  this->MarkupLabelFormat = format;

  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::LabelFormatModifiedEvent);
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::ReplaceListNameInMarkupLabelFormat()
{
  std::string newFormatString = this->MarkupLabelFormat;
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
void vtkMRMLMarkupsNode::ConvertOrientationWXYZToMatrix(double orientationWXYZ[4], double orientationMatrix[9])
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

  // convert the quaternion to a matrix
  double matrix[4][4];
  vtkMatrix4x4::Identity(*matrix);

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
vtkPolyData* vtkMRMLMarkupsNode::GetCurveWorld()
{
  if (this->GetNumberOfControlPoints() < 1)
    {
    return nullptr;
    }
  this->CurvePolyToWorldTransformer->Update();
  vtkPolyData* curvePolyDataWorld = this->CurvePolyToWorldTransformer->GetOutput();
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
  if (this->CurveGenerator->IsInterpolatingCurve())
    {
    return int(floor(interpolatedPointIndex / this->CurveGenerator->GetNumberOfPointsPerInterpolatingSegment()));
    }
  if (this->CurveGenerator->GetPolynomialPointSortingMethod() == vtkCurveGenerator::SORTING_METHOD_MINIMUM_SPANNING_TREE_POSITION)
    {
    // If sorting is based on spanning tree then we can insert point anywhere (so we add to the end for simplicity).
    return this->GetNumberOfControlPoints();
    }
  // In case of approximating curves, there is no clear assignment between control points and curve points.
  vtkWarningMacro("vtkMRMLMarkupsNode::GetControlPointIndexFromInterpolatedPointIndex for non-interpolated"
    " curves, minimum spanning tree sorting is recommended");
  return this->GetNumberOfControlPoints();
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
void vtkMRMLMarkupsNode::GetMarkupPoint(int markupIndex, int pointIndex, double point[3])
{
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
    vtkErrorMacro("vtkMRMLMarkupsNode::GetMarkupPoint failed: only one markup with mutiple control points is supported.");
    }
}
