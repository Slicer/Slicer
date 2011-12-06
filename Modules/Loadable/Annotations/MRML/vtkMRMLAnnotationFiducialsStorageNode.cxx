// Annotation includes
#include "vtkMRMLAnnotationFiducialsStorageNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"

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
  int size = std::string(this->GetAnnotationStorageType()).size();
 
  if (attValue.compare(0,size,this->GetAnnotationStorageType()))
    {
      return 0;
    }

  int sel = 1, vis = 1;
  std::string annotation;
  
  std::string label = std::string("");
  size_t  startPos = 0;
  size_t  endPos =attValue.find(",",startPos);
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
    endPos =attValue.find(",",startPos);
    columnNumber ++;
    }
  
  // Add Fiducials
  if (!this->GetScene())
    {
    vtkErrorMacro("Cannot add Fiducial as MRML Scene is not defined");
    return -1;
    }
  
  
  vtkMRMLAnnotationFiducialNode *newNode = vtkMRMLAnnotationFiducialNode::New();
  this->GetScene()->AddNode(newNode);
  newNode->Delete();
  
  vtkMRMLAnnotationTextDisplayNode *newDisplayNode = newNode->GetAnnotationTextDisplayNode();
  if (!newDisplayNode)
    {
    vtkErrorMacro("Cannot add Fiducial as AnnotationDisplay Node could not be created!");
    return -1;
    }
  newDisplayNode->Copy(refNode->GetAnnotationTextDisplayNode());
  
  vtkMRMLAnnotationPointDisplayNode *newCPDisplayNode = newNode->GetAnnotationPointDisplayNode();
  if (!newCPDisplayNode)
    {
    vtkErrorMacro("Cannot add Fiducial as AnnotationControlPointDisplayNode could not be created!");
    return -1;
    }
  newCPDisplayNode->Copy(refNode->GetAnnotationPointDisplayNode());
  
  if (newNode->SetFiducial(label.c_str(),coord, sel, vis) < 0)
    {
    vtkErrorMacro("Error adding control point to list, coord = " << coord[0] << " " << coord[1] << " " << coord[2]);
    return -1;
    }
  
  // make sure that the list node points to this storage node
  newNode->SetAndObserveStorageNodeID(this->GetID());
  
  // mark it unmodified since read
  newNode->ModifiedSinceReadOff();
  
  newNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, newCPDisplayNode);//vtkMRMLAnnotationNode::DisplayModifiedEvent);
  
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
      char *ptr = strtok(columns, ",");
      while (ptr != NULL)
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
      ptr = strtok(NULL, ",");
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
int vtkMRMLAnnotationFiducialsStorageNode::ReadAnnotation()
{

  vtkSmartPointer< vtkMRMLAnnotationFiducialNode > refNode = vtkSmartPointer< vtkMRMLAnnotationFiducialNode >::New();

  if (refNode == NULL)
    {
      vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a annotation node");
      return 0;
    }

  // open the file for reading input
  fstream fstr;
  if (!this->OpenFileToRead(fstr, refNode))
    {
      return 0;
    }

  vtkMRMLAnnotationPointDisplayNode *refPointDisplayNode = vtkMRMLAnnotationPointDisplayNode::New();
  vtkMRMLAnnotationTextDisplayNode *refDisplayNode = vtkMRMLAnnotationTextDisplayNode::New();

  
  // turn off modified events
  int modFlag = refNode->GetDisableModifiedEvent();
  refNode->DisableModifiedEventOn();
  char line[1024];
  // default column ordering for annotation info - this is exactly the same as for fiducial
  // first pass: line will have label,x,y,z,selected,visible
  int labelColumn = 0;
  int xPointColumn = 1;
  int yPointColumn = 2;
  int zPointColumn = 3;
  int selPointColumn  = 4;
  int visPointColumn  = 5;
  int numPointColumns = 6;

  // create a fake fiducial node to read in fiducial properties
  if (!refNode->GetAnnotationPointDisplayNode())
    {
       std::cerr << "Error in AnnotationPointDisplayNode() " << std::endl;
       
       return 0;
    }  


  while (fstr.good())
    {
    fstr.getline(line, 1024);
    
    // does it start with a #?
        // Property
    if (line[0] == '#')
      {
        if (line[1] == ' ') 
          {
            if (!this->ReadAnnotationFiducialsProperties(refNode, line, labelColumn, xPointColumn, yPointColumn,  zPointColumn, selPointColumn, visPointColumn, numPointColumns))
              {
            if (this->ReadAnnotationPointDisplayProperties(refNode->GetAnnotationPointDisplayNode(), line,"") < 0 )
              {
                return 0;
              }
              }
          }
      }
        else
          {
        if (this->ReadAnnotationFiducialsData(refNode, line, labelColumn, xPointColumn, yPointColumn, zPointColumn, selPointColumn,  
                              visPointColumn, numPointColumns) < 0 ) 
          {
        return 0;
          }
      }
    }   
    refNode->SetDisableModifiedEvent(modFlag);

    fstr.close();

    refPointDisplayNode->Delete();
    refDisplayNode->Delete();
 
    return 1;

}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationFiducialsStorageNode::ReadData()
{
  // do not read if if we are not in the scene (for example inside snapshot)
  if (!this->GetAddToScene())
    {
      return 1;
    }

  if (!this->ReadAnnotation())
    {
      return 0;
    }

  this->SetReadStateIdle();
  
  return 1;
}

//----------------------------------------------------------------------------
// int vtkMRMLAnnotationFiducialsStorageNode::ReadData(vtkMRMLNode *refNode)
// {
//   // do not read if if we are not in the scene (for example inside snapshot)
//   if ( !this->GetAddToScene() || !refNode->GetAddToScene() )
//     {
//     return 1;
//     }
// 
//   vtkDebugMacro("Reading Annotation data");
//   // test whether refNode is a valid node to hold a fiducial list
//   if ( !( refNode->IsA("vtkMRMLAnnotationFiducialsNode"))
//      ) 
//     {
//     vtkErrorMacro("Reference node is not a proper vtkMRMLAnnotationFiducialsNode");
//     return 0;         
//     }
// 
//   if (this->GetFileName() == NULL && this->GetURI() == NULL) 
//     {
//     vtkErrorMacro("ReadData: file name and uri not set");
//     return 0;
//     }
// 
//   Superclass::StageReadData(refNode);
//   if ( this->GetReadState() != this->TransferDone )
//     {
//     // remote file download hasn't finished
//     vtkWarningMacro("ReadData: Read state is pending, returning.");
//     return 0;
//     }
//   
//   std::string fullName = this->GetFullNameFromFileName(); 
// 
//   if (fullName == std::string("")) 
//     {
//     vtkErrorMacro("vtkMRMLAnnotationFiducialsStorageNode: File name not specified");
//     return 0;
//     }
// 
//   // cast the input node
//   vtkMRMLAnnotationFiducialsNode *annotationNode = NULL;
//   if ( refNode->IsA("vtkMRMLAnnotationFiducialsNode") )
//     {
//     annotationNode = dynamic_cast <vtkMRMLAnnotationFiducialsNode *> (refNode);
//     }
// 
//   if (annotationNode == NULL)
//     {
//     vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a annotation node");
//     return 0;
//     }
// 
//   
//   // cast the input node
//   vtkMRMLAnnotationTextDisplayNode *annotationDisplayNode = annotationNode->GetAnnotationTextDisplayNode();
//   if (annotationDisplayNode == NULL)
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
//             if (ptr != NULL)
//               {
//             r = atof(ptr);
//               }
//             ptr = strtok(NULL, ",");
//             if (ptr != NULL)
//               {
//             g = atof(ptr);
//               }
//             ptr = strtok(NULL, ",");
//             if (ptr != NULL)
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
//                   while (ptr != NULL)
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
//                    ptr = strtok(NULL, ",");
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
//               ptr = NULL;
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
//               if (ptr != NULL)
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
//                 ptr = strtok(NULL, ",");
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
//   this->SetReadStateIdle();
//   
//   // make sure that the list node points to this storage node
//   annotationNode->SetAndObserveStorageNodeID(this->GetID());
// 
//   // mark it unmodified since read
//   annotationNode->ModifiedSinceReadOff();
//   
//   return 1;
// }


//----------------------------------------------------------------------------
void vtkMRMLAnnotationFiducialsStorageNode::InitializeSupportedWriteFileTypes()
{
  Superclass::InitializeSupportedWriteFileTypes();
  
  this->SupportedWriteFileTypes->InsertNextValue("Fiducial List CSV (.fcsv)");
}
