/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

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
#include "vtkMatrix4x4.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScene.h"

//----------------------------------------------------------------------------
vtkMRMLVolumeNode::vtkMRMLVolumeNode()
{
  this->StorageNodeID = NULL;
  this->DisplayNodeID = NULL;
  this->TransformNodeID = NULL;

  this->StorageNode = NULL;
  this->DisplayNode = NULL;
  this->TransformNode = NULL;

  this->IJKToRAS = vtkMatrix4x4::New();
  this->IJKToRAS->Identity();

  this->ImageData = NULL;
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
  if (this->TransformNodeID) 
    {
    delete [] this->TransformNodeID;
    this->TransformNodeID = NULL;
    }

  if (this->ImageData)  {
  this->ImageData->Delete();
  }
  
  if (this->StorageNode)  {
  this->StorageNode->Delete();
  }
  if (this->DisplayNode) 
    {
    this->DisplayNode->Delete();
    }
  if (this->TransformNode) 
    {
    this->TransformNode->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  
  if (this->StorageNodeID != NULL) 
    {
    of << indent << "storageNodeRef=\"" << this->StorageNodeID << "\" ";
    }
  if (this->DisplayNodeID != NULL) 
    {
    of << indent << "displayNodeRef=\"" << this->DisplayNodeID << "\" ";
    }
  if (this->TransformNodeID != NULL) 
    {
    of << indent << "transformNodeRef=\"" << this->TransformNodeID << "\" ";
    }
  if (this->IjkToRasDirections != NULL) 
    {
    std::stringstream ss;
    for (int i=0; i<9; i++) 
      {
      ss << this->IjkToRasDirections[i];
      if (i!=8) 
        {
        ss << " ";
        }
      }
    of << indent << "ijkToRasDirections=\"" << ss.str() << "\" ";
    }

}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "ijkToRasDirections")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for (int i=0; i<9; i++) 
        {
        ss >> val;
        this->IjkToRasDirections[i] = val;
        }
      }
    else if (!strcmp(attName, "storageNodeRef")) 
      {
      this->SetStorageNodeID(attValue);
      }
    else if (!strcmp(attName, "displayNodeRef")) 
      {
      this->SetDisplayNodeID(attValue);
      }
    else if (!strcmp(attName, "transformNodeRef")) 
      {
      this->SetTransformNodeID(attValue);
      }
    }  
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVolumeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVolumeNode *node = (vtkMRMLVolumeNode *) anode;

  // Matrices
  for(int i=0; i<9; i++) 
    {
    this->IjkToRasDirections[i] = node->IjkToRasDirections[i];
    }
  if (this->ImageData) 
    {
    this->SetImageData(node->ImageData);
    }
  if (this->StorageNode) 
    {
    this->SetStorageNode(node->StorageNode);
    }  
  if (this->DisplayNode) 
    {
    this->SetDisplayNode(node->DisplayNode);
    }
  if (this->TransformNode) 
    {
    this->SetTransformNode(node->TransformNode);
    }
  this->SetStorageNodeID(node->StorageNodeID);
  this->SetDisplayNodeID(node->DisplayNodeID);
  this->SetTransformNodeID(node->TransformNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  Superclass::PrintSelf(os,indent);
  // Matrices
  os << "IjkToRasDirections:\n";
  for (idx = 0; idx < 9; ++idx) 
    {
    os << indent << ", " << this->IjkToRasDirections[idx];
    }
  os << ")\n";

  os << indent << "StorageNodeID: " <<
    (this->StorageNodeID ? this->StorageNodeID : "(none)") << "\n";

  os << indent << "DisplayNodeID: " <<
    (this->DisplayNodeID ? this->DisplayNodeID : "(none)") << "\n";

  os << indent << "TransformNodeID: " <<
    (this->TransformNodeID ? this->TransformNodeID : "(none)") << "\n";

  if (this->ImageData != NULL) 
    {
    os << indent << "ImageData:\n";
    this->ImageData->PrintSelf(os, indent.GetNextIndent()); 
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetIjkToRasDirections(double dirs[9])
{
  for (int i=0; i<9; i++) 
    {
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
  for (int i=0; i<9; i++) 
    {
    dirs[i] = IjkToRasDirections[i];
    }
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::GetIToRasDirection(double dirs[3])
{
  for (int i=0; i<3; i++) 
    {
    dirs[i] = IjkToRasDirections[i];
    }
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::GetJToRasDirection(double dirs[3])
{
  for (int i=0; i<3; i++) 
    {
    dirs[i] = IjkToRasDirections[3+i];
    }
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::GetKToRasDirection(double dirs[3])
{
  for (int i=0; i<3; i++) 
    {
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
  for (int row=0; row<3; row++) 
    {
    for (int col=0; col<3; col++) 
      {
      mat->SetElement(row, col, IjkToRasDirections[i++]);
      }
    }
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeNode::SetIjkToRasMatrix(vtkMatrix4x4* mat)
{
  int row, col, i=0;
  for (row=0; row<3; row++) 
    {
    for (col=0; col<3; col++) 
      {
      IjkToRasDirections[i++] = mat->GetElement(row, col);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetIJKToRASMatrix(vtkMatrix4x4* mat)
{
  // this is the full matrix including the spacing and origin
  mat->Identity();
  int row, col, i=0;
  for (row=0; row<3; row++) 
    {
    for (col=0; col<3; col++) 
      {
      mat->SetElement(row, col, this->Spacing[row] * IjkToRasDirections[i++]);
      }
    mat->SetElement(row, 3, this->Origin[row]);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetRASToIJKMatrix(vtkMatrix4x4* mat)
{
  this->GetIJKToRASMatrix( mat );
  mat->Invert();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::ComputeIjkToRasFromScanOrder(char *order, vtkMatrix4x4 *IjkToRas)
{
  cout << "NOT IMPLEMENTED YET" << "\n";
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeNode::ComputeScanOrderFromIjkToRas(vtkMatrix4x4 *ijkToRas)
{
  vtkFloatingPointType dir[4]={0,0,1,0};
  vtkFloatingPointType kvec[4];
 
  ijkToRas->MultiplyPoint(dir,kvec);
  int max_comp = 0;
  double max = fabs(kvec[0]);
  
  for (int i=1; i<3; i++) 
    {
    if (fabs(kvec[i]) > max) 
      {
      max = fabs(kvec[i]);
      max_comp=i;
      }   
    }
  
  switch(max_comp) 
    {
    case 0:
      if (kvec[max_comp] > 0 ) 
        {
        return "LR";
        } else 
          {
          return "RL";
          }
      break;
    case 1:     
      if (kvec[max_comp] > 0 ) 
        {
        return "PA";
        } else 
          {
          return "AP";
          }
      break;
    case 2:
      if (kvec[max_comp] > 0 ) 
        {
        return "IS";
        } else
          {
          return "SI";
          }
      break;
    default:
      cerr << "vtkMRMLVolumeNode::ComputeScanOrderFromRasToIjk:\n\tMax components "<< max_comp << " not in valid range 0,1,2\n";
      return "";
    }        
 
}

//-----------------------------------------------------------
void vtkMRMLVolumeNode::UpdateScene(vtkMRMLScene *scene)
{
  if (this->GetStorageNodeID() == NULL) 
    {
    vtkErrorMacro("No reference StorageNodeID found");
    return;
    }

  vtkMRMLNode* mnode = scene->GetNodeByID(this->StorageNodeID);
  if (mnode) 
    {
    vtkMRMLStorageNode *node  = dynamic_cast < vtkMRMLStorageNode *>(mnode);
    node->ReadData(this);
    this->SetStorageNode(node);
    }

  if (this->DisplayNodeID != NULL) 
    {
    mnode = scene->GetNodeByID(this->DisplayNodeID);
    vtkMRMLVolumeDisplayNode *displayNode  = dynamic_cast < vtkMRMLVolumeDisplayNode *>(mnode);
    this->SetDisplayNode(displayNode);
    }

  if (this->TransformNodeID != NULL) 
    {
    mnode = scene->GetNodeByID(this->TransformNodeID);
    if (mnode) 
      {
      vtkMRMLTransformNode *node  = dynamic_cast < vtkMRMLTransformNode *>(mnode);
      this->SetTransformNode(node);
      }
    }
}


 
