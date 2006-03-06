/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:41 $
Version:   $Revision: 1.12 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScene.h"

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeNode");
  if(ret)
    {
      return (vtkMRMLVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeNode");
  if(ret)
    {
      return (vtkMRMLVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode::vtkMRMLVolumeNode()
{
  // Strings
  this->LUTName = NULL;
  this->ScanOrder = NULL;

  // Numbers
  this->FileScalarType = VTK_SHORT;
  this->FileNumberOfScalarComponents = 0;
  this->LabelMap = 0;
  this->Interpolate = 1;
  this->FileLittleEndian = 0;

  this->AutoWindowLevel = 1;
  this->Window = 256;
  this->Level = 128;
  this->AutoThreshold = 0;
  this->ApplyThreshold = 0;
  this->LowerThreshold = VTK_SHORT_MIN;
  this->UpperThreshold = VTK_SHORT_MAX;

  memset(this->FileDimensions,0,2*sizeof(int));
  memset(this->FileSpacing,0,3*sizeof(vtkFloatingPointType));

  // ScanOrder can never be NULL
  this->ScanOrder = new char[3];
  strcpy(this->ScanOrder, "");

  // Initialize 
  this->SetFileDimensions(0, 0, 0);
  this->SetFileSpacing(0, 0, 0);

  // Data
  this->StorageNodeID = NULL;
  this->DisplayNodeID = NULL;
  this->ImageData = NULL;
  this->StorageNode = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode::~vtkMRMLVolumeNode()
{
  if (this->LUTName)
    {
      delete [] this->LUTName;
      this->LUTName = NULL;
    }
  if (this->ScanOrder)
    {
      delete [] this->ScanOrder;
      this->ScanOrder = NULL;
    }


  if (this->StorageNodeID)
    {
      delete [] this->StorageNodeID;
      this->StorageNodeID = NULL;
    }
  if (this->DisplayNodeID)
    {
      delete [] this->DisplayNodeID;
      this->DisplayNodeID = NULL;
    }

  if (this->ImageData) 
    {
      this->ImageData->Delete();
    }
  
  if (this->StorageNode) 
    {
      this->StorageNode->Delete();
    }
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeNode::GetFileScalarTypeAsString()
{
  switch (this->FileScalarType)
    {
    case VTK_VOID:           return "Void"; break;
    case VTK_BIT:            return "Bit"; break;
    case VTK_CHAR:           return "Char"; break;
    case VTK_UNSIGNED_CHAR:  return "UnsignedChar"; break;
    case VTK_SHORT:          return "Short"; break;
    case VTK_UNSIGNED_SHORT: return "UnsignedShort"; break;
    case VTK_INT:            return "Int"; break;
    case VTK_UNSIGNED_INT:   return "UnsignedInt"; break;
    case VTK_LONG:           return "Long"; break;
    case VTK_UNSIGNED_LONG:  return "UnsignedLong"; break;
    case VTK_FLOAT:          return "Float"; break;
    case VTK_DOUBLE:         return "Double"; break;
    }
  return "Short";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::WriteXML(ostream& of, int nIndent)
{
  vtkErrorMacro("NOT IMPLEMENTED YET");
  (void)of; (void)nIndent;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "FileDimensions")) {
      std::stringstream ss;
      ss << attValue;
      ss >> FileDimensions[0];
      ss >> FileDimensions[1];
      ss >> FileDimensions[2];
    }
    else if (!strcmp(attName, "FileSpacing")) {
      std::stringstream ss;
      ss << attValue;
      ss >> FileSpacing[0];
      ss >> FileSpacing[1];
      ss >> FileSpacing[2];
    }
    else if (!strcmp(attName, "FileNumberOfScalarComponents")) {
      std::stringstream ss;
      ss << attValue;
      ss >> FileNumberOfScalarComponents;
    }
    else if (!strcmp(attName, "FileScalarType")) {
      std::stringstream ss;
      ss << attValue;
      ss >> FileScalarType;
    }
    else if (!strcmp(attName, "FileLittleEndian")) {
      std::stringstream ss;
      ss << attValue;
      ss >> FileLittleEndian;
    }
    else if (!strcmp(attName, "IjkToRasDirections")) {
      std::stringstream ss;
      double val;
      ss << attValue;
      for (int i=0; i<9; i++) {
        ss >> val;
        this->IjkToRasDirections[i] = val;
      }
    }
    else if (!strcmp(attName, "StorageNodeID")) {
      this->SetStorageNodeID(attValue);
    }
    else if (!strcmp(attName, "DisplayNodeID")) {
      this->SetDisplayNodeID(attValue);
    }
  }  
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVolumeNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLNode::Copy(anode);
  vtkMRMLVolumeNode *node = (vtkMRMLVolumeNode *) anode;

  // Strings
  this->SetLUTName(node->LUTName);
  this->SetScanOrder(node->ScanOrder);

  // Vectors
  this->SetFileSpacing(node->FileSpacing);
  this->SetFileDimensions(node->FileDimensions);
  
  // Numbers
  this->SetLabelMap(node->LabelMap);
  this->SetFileLittleEndian(node->FileLittleEndian);
  this->SetFileScalarType(node->FileScalarType);
  this->SetFileNumberOfScalarComponents(node->FileNumberOfScalarComponents);
  this->SetAutoWindowLevel(node->AutoWindowLevel);
  this->SetWindow(node->Window);
  this->SetLevel(node->Level);
  this->SetAutoThreshold(node->AutoThreshold);
  this->SetApplyThreshold(node->ApplyThreshold);
  this->SetUpperThreshold(node->UpperThreshold);
  this->SetLowerThreshold(node->LowerThreshold);
  this->SetInterpolate(node->Interpolate);

  // Matrices
  for(int i=0; i<9; i++) {
    this->IjkToRasDirections[i] = node->IjkToRasDirections[i];
  }
  if (this->ImageData) {
    this->SetImageData(node->ImageData);
  }
  if (this->StorageNode) {
    this->SetStorageNode(node->StorageNode);
  }
  this->SetStorageNodeID(node->StorageNodeID);
  this->SetDisplayNodeID(node->DisplayNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "ScanOrder: " <<
    (this->ScanOrder ? this->ScanOrder : "(none)") << "\n";
  os << indent << "LUTName: " <<
    (this->LUTName ? this->LUTName : "(none)") << "\n";

  os << indent << "LabelMap:          " << this->LabelMap << "\n";
  os << indent << "FileLittleEndian:  " << this->FileLittleEndian << "\n";
  os << indent << "FileScalarType:    " << this->FileScalarType << "\n";
  os << indent << "FileNumberOfScalarComponents:  " << this->FileNumberOfScalarComponents << "\n";
  os << indent << "AutoWindowLevel:   " << this->AutoWindowLevel << "\n";
  os << indent << "Window:            " << this->Window << "\n";
  os << indent << "Level:             " << this->Level << "\n";
  os << indent << "AutoThreshold:     " << this->AutoThreshold << "\n";
  os << indent << "ApplyThreshold:    " << this->ApplyThreshold << "\n";
  os << indent << "UpperThreshold:    " << this->UpperThreshold << "\n";
  os << indent << "LowerThreshold:    " << this->LowerThreshold << "\n";
  os << indent << "Interpolate:       " << this->Interpolate << "\n";

  os << "FileSpacing:\n";
  for (idx = 0; idx < 3; ++idx) {
    os << indent << ", " << this->FileSpacing[idx];
  }
  os << ")\n";
  
  os << "FileDimensions:\n";
  for (idx = 0; idx < 3; ++idx) {
    os << indent << ", " << this->FileDimensions[idx];
  }
  os << ")\n";
  
  // Matrices
  os << "IjkToRasDirections:\n";
  for (idx = 0; idx < 9; ++idx) {
    os << indent << ", " << this->IjkToRasDirections[idx];
  }
  os << ")\n";

  os << indent << "StorageNodeID: " <<
    (this->StorageNodeID ? this->StorageNodeID : "(none)") << "\n";

  os << indent << "DisplayNodeID: " <<
    (this->DisplayNodeID ? this->DisplayNodeID : "(none)") << "\n";

  if (this->ImageData != NULL) {
    os << indent << "ImageData:\n";
    this->ImageData->PrintSelf(os, indent.GetNextIndent()); 
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetIjkToRasDirections(double dirs[9])
{
  for (int i=0; i<9; i++) {
    IjkToRasDirections[i] = dirs[i];
  }
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::SetIjkToRasDirections(double ir, double ia, double is,
                                              double jr, double ja, double js,
                                              double kr, double ka, double ks)
{
  IjkToRasDirections[0] = ir;
  IjkToRasDirections[1] = ia;
  IjkToRasDirections[2] = is;
  IjkToRasDirections[3] = jr;
  IjkToRasDirections[4] = ja;
  IjkToRasDirections[5] = js;
  IjkToRasDirections[6] = kr;
  IjkToRasDirections[7] = ka;
  IjkToRasDirections[8] = ks;
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::SetIToRasDirection(double ir, double ia, double is)
{
  IjkToRasDirections[0] = ir;
  IjkToRasDirections[1] = ia;
  IjkToRasDirections[2] = is;
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::SetJToRasDirection(double jr, double ja, double js)
{
  IjkToRasDirections[3] = jr;
  IjkToRasDirections[4] = ja;
  IjkToRasDirections[5] = js;
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::SetKToRasDirection(double kr, double ka, double ks)
{
  IjkToRasDirections[6] = kr;
  IjkToRasDirections[7] = ka;
  IjkToRasDirections[8] = ks;
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::GetIjkToRasDirections(double dirs[9])
{
  for (int i=0; i<9; i++) {
    dirs[i] = IjkToRasDirections[i];
  }
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::GetIToRasDirection(double dirs[3])
{
  for (int i=0; i<3; i++) {
    dirs[i] = IjkToRasDirections[i];
  }
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::GetJToRasDirection(double dirs[3])
{
  for (int i=0; i<3; i++) {
    dirs[i] = IjkToRasDirections[3+i];
  }
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::GetKToRasDirection(double dirs[3])
{
  for (int i=0; i<3; i++) {
    dirs[i] = IjkToRasDirections[6+i];
  }
}

//----------------------------------------------------------------------------

double* vtkMRMLVolumeNode::GetIjkToRasDirections()
{
  return IjkToRasDirections;
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::GetIjkToRasMatrix(vtkMatrix4x4* mat)
{
  mat->Identity();
  int i=0;
  for (int row=0; row<3; row++) {
    for (int col=0; col<3; col++) {
      mat->SetElement(row, col, IjkToRasDirections[i++]);
    }
  }
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::SetIjkToRasMatrix(vtkMatrix4x4* mat)
{
  int i=0;
  for (int row=0; row<3; row++) {
    for (int col=0; col<3; col++) {
      IjkToRasDirections[i++] = mat->GetElement(row, col);
    }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::ComputeIjkToRasFromScanOrder(char *order, vtkMatrix4x4 *IjkToRas)
{
  std::cerr << "NOT IMPLEMENTED YET" << std::endl;
  (void)order; (void)IjkToRas;
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeNode::ComputeScanOrderFromIjkToRas(vtkMatrix4x4 *ijkToRas)
{
  vtkFloatingPointType dir[4]={0,0,1,0};
  vtkFloatingPointType kvec[4];
 
  ijkToRas->MultiplyPoint(dir,kvec);
  int max_comp = 0;
  double max = fabs(kvec[0]);
  
  for (int i=1; i<3; i++) {
    if (fabs(kvec[i]) > max) {
      max = fabs(kvec[i]);
      max_comp=i;
    }   
  }
  
  switch(max_comp) {
  case 0:
    if (kvec[max_comp] > 0 ) {
      return "LR";
    } else {
      return "RL";
    }
    break;
  case 1:     
    if (kvec[max_comp] > 0 ) {
      return "PA";
    } else {
      return "AP";
    }
    break;
  case 2:
    if (kvec[max_comp] > 0 ) {
      return "IS";
    } else {
      return "SI";
    }
    break;
  default:
    cerr << "vtkMRMLVolumeNode::ComputeScanOrderFromRasToIjk:\n\tMax components "<< max_comp << " not in valid range 0,1,2\n";
    return "";
  }        
 
}


void vtkMRMLVolumeNode::UpdateScene(vtkMRMLScene *scene)
{
  if (this->GetStorageNodeID() == NULL) {
    vtkErrorMacro("No reference StorageNodeID found");
    return;
  }

  vtkCollection* nodes = scene->GetNodesByID(StorageNodeID);
  if (nodes->GetNumberOfItems() != 1) {
    vtkErrorMacro("Not unique reference to StorageNode: ID" << StorageNodeID);
  }
  vtkMRMLStorageNode *node  = dynamic_cast < vtkMRMLStorageNode *>(nodes->GetItemAsObject(0));
  if (node) {
    node->ReadData(this);
  }
  
}


// End
