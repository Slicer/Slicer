
#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationLinesStorageNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationLinesNode.h"
#include "vtkMRMLScene.h"
#include "vtkStringArray.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationLinesStorageNode);

//----------------------------------------------------------------------------
vtkMRMLAnnotationLinesStorageNode::vtkMRMLAnnotationLinesStorageNode() = default;

//----------------------------------------------------------------------------
vtkMRMLAnnotationLinesStorageNode::~vtkMRMLAnnotationLinesStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLinesStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLinesStorageNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLAnnotationLinesStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLinesStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationLinesStorageNode::ReadAnnotationLineDisplayProperties(vtkMRMLAnnotationLineDisplayNode *refNode, std::string lineString, std::string preposition)
{
  if (refNode == nullptr)
    {
      vtkErrorMacro("ReadAnnotationLineDisplayProperties: unable to get associated AnnotationPointDisplayNode");
      return -1;
    }

  int flag = Superclass::ReadAnnotationDisplayProperties(refNode, lineString, preposition);
  if (flag)
    {
      return flag;
    }

  size_t pointOffset = preposition.size();
  preposition.insert(0,"# ");

  if (lineString.find(preposition + "LineThickness = ") != std::string::npos)
    {
     std::string str = lineString.substr(18 + pointOffset,std::string::npos);
     vtkDebugMacro("Getting LineThickness, substr = " << str);
     float size = atof(str.c_str());
     refNode->SetLineThickness(size);
     return 1;
    }

  return 0;
}


//----------------------------------------------------------------------------
int vtkMRMLAnnotationLinesStorageNode::ReadAnnotationLinesData(vtkMRMLAnnotationLinesNode *refNode, char line[1024],
                                   int typeColumn, int startIDColumn, int endIDColumn, int selColumn,  int visColumn, int numColumns)
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
  size_t size = std::string(this->GetAnnotationStorageType()).size();

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
  vtkIdType coordID[2] = {-1, -1};
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

      if (columnNumber == startIDColumn)
        {
        coordID[0] = atof(tokenString.c_str());
        }
      else if (columnNumber == endIDColumn)
        {
        coordID[1] = atof(tokenString.c_str());
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

  if (refNode->AddLine(coordID[0],coordID[1], sel, vis) < 0 )
    {
    vtkErrorMacro("Error adding list to list, coordID = " << coordID[0] << " " << coordID[1]);
    return -1;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationLinesStorageNode::ReadAnnotationLinesProperties(vtkMRMLAnnotationLinesNode *vtkNotUsed(refNode), char line[1024], int &typeColumn,
                                    int& startIDColumn,    int& endIDColumn, int& selColumn, int& visColumn, int& numColumns)
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


  if (lineString.find(preposition + "Columns = ") != std::string::npos)
    {
      std::string str = lineString.substr(12 + pointOffset, std::string::npos);

      vtkDebugMacro("Getting column order for the fids, substr = " << str.c_str());
      // reset all of them
      typeColumn= startIDColumn = endIDColumn = selColumn = visColumn = -1;
      numColumns = 0;
      char *columns = (char *)str.c_str();
      char *ptr = strtok(columns, "|");
      while (ptr != nullptr)
    {
      if (strcmp(ptr, "type") == 0)
        {
          typeColumn = numColumns ;
        }
      else if (strcmp(ptr, "startPointID") == 0)
        {
          startIDColumn =  numColumns;
        }
      else if (strcmp(ptr, "endPointID") == 0)
        {
          endIDColumn =  numColumns;
        }
      else if (strcmp(ptr, "sel") == 0)
        {
          selColumn =  numColumns;
        }
      else if (strcmp(ptr, "vis" ) == 0)
        {
          visColumn =  numColumns;
        }
      ptr = strtok(nullptr, "|");
      numColumns++;
    }
      // set the total number of columns
      vtkDebugMacro("Got " << numColumns << " columns, type = " << typeColumn << ", startPointID = " << startIDColumn << ",  endPointID = " << endIDColumn << ", sel = " <<  selColumn << ", vis = " << visColumn);
      return 1;
    }
  return 0;
}


//----------------------------------------------------------------------------
// assumes that the node is already reset
int vtkMRMLAnnotationLinesStorageNode::ReadAnnotation(vtkMRMLAnnotationLinesNode *refNode)
{

  if (refNode == nullptr)
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


 vtkMRMLAnnotationLineDisplayNode *aLineDisplayNode = refNode->GetAnnotationLineDisplayNode();


  // turn off modified events
  int modFlag = refNode->GetDisableModifiedEvent();
  refNode->DisableModifiedEventOn();
  char line[1024];
  // default column ordering for annotation info - this is exactly the same as for fiducial
  // first pass: line will have label,x,y,z,selected,visible
  int typePointColumn = 0;
  int startIDColumn = 1;
  int endIDColumn = 2;
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
            if (!this->ReadAnnotationLinesProperties(refNode, line, typePointColumn, startIDColumn, endIDColumn, selPointColumn, visPointColumn, numPointColumns))
              {
            if (this->ReadAnnotationLineDisplayProperties(aLineDisplayNode, line,this->GetAnnotationStorageType()) < 0 )
              {
                return 0;
              }
              }
          }
      }
        else
          {
        if (this->ReadAnnotationLinesData(refNode, line, typePointColumn, startIDColumn, endIDColumn, selPointColumn,
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
bool vtkMRMLAnnotationLinesStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLAnnotationLinesNode");
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationLinesStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  // cast the input node
  vtkMRMLAnnotationLinesNode *aNode =
    vtkMRMLAnnotationLinesNode::SafeDownCast(refNode);

  if (aNode == nullptr)
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

  aNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, aNode);//vtkMRMLAnnotationNode::DisplayModifiedEvent);

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationLinesStorageNode::WriteAnnotationLineDisplayProperties(fstream& of, vtkMRMLAnnotationLineDisplayNode *refNode, std::string preposition)
{
  if (!refNode)
   {
   return 0;
   }

  if (!this->WriteAnnotationDisplayProperties(of,refNode, preposition))
    {
    return 0;
    }

  preposition.insert(0,"# ");
  of << preposition + "LineThickness = " << refNode->GetLineThickness() << endl;

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationLinesStorageNode::WriteAnnotationLinesProperties(fstream& of, vtkMRMLAnnotationLinesNode *refNode)
{
   // put down a header
  if (refNode == nullptr)
    {
    vtkErrorMacro("WriteAnnotationLinesProperties: ref node is null");
    return 0;
    }

  vtkMRMLAnnotationLineDisplayNode *annDisNode = refNode->GetAnnotationLineDisplayNode();

  this->WriteAnnotationLineDisplayProperties(of, annDisNode, this->GetAnnotationStorageType());
  of << "# " << this->GetAnnotationStorageType() << "Columns = type|startPointID|endPointID|sel|vis" << endl;

  return 1;

}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationLinesStorageNode::WriteAnnotationLinesData(fstream& of, vtkMRMLAnnotationLinesNode *refNode)
{
  if (!refNode)
    {
    return 0;
    }
  for (int i = 0; i < refNode->GetNumberOfLines(); i++)
    {
      vtkIdType pointIDs[2];
      refNode->GetEndPointsId(i,pointIDs);
      int sel = refNode->GetAnnotationAttribute(i, vtkMRMLAnnotationLinesNode::LINE_SELECTED);
      int vis = refNode->GetAnnotationAttribute(i, vtkMRMLAnnotationLinesNode::LINE_VISIBLE);
      of << this->GetAnnotationStorageType() << "|" << pointIDs[0] << "|" <<  pointIDs[1]  << "|" << sel << "|" << vis << endl;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationLinesStorageNode::WriteAnnotationDataInternal(vtkMRMLNode *refNode, fstream& of)
{
  int retval = this->Superclass::WriteAnnotationDataInternal(refNode,of);
  if (!retval)
    {
    vtkWarningMacro("WriteAnnotationDataInternal: with stream: can't call WriteAnnotationDataInternal on superclass, retval = " << retval);
    return 0;
    }

  // cast the input nod
  vtkMRMLAnnotationLinesNode *aNode = vtkMRMLAnnotationLinesNode::SafeDownCast(refNode);

  if (aNode == nullptr)
    {
    vtkErrorMacro("WriteAnnotationDataInternal: unable to cast input node " << refNode->GetID() << " to a known annotation line node");
    return 0;
    }

  // Control Points Properties
  if (!WriteAnnotationLinesProperties(of, aNode))
    {
    vtkWarningMacro("WriteAnnotationDataInternal with stream: error writing annotation lines properties");
    return 0;
    }

  if (!WriteAnnotationLinesData(of, aNode))
    {
    return 0;
    }

  vtkDebugMacro("WriteAnnotationDataInternal with stream: returning 1");
  return 1;
}

