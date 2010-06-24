#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h"
#include "vtkMRMLAnnotationAngleStorageNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkStringArray.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationAngleStorageNode* vtkMRMLAnnotationAngleStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationAngleStorageNode");
  if(ret)
    {
    return (vtkMRMLAnnotationAngleStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationAngleStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLAnnotationAngleStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationAngleStorageNode");
  if(ret)
    {
    return (vtkMRMLAnnotationAngleStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationAngleStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationAngleStorageNode::vtkMRMLAnnotationAngleStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationAngleStorageNode::~vtkMRMLAnnotationAngleStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationAngleStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationAngleStorageNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLAnnotationAngleStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationAngleStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationAngleStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

 //----------------------------------------------------------------------------
int vtkMRMLAnnotationAngleStorageNode::ReadAnnotationAngleData(vtkMRMLAnnotationAngleNode *refNode, char line[1024],
                                   int typeColumn, int line1IDColumn, int line2IDColumn, int selColumn,  int visColumn, int numColumns)
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
  vtkIdType lineID[2] = {-1, -1};
  while (startPos != std::string::npos && (columnNumber < numColumns)) 
    {
      if (startPos != endPos) 
    {
      const char* ptr;
      if (endPos == std::string::npos) 
        {
          ptr = attValue.substr(startPos,endPos).c_str();
        }
      else
        {
          ptr = attValue.substr(startPos,endPos-startPos).c_str(); 
        }
      
      if (columnNumber == line1IDColumn)
        {
          lineID[0] = atoi(ptr);
        }
      else if (columnNumber == line2IDColumn)
        {
          lineID[1] = atoi(ptr);
        }
     else if (columnNumber == selColumn)
        {
          sel = atoi(ptr);
        }
      else if (columnNumber == visColumn)
        {
          vis = atoi(ptr);
        }
    }
      startPos = endPos +1;
      endPos =attValue.find("|",startPos);
      columnNumber ++;
    }

  if (refNode->SetAngle(lineID[0],lineID[1], sel, vis) < 0 ) 
    {
      vtkErrorMacro("Error setting angle , lineID = " << lineID[0] << " " << lineID[1]);
      return -1;
    }
  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLAnnotationAngleStorageNode::ReadAnnotationAngleProperties(vtkMRMLAnnotationAngleNode *refNode, char line[1024], int &typeColumn, 
                                     int& line1IDColumn, int& line2IDColumn, int& selColumn, int& visColumn, int& numColumns)
{

 if (line[0] != '#' || line[1] != ' ') 
    {
      return 0;
    }

 vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");
  // TODO: parse out the display node settings
  // if there's a space after the hash, try to find options

 std::string preposition = std::string("# ") + this->GetAnnotationStorageType();
 int pointOffset = preposition.size();

 vtkDebugMacro("Have a possible option in line " << line);
 std::string lineString = std::string(line);

 if (lineString.find(preposition + "Resolution = ") != std::string::npos)
   {
     std::string str = lineString.substr(13 + pointOffset,std::string::npos);
     vtkDebugMacro("Getting Resolution, substr = " << str);
     float size = atof(str.c_str());
     refNode->SetResolution(size);
     return 1;
   }
 else if (lineString.find(preposition + "LabelFormat = ") != std::string::npos)
   {
     std::string str = lineString.substr(14 + pointOffset,std::string::npos);
     vtkDebugMacro("Getting LabelFormat, substr = " << str);
     refNode->SetLabelFormat(str.c_str());
     return 1;
   } 
 else if (lineString.find(preposition + "Columns = ") != std::string::npos)
    {
      std::string str = lineString.substr(10 + pointOffset, std::string::npos);
      
      vtkDebugMacro("Getting column order for the fids, substr = " << str.c_str());
      // reset all of them
      typeColumn= line1IDColumn = line2IDColumn = selColumn = visColumn = -1;
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
      else if (strcmp(ptr, "line2ID") == 0)
        {
          line2IDColumn =  numColumns;
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
      vtkDebugMacro("Got " << numColumns << " columns, type = " << typeColumn << ", line1ID = " << line1IDColumn << ",  line2ID = " << line2IDColumn << ", sel = " <<  selColumn << ", vis = " << visColumn);
      return 1;
    }
  return 0;
}


//----------------------------------------------------------------------------
// assumes that the node is already reset
int vtkMRMLAnnotationAngleStorageNode::ReadAnnotation(vtkMRMLAnnotationAngleNode *refNode)
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
  int line2IDColumn = 2;
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
        this->ReadAnnotationAngleProperties(refNode, line, typePointColumn, line1IDColumn, line2IDColumn, selPointColumn, visPointColumn, numPointColumns); 
      }
        else
          {
        if (this->ReadAnnotationAngleData(refNode, line, typePointColumn, line1IDColumn, line2IDColumn, selPointColumn,  
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
int vtkMRMLAnnotationAngleStorageNode::ReadData(vtkMRMLNode *refNode)
{
  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !this->GetAddToScene() || !refNode->GetAddToScene() )
    {
      return 1;
    }

  // cast the input node
  vtkMRMLAnnotationAngleNode *aNode = dynamic_cast <vtkMRMLAnnotationAngleNode *> (refNode);

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
int vtkMRMLAnnotationAngleStorageNode::WriteAnnotationAngleProperties(fstream& of, vtkMRMLAnnotationAngleNode *refNode)
{
   // put down a header
  if (refNode == NULL)
    {
      return 0;
    }

  of << "# " << this->GetAnnotationStorageType() << "Resolution = " << refNode->GetResolution() << endl;
  of << "# " << this->GetAnnotationStorageType() << "LabelFormat = " << refNode->GetLabelFormat() << endl;
  of << "# " << this->GetAnnotationStorageType() << "Columns = type|line1ID|line2ID|sel|vis" << endl;

  return 1;

}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationAngleStorageNode::WriteAnnotationAngleData(fstream& of, vtkMRMLAnnotationAngleNode *refNode)
{
  if (!refNode)
    {
      return;
    }
  int sel = refNode->GetSelected();
  int vis = refNode->GetVisible();
  of << this->GetAnnotationStorageType() << "|" << 0 << "|" <<  1  << "|" << sel << "|" << vis << endl;   

}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationAngleStorageNode::WriteData(vtkMRMLNode *refNode)
{
  // open the file for writing
  fstream of;
  if (!this->OpenFileToWrite(of)) 
    {
      return 0;
    } 

  int flag = this->WriteData(refNode,of);

  of.close();

  Superclass::StageWriteData(refNode);
  
  return flag;
}


//----------------------------------------------------------------------------
int vtkMRMLAnnotationAngleStorageNode::WriteData(vtkMRMLNode *refNode, fstream& of)
{
  
  if (!Superclass::WriteData(refNode,of))
    {
      return 0;
    }

  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLAnnotationAngleNode") ) )
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLAnnotationAngleNode");
    return 0;         
    }


  // cast the input nod
  vtkMRMLAnnotationAngleNode *aNode = dynamic_cast <vtkMRMLAnnotationAngleNode *> (refNode);

  if (aNode == NULL)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known annotation line node");
    return 0;
    }

  // Control Points Properties
  if (!WriteAnnotationAngleProperties(of, aNode))
    {
      return 0;
    }

  WriteAnnotationAngleData(of, aNode);

  return 1;
}

