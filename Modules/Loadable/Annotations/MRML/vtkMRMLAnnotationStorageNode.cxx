
#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationStorageNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationNode.h"
//#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLScene.h"
#include "vtkStringArray.h"


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationStorageNode);

//----------------------------------------------------------------------------
vtkMRMLAnnotationStorageNode::vtkMRMLAnnotationStorageNode()
{
  //this->Debug =1;
  this->DefaultWriteFileExtension = "acsv";
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationStorageNode::~vtkMRMLAnnotationStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLAnnotationStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::ReadAnnotationDisplayProperties(vtkMRMLAnnotationDisplayNode *annotationDisplayNode, std::string lineString, std::string preposition)
{
  if (annotationDisplayNode == nullptr)
    {
    vtkErrorMacro("ReadAnnotationDisplayProperties: null associated AnnotationDisplayNode");
    return -1;
    }

 size_t textOffset = preposition.size();
 preposition.insert(0,"# ");

 if (lineString.find(preposition + "Visibility = ") != std::string::npos)
   {
     std::string str = lineString.substr(15 + textOffset,std::string::npos);
     vtkDebugMacro("Getting visibility, substr = " << str);
     int vis = atoi(str.c_str());
     annotationDisplayNode->SetVisibility(vis);
     return 1;
   }

  if (lineString.find(preposition + "Color = ") != std::string::npos ||
      lineString.find(preposition + "SelectedColor = ") != std::string::npos)
   {
     std::string str;
     if (lineString.find(preposition + "Color = ") != std::string::npos)
       {
     str = lineString.substr(10 + textOffset, std::string::npos);
       }
     else
       {
     str = lineString.substr(18 + textOffset, std::string::npos);
       }
     vtkDebugMacro("Getting colors, substr = " << str.c_str());
     // the rgb values are separated by commas
     float r = 0.0, g = 0.0, b = 0.0;
     char *ptr;
     char *colours = (char *)(str.c_str());
     ptr = strtok(colours, ",");
     if (ptr != nullptr)
       {
     r = atof(ptr);
       }
     ptr = strtok(nullptr, ",");
     if (ptr != nullptr)
       {
     g = atof(ptr);
       }
     ptr = strtok(nullptr, ",");
     if (ptr != nullptr)
       {
     b = atof(ptr);
       }
     // now set the correct value
     if (lineString.find(preposition + "Color = ") != std::string::npos)
       {
     annotationDisplayNode->SetColor(r,g,b);
       }
     else
       {
     annotationDisplayNode->SetSelectedColor(r,g,b);
       }
     return 1;
   }

 if (lineString.find(preposition + "Opacity = ") != std::string::npos)
   {
     std::string str = lineString.substr(12 + textOffset, std::string::npos);
     vtkDebugMacro("Getting opacity, substr = " << str.c_str());
     float val = atof(str.c_str());
     annotationDisplayNode->SetOpacity(val);
     return 1;
   }

 if (lineString.find(preposition + "Ambient = ") != std::string::npos)
   {
     std::string str = lineString.substr(12 + textOffset, std::string::npos);
     vtkDebugMacro("Getting ambient, substr = " << str.c_str());
     float val = atof(str.c_str());
     annotationDisplayNode->SetAmbient(val);
     return 1;
   }
 if (lineString.find(preposition + "Diffuse = ") != std::string::npos)
   {
     std::string str = lineString.substr(12 + textOffset, std::string::npos);
     vtkDebugMacro("Getting diffuse, substr = " << str.c_str());
     float val = atof(str.c_str());
     annotationDisplayNode->SetDiffuse(val);
     return 1;
   }

 if (lineString.find(preposition + "Specular = ") != std::string::npos)
   {
     std::string str = lineString.substr(13 + textOffset, std::string::npos);
     vtkDebugMacro("Getting specular, substr = " << str.c_str());
     float val = atof(str.c_str());
     annotationDisplayNode->SetSpecular(val);
     return 1;
   }

 if (lineString.find(preposition + "Power = ") != std::string::npos)
   {
     std::string str = lineString.substr(10 + textOffset, std::string::npos);
     vtkDebugMacro("Getting power, substr = " << str.c_str());
     float val = atof(str.c_str());
     annotationDisplayNode->SetPower(val);
     return 1;
   }

 if (lineString.find(preposition + "Opacity = ") != std::string::npos)
   {
     std::string str = lineString.substr(12 + textOffset, std::string::npos);
     vtkDebugMacro("Getting opacity, substr = " << str.c_str());
     float val = atof(str.c_str());
     annotationDisplayNode->SetOpacity(val);
     return 1;
   }

 return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::ReadAnnotationTextDisplayProperties(vtkMRMLAnnotationTextDisplayNode *annotationDisplayNode, std::string lineString, std::string preposition)
{
  if (annotationDisplayNode == nullptr)
    {
      vtkErrorMacro("ReadAnnotationTextDisplayProperties: null associated AnnotationTextDisplayNode");
      return -1;
    }

  if (this->ReadAnnotationDisplayProperties(annotationDisplayNode, lineString, preposition))
    {
      return 1;
    }

 size_t textOffset = preposition.size();
 preposition.insert(0,"# ");


 if (lineString.find(preposition + "Scale = ") != std::string::npos)
   {
     std::string str = lineString.substr(10 + textOffset, std::string::npos);
     vtkDebugMacro("Getting textScale, substr = " << str.c_str());
     float scale = atof(str.c_str());
     annotationDisplayNode->SetTextScale(scale);
     return 1;
   }
 return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::OpenFileToRead(fstream& fstr, vtkMRMLNode *refNode)
{
  vtkDebugMacro("Reading Annotation data");
  // test whether refNode is a valid node to hold a text list
  if ( !refNode ||
       !( refNode->IsA("vtkMRMLAnnotationNode"))
     )
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLAnnotationNode");
    return 0;
    }

  if (this->GetFileName() == nullptr && this->GetURI() == nullptr)
    {
    vtkErrorMacro("OpenFileToRead: file name and uri not set");
    return 0;
    }

  Superclass::StageReadData(refNode);

  if ( this->GetReadState() != this->TransferDone )
    {
    // remote file download hasn't finished
    vtkWarningMacro("OpenFileToRead: Read state is pending, returning.");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();

  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLAnnotationStorageNode: File name not specified");
    return 0;
    }

  fstr.open(fullName.c_str(), fstream::in);

  if (!fstr.is_open())
    {
      vtkErrorMacro("OpenFileToRead: unable to open file " << fullName.c_str() << " for reading");
      return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::ReadAnnotationTextData(vtkMRMLAnnotationNode *refNode, char line[1024], int typeColumn, int annotationColumn,  int selColumn,  int visColumn, int numColumns)
{
  if (!refNode)
    {
    vtkErrorMacro("ReadAnnotationTextData: reference node is null.");
    return 0;
    }

  if (typeColumn)
    {
    vtkErrorMacro("Type column has to be zero but is" << typeColumn);
    return 0;
    }
  // is it empty?
  if (line[0] == '\0')
    {
    vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
    return 0;
    }

  vtkDebugMacro("ReadAnnotationTextData: got a line: \n\"" << line << "\"\n\tAnnotation Storage type = " << this->GetAnnotationStorageType());

  std::string attValue(line);
  size_t size = std::string(this->GetAnnotationStorageType()).size();

  if (!attValue.compare(0,size,this->GetAnnotationStorageType()))
    {
    int sel = 1, vis = 1;
    std::string annotation;

    // Jump over type
    size_t  startPos =attValue.find("|",0) +1;
    size_t  endPos =attValue.find("|",startPos);
    int columnNumber = 1;
    while (startPos != std::string::npos && (columnNumber < numColumns))
      {
      if (startPos != endPos)
        {
        vtkDebugMacro("ReadAnnotationTextData: columnNumber = " << columnNumber << ", numColumns = " << numColumns << ", annotation column = " << annotationColumn);
        std::string tokenString;
        if (endPos == std::string::npos)
          {
          tokenString = attValue.substr(startPos,endPos);
          }
        else
          {
          tokenString = attValue.substr(startPos,endPos-startPos);
          }
        if (columnNumber == annotationColumn)
          {
          annotation = tokenString;
          vtkDebugMacro("ReadAnnotationTextData: annotation = " << annotation.c_str());
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
    vtkDebugMacro("ReadAnnotationTextData: text string = " << annotation.c_str());
    if (refNode->AddText(annotation.c_str(), sel, vis) < 0 )
      {
      vtkErrorMacro("Error adding text to list, annotation = " << annotation);
      return 0;
      }
    else
      {
      return 1;
      }
    }
  return 0;
}
//----------------------------------------------------------------------------

int vtkMRMLAnnotationStorageNode::ReadAnnotationTextProperties(vtkMRMLAnnotationNode *refNode, char line[1024], int &typeColumn,
                               int& annotationColumn, int& selColumn, int& visColumn, int& numColumns)
{
  if (!refNode)
    {
    return 0;
    }
  if (line[0] != '#' || line[1] != ' ')
    {
      return 0;
    }

  int flag = this->ReadAnnotationTextDisplayProperties(refNode->GetAnnotationTextDisplayNode(), line,this->GetAnnotationStorageType());
  if (flag)
    {
    return flag;
    }

  vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");
  // TODO: parse out the display node settings
  // if there's a space after the hash, try to find options
  std::string preposition = std::string("# ") + this->GetAnnotationStorageType();
  vtkIdType  typeOffset = std::string(this->GetAnnotationStorageType()).size();;

  vtkDebugMacro("Have a possible option in line " << line);
  std::string lineString = std::string(line);
  if (lineString.find("# Name = ") != std::string::npos)
    {
      std::string str = lineString.substr(9,std::string::npos);
      vtkDebugMacro("Getting name, substr = " << str);
      refNode->SetName(str.c_str());
      return 1;
    }

  if (lineString.find(preposition + "Columns = ") != std::string::npos)
    {
      std::string str = lineString.substr(12 + typeOffset, std::string::npos);

      vtkDebugMacro("Getting column order for the fids, substr = " << str.c_str());
      // reset all of them
      typeColumn= annotationColumn = selColumn = visColumn = -1;
      numColumns = 0;
      char *columns = (char *)str.c_str();
      char *ptr = strtok(columns, "|");
      while (ptr != nullptr)
    {
      if (strcmp(ptr, "type") == 0)
        {
          typeColumn = numColumns ;
        }
      else if (strcmp(ptr, "annotation") == 0)
        {
          annotationColumn =  numColumns;
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
      vtkDebugMacro("Got " << numColumns << " columns, text = " << annotationColumn << ", sel = " <<  selColumn << ", vis = " << visColumn);
      return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
// assumes that ResetAnnotations is executed
int vtkMRMLAnnotationStorageNode::ReadAnnotation(vtkMRMLAnnotationNode *annotationNode)
{
  if (annotationNode == nullptr)
    {
      vtkErrorMacro("ReadAnnotation: input is NULL");
      return 0;
    }

  // open the file for reading input
  fstream fstr;
  if (!this->OpenFileToRead(fstr, annotationNode))
    {
      return 0;
    }

  // turn off modified events
  int modFlag = annotationNode->GetDisableModifiedEvent();
  annotationNode->DisableModifiedEventOn();
  char line[1024];
  // default column ordering for annotation info - this is exactly the same as for fiducial
  // first pass: line will have label,x,y,z,selected,visible
  int typeColumn = 0;
  int annotationColumn = 1;
  int selColumn  = 2;
  int visColumn  = 3;
  int numColumns = 4;

  while (fstr.good())
    {
    fstr.getline(line, 1024);

    // does it start with a #?
        // Property
    if (line[0] == '#')
      {
      if (line[1] == ' ')
        {
        if (this->ReadAnnotationTextProperties(annotationNode, line, typeColumn, annotationColumn, selColumn, visColumn, numColumns) < 0 )
          {
          annotationNode->SetDisableModifiedEvent(modFlag);
          return 0;
          }
        }
      }
    else
      {
      this->ReadAnnotationTextData(annotationNode, line, typeColumn, annotationColumn,  selColumn,  visColumn, numColumns);
      }
    }
  annotationNode->SetDisableModifiedEvent(modFlag);

  fstr.close();

  return 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLAnnotationStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLAnnotationNode");
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  /*
  // special case: if this annotation is in a hierarchy, the hierarchy took
  // care of reading it already
  vtkMRMLHierarchyNode *hnode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(refNode->GetScene(), refNode->GetID());
  if (hnode &&
      hnode->GetParentNodeID())
    {
    vtkWarningMacro("ReadData: refNode " << refNode->GetName() << " is in a hierarchy, " << hnode->GetName() << ", assuming that it read it in already");
    return 1;
    }
  */
  // cast the input node
  vtkMRMLAnnotationNode *annotationNode =
    vtkMRMLAnnotationNode::SafeDownCast (refNode);

  if (annotationNode == nullptr)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a annotation node");
    return 0;
    }

  // clear out the list
  annotationNode->ResetAnnotations();
  if (!this->ReadAnnotation(annotationNode))
    {
      return 0;
    }

  this->InvokeEvent(vtkMRMLScene::NodeAddedEvent, annotationNode);

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::WriteAnnotationDisplayProperties(fstream& of, vtkMRMLAnnotationDisplayNode *refNode, std::string preposition)
{
 if (!refNode)
   {
   return 0;
   }
 preposition.insert(0,"# ");
 of << preposition + "Visibility = " << refNode->GetVisibility() << endl;
 double *colour = refNode->GetColor();
 of << preposition + "Color = " << colour[0] << "," << colour[1] << "," << colour[2] << endl;
 colour = refNode->GetSelectedColor();
 of << preposition + "SelectedColor = " << colour[0] << "," << colour[1] << "," << colour[2] << endl;
 of << preposition + "Opacity = " << refNode->GetOpacity() << endl;
 of << preposition + "Ambient = " << refNode->GetAmbient() << endl;
 of << preposition + "Diffuse = " << refNode->GetDiffuse() << endl;
 of << preposition + "Specular = " << refNode->GetSpecular() << endl;
 of << preposition + "Power = " << refNode->GetPower() << endl;

 return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::WriteAnnotationTextDisplayProperties(fstream& of, vtkMRMLAnnotationTextDisplayNode *refNode, std::string preposition)
{
 if (!refNode)
   {
   vtkErrorMacro("WriteAnnotationTextDisplayProperties: null annotation text display node");
   return 0;
   }

 if (!this->WriteAnnotationDisplayProperties(of, refNode, preposition))
   {
   return 0;
   }

 preposition.insert(0,"# ");
 of << preposition + "Scale = " << refNode->GetTextScale() << endl;

 return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::WriteAnnotationTextProperties(fstream& of, vtkMRMLAnnotationNode *refNode)
{
  // put down a header
  if (refNode == nullptr)
    {
    vtkWarningMacro("WriteAnnotationTextProperties: reference node is null");
    return 0;
    }
  vtkMRMLAnnotationTextDisplayNode *annDisNode = refNode->GetAnnotationTextDisplayNode();
  if (annDisNode == nullptr)
    {
    vtkWarningMacro("WriteAnnotationTextProperties: annotation text display node is null");
    return 0;
    }

  of << "# Name = " << refNode->GetName() << endl;
  this->WriteAnnotationTextDisplayProperties(of, annDisNode,this->GetAnnotationStorageType());
  of << "# " << this->GetAnnotationStorageType() <<"Columns = type|annotation|sel|vis" << endl;

  return 1;
}

//--------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::WriteAnnotationData(fstream& of, vtkMRMLAnnotationNode *refNode)
{
  if (!refNode)
    {
    vtkWarningMacro("WriteAnnotationData: reference node is null");
    return 0;
    }
  // if change the ones being included, make sure to update the parsing in ReadData
  for (int i = 0; i < refNode->GetNumberOfTexts(); i++)
    {
    vtkStdString nodeText = refNode->GetText(i);
    vtkDebugMacro("WriteAnnotationData: nodeText " << i << " is " << nodeText.c_str());
    int sel = refNode->GetAnnotationAttribute(i, vtkMRMLAnnotationNode::TEXT_SELECTED);
    int vis = refNode->GetAnnotationAttribute(i, vtkMRMLAnnotationNode::TEXT_VISIBLE);
    of << this->GetAnnotationStorageType() << "|" << nodeText.c_str() << "|" << sel << "|" << vis << endl;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::OpenFileToWrite(fstream& of)
{
  if (this->GetFileName() == nullptr)
    {
    vtkErrorMacro("OpenFileToWrite: file name is not set");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("OpenFileToWrite: File name not specified");
    return 0;
    }

  of.open(fullName.c_str(), fstream::out);

  if (!of.is_open())
    {
    vtkErrorMacro("OpenFileToWrite: unable to open file " << fullName.c_str() << " for writing");
    return 0;
    }

  of << "# Annotation file " << (this->GetFileName() != nullptr ? this->GetFileName() : "null") << endl;


  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::WriteAnnotationDataInternal(vtkMRMLNode *refNode, fstream &of)
{
  vtkDebugMacro("vtkMRMLAnnotationStorageNode::WriteData");

  // cast the input node
  vtkMRMLAnnotationNode *annotationNode =
    vtkMRMLAnnotationNode::SafeDownCast(refNode);

  if (annotationNode == nullptr)
    {
    vtkErrorMacro("WriteAnnotationDataInternal: unable to cast input node " << refNode->GetID() << " to a known annotation node");
    return 0;
    }

  if (!this->WriteAnnotationTextProperties(of, annotationNode))
    {
    return 0;
    }
  if (!this->WriteAnnotationData(of, annotationNode))
    {
    return 0;
    }
  vtkDebugMacro("WriteAnnotationDataInternal: returning 1");
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  if (!refNode)
    {
    vtkWarningMacro("WriteDataInternal: reference node is null.");
    return 0;
    }

  // open the file for writing
  fstream of;
  if (!this->OpenFileToWrite(of))
    {
    vtkWarningMacro("WriteDataInternal: unable to open file to write");
    return 0;
    }

  int flag = this->WriteAnnotationDataInternal(refNode,of);

  of.close();

  vtkDebugMacro("WriteDataInternal: returning " << flag);
  return flag;
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Annotation List CSV (.acsv)");
  this->SupportedReadFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Annotation List CSV (.acsv)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}
