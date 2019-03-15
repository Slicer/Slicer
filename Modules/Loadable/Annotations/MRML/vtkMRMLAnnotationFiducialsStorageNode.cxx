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
// int vtkMRMLAnnotationFiducialsStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
// {
//   std::string fullName = this->GetFullNameFromFileName();
//
//   if (fullName.empty())
//     {
//     vtkErrorMacro("vtkMRMLAnnotationFiducialsStorageNode: File name not specified");
//     return 0;
//     }
//
//   // cast the input node
//   vtkMRMLAnnotationFiducialsNode *annotationNode = nullptr;
//   if ( refNode->IsA("vtkMRMLAnnotationFiducialsNode") )
//     {
//     annotationNode = dynamic_cast <vtkMRMLAnnotationFiducialsNode *> (refNode);
//     }
//
//   if (annotationNode == nullptr)
//     {
//     vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a annotation node");
//     return 0;
//     }
//
//
//   // cast the input node
//   vtkMRMLAnnotationTextDisplayNode *annotationDisplayNode = annotationNode->GetAnnotationTextDisplayNode();
//   if (annotationDisplayNode == nullptr)
//     {
//       vtkErrorMacro("ReadData: unable to get associated AnnotationTextDisplayNode");
//       return 0;
//     }
//
//
//   // open the file for reading input
//   fstream fstr;
//
//   fstr.open(fullName.c_str(), fstream::in);
//
//   if (fstr.is_open())
//     {
//       // clear out the list
//       annotationNode->ResetAnnotations();
//
//       // turn off modified events
//       int modFlag = annotationNode->GetDisableModifiedEvent();
//       annotationNode->DisableModifiedEventOn();
//       char line[1024];
//       // default column ordering for annotation info - this is exactly the same as for fiducial
//       // first pass: line will have label,x,y,z,selected,visible
//       int labelColumn = 0;
//       int xColumn = 1;
//       int yColumn = 2;
//       int zColumn = 3;
//       int selColumn = 4;
//       int visColumn = 5;
//       int numColumns = 6;
//       // save the valid lines in a vector, parse them once know the max id
//       std::vector<std::string>lines;
//       while (fstr.good())
//       {
//     fstr.getline(line, 1024);
//
//     // does it start with a #?
//     if (line[0] == '#')
//       {
//         vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");
//         // TODO: parse out the display node settings
//         // if there's a space after the hash, try to find options
//         if (line[1] == ' ')
//           {
//         vtkDebugMacro("Have a possible option in line " << line);
//         std::string lineString = std::string(line);
//         if (lineString.find("# numPoints = ") != std::string::npos)
//           {
//             vtkDebugMacro("Skipping numPoints");
//           }
//         else if (lineString.find("# name = ") != std::string::npos)
//           {
//             std::string str = lineString.substr(9,std::string::npos);
//             vtkDebugMacro("Getting name, substr = " << str);
//             annotationNode->SetName(str.c_str());
//           }
//         //else if (lineString.find("# symbolScale = ") != std::string::npos)
//         //  {
//         //    std::string str = lineString.substr(16,std::string::npos);
//         //    vtkDebugMacro("Getting symbolScale, substr = " << str);
//         //    float scale = atof(str.c_str());
//         //    annotationDisplayNode->SetGlyphScale(scale);
//         //  }
//         //else if (lineString.find("# symbolType = ") != std::string::npos)
//         //  {
//         //    std::string str = lineString.substr(15,std::string::npos);
//         //    vtkDebugMacro("Getting symbolType, substr = " << str);
//         //    int t = atoi(str.c_str());
//         //    annotationDisplayNode->SetGlyphType(t);
//         //  }
//         else if (lineString.find("# visibility = ") != std::string::npos)
//           {
//             std::string str = lineString.substr(15,std::string::npos);
//             vtkDebugMacro("Getting visibility, substr = " << str);
//             int vis = atoi(str.c_str());
//             annotationDisplayNode->SetVisibility(vis);
//           }
//         else if (lineString.find("# textScale = ") != std::string::npos)
//           {
//             std::string str = lineString.substr(14, std::string::npos);
//             vtkDebugMacro("Getting textScale, substr = " << str.c_str());
//             float scale = atof(str.c_str());
//             annotationDisplayNode->SetTextScale(scale);
//           }
//         else if (lineString.find("# color = ") != std::string::npos ||
//              lineString.find("# selectedColor = ") != std::string::npos)
//           {
//             std::string str;
//             if (lineString.find("# color = ") != std::string::npos)
//               {
//             str = lineString.substr(10, std::string::npos);
//               }
//             else
//               {
//             str = lineString.substr(18, std::string::npos);
//               }
//             vtkDebugMacro("Getting colors, substr = " << str.c_str());
//             // the rgb values are separated by commas
//             float r = 0.0, g = 0.0, b = 0.0;
//             char *ptr;
//             char *colours = (char *)(str.c_str());
//             ptr = strtok(colours, ",");
//             if (ptr != nullptr)
//               {
//             r = atof(ptr);
//               }
//             ptr = strtok(nullptr, ",");
//             if (ptr != nullptr)
//               {
//             g = atof(ptr);
//               }
//             ptr = strtok(nullptr, ",");
//             if (ptr != nullptr)
//               {
//             b = atof(ptr);
//               }
//             // now set the correct value
//             if (lineString.find("# color = ") != std::string::npos)
//               {
//             annotationDisplayNode->SetColor(r,g,b);
//               }
//             else
//               {
//             annotationDisplayNode->SetSelectedColor(r,g,b);
//               }
//           }
//         else if (lineString.find("# opacity = ") != std::string::npos)
//           {
//             std::string str = lineString.substr(12, std::string::npos);
//             vtkDebugMacro("Getting opacity, substr = " << str.c_str());
//             float val = atof(str.c_str());
//             annotationDisplayNode->SetOpacity(val);
//           }
//         else if (lineString.find("# ambient = ") != std::string::npos)
//           {
//             std::string str = lineString.substr(12, std::string::npos);
//             vtkDebugMacro("Getting ambient, substr = " << str.c_str());
//             float val = atof(str.c_str());
//             annotationDisplayNode->SetAmbient(val);
//                   }
//                 else if (lineString.find("# diffuse = ") != std::string::npos)
//                   {
//                   std::string str = lineString.substr(12, std::string::npos);
//                   vtkDebugMacro("Getting diffuse, substr = " << str.c_str());
//                   float val = atof(str.c_str());
//                   annotationDisplayNode->SetDiffuse(val);
//                   }
//                 else if (lineString.find("# specular = ") != std::string::npos)
//                   {
//                   std::string str = lineString.substr(13, std::string::npos);
//                   vtkDebugMacro("Getting specular, substr = " << str.c_str());
//                   float val = atof(str.c_str());
//                   annotationDisplayNode->SetSpecular(val);
//                   }
//                 else if (lineString.find("# power = ") != std::string::npos)
//                   {
//                   std::string str = lineString.substr(10, std::string::npos);
//                   vtkDebugMacro("Getting power, substr = " << str.c_str());
//                   float val = atof(str.c_str());
//                   annotationDisplayNode->SetPower(val);
//                   }
//                 else if (lineString.find("# opacity = ") != std::string::npos)
//                   {
//                   std::string str = lineString.substr(12, std::string::npos);
//                   vtkDebugMacro("Getting opacity, substr = " << str.c_str());
//                   float val = atof(str.c_str());
//                   annotationDisplayNode->SetOpacity(val);
//                   }
//                 else if (lineString.find("# locked = ") != std::string::npos)
//                   {
//                   std::string str = lineString.substr(10, std::string::npos);
//                   vtkDebugMacro("Getting locked, substr = " << str.c_str());
//                   int val = atoi(str.c_str());
//                   annotationDisplayNode->SetLocked(val);
//                   }
//                 //else if (lineString.find("# numberingScheme = ") != std::string::npos)
//                 //  {
//                 //  std::string str = lineString.substr(10, std::string::npos);
//                 //  vtkDebugMacro("Getting numberingScheme, substr = " << str.c_str());
//                 //  int val = atoi(str.c_str());
//                 //  annotationNode->SetNumberingScheme(val);
//                 //  }
//                 else if (lineString.find("# columns = ") != std::string::npos)
//                   {
//                   std::string str = lineString.substr(12, std::string::npos);
//                   vtkDebugMacro("Getting column order for the fids, substr = " << str.c_str());
//                   // reset all of them
//                   labelColumn = -1, xColumn = -1, yColumn = -1, zColumn = -1, selColumn = -1, visColumn = -1;
//                   int columnNumber = 0;
//                   char *columns = (char *)str.c_str();
//                   char *ptr = strtok(columns, ",");
//                   while (ptr != nullptr)
//                    {
//                    if (strcmp(ptr, "label") == 0)
//                      {
//                      labelColumn = columnNumber;
//                      }
//                    else if (strcmp(ptr, "x") == 0)
//                      {
//                      xColumn = columnNumber;
//                      }
//                    else if (strcmp(ptr, "y") == 0)
//                      {
//                      yColumn = columnNumber;
//                      }
//                    else if (strcmp(ptr, "z") == 0)
//                      {
//                      zColumn = columnNumber;
//                      }
//                    else if (strcmp(ptr, "sel") == 0)
//                      {
//                      selColumn = columnNumber;
//                      }
//                    else if (strcmp(ptr, "vis" ) == 0)
//                      {
//                      visColumn = columnNumber;
//                      }
//                    ptr = strtok(nullptr, ",");
//                    columnNumber++;
//                    }
//                  // set the total number of columns
//                  numColumns = columnNumber;
//                  vtkDebugMacro("Got " << numColumns << " columns, label = " << labelColumn << ", x = " << xColumn << ", y = " << yColumn << ", z = " << zColumn << ", sel = " <<  selColumn << ", vis = " << visColumn);
//                  }
//           }
//       }
//         else
//           {
//           // is it empty?
//           if (line[0] == '\0')
//             {
//             vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
//             }
//           else
//             {
//             vtkDebugMacro("got a line: \n\"" << line << "\"");
//             char *ptr;
//             // if the label text is missing, strtok will return a pointer to the
//             // x value, will need to call strtok with the line on the next try
//             bool reTokenise = false;
//             if (strncmp(line, ",", 1) == 0)
//               {
//               ptr = nullptr;
//               reTokenise = true;
//               }
//             else
//               {
//               ptr = strtok(line, ",");
//               }
//             std::string label = std::string("");
//         double coord[3] = {0.0, 0.0, 0.0};
//             int sel = 1, vis = 1;
//             int columnNumber = 0;
//             while (columnNumber < numColumns)
//               {
//               if (ptr != nullptr)
//                 {
//                 if (columnNumber == labelColumn)
//                   {
//                   label = std::string(ptr);
//                   }
//                 else if (columnNumber == xColumn)
//                   {
//                   coord[0] = atof(ptr);
//                   }
//                 else if (columnNumber == yColumn)
//                   {
//                   coord[1] = atof(ptr);
//                   }
//                 else if (columnNumber == zColumn)
//                   {
//                   coord[2] = atof(ptr);
//                   }
//                 else if (columnNumber == selColumn)
//                   {
//                   sel = atoi(ptr);
//                   }
//                 else if (columnNumber == visColumn)
//                   {
//                   vis = atoi(ptr);
//                   }
//                 }
//               if (reTokenise == false)
//                 {
//                 ptr = strtok(nullptr, ",");
//                 }
//               else
//                 {
//                 ptr = strtok(line, ",");
//                 // turn it off
//                 reTokenise = false;
//                 }
//               columnNumber++;
//               } // end while over columns
//         //KP: label.c_str() currently leave it out -think about it later
//         if (annotationNode->AddControlPoint(coord, sel, vis) < 0 )
//           {
//         vtkErrorMacro("Error adding control point to list, label = " << label.c_str());
//           }
//           } // point line
//         }
//       }
//     annotationNode->SetDisableModifiedEvent(modFlag);
//     annotationNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, annotationNode);//vtkMRMLAnnotationFiducialsNode::DisplayModifiedEvent);
//     fstr.close();
//     }
//   else
//     {
//     vtkErrorMacro("ERROR opening annotation file " << this->FileName << endl);
//     return 0;
//     }
//
//   return 1;
// }


//----------------------------------------------------------------------------
void vtkMRMLAnnotationFiducialsStorageNode::InitializeSupportedWriteFileTypes()
{
  this->Superclass::InitializeSupportedWriteFileTypes();
  this->SupportedWriteFileTypes->InsertNextValue("Fiducial List CSV (.fcsv)");
}
