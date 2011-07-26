
#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationRulerStorageNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLScene.h"
#include "vtkStringArray.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationRulerStorageNode* vtkMRMLAnnotationRulerStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationRulerStorageNode");
  if(ret)
    {
    return (vtkMRMLAnnotationRulerStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationRulerStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLAnnotationRulerStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationRulerStorageNode");
  if(ret)
    {
    return (vtkMRMLAnnotationRulerStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationRulerStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationRulerStorageNode::vtkMRMLAnnotationRulerStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationRulerStorageNode::~vtkMRMLAnnotationRulerStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerStorageNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLAnnotationRulerStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

 //----------------------------------------------------------------------------
int vtkMRMLAnnotationRulerStorageNode::ReadAnnotationRulerData(vtkMRMLAnnotationRulerNode *refNode, char line[1024],
                                   int typeColumn, int line1IDColumn, int selColumn,  int visColumn, int numColumns)
{
  if (!refNode)
    {
      return -1;
    }

  if (typeColumn)
    {
      vtkErrorMacro("Type column has to be zero !");
      return -1;
    }

  // is it empty?

  if (line[0] == '\0')
    {
      vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
      return 1;
    }

  vtkDebugMacro("got a line: \n\"" << line << "\""); 

  std::string attValue(line);
  int size = std::string(this->GetAnnotationStorageType()).size();
 
  if (attValue.compare(0,size,this->GetAnnotationStorageType()))
    {
      return 0;
    }

  int sel = 1, vis = 1;
  std::string annotation;
  
  // Jump over type 
  size_t  startPos =attValue.find("|",0) +1;
  size_t  endPos =attValue.find("|",startPos);
  int columnNumber = 1;
  vtkIdType lineID = -1;
  while (startPos != std::string::npos && (columnNumber < numColumns)) 
    {
    if (startPos != endPos) 
      {
      std::string tokenString;
      if (endPos == std::string::npos) 
        {
        tokenString = attValue.substr(startPos,endPos);
        }
      else
        {
        tokenString = attValue.substr(startPos,endPos-startPos);
        }
      
      if (columnNumber == line1IDColumn)
        {
        lineID = atoi(tokenString.c_str());
        }
     else if (columnNumber == selColumn)
        {
        sel = atoi(tokenString.c_str());
        }
      else if (columnNumber == visColumn)
        {
        vis = atoi(tokenString.c_str());
        }
      }
    startPos = endPos +1;
    endPos =attValue.find("|",startPos);
    columnNumber ++;
    }

  if (refNode->SetRuler(lineID, sel, vis) < 0 ) 
    {
    vtkErrorMacro("Error setting angle , lineID = " << lineID);
    return -1;
    }

  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLAnnotationRulerStorageNode::ReadAnnotationRulerProperties(vtkMRMLAnnotationRulerNode *refNode, char line[1024], int &typeColumn, 
                                     int& line1IDColumn, int& selColumn, int& visColumn, int& numColumns)
{
  // cout << "vtkMRMLAnnotationRulerStorageNode::ReadAnnotationRulerProperties " << line << endl;
 if (line[0] != '#' || line[1] != ' ') 
    {
      return 0;
    }
 vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");

 std::string preposition = std::string("# ") + this->GetAnnotationStorageType();
 int pointOffset = preposition.size();

 //this->DebugOn(); 
 vtkDebugMacro("Have a possible option in line " << line);
 std::string lineString = std::string(line);
 
 if (lineString.find(preposition + "Resolution = ") != std::string::npos)
   {
     std::string str = lineString.substr(13 + pointOffset,std::string::npos);
     vtkDebugMacro("Getting Resolution, substr = " << str);
     float size = atof(str.c_str());
     refNode->SetResolution(size);
     //this->DebugOff();

     return 1;
   }
 else if (lineString.find(preposition + "DistanceAnnotationFormat = ") != std::string::npos)
   {
     std::string str = lineString.substr(27 + pointOffset,std::string::npos);
     vtkDebugMacro("Getting DistanceAnnotationFormat, substr = " << str);
     refNode->SetDistanceAnnotationFormat(str.c_str());
     //this->DebugOff();
     return 1;
   } 
 else if (lineString.find(preposition + "Columns = ") != std::string::npos)
    {
      std::string str = lineString.substr(10 + pointOffset, std::string::npos);
      
      vtkDebugMacro("Getting column order for the fids, substr = " << str.c_str());
      // reset all of them
      typeColumn= line1IDColumn = selColumn = visColumn = -1;
      numColumns = 0;
      char *columns = (char *)str.c_str();
      char *ptr = strtok(columns, "|");
      while (ptr != NULL)
    {
      if (strcmp(ptr, "type") == 0)
        {
          typeColumn = numColumns ;
        }
      else if (strcmp(ptr, "line1ID") == 0)
        {
          line1IDColumn =  numColumns;
        }
      else if (strcmp(ptr, "sel") == 0)
        {
          selColumn =  numColumns;
        }
      else if (strcmp(ptr, "vis" ) == 0)
        {
          visColumn =  numColumns;
        }
      ptr = strtok(NULL, "|");
      numColumns++;
    }
      // set the total number of columns
      vtkDebugMacro("Got " << numColumns << " columns, type = " << typeColumn << ", line1ID = " << line1IDColumn << ", sel = " <<  selColumn << ", vis = " << visColumn);
      //this->DebugOff();
      return 1;
    }
 //this->DebugOff();
  return 0;
}


//----------------------------------------------------------------------------
// assumes that the node is already reset
int vtkMRMLAnnotationRulerStorageNode::ReadAnnotation(vtkMRMLAnnotationRulerNode *refNode)
{

  if (refNode == NULL)
    {
      vtkErrorMacro("ReadAnnotation: unable to cast input node " << refNode->GetID() << " to a annotation node");
      return 0;
    }

  if (!Superclass::ReadAnnotation(refNode)) 
    {
      return 0;
    }

  // open the file for reading input
  fstream fstr;
  if (!this->OpenFileToRead(fstr, refNode))
    {
      return 0;
    }

  // turn off modified events
  int modFlag = refNode->GetDisableModifiedEvent();
  refNode->DisableModifiedEventOn();
  char line[1024];
  // default column ordering for annotation info - this is exactly the same as for fiducial
  // first pass: line will have label,x,y,z,selected,visible
  int typePointColumn = 0;
  int line1IDColumn = 1;
  int selPointColumn  = 4;
  int visPointColumn  = 5;
  int numPointColumns = 6;

  while (fstr.good())
    {
    fstr.getline(line, 1024);
    
    // does it start with a #?
        // Property
    if ((line[0] == '#') && (line[1] == ' ')) 
      {
        this->ReadAnnotationRulerProperties(refNode, line, typePointColumn, line1IDColumn, selPointColumn, visPointColumn, numPointColumns); 
      }
        else
          {
        if (this->ReadAnnotationRulerData(refNode, line, typePointColumn, line1IDColumn, selPointColumn,  
                          visPointColumn, numPointColumns) < 0 ) 
          {
        return 0;
          }
      }
    }   
    refNode->SetDisableModifiedEvent(modFlag);

    fstr.close();

    return 1;

}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationRulerStorageNode::ReadData(vtkMRMLNode *refNode)
{
  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !this->GetAddToScene() || !refNode->GetAddToScene() )
    {
      return 1;
    }

  // cast the input node
  vtkMRMLAnnotationRulerNode *aNode = dynamic_cast <vtkMRMLAnnotationRulerNode *> (refNode);

  if (aNode == NULL)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a annotation control point node");
    return 0;
    }

  // clear out the list
  aNode->ResetAnnotations();

  if (!this->ReadAnnotation(aNode))
    {
      return 0;
    }

  this->SetReadStateIdle();
  
  // make sure that the list node points to this storage node
  aNode->SetAndObserveStorageNodeID(this->GetID());
  
  // mark it unmodified since read
  aNode->ModifiedSinceReadOff();

  aNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, aNode);//vtkMRMLAnnotationNode::DisplayModifiedEvent);

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationRulerStorageNode::WriteAnnotationRulerProperties(fstream& of, vtkMRMLAnnotationRulerNode *refNode)
{
   // put down a header
  if (refNode == NULL)
    {
    vtkWarningMacro("WriteAnnotationRulerProperties: ref node is null");
    return 0;
    }

  of << "# " << this->GetAnnotationStorageType() << "Resolution = " << refNode->GetResolution() << endl;
  of << "# " << this->GetAnnotationStorageType() << "DistanceAnnotationFormat = " << refNode->GetDistanceAnnotationFormat() << endl;
  of << "# " << this->GetAnnotationStorageType() << "Columns = type|line1ID|sel|vis" << endl;

  return 1;

}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerStorageNode::WriteAnnotationRulerData(fstream& of, vtkMRMLAnnotationRulerNode *refNode)
{
  if (!refNode)
    {
    vtkWarningMacro("WriteAnnotationRulerData: reference node is null");
    return;
    }
  int sel = refNode->GetSelected();
  int vis = refNode->GetVisible(); 
  of << this->GetAnnotationStorageType() << "|" << 0  << "|" << sel << "|" << vis << endl;   

}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationRulerStorageNode::WriteData(vtkMRMLNode *refNode)
{
  // open the file for writing
  fstream of;
  if (!this->OpenFileToWrite(of)) 
    {
    vtkWarningMacro("WriteData: cannot open file to write");
    return 0;
    } 

  int flag = this->WriteData(refNode,of);

  of.close();

  Superclass::StageWriteData(refNode);

  vtkDebugMacro("RulerStorageNode: WriteData: returning " << flag);
  return flag;
}


//----------------------------------------------------------------------------
int vtkMRMLAnnotationRulerStorageNode::WriteData(vtkMRMLNode *refNode, fstream& of)
{

  int retval = Superclass::WriteData(refNode,of);
  if (!retval)
    {
    vtkWarningMacro("Ruler: WriteData: unable to call superclass WriteData, retval = " << retval);
    return 0;
    }

  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLAnnotationRulerNode") ) )
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLAnnotationRulerNode");
    return 0;         
    }


  // cast the input nod
  vtkMRMLAnnotationRulerNode *aNode = dynamic_cast <vtkMRMLAnnotationRulerNode *> (refNode);

  if (aNode == NULL)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known annotation line node");
    return 0;
    }

  // Control Points Properties
  if (!WriteAnnotationRulerProperties(of, aNode))
    {
    vtkWarningMacro("Ruler: WriteData: failure in WriteAnnotationRulerProperties");
    return 0;
    }

  WriteAnnotationRulerData(of, aNode);

  return 1;
}

