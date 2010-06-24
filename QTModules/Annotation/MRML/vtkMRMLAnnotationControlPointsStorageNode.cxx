#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h"
#include "vtkMRMLAnnotationControlPointsStorageNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationControlPointsNode.h"
#include "vtkStringArray.h"

// KP: Deal with TEXT - annotation label 

//------------------------------------------------------------------------------
vtkMRMLAnnotationControlPointsStorageNode* vtkMRMLAnnotationControlPointsStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationControlPointsStorageNode");
  if(ret)
    {
    return (vtkMRMLAnnotationControlPointsStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationControlPointsStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLAnnotationControlPointsStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationControlPointsStorageNode");
  if(ret)
    {
    return (vtkMRMLAnnotationControlPointsStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationControlPointsStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationControlPointsStorageNode::vtkMRMLAnnotationControlPointsStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationControlPointsStorageNode::~vtkMRMLAnnotationControlPointsStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsStorageNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLAnnotationControlPointsStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}
//----------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsStorageNode::ReadAnnotationPointDisplayProperties(vtkMRMLAnnotationPointDisplayNode *refNode, std::string lineString, std::string preposition)
{
  if (refNode == NULL)
    {
      vtkErrorMacro("ReadAnnotationPointDisplayProperties: unable to get associated AnnotationPointDisplayNode"); 
      return -1;
    }

  int flag = Superclass::ReadAnnotationDisplayProperties(refNode, lineString, preposition);
  if (flag)
    {
      return flag; 
    }

  int pointOffset = preposition.size();
  preposition.insert(0,"# ");

  if (lineString.find(preposition + "SymbolScale = ") != std::string::npos)
    {
     std::string str = lineString.substr(16 + pointOffset,std::string::npos);
     vtkDebugMacro("Getting symbolScale, substr = " << str);
     float scale = atof(str.c_str());
     refNode->SetSymbolScale(scale);
     return 1;
    }

  if (lineString.find(preposition + "SymbolType = ") != std::string::npos)
    {
      std::string str = lineString.substr(15 + pointOffset,std::string::npos);
      vtkDebugMacro("Getting symbolType, substr = " << str);
      int t = atoi(str.c_str());
      refNode->SetGlyphType(t);
      return 1;
    }


  return 0;
}

 
//----------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsStorageNode::ReadAnnotationControlPointsData(vtkMRMLAnnotationControlPointsNode *refNode, char line[1024],
                                   int typeColumn, int xColumn, int yColumn, int zColumn,  int selColumn,  int visColumn, int numColumns)
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
  double coord[3];
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
      
      if (columnNumber == xColumn)
        {
          coord[0] = atof(ptr);
        }
      else if (columnNumber == yColumn)
        {
          coord[1] = atof(ptr);
        }
      else if (columnNumber == zColumn)
        {
          coord[2] = atof(ptr);
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

  if (refNode->AddControlPoint(coord, sel, vis) < 0 ) 
    {
      vtkErrorMacro("Error adding control point to list, coord = " << coord[0] << " " << coord[1] << " " << coord[2]);
      return -1;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsStorageNode::ReadAnnotationControlPointsProperties(vtkMRMLAnnotationControlPointsNode *refNode, char line[1024], int &typeColumn, 
                                    int& xColumn,    int& yColumn,     int& zColumn, int& selColumn, int& visColumn, int& numColumns)
{
 if (line[0] != '#' || line[1] != ' ') 
    {
      return 0;
    }


  vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");
  // TODO: parse out the display node settings
  // if there's a space after the hash, try to find options
  std::string preposition = std::string("# ") + this->GetAnnotationStorageType();
  vtkIdType  pointOffset = std::string(this->GetAnnotationStorageType()).size();;
  
  vtkDebugMacro("Have a possible option in line " << line);
  std::string lineString = std::string(line);


 if (lineString.find(preposition + "NumberingScheme = ") != std::string::npos)
   {
     std::string str = lineString.substr(19 +  pointOffset, std::string::npos);
     vtkDebugMacro("Getting numberingScheme, substr = " << str.c_str());
     int val = atoi(str.c_str());
     refNode->SetNumberingScheme(val);
      return 1;
   }

  if (lineString.find(preposition + "Columns = ") != std::string::npos)
    {
      std::string str = lineString.substr(12 + pointOffset, std::string::npos);
      
      vtkDebugMacro("Getting column order for the fids, substr = " << str.c_str());
      // reset all of them
      typeColumn= xColumn = yColumn = zColumn = selColumn = visColumn = -1;
      numColumns = 0;
      char *columns = (char *)str.c_str();
      char *ptr = strtok(columns, "|");
      while (ptr != NULL)
    {
      if (strcmp(ptr, "type") == 0)
        {
          typeColumn = numColumns ;
        }
      else if (strcmp(ptr, "x") == 0)
        {
          xColumn =  numColumns;
        }
      else if (strcmp(ptr, "y") == 0)
        {
          yColumn =  numColumns;
        }
      else if (strcmp(ptr, "z") == 0)
        {
          zColumn =  numColumns;
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
      vtkDebugMacro("Got " << numColumns << " columns, type = " << typeColumn << ", x = " << xColumn << ",  y = " << yColumn << ",  z = " << zColumn << ", sel = " <<  selColumn << ", vis = " << visColumn);
      return 1;
    }
  return 0;
}


//----------------------------------------------------------------------------
// assumes that the node is already reset
int vtkMRMLAnnotationControlPointsStorageNode::ReadAnnotation(vtkMRMLAnnotationControlPointsNode *refNode)
{

  if (refNode == NULL)
    {
      vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a annotation node");
      return 0;
    }

  if (!vtkMRMLAnnotationStorageNode::ReadAnnotation(refNode)) 
    {
      return 0;
    }

  // open the file for reading input
  fstream fstr;
  if (!this->OpenFileToRead(fstr, refNode))
    {
      return 0;
    }

  
 vtkMRMLAnnotationPointDisplayNode *aPointDisplayNode = refNode->GetAnnotationPointDisplayNode();

  
  // turn off modified events
  int modFlag = refNode->GetDisableModifiedEvent();
  refNode->DisableModifiedEventOn();
  char line[1024];
  // default column ordering for annotation info - this is exactly the same as for fiducial
  // first pass: line will have label,x,y,z,selected,visible
  int typePointColumn = 0;
  int xPointColumn = 1;
  int yPointColumn = 2;
  int zPointColumn = 3;
  int selPointColumn  = 4;
  int visPointColumn  = 5;
  int numPointColumns = 6;

  while (fstr.good())
    {
    fstr.getline(line, 1024);
    
    // does it start with a #?
        // Property
    if (line[0] == '#')
      {
        if (line[1] == ' ') 
          {
            if (!this->ReadAnnotationControlPointsProperties(refNode, line, typePointColumn, xPointColumn, yPointColumn,  zPointColumn, selPointColumn, visPointColumn, numPointColumns))
              {
            if (this->ReadAnnotationPointDisplayProperties(aPointDisplayNode, line,this->GetAnnotationStorageType()) < 0 )
              {
                return 0;
              }
              }
          }
      }
        else
          {
        if (this->ReadAnnotationControlPointsData(refNode, line, typePointColumn, xPointColumn, yPointColumn, zPointColumn, selPointColumn,  
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
int vtkMRMLAnnotationControlPointsStorageNode::ReadData(vtkMRMLNode *refNode)
{
  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !this->GetAddToScene() || !refNode->GetAddToScene() )
    {
      return 1;
    }

  // cast the input node
  vtkMRMLAnnotationControlPointsNode *aCPNode = dynamic_cast <vtkMRMLAnnotationControlPointsNode *> (refNode);

  if (aCPNode == NULL)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a annotation control point node");
    return 0;
    }

  // clear out the list
  aCPNode->ResetAnnotations();

  if (!this->ReadAnnotation(aCPNode))
    {
      return 0;
    }

  this->SetReadStateIdle();
  
  // make sure that the list node points to this storage node
  aCPNode->SetAndObserveStorageNodeID(this->GetID());
  
  // mark it unmodified since read
  aCPNode->ModifiedSinceReadOff();

  aCPNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, aCPNode);//vtkMRMLAnnotationNode::DisplayModifiedEvent);

  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsStorageNode::WriteAnnotationPointDisplayProperties(fstream& of, vtkMRMLAnnotationPointDisplayNode *refNode, std::string preposition)
{
  if (!refNode)
   {
     return;
   }
  WriteAnnotationDisplayProperties(of,refNode, preposition);

  preposition.insert(0,"# ");
  of << preposition + "SymbolScale = " << refNode->GetSymbolScale() << endl;
  of << preposition + "SymbolType = " << refNode->GetGlyphType() << endl;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsStorageNode::WriteAnnotationControlPointsProperties(fstream& of, vtkMRMLAnnotationControlPointsNode *refNode)
{
   // put down a header
  if (refNode == NULL)
    {
      return 0;
    }

  vtkMRMLAnnotationPointDisplayNode *annPointDisNode = refNode->GetAnnotationPointDisplayNode();
  if (annPointDisNode == NULL)
    {
      return 0;
    }


  of << "# " << this->GetAnnotationStorageType() << "NumberingScheme = " << refNode->GetNumberingScheme() << endl;
  this->WriteAnnotationPointDisplayProperties(of, annPointDisNode, this->GetAnnotationStorageType());
  of << "# " << this->GetAnnotationStorageType() << "Columns = type|x|y|z|sel|vis" << endl;

  return 1;

}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationControlPointsStorageNode::WriteAnnotationControlPointsData(fstream& of, vtkMRMLAnnotationControlPointsNode *refNode)
{
  if (!refNode)
    {
      return;
    }
  for (int i = 0; i < refNode->GetNumberOfControlPoints(); i++)
    {
      double *coord = refNode->GetControlPointCoordinates(i);
      int sel = refNode->GetAnnotationAttribute(i, vtkMRMLAnnotationControlPointsNode::CP_SELECTED);
      int vis = refNode->GetAnnotationAttribute(i, vtkMRMLAnnotationControlPointsNode::CP_VISIBLE);
      of << this->GetAnnotationStorageType() << "|" << coord[0] << "|" << coord[1] << "|" << coord[2] << "|" << sel << "|" << vis << endl;   
    }

}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsStorageNode::WriteData(vtkMRMLNode *refNode,  fstream &of)
{
  if (!Superclass::WriteData(refNode,of)) 
    {
      return 0;
    }

  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLAnnotationControlPointsNode") ) )
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLAnnotationControlPointsNode");
    return 0;         
    }

  // cast the input nod
  vtkMRMLAnnotationControlPointsNode *annCPNode = dynamic_cast <vtkMRMLAnnotationControlPointsNode *> (refNode);

  if (annCPNode == NULL)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known annotation control point node");
    return 0;
    }


  // Control Points Properties
  if (!WriteAnnotationControlPointsProperties(of, annCPNode))
    {
      return 0;
    }

  // Control Points
  WriteAnnotationControlPointsData(of, annCPNode);
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationControlPointsStorageNode::WriteData(vtkMRMLNode *refNode)
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
void vtkMRMLAnnotationControlPointsStorageNode::InitializeSupportedWriteFileTypes()
{
  Superclass::InitializeSupportedWriteFileTypes();
}
