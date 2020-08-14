// MRML includes
#include "vtkMRMLAnnotationControlPointsNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationControlPointsStorageNode.h"
#include <vtkMRMLScene.h>

// VTK includes
#include "vtkBitArray.h"
#include "vtkObjectFactory.h"
#include <vtkPolyData.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <sstream>

#define NUMERIC_ZERO 1.0e-6

// KPs Todos
// - create specific event for node modification
// - talk to Steve if we have to do anything when UpdatingScene
// - NumberingScheme should not be in annotation node - should be in fiducial nodes - just put it here right now

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationControlPointsNode);

vtkMRMLAnnotationControlPointsNode::vtkMRMLAnnotationControlPointsNode()
{
  this->NumberingScheme = UseID;
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::WriteXML(ostream& of, int nIndent)
{
  /*
  // special case: if this annotation is in a hierarchy, the hierarchy took
  // care of writing it already
  vtkMRMLHierarchyNode *hnode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(this->GetScene(), this->GetID());

  if (hnode &&
  hnode->GetParentNodeID())
  {
  vtkWarningMacro("WriteXML: node " << this->GetName() << " is in a hierarchy, " << hnode->GetName() << ", assuming that it wrote it out already");
  return;
  }
  */
  // cout << "vtkMRMLAnnotationControlPointsNode::WriteXML start" << endl;
  Superclass::WriteXML(of, nIndent);

  if (this->GetPoints())
    {
    vtkPoints *points = this->GetPoints();
    int n = points->GetNumberOfPoints();

    of << " ctrlPtsCoord=\"";
    for (int i = 0; i < n; i++)
      {
      double* ptr = points->GetPoint(i);
      of << ptr[0] << " "<<  ptr[1] << " "<<  ptr[2];
      if (i < n-1)
        {
        of << "|";
        }
      }
    of << "\"";


    for (int j = NUM_TEXT_ATTRIBUTE_TYPES; j < NUM_CP_ATTRIBUTE_TYPES; j++)
      {
      of << " " <<this->GetAttributeTypesEnumAsString(j)<<"=\"";
      for (int i = 0; i < n-1; i++)
        {
        of << this->GetAnnotationAttribute(i, j) << " ";
        }
      if (n)
        {
        of << this->GetAnnotationAttribute(n-1, j);
        }
      of << "\"";
      }
    }
  else
    {
    of << " ctrlPtsCoord=\"\"";
    for (int j = NUM_TEXT_ATTRIBUTE_TYPES; j < NUM_CP_ATTRIBUTE_TYPES; j++)
      {
      of << " " << this->GetAttributeTypesEnumAsString(j) << "=\"\"";
      }
    }

  of << " ctrlPtsNumberingScheme=\"" << this->NumberingScheme << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::
WriteCLI(std::vector<std::string>& commandLine, std::string prefix,
         int coordinateSystem, int multipleFlag)
{
  Superclass::WriteCLI(commandLine, prefix, coordinateSystem, multipleFlag);

  // Ignoring multipleFlag, because by convention there is only one annotation
  // per node, so if there's a 6 point ROI, it needs to have all of it's
  // points written out. The multiple flag is managed at the CLI module logic
  // level where it determines which child nodes in an annotation hierarchy
  // are added to the command line

  if (this->GetPoints())
    {
    vtkPoints *points = this->GetPoints();
    int n = points->GetNumberOfPoints();

    if (multipleFlag == false &&
        n > 1)
      {
      vtkWarningMacro("WriteCLI - Ignoring 'multipleFlag' and writing all "
                      << n << " points for annotation " << this->GetID()
                      << ". For more details see "
                      << "https://github.com/Slicer/Slicer/issues/1910");
      }

    std::stringstream ss;
    for (int i = 0; i < n; i++ )
      {
      double* ptr = points->GetPoint(i);
      if (i==0 && prefix.compare("") != 0)
        {
        ss << prefix << " ";
        }
      else if (i>0)
        {
        ss << ",";
        }
      if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemRAS)
        {
        // RAS
        ss << ptr[0] << "," <<  ptr[1] << "," <<  ptr[2] ;
        }
      else if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
        {
        // LPS
        double lps[3];
        lps[0] = -1.0 * ptr[0];
        lps[1] = -1.0 * ptr[1];
        lps[2] = ptr[2];
        ss << lps[0] << "," <<  lps[1] << "," <<  lps[2] ;
        }
      }
    commandLine.push_back(ss.str());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLAnnotationControlPointsNode::ReadXMLAttributes start"<< endl;

  int disabledModify = this->StartModify();

  // this->ResetAnnotations();
  Superclass::ReadXMLAttributes(atts);


  while (*atts != nullptr)
    {
    const char* attName = *(atts++);
    std::string attValue(*(atts++));


    if (!strcmp(attName, "ctrlPtsCoord"))
      {
    std::string valStr(attValue);
    std::replace(valStr.begin(), valStr.end(), '|', ' ');

    std::stringstream ss;
    ss << valStr;
    float d;
    std::vector<double> tmpVec;
    while (ss >> d)
        {
      tmpVec.push_back(d);
        }

    for (int i = 0; i < int(tmpVec.size()); i += 3)
      {
        double tmpDoubleVec[3] = {tmpVec[i],tmpVec[i + 1],tmpVec[i + 2]};
        this->AddControlPoint(tmpDoubleVec,0,0);
      }
      }
    else if  (!strcmp(attName, "ctrlPtsNumberingScheme"))
      {
    std::stringstream ss;
    ss << attValue;
    ss >> this->NumberingScheme;
      }
    else
      {
    int j = NUM_TEXT_ATTRIBUTE_TYPES;
    while (j < NUM_CP_ATTRIBUTE_TYPES)
      {
        if (!strcmp(attName, this->GetAttributeTypesEnumAsString(j)))
          {
        std::stringstream ss;
        ss << attValue;
        double value;
        vtkIdType id = 0;
        while (ss >> value)
          {
            this->SetAnnotationAttribute(id,j, value);
            id ++;
          }
        j = NUM_CP_ATTRIBUTE_TYPES;
          }
        j++;
      }
      }
    }
  this->EndModify(disabledModify);
}


//-----------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
//void vtkMRMLAnnotationControlPointsNode::PrintSelf(ostream& os, vtkIndent indent)
//{
//  vtkMRMLModelNode::PrintSelf(os,indent);
//  os << endl;
//  this->PrintAnnotationInfo(os,indent,0);
//}
//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag)
{
  if (titleFlag)
    {

      os <<indent << "vtkMRMLAnnotationControlPointsNode: Annotation Summary";
      if (this->GetName())
    {
      os << " of " << this->GetName();
    }
      os << endl;
    }

  Superclass::PrintAnnotationInfo(os, indent, 0);

  if (this->GetPoints())
    {
    os << indent << "ctrlPtsCoord: " ;
    vtkPoints *points = this->GetPoints();
    int n = points->GetNumberOfPoints();
    for (int i = 0; i < n; i++ )
      {
      double* ptr = points->GetPoint(i);
      os << ptr[0] << " "<<  ptr[1] << " "<<  ptr[2] ;
      if (i < n-1)
        {
          os << " | ";
        }
    }
      os << endl;

      for (int j = NUM_TEXT_ATTRIBUTE_TYPES ; j < NUM_CP_ATTRIBUTE_TYPES; j ++)
    {
      os << indent << this->GetAttributeTypesEnumAsString(j) <<": ";
      for (int i = 0; i < n; i++ )
        {
          os << this->GetAnnotationAttribute(i,j) << " " ;
        }
      os << endl;
    }
    }
  else
    {
      os << indent << "ctrlPtsCoord: None" << endl;
      for (int j = NUM_TEXT_ATTRIBUTE_TYPES  ; j < NUM_CP_ATTRIBUTE_TYPES; j ++)
    {
      os << indent << this->GetAttributeTypesEnumAsString(j) << ": None" << endl;
    }
    }

  os << indent << "ctrlPtsNumberingScheme: " << this->GetNumberingSchemeAsString() << " (" << this->NumberingScheme << ")\n";
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationPointDisplayNode* vtkMRMLAnnotationControlPointsNode::GetAnnotationPointDisplayNode()
{
  int nnodes = this->GetNumberOfDisplayNodes();
  vtkMRMLAnnotationPointDisplayNode *node = nullptr;
  for (int n=0; n<nnodes; n++)
    {
      // vtkMRMLDisplayableNode stores the ids in DisplayNodeIDs
    node = vtkMRMLAnnotationPointDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (node && node->IsA("vtkMRMLAnnotationPointDisplayNode"))
      {
    return node;
      }
    }
  return nullptr;
}


//---------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::CreateAnnotationPointDisplayNode()
{
  vtkMRMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (node) return;
  if (!this->GetScene())
    {
      vtkErrorMacro("vtkMRMLAnnotationControlPointsNode::CreateAnnotationControlPointDisplayNode Annotation: No scene defined" ) ;
      return;
    }

  node = vtkMRMLAnnotationPointDisplayNode::New();
  node->SetScene(this->GetScene());
  this->GetScene()->AddNode(node);

  // vtkMRMLDisplayableNode stores the ids in DisplayNodeIDs
  this->AddAndObserveDisplayNodeID(node->GetID());
  node->Delete();

  // This assumes I want to display the poly data , which I do not want to as it is displayed by widgets
  // node->SetPolyData(this->GetPolyData());

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::ResetAnnotations()
{
  Superclass::ResetAnnotations();
  this->ResetControlPoints();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::CreatePolyData()
{
  Superclass::CreatePolyData();
  vtkPoints *polyPoint = this->GetPoints();
  if (!polyPoint)
    {
    polyPoint = vtkPoints::New();
    this->GetPolyData()->SetPoints(polyPoint);
    polyPoint->Delete();
    }
  vtkDebugMacro("CreatePolyData: " << this->GetPoints());
  // This assumes I want to display the poly data , which I do not want to as it is displayed by widgets
  //if (this->GetAnnotationPointDisplayNode())
  //  {
  //    this->GetAnnotationPointDisplayNode()->SetPolyData(this->GetPolyData());
  //  }


}
//---------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::ResetControlPoints()
{
  this->CreatePolyData();

  this->GetPoints()->Reset();

  this->ResetControlPointsAttributesAll();
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsNode::GetNumberOfControlPoints()
{
  if (!this->GetPoints())
    {
    this->ResetControlPoints();
    }
  vtkPoints *points = this->GetPoints();
  return points->GetNumberOfPoints();
}


//---------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::DeleteControlPoint(int id)
{
  if (!this->GetPoints())
    {
    this->ResetControlPoints();
    }

  vtkPoints *points = this->GetPoints();
  int n = points->GetNumberOfPoints();
  if (id< 0 || id > n-1)
    {
      vtkErrorMacro("Annotation " << this->GetName() << " id is out of range !");
      return ;
    }

  // create event in hearder when deleted
  for (int i = id; i < n-1; i++ )
    {
      points->SetPoint(i,points->GetPoint(i+1));
    }
  points->SetNumberOfPoints(n-1);

  for (int j = NUM_TEXT_ATTRIBUTE_TYPES ; j < NUM_CP_ATTRIBUTE_TYPES; j ++)
    {
      vtkBitArray *dataArray = dynamic_cast <vtkBitArray *> (this->GetAnnotationAttributes(j));
      if (!dataArray || dataArray->GetSize() != n)
        {
          vtkErrorMacro("Annotation " << this->GetName() << " Attribute " << this->GetAttributeTypesEnumAsString(j)
                        << " is out of sync with PolyData->GetPoints()! Size of Attribute:" << dataArray->GetSize()
                        << " Number of Points: " << n);
        }
      else
    {
      this->DeleteAttribute(id,j);
    }
    }
}


//---------------------------------------------------------------------------
/// Return the RAS coordinates of point ID.
/// Try to limit calling this function because it is performance critical.
/// Also, when queried again it resets all former pointers. Copying is therefore necessary.
double* vtkMRMLAnnotationControlPointsNode::GetControlPointCoordinates(vtkIdType id)
{

  if (!this->GetPoints() ||
      this->GetPoints()->GetNumberOfPoints() <= id)
    {
    vtkErrorMacro("vtkMRMLAnnotationControlPointsNode::GetControlPointWorldCoordinates() no control point with index" << id) ;
    return nullptr;
    }

  return this->GetPoints()->GetPoint(id);
}

//---------------------------------------------------------------------------
/// Return the RAS coordinates in the World coordinates (including all transformations to parents) of point ID.
/// Try to limit calling this function because it is performance critical.
/// Also, when queried again it resets all former pointers. Copying is therefore necessary.
void vtkMRMLAnnotationControlPointsNode::GetControlPointWorldCoordinates(vtkIdType id, double *point)
{

  if (!this->GetPoints() ||
      this->GetPoints()->GetNumberOfPoints() <= id)
    {
    vtkErrorMacro("vtkMRMLAnnotationControlPointsNode::GetControlPointWorldCoordinates() no control point with index" << id) ;
    return;
    }
  double *p = this->GetPoints()->GetPoint(id);
  double localPoint[] = {p[0],p[1],p[2],1};
  this->TransformPointToWorld(localPoint, point);
}


//---------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::ResetControlPointsAttributesAll() {
  for (int j = NUM_TEXT_ATTRIBUTE_TYPES ; j < NUM_CP_ATTRIBUTE_TYPES; j ++) {
    this->ResetAttributes(j);
  }
}

//-------------------------------------------------------------------------
const char *vtkMRMLAnnotationControlPointsNode::GetAttributeTypesEnumAsString(int val)
{
  if (val < vtkMRMLAnnotationNode::NUM_TEXT_ATTRIBUTE_TYPES) {
    return vtkMRMLAnnotationNode::GetAttributeTypesEnumAsString(val);
  }
  if (val == CP_SELECTED) return "ctrlPtsSelected";
  if (val == CP_VISIBLE) return "ctrlPtsVisible";
  return "(unknown)";
};


//-------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLAnnotationControlPointsNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLAnnotationControlPointsStorageNode"));
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsNode::SetControlPoint(int id, double newControl[3],int selectedFlag, int visibleFlag)
{
  // cout << "vtkMRMLAnnotationControlPointsNode::SetControlPoint: ID:  " << id << " CtrlPt: " <<  newControl[0] << " " <<  newControl[1] << " " <<  newControl[2] << endl;
  if (id < 0)
  {
    vtkErrorMacro("Invalid ID");
    return 0;
  }

  vtkPoints *points = this->GetPoints();
  // Create if not there
  if (!points)
    {
    this->ResetControlPoints();
    points = this->GetPoints();
    }
  else  if (points->GetNumberOfPoints() > id)
    {
    // check if is different to prevent recursive event loops
    double *pnt = points->GetPoint(id);
    if (pnt && fabs(pnt[0]-newControl[0]) < NUMERIC_ZERO &&
               fabs(pnt[1]-newControl[1]) < NUMERIC_ZERO &&
               fabs(pnt[2]-newControl[2]) < NUMERIC_ZERO)
      {
      return 1;
      }
    }

  assert(points);
  points->InsertPoint(id, newControl);
  // cout << "New ControlPoints Coord:" << this->GetControlPointCoordinates(id)[0] << " " << this->GetControlPointCoordinates(id)[1] << " " << this->GetControlPointCoordinates(id)[2] << endl;
  //vtkIndent blub;
  // points->PrintSelf(cout,blub);


  for (int j = NUM_TEXT_ATTRIBUTE_TYPES ; j < NUM_CP_ATTRIBUTE_TYPES; j ++)
    {
      this->SetAttributeSize(j,points->GetNumberOfPoints());
    }
  this->SetAnnotationAttribute(id, CP_SELECTED, selectedFlag);
  this->SetAnnotationAttribute(id, CP_VISIBLE, visibleFlag);

  //this->InvokeEvent(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent);
  this->StorableModifiedTime.Modified();
  this->Modified();

  return 1;
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsNode::SetControlPointWorldCoordinates(int id, double p[3])
{
  double worldPoint[] = {p[0],p[1],p[2],1};
  double localPoint[4];
  this->TransformPointFromWorld(worldPoint, localPoint);

  return this->SetControlPoint(id, localPoint);

}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsNode::SetControlPointWorldCoordinates(int id, double p[3], int selectedFlag, int visibleFlag)
{
  double worldPoint[] = {p[0],p[1],p[2],1};
  double localPoint[4];
  this->TransformPointFromWorld(worldPoint, localPoint);

  return this->SetControlPoint(id, localPoint, selectedFlag, visibleFlag);

}
//---------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsNode::SetControlPoint(int id, double newControl[3])
{
  // cout << "vtkMRMLAnnotationControlPointsNode::SetControlPoint: ID:  " << id << " CtrlPt: " <<  newControl[0] << " " <<  newControl[1] << " " <<  newControl[2] << endl;
  if (id < 0)
  {
    vtkErrorMacro("Invalid ID");
    return 0;
  }

  // Create if not there
  if (!this->GetPoints()) {
    this->ResetControlPoints();
  }

  int selectedFlag = this->GetAnnotationAttribute(id, CP_SELECTED);
  if (selectedFlag == -1)
    {
    // default value
    selectedFlag = 0;
    }
  int visibleFlag = this->GetAnnotationAttribute(id, CP_VISIBLE);
  if (visibleFlag == -1)
    {
    // default value
    visibleFlag = 1;
    }

  vtkPoints *points = this->GetPoints();
  points->InsertPoint(id, newControl);
  // cout << "New ControlPoints Coord:" << this->GetControlPointCoordinates(id)[0] << " " << this->GetControlPointCoordinates(id)[1] << " " << this->GetControlPointCoordinates(id)[2] << endl;
  //vtkIndent blub;
  // points->PrintSelf(cout,blub);


  for (int j = NUM_TEXT_ATTRIBUTE_TYPES ; j < NUM_CP_ATTRIBUTE_TYPES; j ++)
    {
      this->SetAttributeSize(j,points->GetNumberOfPoints());
    }
  this->SetAnnotationAttribute(id, CP_SELECTED, selectedFlag);
  this->SetAnnotationAttribute(id, CP_VISIBLE, visibleFlag);
  //this->InvokeEvent(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent);
  this->StorableModifiedTime.Modified();
  this->Modified();

  return 1;
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsNode::AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag)
{
  // Create if not there
  if (!this->GetPoints())
    {
    this->ResetControlPoints();
    }

  vtkPoints *points = this->GetPoints();
  int n = points->GetNumberOfPoints();
  if (this->SetControlPoint(n, newControl,selectedFlag, visibleFlag))
    {
      return n;
    }
  return -1;
}

//----------------------------------------------------------------------------
const char* vtkMRMLAnnotationControlPointsNode::GetNumberingSchemeAsString()
{
  return this->GetNumberingSchemeAsString(this->NumberingScheme);
}

//----------------------------------------------------------------------------
const char* vtkMRMLAnnotationControlPointsNode::GetNumberingSchemeAsString(int scheme)
{
  if (scheme == this->UseID)
    {
    return "UseID";
    }
  if (scheme == this->UseIndex)
    {
    return "UseIndex";
    }
  if (scheme == this->UsePrevious)
    {
    return "UsePrevious";
    }
  return "UNKNOWN";
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::SetNumberingSchemeFromString(const char *schemeString)
{
  if (!strcmp(schemeString, "UseID"))
    {
    this->SetNumberingScheme(this->UseID);
    }
  else if (!strcmp(schemeString, "UseIndex"))
    {
    this->SetNumberingScheme(this->UseIndex);
    }
  else if (!strcmp(schemeString, "UsePrevious"))
    {
    this->SetNumberingScheme(this->UsePrevious);
    }
  else
    {
    vtkErrorMacro("Invalid numbering scheme string: " << schemeString);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode
::SetNumberingScheme(int numberingScheme)
{
  if (numberingScheme == this->NumberingScheme)
    {
    return;
    }
  this->NumberingScheme = numberingScheme;
  this->StorableModifiedTime.Modified();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::Initialize(vtkMRMLScene* mrmlScene)
{
  if (!mrmlScene)
  {
    vtkErrorMacro("Scene was null!");
    return;
  }

  // we need to disable the modified event which would get fired when we set the new displayNode
  this->DisableModifiedEventOn();
  // at this point we need to have a scene, else the displayNodes can not be created
  this->SetScene(mrmlScene);
  this->CreateAnnotationPointDisplayNode();
  this->DisableModifiedEventOff();

  Superclass::Initialize(mrmlScene);
}
