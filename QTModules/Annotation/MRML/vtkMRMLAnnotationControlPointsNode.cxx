#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationControlPointsNode.h"
#include "vtkBitArray.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkDataSetAttributes.h"
#include "vtkPointData.h"
#include "vtkStringArray.h"
#include "vtkMRMLAnnotationControlPointsStorageNode.h"

// KPs Todos 
// - create specific event for node modification
// - talk to Steve if we have to do anything when UpdatingScene 
// - NumberingScheme should not be in annotation node - should be in fiducial nodes - just put it here right now 

//------------------------------------------------------------------------------
vtkMRMLAnnotationControlPointsNode* vtkMRMLAnnotationControlPointsNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationControlPointsNode");
  if(ret)
    {
    return (vtkMRMLAnnotationControlPointsNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationControlPointsNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationControlPointsNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationControlPointsNode");
  if(ret)
    {
    return (vtkMRMLAnnotationControlPointsNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationControlPointsNode;
}

vtkMRMLAnnotationControlPointsNode::vtkMRMLAnnotationControlPointsNode() 
{
  this->NumberingScheme = UseID; 
} 

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::WriteXML(ostream& of, int nIndent)
{
  // cout << "vtkMRMLAnnotationControlPointsNode::WriteXML start" << endl;
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);

  if (this->PolyData &&  this->PolyData->GetPoints()) 
    { 
      vtkPoints *points = this->PolyData->GetPoints(); 
      int n = points->GetNumberOfPoints();

      of << indent << " ctrlPtsCoord=\"";
      for (int i = 0; i < n; i++ ) 
    {
      double* ptr = points->GetPoint(i);
      of << ptr[0] << " "<<  ptr[1] << " "<<  ptr[2] ;
      if (i < n-1) 
        { 
          of << "|";
        }
    }
      of << "\"";
 

      for (int j = NUM_TEXT_ATTRIBUTE_TYPES ; j < NUM_CP_ATTRIBUTE_TYPES; j ++) 
    {
      of << indent << " " <<this->GetAttributeTypesEnumAsString(j)<<"=\"";
      for (int i = 0; i < n-1; i++ ) 
        {
          of << this->GetAnnotationAttribute(i,j) << " " ;
        }
      if (n) 
        {
          of << this->GetAnnotationAttribute(n-1,j);
        }
      of << "\"";
    }  
    }
  else 
    {
      of << indent << " ctrlPtsCoord=\"\"";
      for (int j = NUM_TEXT_ATTRIBUTE_TYPES ; j < NUM_CP_ATTRIBUTE_TYPES; j ++) 
    {
      of << indent << " " << this->GetAttributeTypesEnumAsString(j) << "=\"\"";
    }
    }

  of << indent << " ctrlPtsNumberingScheme=\"" << this->NumberingScheme << "\"";
  
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::WriteCLI(std::ostringstream& ss, std::string prefix)
{
  Superclass::WriteCLI(ss, prefix);

  if (this->PolyData &&  this->PolyData->GetPoints()) 
    { 
    vtkPoints *points = this->PolyData->GetPoints(); 
    int n = points->GetNumberOfPoints();
    
    for (int i = 0; i < n; i++ ) 
      {
      double* ptr = points->GetPoint(i);
      if (prefix.compare("") != 0)
        {
        ss << prefix << " ";
        }
      ss << ptr[0] << "," <<  ptr[1] << "," <<  ptr[2] ;
      if (i < n-1) 
        { 
          ss << " ";
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLAnnotationControlPointsNode::ReadXMLAttributes start"<< endl;

  int disabledModify = this->StartModify();

  // this->ResetAnnotations();
  Superclass::ReadXMLAttributes(atts);

  
  while (*atts != NULL) 
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

  if (this->PolyData &&  this->PolyData->GetPoints()) 
    { 
      os << indent << "ctrlPtsCoord: " ;
 
      vtkPoints *points = this->PolyData->GetPoints(); 
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
  vtkMRMLAnnotationPointDisplayNode *node = NULL;
  for (int n=0; n<nnodes; n++)
    {
      // vtkMRMLDisplayableNode stores the ids in DisplayNodeIDs 
    node = vtkMRMLAnnotationPointDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (node && node->IsA("vtkMRMLAnnotationPointDisplayNode")) 
      {
    return node;
      }
    }
  return 0;
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
  this->GetScene()->AddNode(node);
  node->Delete();
  // vtkMRMLDisplayableNode stores the ids in DisplayNodeIDs 
  this->AddAndObserveDisplayNodeID(node->GetID());
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
  vtkPoints *polyPoint = this->GetPolyData()->GetPoints();
  if (!polyPoint) 
    {
      polyPoint = vtkPoints::New();
      this->GetPolyData()->SetPoints(polyPoint);
      polyPoint->Delete();  
    }
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
  
  this->PolyData->GetPoints()->Reset();
  
  this->ResetControlPointsAttributesAll();
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsNode::GetNumberOfControlPoints()
{
  if (!this->PolyData || !this->PolyData->GetPoints()) {
    this->ResetControlPoints();
  }
  vtkPoints *points = this->PolyData->GetPoints(); 
 
  return points->GetNumberOfPoints();
}


//---------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::DeleteControlPoint(int id)
{
  if (!this->PolyData || !this->PolyData->GetPoints()) 
    {
      this->ResetControlPoints();
    }

  vtkPoints *points = this->PolyData->GetPoints();  
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
          vtkErrorMacro("Annotation " << this->GetName() << " Attribute " << this->GetAttributeTypesEnumAsString(j) << " is out of sync with PolyData->GetPoints()! Size of Attribute:" << dataArray->GetSize() << " Number of Points: " << n); 
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

  if (!this->PolyData || !this->PolyData->GetPoints()) 
    {
      return 0;
    }

  return this->PolyData->GetPoint(id);
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
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLAnnotationControlPointsStorageNode::New());
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

  // Create if not there
  if (!this->PolyData || !this->PolyData->GetPoints()) {
    this->ResetControlPoints();
  }

  vtkPoints *points = this->PolyData->GetPoints(); 
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
  this->InvokeEvent(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent);
 
  return 1;
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsNode::AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag)
{
  // Create if not there
  if (!this->PolyData || !this->PolyData->GetPoints()) {
    this->ResetControlPoints();
  }

  vtkPoints *points = this->PolyData->GetPoints(); 
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
  int changed = 1;
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
    changed = 0;
    }
  if (changed)
    {
    this->ModifiedSinceReadOn();
    }  
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsNode::Initialize(vtkMRMLScene* mrmlScene)
{
  if (!mrmlScene)
  {
    vtkErrorMacro("Scene was null!")
    return;
  }
  Superclass::Initialize(mrmlScene);
  // we need to disable the modified event which would get fired when we set the new displayNode
  this->DisableModifiedEventOn();
  this->CreateAnnotationPointDisplayNode();
  this->DisableModifiedEventOff();
}
