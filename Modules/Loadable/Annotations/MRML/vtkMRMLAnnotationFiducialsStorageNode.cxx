// Annotation includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationFiducialsStorageNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"


// MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkSmartPointer.h"

// STd includes

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationFiducialsStorageNode);


//----------------------------------------------------------------------------
int vtkMRMLAnnotationFiducialsStorageNode::ReadAnnotationFiducialsData(vtkMRMLAnnotationFiducialNode *refNode, char line[1024], int labelColumn, int xColumn, int yColumn, int zColumn,  int selColumn,  int visColumn, int numColumns)
{
  if (!refNode)
    {
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
  const char * separatorString = "|";
  std::string label = std::string("");
  size_t  startPos = 0;
  size_t  endPos =attValue.find(separatorString,startPos);
  int columnNumber = 0;
  double coord[3];
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

      if (columnNumber == labelColumn)
        {
        // this isn't actually the label, it's the storage type
        label = tokenString;
        }
      else if (columnNumber == xColumn)
        {
        coord[0] = atof(tokenString.c_str());
        }
      else if (columnNumber == yColumn)
        {
        coord[1] = atof(tokenString.c_str());
        }
      else if (columnNumber == zColumn)
        {
        coord[2] = atof(tokenString.c_str());
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
    endPos =attValue.find(separatorString,startPos);
    columnNumber ++;
    }

  // Add Fiducials


  // refNode->SetFiducialLabel(label.c_str());
  if (refNode->SetFiducial(coord, sel, vis) < 0)
    {
    vtkErrorMacro("Error adding control point to list, coord = " << coord[0] << " " << coord[1] << " " << coord[2]);
    return -1;
    }
  //newNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, newCPDisplayNode);//vtkMRMLAnnotationNode::DisplayModifiedEvent);

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationFiducialsStorageNode::ReadAnnotationFiducialsProperties(vtkMRMLAnnotationFiducialNode *refNode, char line[1024], int &labelColumn,
                                    int& xColumn,    int& yColumn,     int& zColumn, int& selColumn, int& visColumn, int& numColumns)
{
 if (line[0] != '#' || line[1] != ' ')
    {
    return 0;
    }


  vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");
  // TODO: parse out the display node settings
  // if there's a space after the hash, try to find options
  std::string preposition = std::string("# ");
  vtkIdType  pointOffset = std::string(this->GetAnnotationStorageType()).size();;

  vtkDebugMacro("Have a possible option in line " << line);
  std::string lineString = std::string(line);

  const char * separatorString = "|";

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
    labelColumn= xColumn = yColumn = zColumn = selColumn = visColumn = -1;
    numColumns = 0;
    char *columns = (char *)str.c_str();
    char *ptr = strtok(columns, separatorString);
    while (ptr != nullptr)
      {
      if (strcmp(ptr, "label") == 0)
        {
        labelColumn = numColumns ;
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
      ptr = strtok(nullptr, separatorString);
      numColumns++;
      }
    // set the total number of columns
    vtkDebugMacro("Got " << numColumns << " columns, label = " << labelColumn << ", x = " << xColumn << ",  y = " << yColumn << ",  z = " << zColumn << ", sel = " <<  selColumn << ", vis = " << visColumn);
    return 1;
    }
  return 0;
}


//----------------------------------------------------------------------------
// assumes that the node is already reset
int vtkMRMLAnnotationFiducialsStorageNode::ReadAnnotation(vtkMRMLAnnotationFiducialNode *refNode)
{

  if (refNode == nullptr)
    {
    vtkErrorMacro("ReadAnnotation: unable to read into a null annotation fiducial node node");
    return 0;
    }

  // open the file for reading input
  fstream fstr;
  if (!this->OpenFileToRead(fstr, refNode))
    {
    return 0;
    }

  refNode->CreateAnnotationTextDisplayNode();
  refNode->CreateAnnotationPointDisplayNode();

  // turn off modified events
  int modFlag = refNode->GetDisableModifiedEvent();
  refNode->DisableModifiedEventOn();
  char line[1024];
  // default column ordering for text annotation info
  int typeColumn = 0;
  int annotationColumn = 1;
  int selColumn  = 2;
  int visColumn  = 3;
  int numColumns = 4;

  // default column ordering for point annotation info - this is exactly the same as for fiducial
  // first pass: line will have label,x,y,z,selected,visible
  int labelColumn = 0;
  int xPointColumn = 1;
  int yPointColumn = 2;
  int zPointColumn = 3;
  int selPointColumn  = 4;
  int visPointColumn  = 5;
  int numPointColumns = 6;

  // go line by line through the file
  while (fstr.good())
    {
    fstr.getline(line, 1024);
    vtkDebugMacro("ReadAnnotations: working on line: '" << line << "'");
    if (line[0] == '\0')
      {
      // is empty
      continue;
      }
    // does it start with a # ?
        // Property
    if (line[0] == '#')
      {
      if (line[1] == ' ')
        {
        // it could be defining the text properties
        int retval =  this->ReadAnnotationTextProperties(refNode, line, typeColumn, annotationColumn, selColumn, visColumn, numColumns);
        if (retval <= 0)
          {
          // it could be defining the fiducial properties
          if (!this->ReadAnnotationFiducialsProperties(refNode, line, labelColumn, xPointColumn, yPointColumn,  zPointColumn, selPointColumn, visPointColumn, numPointColumns))
            {
            // or it could be defining the point display properties
            if (this->ReadAnnotationPointDisplayProperties(refNode->GetAnnotationPointDisplayNode(), line, "point") < 0 )
              {
              vtkWarningMacro("ReadAnnotation: have a # line that can't parse:\n'" << line << "'");
              // skip, may be the file name/header line
              }
            else { vtkDebugMacro("ReadAnnotation: read point disp properties: '" << line << "'" ); }
            }
           else { vtkDebugMacro("ReadAnnotation: read fids properties: '" << line << "'" ); }
          }
        else { vtkDebugMacro("ReadAnnotation: read annot text properties returned " << retval << " for line: '" << line << "'" ); }
        }
      }
    else
      {
      // could be text data
      if (!this->ReadAnnotationTextData(refNode, line, typeColumn, annotationColumn,  selColumn,  visColumn, numColumns))
        {
        // could be point data
        if (this->ReadAnnotationFiducialsData(refNode, line, labelColumn, xPointColumn, yPointColumn, zPointColumn, selPointColumn,
                                              visPointColumn, numPointColumns) < 0 )
          {
          vtkDebugMacro("ReadAnnotation: read annotation text data and fids data both failed on line: '" << line << "'" );
          }
        else
          {
          vtkDebugMacro("ReadAnnotation: read annotation fid data succeeded on line: '" << line << "'" );
          }
        }
      else
        {
        vtkDebugMacro("ReadAnnotation: read annotation text data succeeded on line: '" << line << "'" );
        }
      }
    }
    refNode->SetDisableModifiedEvent(modFlag);

    fstr.close();

    vtkDebugMacro("ReadAnnotation: done" );

    return 1;

}

//----------------------------------------------------------------------------
bool vtkMRMLAnnotationFiducialsStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLAnnotationFiducialNode");
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationFiducialsStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLAnnotationFiducialNode *fiducialNode =
    vtkMRMLAnnotationFiducialNode::SafeDownCast(refNode);

  int res = this->ReadAnnotation(fiducialNode);

  return res;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationFiducialsStorageNode::ReadOneFiducial(fstream & fstr, vtkMRMLAnnotationFiducialNode *refNode)
{
  if (refNode == nullptr)
    {
    vtkErrorMacro("ReadOneFiducial: can't read into a null node");
    return 0;
    }

  // do not read if if we are not in the scene (for example inside snapshot)
  if (!this->GetAddToScene())
    {
    return 1;
    }

  if (!fstr.is_open())
    {
    vtkErrorMacro("ReadOneFiducial: file isn't open");
    return 0;
    }
  refNode->CreateAnnotationTextDisplayNode();
  refNode->CreateAnnotationPointDisplayNode();

  // turn off modified events
  int modFlag = refNode->GetDisableModifiedEvent();
  refNode->DisableModifiedEventOn();
  char line[1024];
  // default column ordering for text annotation info
  int typeColumn = 0;
  int annotationColumn = 1;
  int selColumn  = 2;
  int visColumn  = 3;
  int numColumns = 4;

  // default column ordering for point annotation info - this is exactly the same as for fiducial
  // first pass: line will have label,x,y,z,selected,visible
  int labelColumn = 0;
  int xPointColumn = 1;
  int yPointColumn = 2;
  int zPointColumn = 3;
  int selPointColumn  = 4;
  int visPointColumn  = 5;
  int numPointColumns = 6;

  // go line by line through the file
  bool doneOne = false;

  while (fstr.good() && !doneOne)
    {
    fstr.getline(line, 1024);
    vtkDebugMacro("ReadOneFiducials: working on line: '" << line << "'");
    if (line[0] == '\0')
      {
      // is empty
      continue;
      }
    // does it start with a # ?
        // Property
    if (line[0] == '#')
      {
      if (line[1] == ' ')
        {
        // it could be defining the text properties
        int retval =  this->ReadAnnotationTextProperties(refNode, line, typeColumn, annotationColumn, selColumn, visColumn, numColumns);
        if (retval <= 0)
          {
          // it could be defining the fiducial properties
          if (!this->ReadAnnotationFiducialsProperties(refNode, line, labelColumn, xPointColumn, yPointColumn,  zPointColumn, selPointColumn, visPointColumn, numPointColumns))
            {
            // or it could be defining the point display properties
            if (this->ReadAnnotationPointDisplayProperties(refNode->GetAnnotationPointDisplayNode(), line, "point") < 0 )
              {
              vtkWarningMacro("ReadOneFiducial: have a # line that can't parse:\n'" << line << "'");
              // skip, may be the file name/header line
              }
            else { vtkDebugMacro("ReadOneFiducial: read point disp properties: '" << line << "'" ); }
            }
           else { vtkDebugMacro("ReadOneFiducial: read fids properties: '" << line << "'" ); }
          }
        else { vtkDebugMacro("ReadOneFiducial: read annot text properties returned " << retval << " for line: '" << line << "'" ); }
        }
      }
    else
      {
      // could be text data
      if (!this->ReadAnnotationTextData(refNode, line, typeColumn, annotationColumn,  selColumn,  visColumn, numColumns))
        {
        // could be point data
        if (this->ReadAnnotationFiducialsData(refNode, line, labelColumn, xPointColumn, yPointColumn, zPointColumn, selPointColumn,
                                              visPointColumn, numPointColumns) < 0 )
          {
          vtkDebugMacro("ReadOneFiducial: read annotation text data and fids data both failed on line: '" << line << "'" );
          }
        else
          {
          vtkDebugMacro("ReadOneFiducial: read annotation fid data succeeded on line: '" << line << "'" );
          // have read one point, this should be the last thing, so say we're done
          doneOne = true;
          }
        }
      else
        {
        vtkDebugMacro("ReadOneFiducial: read annotation text data succeeded on line: '" << line << "'" );
        }
      }
    }

  refNode->SetDisableModifiedEvent(modFlag);

  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationFiducialsStorageNode::InitializeSupportedWriteFileTypes()
{
  this->Superclass::InitializeSupportedWriteFileTypes();
  this->SupportedWriteFileTypes->InsertNextValue("Fiducial List CSV (.fcsv)");
}
