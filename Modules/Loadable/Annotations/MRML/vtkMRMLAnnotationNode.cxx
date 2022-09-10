// MRML includes
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationStorageNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkBitArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStringArray.h>

// STD includes
#include <sstream>

// KPs Todos
// - create specific event for node modification
// - talk to Steve if we have to do anything when UpdatingScene
// - NumberingScheme should not be in annotation node - should be in fiducial nodes - just put it here right now


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationNode);


//----------------------------------------------------------------------------
vtkMRMLAnnotationNode::vtkMRMLAnnotationNode()
{
  this->TextList = vtkStringArray::New();
  this->ReferenceNodeID = nullptr;
  this->Locked = 0;
  this->m_Backup = nullptr;

}

//----------------------------------------------------------------------------
vtkMRMLAnnotationNode::~vtkMRMLAnnotationNode()
{
  this->TextList->Delete();
  if (this->ReferenceNodeID)
    {
      delete [] this->ReferenceNodeID;
    }

  if (this->m_Backup)
    {
    this->m_Backup->Delete();
    this->m_Backup = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationNode::WriteXML(ostream& of, int nIndent)
{
  // cout << "vtkMRMLAnnotationNode::WriteXML(ostream& of, int nIndent) start" << endl;
  // vtkMRMLDisplayableNode::WriteXML(of,nIndent);
  Superclass::WriteXML(of,nIndent);

  of << " referenceNodeID=\"" << (this->ReferenceNodeID ? this->GetReferenceNodeID() : "None") << "\"";
  of << " locked=\"" << this->Locked << "\"";

  int textLength = this->TextList->GetNumberOfValues();
  of << " textList=\"";

  if (textLength)
    {
    for (int i = 0 ; i < textLength - 1; i++)
      {
      of << this->TextList->GetValue(i) << "|";
      }
    of <<  this->TextList->GetValue(textLength -1);
    }
  of << "\"";

  for (int j = 0 ; j < NUM_TEXT_ATTRIBUTE_TYPES; j ++)
    {
    of << " " << this->GetAttributeTypesEnumAsString(j) << "=\"";
    if (textLength && this->GetPolyData() && this->GetPolyData()->GetPointData())
      {
      for (int i = 0 ; i < textLength - 1; i++)
        {
        of << this->GetAnnotationAttribute(i,j) << " " ;
        }
      of << this->GetAnnotationAttribute(textLength - 1,j);
      }
    of << "\"";
    }
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLAnnotationNode::ReadXMLAttributes start"<< endl;

  int disabledModify = this->StartModify();
  this->ResetAnnotations();

  // vtkMRMLDisplayableNode::ReadXMLAttributes(atts);
  Superclass::ReadXMLAttributes(atts);

  while (*atts != nullptr)
    {
    const char* attName = *(atts++);
    std::string attValue(*(atts++));

    if (!strcmp(attName, "textList") && attValue.size())
      {
    std::string tmpStr;
    size_t  startPos = 0;
    size_t  endPos =attValue.find("|",startPos);
    while (endPos != std::string::npos) {
      if (endPos == startPos) this->AddText(nullptr,1,1);
      else {
        this->AddText(attValue.substr(startPos,endPos-startPos).c_str(),1,1);
      }
      startPos = endPos +1;
      endPos =attValue.find("|",startPos);
    }
      this->AddText(attValue.substr(startPos,endPos).c_str(),1,1);
      }
    else if (!strcmp(attName, "referenceNodeID"))
      {
      this->SetReferenceNodeID(attValue.c_str());
      }
    else if (!strcmp(attName, "locked"))
      {
      this->SetLocked(atof(attValue.c_str()));
      }
    else
      {
      int j = 0;
      while (j < NUM_TEXT_ATTRIBUTE_TYPES)
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
          j = NUM_TEXT_ATTRIBUTE_TYPES;
            }
          j++;
        }
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLAnnotationNode *node = vtkMRMLAnnotationNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  this->SetLocked(node->GetLocked());
  this->TextList->DeepCopy(node->TextList);

  if (node->GetPolyData())
    {
    // The copy in vtkMRMLDisplayableNode just copies the poly data pointer, so
    // when an annotation node is in a scene view, it's control
    // point coordinates are over written by current scene changes
    vtkPolyData *poly = vtkPolyData::New();
    poly->DeepCopy(node->GetPolyData());
    this->SetAndObservePolyData(poly);
    //poly->ReleaseData();
    poly->Delete();
    }
}


//-----------------------------------------------------------
void vtkMRMLAnnotationNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  // Nothing to do at this point  bc vtkMRMLAnnotationTextDisplayNode is subclass of vtkMRMLModelDisplayNode
  // => will be taken care of by vtkMRMLModelDisplayNode

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationNode::ProcessMRMLEvents( vtkObject *caller,
                                               unsigned long event,
                                               void *callData )
{
  /// Display properties are saved in file, mark data as dirty.
  if (vtkMRMLDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->StorableModifiedTime.Modified();
    }

  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  // Not necessary bc vtkMRMLAnnotationTextDisplayNode is subclass of vtkMRMLModelDisplayNode
  // => will be taken care of  in vtkMRMLModelNode
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << endl;
  this->PrintAnnotationInfo(os,indent,0);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationNode::PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag)
{
  if (titleFlag)
    {

    os <<indent << "vtkMRMLAnnotationNode: Annotation Summary";
    if (this->GetName())
      {
      os << " of " << this->GetName();
      }
    os << endl;
    }


  os << indent << "ReferenceNodeID: " << ( (this->ReferenceNodeID) ? this->ReferenceNodeID : "None" ) << "\n";
  os << indent << "Selected: " << this->Selected << "\n";
  os << indent << "Locked: " << this->Locked << "\n";
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

  for (int j = 0 ; j < NUM_TEXT_ATTRIBUTE_TYPES; j ++)
    {
    os << indent << this->GetAttributeTypesEnumAsString(j) <<": ";
    if (this->GetNumberOfTexts())
      {
      for (int i = 0; i <  this->GetNumberOfTexts(); i++ )
        {
        os << this->GetAnnotationAttribute(i,j) << " " ;
        }
      os << endl;
      }
    else
      {
      os << " None" << endl;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationNode::ResetAnnotations()
{
  if (!this->TextList)
    {
    this->TextList = vtkStringArray::New();
    }
  else
    {
    this->TextList->Initialize();
    }

  this->ResetTextAttributesAll();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationNode::CreatePolyData()
{
  if (!this->GetPolyData())
    {
      vtkPolyData *poly = vtkPolyData::New();
      this->SetAndObservePolyData(poly);
      // Releasing data for pipeline parallism.
      // Filters will know it is empty.
      poly->ReleaseData();
      poly->Delete();

      // This assumes I want to display the poly data , which I do not want to as it is displayed by widgets
      //if (this->GetAnnotationTextDisplayNode())
      //    {
      //      this->GetAnnotationTextDisplayNode()->SetPolyData(poly);
      //    }
    }

}

//---------------------------------------------------------------------------
vtkPoints* vtkMRMLAnnotationNode::GetPoints()
{
  return this->GetPolyData() ? this->GetPolyData()->GetPoints() : nullptr;
}

//---------------------------------------------------------------------------
vtkCellArray* vtkMRMLAnnotationNode::GetLines()
{
  return this->GetPolyData() ? this->GetPolyData()->GetLines() : nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationNode::ResetTextAttributesAll() {
  this->CreatePolyData();

  for (int j = 0 ; j < NUM_TEXT_ATTRIBUTE_TYPES; j ++) {
    this->ResetAttributes(j);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationNode::ResetAttributes(int id) {
  if (!this->GetPolyData() || !this->GetPolyData()->GetPointData())
    {
    vtkErrorMacro("Annotation: "<< this->GetName()
                  << " PolyData or  PolyData->GetPointData() is NULL" );
    return;
    }

  if ((id < 0 ))
    {
      vtkErrorMacro("Annotation: "<< this->GetName() << " ID is out of range");
      return;
    }


  vtkBitArray *attArray = dynamic_cast <  vtkBitArray *> (this->GetAnnotationAttributes(id));
  if (!attArray) {
    attArray =  vtkBitArray::New();
    attArray->SetName(this->GetAttributeTypesEnumAsString(id));
    this->GetPolyData()->GetPointData()->AddArray(attArray);
    attArray->Delete();
  }
  attArray->Initialize();
}



//---------------------------------------------------------------------------
vtkDataArray* vtkMRMLAnnotationNode::GetAnnotationAttributes(int att)
{
  if (!this->GetPolyData() || !this->GetPolyData()->GetPointData())
    {
      vtkErrorMacro("Annotation: " << this->GetName() << " PolyData or  PolyData->GetPointData() is NULL" );
      return nullptr;
    }

  return this->GetPolyData()->GetPointData()->GetScalars(this->GetAttributeTypesEnumAsString(att));
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationNode::GetAnnotationAttribute(vtkIdType id, int att)
{
  vtkBitArray *attArray = dynamic_cast <  vtkBitArray *> (this->GetAnnotationAttributes(att));
  if (attArray)
    {
      return attArray->GetValue(id);
    }
  vtkErrorMacro("Annotation: " << this->GetName() << " Attributes for " << att << " are not defined");
  return  -1;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationNode::SetAnnotationAttribute(vtkIdType id, int att, double value)
{
  vtkBitArray *attArray = dynamic_cast <  vtkBitArray *> (this->GetAnnotationAttributes(att));
  if (!attArray)
    {
      return;
    }
  if (id < 0 || id >= attArray->GetSize())
    {
      vtkErrorMacro("SetAnnotationAttribute: id is out of range: id " << id << " Size: " <<  attArray->GetSize());
      return ;
    }
  attArray->SetValue(id,value);
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationNode::DeleteAttribute(vtkIdType idEntry, vtkIdType idAtt)
{
  vtkBitArray *dataArray = dynamic_cast <vtkBitArray *> (this->GetAnnotationAttributes(idAtt));
  if (!dataArray)
    {
      vtkErrorMacro("Annotation " << this->GetName() << " Attribute " << idAtt << " does not exist");
      return 0;
    }
  int n = dataArray->GetSize();
  if (idEntry < 0 || idEntry >= n)
    {
      vtkErrorMacro("Annotation " << this->GetName() << " Annotation out of range");
      return 0;
    }

  for (int i = idEntry; i < n-1; i++ )
    {
      dataArray->SetValue(i,dataArray->GetValue(i+1));
    }
  // Every attribute has its own data array - that is why it works
  dataArray->Resize(n-1);

  return 1;
}



//---------------------------------------------------------------------------
void vtkMRMLAnnotationNode::SetText(int id, const char *newText,int selectedFlag, int visibleFlag)
{
  if (id < 0)
    {
    vtkErrorMacro("Invalid ID");
    return;
    }
  if (!this->TextList)
    {
    vtkErrorMacro("TextList is NULL");
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
       (this->TextList->GetValue(id) == newString) &&
       (this->GetAnnotationAttribute(id, TEXT_SELECTED) == selectedFlag) &&
       (this->GetAnnotationAttribute(id, TEXT_VISIBLE) == visibleFlag) ) )
    {
    return;
    }

  if (!this->GetPolyData() || !this->GetPolyData()->GetPointData())
    {
    this->ResetTextAttributesAll();
    }

  this->TextList->InsertValue(id,newString);

  for (int j = 0 ; j < NUM_TEXT_ATTRIBUTE_TYPES; j ++)
    {
    this->SetAttributeSize(j,this->GetNumberOfTexts());
    }
  this->SetAnnotationAttribute(id, TEXT_SELECTED, selectedFlag);
  this->SetAnnotationAttribute(id, TEXT_VISIBLE, visibleFlag);

  if(!this->GetDisableModifiedEvent())
  {
  // invoke a modified event
  this->InvokeEvent(vtkCommand::ModifiedEvent);
  }

}

void vtkMRMLAnnotationNode::SetAttributeSize(vtkIdType  id, vtkIdType n)
{
  vtkBitArray *dataArray = dynamic_cast <vtkBitArray *> (this->GetAnnotationAttributes(id));
  if (!dataArray)
    {
      this->ResetAttributes(id);
      dataArray = dynamic_cast <vtkBitArray *> (this->GetAnnotationAttributes(id));
    }
  dataArray->Resize(n);
}


//-------------------------------------------------------------------------
int vtkMRMLAnnotationNode::AddText(const char *newText,int selectedFlag, int visibleFlag)
{
  if (!this->TextList) {
    vtkErrorMacro("Annotation: For " << this->GetName() << " text is not defined");
    return -1 ;
  }
  int n = this->GetNumberOfTexts();
  this->SetText(n,newText,selectedFlag, visibleFlag);

  return n;
}

//-------------------------------------------------------------------------
vtkStdString  vtkMRMLAnnotationNode::GetText(int n)
{
  if ((this->GetNumberOfTexts() <= n) || n < 0 )
    {
      return vtkStdString();
    }
  return this->TextList->GetValue(n);
}

//-------------------------------------------------------------------------
int  vtkMRMLAnnotationNode::DeleteText(int id)
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

  for (int i = id; i < n-1; i++ )
    {
      this->TextList->SetValue(i,this->GetText(i+1));
    }

  this->TextList->Resize(n-1);

  if (!this->GetPolyData() || !this->GetPolyData()->GetPointData())
    {
      this->ResetTextAttributesAll();
      return 1;
    }

  for (int j = 0 ; j < NUM_TEXT_ATTRIBUTE_TYPES; j ++)
    {
      vtkBitArray *dataArray = dynamic_cast <vtkBitArray *> (this->GetAnnotationAttributes(j));
      if (!dataArray)
    {
      this->ResetAttributes(j);
      dataArray = dynamic_cast <vtkBitArray *> (this->GetAnnotationAttributes(j));
      dataArray->Resize(this->GetNumberOfTexts());
    }
      else
    {
      this->DeleteAttribute(id,j);
    }
    }
  return 1;
}


//-------------------------------------------------------------------------
int vtkMRMLAnnotationNode::GetNumberOfTexts() {
  if (!this->TextList)
    {
      return -1;
    }
  return this->TextList->GetNumberOfValues();
}


//-------------------------------------------------------------------------
const char *vtkMRMLAnnotationNode::GetAttributeTypesEnumAsString(int val)
{
  if (val == TEXT_SELECTED) return "textSelected";
  if (val == TEXT_VISIBLE) return "textVisible";
  return "(unknown)";
};

//---------------------------------------------------------------------------
std::string vtkMRMLAnnotationNode::GetDefaultStorageNodeClassName(const char* filename /* =nullptr */)
{
  return vtkMRMLStorableNode::GetDefaultStorageNodeClassName(filename);
}

//-------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLAnnotationNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLAnnotationStorageNode"));
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayNode* vtkMRMLAnnotationNode::GetAnnotationTextDisplayNode()
{
  int nnodes = this->GetNumberOfDisplayNodes();
  vtkMRMLAnnotationTextDisplayNode *node = nullptr;
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLAnnotationTextDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (node && node->IsA("vtkMRMLAnnotationTextDisplayNode"))
      {
    return node;
      }
    }
  return nullptr;
}


//---------------------------------------------------------------------------
void vtkMRMLAnnotationNode::CreateAnnotationTextDisplayNode()
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (node) return;
  if (!this->GetScene())
    {
      vtkErrorMacro("vtkMRMLAnnotationNode::CreateAnnotationTextDisplayNode Annotation: No scene defined" );
      return;
    }

  node = vtkMRMLAnnotationTextDisplayNode::New();
  node->SetScene(this->GetScene());
  this->GetScene()->AddNode(node);
  node->Delete();
  this->AddAndObserveDisplayNodeID(node->GetID());
  // This assumes I want to display the poly data , which I do not want to as it is displayed by widgets
  // node->SetPolyData(this->GetPolyData());
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationNode::SetTextScale(double textScale)
{
  this->GetAnnotationTextDisplayNode()->SetTextScale(textScale);
  this->InvokeEvent(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationNode::GetTextScale()
{
  return this->GetAnnotationTextDisplayNode()->GetTextScale();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationNode::SetLocked(int locked)
{
  if (this->Locked == locked)
    {
    return;
    }
  this->Locked = locked;
  if(!this->GetDisableModifiedEvent())
    {
    // invoke a lock modified event
    this->InvokeEvent(vtkMRMLAnnotationNode::LockModifiedEvent);
    }
  this->StorableModifiedTime.Modified();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationNode::Initialize(vtkMRMLScene* mrmlScene)
{
  if (!mrmlScene)
  {
    vtkErrorMacro("Scene was null!");
    return;
  }

  // we need to disable the modified event which would get fired when we set the new displayNode
  this->DisableModifiedEventOn();
  this->CreateAnnotationTextDisplayNode();
  this->DisableModifiedEventOff();

  // A node should only be added to the scene if it is not added already
  // (there is not check in mrmlScene, and it causes problems).
  if (!mrmlScene->IsNodePresent(this))
    {
    mrmlScene->AddNode(this);
    }
}
