/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

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
  this->LabelMap = 0;

  this->StorageNodeID = NULL;
  this->DisplayNodeID = NULL;
  this->ImageData = NULL;
  this->StorageNode = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode::~vtkMRMLVolumeNode()
{
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
void vtkMRMLVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->StorageNodeID != NULL) {
    of << indent << "StorageNodeID='" << this->StorageNodeID << "' ";
  }
  if (this->DisplayNodeID != NULL) {
    of << indent << "DisplayNodeID='" << this->DisplayNodeID << "' ";
  }
  if (this->IjkToRasDirections != NULL) {
    std::stringstream ss;
    for (int i=0; i<9; i++) {
      ss << this->IjkToRasDirections[i];
      if (i!=8) {
        ss << " ";
      }
    }
    of << indent << "IjkToRasDirections='" << ss.str() << "' ";
  }

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
    if (!strcmp(attName, "IjkToRasDirections")) {
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
  Superclass::Copy(anode);
  vtkMRMLVolumeNode *node = (vtkMRMLVolumeNode *) anode;

  this->SetLabelMap(node->LabelMap);

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
  os << indent << "LabelMap:          " << this->LabelMap << "\n";
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

  vtkCollection* nodes = scene->GetNodesByID(this->StorageNodeID);
  if (nodes->GetNumberOfItems() != 1) {
    vtkErrorMacro("Not unique reference to StorageNode: ID" << StorageNodeID);
  }
  vtkMRMLStorageNode *node  = dynamic_cast < vtkMRMLStorageNode *>(nodes->GetItemAsObject(0));
  if (node) {
    node->ReadData(this);
  }
  
  if (this->DisplayNodeID != NULL) {
  nodes = scene->GetNodesByID(this->DisplayNodeID);
    if (nodes->GetNumberOfItems() != 1) {
      vtkErrorMacro("Not unique reference to DisplayNodeID: ID" << StorageNodeID);
    }
    vtkMRMLVolumeDisplayNode *displayNode  = dynamic_cast < vtkMRMLVolumeDisplayNode *>(nodes->GetItemAsObject(0));

    this->SetDisplayNode(displayNode);
  }
  
}


// End
