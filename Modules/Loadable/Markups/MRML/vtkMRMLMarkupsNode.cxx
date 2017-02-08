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
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsStorageNode.h"
#include "vtkMRMLTransformNode.h"

// Slicer MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkBitArray.h>
#include <vtkCommand.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStringArray.h>

// STD includes
#include <sstream>
#include <algorithm>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsNode::vtkMRMLMarkupsNode()
{
  this->TextList = vtkStringArray::New();
  this->Locked = 0;
  this->MarkupLabelFormat = std::string("%N-%d");
  this->MaximumNumberOfMarkups = 0;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode::~vtkMRMLMarkupsNode()
{
  this->TextList->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkIndent indent(nIndent);

  of << indent << " locked=\"" << this->Locked << "\"";
  of << indent << " markupLabelFormat=\"" << this->MarkupLabelFormat.c_str() << "\"";

  int textLength = this->TextList->GetNumberOfValues();

  for (int i = 0 ; i < textLength; i++)
    {
    of << " textList" << i << "=\"" << this->TextList->GetValue(i) << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->RemoveAllMarkups();
  this->RemoveAllTexts();

  Superclass::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strncmp(attName, "textList", 9))
      {
      this->AddText(attValue);
      }
    else if (!strcmp(attName, "locked"))
      {
      this->SetLocked(atof(attValue));
      }
    else if (!strcmp(attName, "markupLabelFormat"))
      {
      this->SetMarkupLabelFormat(attValue);
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);

  vtkMRMLMarkupsNode *node = (vtkMRMLMarkupsNode *) anode;
  if (!node)
    {
    return;
    }

  this->SetLocked(node->GetLocked());
  this->SetMarkupLabelFormat(node->GetMarkupLabelFormat());
  this->TextList->DeepCopy(node->TextList);

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
    if (this->GetNumberOfMarkups() != 0 &&
        node->GetNumberOfMarkups() == 0)
      {
      // just return for now
      vtkWarningMacro("MarkupsNode Copy: Scene view is restoring and list to restore is empty, skipping copy of points");
      return;
      }
    }

  this->Markups.clear();
  int numMarkups = node->GetNumberOfMarkups();
  for (int n = 0; n < numMarkups; n++)
    {
    Markup *markup = node->GetNthMarkup(n);
    this->AddMarkup(*markup);
    }

  // set max number of markups after adding the new ones
  this->MaximumNumberOfMarkups = node->MaximumNumberOfMarkups;
}


//-----------------------------------------------------------
void vtkMRMLMarkupsNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::PrintMarkup(ostream& os, vtkIndent indent, Markup *markup)
{
  if (!markup)
    {
    return;
    }

  os << indent.GetNextIndent() << "ID = " << markup->ID.c_str() << "\n";
  os << indent.GetNextIndent() << "Label = " << markup->Label.c_str() << "\n";
  os << indent.GetNextIndent() << "Description = " << markup->Description.c_str() << "\n";
  os << indent.GetNextIndent() << "Associated node id = " << markup->AssociatedNodeID.c_str() << "\n";
  os << indent.GetNextIndent() << "Selected = " << markup->Selected << "\n";
  os << indent.GetNextIndent() << "Locked = " << markup->Locked << "\n";
  os << indent.GetNextIndent() << "Visibility = " << markup->Visibility << "\n";
  int numPoints = markup->points.size();
  for (int p = 0; p < numPoints; p++)
    {
    vtkVector3d point = markup->points[p];
    os << indent.GetNextIndent() << "p" << p << ": " << point.GetX() << ", " << point.GetY() << ", " << point.GetZ() << "\n";
    }
  os << indent.GetNextIndent() << "Orientation = "
     << markup->OrientationWXYZ[0] << ","
     << markup->OrientationWXYZ[1] << ","
     << markup->OrientationWXYZ[2] << ","
     << markup->OrientationWXYZ[3] << "\n";
}


//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Locked: " << this->Locked << "\n";
  os << indent << "MarkupLabelFormat: " << this->MarkupLabelFormat.c_str() << "\n";
  for (int i = 0; i < this->GetNumberOfMarkups(); i++)
    {
    os << indent << "Markup " << i << ":\n";
    Markup *markup = this->GetNthMarkup(i);
    this->PrintMarkup(os, indent, markup);
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
void vtkMRMLMarkupsNode::RemoveAllMarkups()
{
  int wasModifying = this->StartModify();

  this->SetLocked(0); // Should this be done here ?

  while(this->Markups.size() > 0)
    {
    this->RemoveMarkup(0);
    }
  this->MaximumNumberOfMarkups = 0;

  this->EndModify(wasModifying);
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
  if (((this->TextList->GetNumberOfValues() == 0) && (newText == NULL || newString == "")) ||
      ((this->TextList->GetNumberOfValues() > id) &&
       (this->TextList->GetValue(id) == newString)
        ) )
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
  if ((this->GetNumberOfTexts() <= n) || n < 0 )
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

  for (int i = id; id < n-1; i++ )
    {
      this->TextList->SetValue(i,this->GetText(i+1));
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
  return this->TextList->GetNumberOfValues();
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsNode::RemoveAllTexts()
{
  this->TextList->Initialize();
}

//-------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLMarkupsNode::CreateDefaultStorageNode()
{
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLMarkupsStorageNode::New());
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
//  this->ModifiedSinceReadOn();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::MarkupExists(int n)
{
  if (n < 0)
    {
    vtkErrorMacro("MarkupExists: n of " << n << " must be greater than or equal to zero.");
    return false;
    }
  if (n >= this->GetNumberOfMarkups())
    {
    vtkErrorMacro("MarkupExists: n of " << n << " must be less than the current number of markups, " << this->GetNumberOfMarkups());
    return false;
    }
  else
    {
    return true;
    }
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNumberOfMarkups()
{
  return this->Markups.size();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::PointExistsInMarkup(int p, int n)
{
  if (!this->MarkupExists(n))
    {
    return false;
    }
  if (p < 0)
    {
    vtkErrorMacro("PointExistsInMarkup: point index of " << p << " is less than 0");
    return false;
    }
  int numPoints = this->GetNumberOfPointsInNthMarkup(n);

  if (p >=  numPoints )
    {
    vtkErrorMacro("PointExistsInMarkup: point index of " << p << " must be less than the current number of points in markup " << n << ", " << numPoints);
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
Markup *vtkMRMLMarkupsNode::GetNthMarkup(int n)
{
  if (this->MarkupExists(n))
    {
    return &(this->Markups[n]);
    }

  return NULL;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode:: GetNumberOfPointsInNthMarkup(int n)
{
  vtkDebugMacro("GetNumberOfPointsInNthMarkup: n = " << n << ", number of marksups = " << this->GetNumberOfMarkups());
  if (!this->MarkupExists(n))
    {
    return 0;
    }
  Markup *markupN = this->GetNthMarkup(n);
  if (markupN)
    {
    return markupN->points.size();
    }
  else
    {
    return 0;
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::InitMarkup(Markup *markup)
{
  if (!markup)
    {
    vtkErrorMacro("InitMarkup: null markup!");
    return;
    }

  // generate a unique id based on list policy
  std::string id = this->GenerateUniqueMarkupID();
  markup->ID = id;

  // set a default label with a number higher than others in the list
  if (markup->Label.empty())
    {
    int number = this->MaximumNumberOfMarkups + 1;
    std::string formatString = this->ReplaceListNameInMarkupLabelFormat();
    char buff[MARKUPS_BUFFER_SIZE];
    sprintf(buff, formatString.c_str(), number);
    markup->Label = std::string(buff);
    }

  // use an empty description
  markup->Description = std::string("");
  // use an empty associated node id
  markup->AssociatedNodeID = std::string("");

  // orientatation is 0 around the z axis
  markup->OrientationWXYZ[0] = 0.0;
  markup->OrientationWXYZ[1] = 0.0;
  markup->OrientationWXYZ[2] = 0.0;
  markup->OrientationWXYZ[3] = 1.0;

  // set the flags
  markup->Selected = true;
  markup->Locked = false;
  markup->Visibility = true;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddMarkup(Markup markup)
{
  this->Markups.push_back(markup);
  this->MaximumNumberOfMarkups++;

  int markupIndex = this->GetNumberOfMarkups() - 1;

  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::MarkupAddedEvent, (void*)&markupIndex);
  return markupIndex;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddMarkupWithNPoints(int n, std::string label /*=std::string()*/, vtkVector3d* point /*=NULL*/)
{
  int markupIndex = -1;
  if (n < 0)
    {
    vtkErrorMacro("AddMarkupWithNPoints: invalid number of points " << n);
    return markupIndex;
    }
  Markup markup;
  markup.Label = label;
  this->InitMarkup(&markup);
  if (point != NULL)
    {
    markup.points = std::vector<vtkVector3d>(n,*point);
    }
  else
    {
    markup.points = std::vector<vtkVector3d>(n,vtkVector3d(0,0,0));
    }
  return this->AddMarkup(markup);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddPointToNewMarkup(vtkVector3d point, std::string label /*=std::string()*/)
{
  return this->AddMarkupWithNPoints(1, label, &point);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddPointWorldToNewMarkup(vtkVector3d pointWorld, std::string label /*=std::string()*/)
{
  vtkVector3d point;
  this->TransformPointFromWorld(pointWorld, point);
  return this->AddMarkupWithNPoints(1, label, &point);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddPointToNthMarkup(vtkVector3d point, int n)
{
  int pointIndex = 0;
  if (this->MarkupExists(n))
    {
    this->Markups[n].points.push_back(point);
    }
  return pointIndex;
}

//-----------------------------------------------------------
vtkVector3d vtkMRMLMarkupsNode::GetMarkupPointVector(int markupIndex, int pointIndex)
{
  vtkVector3d point;
  point.SetX(0.0);
  point.SetY(0.0);
  point.SetZ(0.0);
  if (!PointExistsInMarkup(pointIndex, markupIndex))
    {
    return point;
    }
  point = this->GetNthMarkup(markupIndex)->points[pointIndex];
  return point;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetMarkupPoint(int markupIndex, int pointIndex, double point[3])
{
  vtkVector3d vectorPoint = this->GetMarkupPointVector(markupIndex, pointIndex);
  point[0] = vectorPoint.GetX();
  point[1] = vectorPoint.GetY();
  point[2] = vectorPoint.GetZ();
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetMarkupPointLPS(int markupIndex, int pointIndex, double point[3])
{
  vtkVector3d vectorPoint = this->GetMarkupPointVector(markupIndex, pointIndex);
  point[0] = -1.0 * vectorPoint.GetX();
  point[1] = -1.0 * vectorPoint.GetY();
  point[2] = vectorPoint.GetZ();
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::GetMarkupPointWorld(int markupIndex, int pointIndex, double worldxyz[4])
{
  vtkVector3d world;
  this->TransformPointToWorld(this->GetMarkupPointVector(markupIndex, pointIndex), world);
  worldxyz[0] = world[0];
  worldxyz[1] = world[1];
  worldxyz[2] = world[2];
  worldxyz[3] = 1;
  return 1;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::RemoveMarkup(int m)
{
  if (this->MarkupExists(m))
    {
    vtkDebugMacro("RemoveMarkup: m = " << m << ", markups size = " << this->Markups.size());
    this->Markups.erase(this->Markups.begin() + m);

    this->Modified();
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::MarkupRemovedEvent, (void*)&m);
    }
}

//-----------------------------------------------------------
bool vtkMRMLMarkupsNode::InsertMarkup(Markup m, int targetIndex)
{
  int listSize = this->GetNumberOfMarkups();

  int destIndex = targetIndex;
  if (targetIndex < 0)
    {
    destIndex = 0;
    }
  else if (targetIndex > listSize)
    {
    destIndex = listSize;
    }
  vtkDebugMacro("InsertMarkup: list size = " << listSize
                << ", input target index = " << targetIndex
                << ", adjusted destination index = " << destIndex);

  std::vector < Markup >::iterator pos;
  pos = this->Markups.begin() + destIndex;

  std::vector < Markup >::iterator result;
  result = this->Markups.insert(pos, m);

  // sanity check
  if (result->Label.compare(m.Label) != 0)
    {
    vtkErrorMacro("InsertMarkup: failed to insert a markup at index " << destIndex
                  << ", expected label on that markup to be " << m.Label.c_str()
                  << " but got " << result->Label.c_str());
    return false;
    }

  // let observers know that a markup was added
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::MarkupAddedEvent, (void*)&targetIndex);

  return true;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::CopyMarkup(Markup *source, Markup *target)
{
  if (source == NULL || target == NULL)
    {
    return;
    }

  target->ID = source->ID;
  target->Label = source->Label;
  target->Description = source->Description;
  target->AssociatedNodeID = source->AssociatedNodeID;
  target->Selected = source->Selected;
  target->Locked = source->Locked;
  target->Visibility = source->Visibility;
  // now iterate over the points
  target->points.clear();
  int numPoints = source->points.size();
  for (int p = 0; p < numPoints; p++)
    {
    vtkVector3d point = source->points[p];
    target->points.push_back(point);
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SwapMarkups(int m1, int m2)
{
  if (!this->MarkupExists(m1))
    {
    vtkErrorMacro("SwapMarkups: first markup index is out of range 0-" << this->GetNumberOfMarkups() -1 << ", m1 = " << m1);
    return;
    }
  if (!this->MarkupExists(m2))
    {
    vtkErrorMacro("SwapMarkups: second markup index is out of range 0-" << this->GetNumberOfMarkups() -1 << ", m2 = " << m2);
    return;
    }

  Markup *m1Markup = this->GetNthMarkup(m1);
  Markup m1MarkupBackup;
  // make a copy of the first markup
  this->CopyMarkup(m1Markup, &m1MarkupBackup);
  // copy the second markup into the first
  this->CopyMarkup(this->GetNthMarkup(m2), m1Markup);
  // and copy the backup of the first one into the second
  this->CopyMarkup(&m1MarkupBackup, this->GetNthMarkup(m2));

  // and let listeners know that two markups have changed
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&m1);
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&m2);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetMarkupPointFromPointer(const int markupIndex, const int pointIndex,
                                        const double * pos)
{
  if (!pos)
    {
    vtkErrorMacro("SetMarkupPointFromPointer: invalid position pointer!");
    return;
    }
  this->SetMarkupPoint(markupIndex, pointIndex, pos[0], pos[1], pos[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetMarkupPointFromArray(const int markupIndex, const int pointIndex,
                                        const double pos[3])
{
  this->SetMarkupPoint(markupIndex, pointIndex, pos[0], pos[1], pos[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetMarkupPoint(const int markupIndex, const int pointIndex,
                                        const double x, const double y, const double z)
{
  if (!this->PointExistsInMarkup(pointIndex, markupIndex))
    {
    return;
    }
  Markup *markup = this->GetNthMarkup(markupIndex);
  if (markup)
    {
    markup->points[pointIndex].SetX(x);
    markup->points[pointIndex].SetY(y);
    markup->points[pointIndex].SetZ(z);
    }
  else
    {
    vtkErrorMacro("SetMarkupPoint: unable to get markup " << markupIndex);
    }
  // throw an event to let listeners know the position has changed
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, (void*)&markupIndex);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetMarkupPointLPS(const int markupIndex, const int pointIndex,
                                        const double x, const double y, const double z)
{
  double r, a, s;
  r = -x;
  a = -y;
  s = z;
  this->SetMarkupPoint(markupIndex, pointIndex, r, a, s);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetMarkupPointWorld(const int markupIndex, const int pointIndex,
                                             const double x, const double y, const double z)
{
  if (!this->PointExistsInMarkup(pointIndex, markupIndex))
    {
    return;
    }
  vtkVector3d markupxyz;
  TransformPointFromWorld(vtkVector3d(x,y,z), markupxyz);
  this->SetMarkupPoint(markupIndex, pointIndex, markupxyz[0], markupxyz[1], markupxyz[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupOrientationFromPointer(int n, const double *orientation)
{
  if (!orientation)
    {
    vtkErrorMacro("Invalid orientation pointer!");
    return;
    }
  this->SetNthMarkupOrientation(n, orientation[0], orientation[1], orientation[2], orientation[3]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupOrientationFromArray(int n, const double orientation[4])
{
  this->SetNthMarkupOrientation(n, orientation[0], orientation[1], orientation[2], orientation[3]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupOrientation(int n, double w, double x, double y, double z)
{
  if (!this->MarkupExists(n))
    {
    return;
    }
  Markup *markup = this->GetNthMarkup(n);
  if (!markup)
    {
    return;
    }
  markup->OrientationWXYZ[0] = w;
  markup->OrientationWXYZ[1] = x;
  markup->OrientationWXYZ[2] = y;
  markup->OrientationWXYZ[3] = z;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetNthMarkupOrientation(int n, double orientation[4])
{
  if (!this->MarkupExists(n))
    {
    return;
    }
  Markup *markup = this->GetNthMarkup(n);
  if (!markup)
    {
    return;
    }
  orientation[0] = markup->OrientationWXYZ[0];
  orientation[1] = markup->OrientationWXYZ[1];
  orientation[2] = markup->OrientationWXYZ[2];
  orientation[3] = markup->OrientationWXYZ[3];
}

//-----------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthMarkupAssociatedNodeID(int n)
{
  std::string id = std::string("");
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      id = markup->AssociatedNodeID;
      }
    }
  else
    {
    vtkErrorMacro("GetNthMarkupAssociatedNodeID: markup " << n << " doesn't exist");
    }
  return id;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupAssociatedNodeID(int n, std::string id)
{
  vtkDebugMacro("SetNthMarkupAssociatedNodeID: n = " << n << ", id = '" << id.c_str() << "'");
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      vtkDebugMacro("Changing markup " << n << " associated node id from " << markup->AssociatedNodeID.c_str() << " to " << id.c_str());
      markup->AssociatedNodeID = std::string(id.c_str());
      int markupIndex = n;
      this->Modified();
      this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&markupIndex);
      }
    }
  else
    {
    vtkErrorMacro("SetNthMarkupAssociatedNodeID: markup " << n << " doesn't exist, can't set id to " << id);
    }
}

//-----------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthMarkupID(int n)
{
  std::string id = std::string("");
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      id = markup->ID;
      }
    }
  else
    {
    vtkErrorMacro("GetNthMarkupID: markup " << n << " doesn't exist");
    }
  return id;
}

//-------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetMarkupIndexByID(const char* markupID)
{
  if (!markupID)
    {
    return -1;
    }

  int numberOfMarkups = this->GetNumberOfMarkups();
  for (int i = 0; i < numberOfMarkups; ++i)
    {
    Markup* compareMarkup = this->GetNthMarkup(i);
    if (compareMarkup &&
        strcmp(compareMarkup->ID.c_str(), markupID) == 0)
      {
      return i;
      }
    }
  return -1;
}

//-------------------------------------------------------------------------
Markup* vtkMRMLMarkupsNode::GetMarkupByID(const char* markupID)
{
  if (!markupID)
    {
    return NULL;
    }

  int markupIndex = this->GetMarkupIndexByID(markupID);
  if (markupIndex >= 0 && markupIndex < this->GetNumberOfMarkups())
    {
    return this->GetNthMarkup(markupIndex);
    }
  return NULL;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupID(int n, std::string id)
{
  vtkDebugMacro("SetNthMarkupID: n = " << n << ", id = '" << id.c_str() << "'");
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      if (markup->ID.compare(id) != 0)
        {
        vtkDebugMacro("Changing markup " << n << " associated node id from " << markup->ID.c_str() << " to " << id.c_str());
        markup->ID = std::string(id.c_str());
        }
      else
        {
        vtkDebugMacro("SetNthMarkupID: not changing, was the same: " << markup->ID);
        }
      }
    }
  else
    {
    vtkWarningMacro("SetNthMarkupID: markup " << n << " doesn't exist, can't set id to " << id);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthMarkupSelected(int n)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      return markup->Selected;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupSelected(int n, bool flag)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      if (markup->Selected != flag)
        {
        markup->Selected = flag;
        int markupIndex = n;
        this->Modified();
        this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&markupIndex);
        }
      }
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthMarkupLocked(int n)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      return markup->Locked;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupLocked(int n, bool flag)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      if (markup->Locked != flag)
        {
        markup->Locked = flag;
        int markupIndex = n;
        this->Modified();
        this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&markupIndex);
        }
      }
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthMarkupVisibility(int n)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      return markup->Visibility;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupVisibility(int n, bool flag)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      if (markup->Visibility != flag)
        {
        markup->Visibility = flag;
        int markupIndex = n;
        this->Modified();
        this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&markupIndex);
        }
      }
    }
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthMarkupLabel(int n)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      return markup->Label;
      }
    }
  return std::string("");
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupLabel(int n, std::string label)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      if (markup->Label.compare(label))
        {
        markup->Label = label;
        int markupIndex = n;
        this->Modified();
        this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&markupIndex);
        }
      }
    }
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthMarkupDescription(int n)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      return markup->Description;
      }
    }
  return std::string("");
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupDescription(int n, std::string description)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      if (markup->Description.compare(description))
        {
        markup->Description = description;
        int markupIndex = n;
        this->Modified();
        this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&markupIndex);
        }
      }
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
  int numMarkups = this->GetNumberOfMarkups();
  double xyzIn[3];
  double xyzOut[3];
  for (int m=0; m<numMarkups; m++)
    {
    int numPoints = this->GetNumberOfPointsInNthMarkup(m);
    for (int n=0; n<numPoints; n++)
      {
      this->GetMarkupPoint(m, n, xyzIn);
      transform->TransformPoint(xyzIn,xyzOut);
      this->SetMarkupPointFromArray(m, n, xyzOut);
      }
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

  int numMarkups = this->GetNumberOfMarkups();

  // check if the coordinate system flag is set to LPS, otherwise assume RAS
  bool useLPS = false;
  if (coordinateSystem == 1)
    {
    useLPS = true;
    }

  // loop over the markups
  for (int m=0; m<numMarkups; m++)
    {
    // only use selected markups
    if (this->GetNthMarkupSelected(m))
      {
      int numPoints = this->GetNumberOfPointsInNthMarkup(m);
      // loop over the points
      for (int n=0; n<numPoints; n++)
        {
        std::stringstream ss;
        double point[3];
        if (useLPS)
          {
          this->GetMarkupPointLPS(m, n, point);
          }
        else
          {
          this->GetMarkupPoint(m, n, point);
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
        }
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
bool vtkMRMLMarkupsNode::ResetNthMarkupID(int n)
{
  // is n in the list?
  if (!this->MarkupExists(n))
    {
    return false;
    }
  // first generate a new id
  std::string newID = this->GenerateUniqueMarkupID();

  // then set it
  this->SetNthMarkupID(n, newID);

  return true;
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GenerateUniqueMarkupID()
{
  std::string id;

  if (this->Scene)
    {
    // base it on the class of this node so that they're unique across lists
    id = this->Scene->GenerateUniqueName(this->GetClassName());
    // the first time this is called, the return will be the bare class name
    // with no number, the next one will have _1 appended. To unify it, check
    // for the first case and append _0
    if (id.compare(this->GetClassName()) == 0)
      {
      id = id + std::string("_0");
      }
    }
  else
    {
    vtkDebugMacro("InitMarkup: markups node isn't in a scene yet, can't guarantee a unique id.");
    // use the maximum number of markups to get a number unique to this list
    int numberOfMarkups = this->MaximumNumberOfMarkups;
    // increment by one so as not to start with 0
    numberOfMarkups++;
    // put the number in a string
    std::stringstream ss;
    ss << numberOfMarkups;
    ss >> id;
    }

  return id;
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
    if (this->GetName() != NULL)
      {
      name = std::string(this->GetName());
      }
    newFormatString.replace(replacePos, 2, name);
    }
  return newFormatString;
}
