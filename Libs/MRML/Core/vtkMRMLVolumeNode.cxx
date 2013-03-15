/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

// STD includes

//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkMRMLVolumeNode, ImageData, vtkImageData);

//----------------------------------------------------------------------------
vtkMRMLVolumeNode::vtkMRMLVolumeNode()
{
  int i,j;

  for(i=0; i<3; i++) 
    {
    for(j=0; j<3; j++) 
      {
      this->IJKToRASDirections[i][j] = (i == j) ? 1.0 : 0.0;
      }
    }
 
  for(i=0; i<3; i++) 
    {
    this->Spacing[i] = 1.0;
    }

  for(i=0; i<3; i++) 
    {
    this->Origin[i] = 0.0;
    }

  this->ImageData = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode::~vtkMRMLVolumeNode()
{
  this->SetAndObserveImageData(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  
  std::stringstream ss;
  for(int i=0; i<3; i++) 
    {
    for(int j=0; j<3; j++) 
      {
      ss << this->IJKToRASDirections[i][j] << " ";
      if ( i != 2 && j != 2 )
        {
        ss << "  ";
        }
      }
    }
    of << indent << " ijkToRASDirections=\"" << ss.str() << "\"";


  of << indent << " spacing=\"" 
    << this->Spacing[0] << " " << this->Spacing[1] << " " << this->Spacing[2] << "\"";

  of << indent << " origin=\"" 
    << this->Origin[0] << " " << this->Origin[1] << " " << this->Origin[2] << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "ijkToRASDirections")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        for(int j=0; j<3; j++) 
          {
          ss >> val;
          this->IJKToRASDirections[i][j] = val;
          }
        }
      }
    if (!strcmp(attName, "spacing")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->Spacing[i] = val;
        }
      }
    if (!strcmp(attName, "origin")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->Origin[i] = val;
        }
      }
   }  

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVolumeNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  // don't modify the input node
  int amode = anode->GetDisableModifiedEvent();
  anode->DisableModifiedEventOn();

  Superclass::Copy(anode);
  vtkMRMLVolumeNode *node = (vtkMRMLVolumeNode *) anode;

  // Matrices

  // workaround the problem when no spacing/origin/orientation is specified in the snapshots
  // don't overwrite good values with defaults
  if (node->GetAddToScene())
    {
    for(int i=0; i<3; i++) 
      {
      this->Origin[i] = node->Origin[i];
      this->Spacing[i] = node->Spacing[i];
      for(int j=0; j<3; j++) 
        {
        this->IJKToRASDirections[i][j] = node->IJKToRASDirections[i][j];
        }
      }
    }

  if (node->ImageData != NULL)
    {
    // Only copy bulk data if it exists - this handles the case
    // of restoring from SceneViews, where the nodes will not 
    // have bulk data.
    this->SetAndObserveImageData(node->ImageData);
    }

  anode->SetDisableModifiedEvent(amode);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::CopyOrientation(vtkMRMLVolumeNode *node)
{

  // Matrices
  for(int i=0; i<3; i++) 
    {
    for(int j=0; j<3; j++) 
      {
      this->IJKToRASDirections[i][j] = node->IJKToRASDirections[i][j];
      }
    }
  this->SetOrigin(node->GetOrigin());
  this->SetSpacing(node->GetSpacing());
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  // Matrices
  os << "IJKToRASDirections:\n";

  int i,j;

  for(i=0; i<3; i++) 
    {
    for(j=0; j<3; j++) 
      {
      os << indent << " " << this->IJKToRASDirections[i][j];
      }
      os << indent << "\n";
    }
  os << "\n";

  os << "Origin:";
  for(j=0; j<3; j++) 
    {
    os << indent << " " << this->Origin[j];
    }
  os << "\n";
  os << "Spacing:";
  for(j=0; j<3; j++) 
    {
    os << indent << " " << this->Spacing[j];
    }
  os << "\n";

  if (this->ImageData != NULL) 
    {
    os << indent << "ImageData:\n";
    this->ImageData->PrintSelf(os, indent.GetNextIndent()); 
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetIJKToRASDirections(double dirs[3][3])
{
  for (int i=0; i<3; i++) 
    {
    for (int j=0; j<3; j++) 
      {
      IJKToRASDirections[i][j] = dirs[i][j];
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetIJKToRASDirections(double ir, double ia, double is,
                                              double jr, double ja, double js,
                                              double kr, double ka, double ks)
{
  IJKToRASDirections[0][0] = ir;
  IJKToRASDirections[0][1] = ia;
  IJKToRASDirections[0][2] = is;
  IJKToRASDirections[1][0] = jr;
  IJKToRASDirections[1][1] = ja;
  IJKToRASDirections[1][2] = js;
  IJKToRASDirections[2][0] = kr;
  IJKToRASDirections[2][1] = ka;
  IJKToRASDirections[2][2] = ks;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetIToRASDirection(double ir, double ia, double is)
{
  IJKToRASDirections[0][0] = ir;
  IJKToRASDirections[1][0] = ia;
  IJKToRASDirections[2][0] = is;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetJToRASDirection(double jr, double ja, double js)
{
  IJKToRASDirections[0][1] = jr;
  IJKToRASDirections[1][1] = ja;
  IJKToRASDirections[2][1] = js;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetKToRASDirection(double kr, double ka, double ks)
{
  IJKToRASDirections[0][2] = kr;
  IJKToRASDirections[1][2] = ka;
  IJKToRASDirections[2][2] = ks;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetIToRASDirection(double dirs[3])
{
  for (int i=0; i<3; i++) 
    {
    dirs[i] = IJKToRASDirections[i][0];
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetJToRASDirection(double dirs[3])
{
  for (int i=0; i<3; i++) 
    {
    dirs[i] = IJKToRASDirections[i][1];
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetKToRASDirection(double dirs[3])
{
  for (int i=0; i<3; i++) 
    {
    dirs[i] = IJKToRASDirections[i][2];
    }
}

//----------------------------------------------------------------------------
double vtkMRMLVolumeNode::GetMinSpacing()
{
  if (this->GetSpacing() == NULL)
    {
    return 0;
    }
  double minSpace = this->GetSpacing()[0];
  for(int i = 1; i < 3; ++i)
    {
    minSpace = std::min(this->GetSpacing()[i], minSpace);
    }
  return minSpace;
}

//----------------------------------------------------------------------------
double vtkMRMLVolumeNode::GetMaxSpacing()
{
  if (this->GetSpacing() == NULL)
    {
    return 0;
    }
  double maxSpace = this->GetSpacing()[0];
  for(int i = 1; i < 3; ++i)
    {
    maxSpace = std::max(this->GetSpacing()[i], maxSpace);
    }
  return maxSpace;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetIJKToRASMatrix(vtkMatrix4x4* argMat)
{
  if (argMat == NULL) 
    {
    return;
    }
  vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
  mat->DeepCopy(argMat);

  // normalize direction vectors
  double spacing[3];
  int col;
  for (col=0; col<3; col++) 
    {
    double len =0;
    int row;
    for (row=0; row<3; row++) 
      {
      len += mat->GetElement(row, col) * mat->GetElement(row, col);
      }
    len = sqrt(len);
    spacing[col] = len;
    for (row=0; row<3; row++) 
      {
      mat->SetElement(row, col,  mat->GetElement(row, col)/len);
      }
    }

  int row;
  for (row=0; row<3; row++) 
    {
    for (col=0; col<3; col++) 
      {
      this->IJKToRASDirections[row][col] = mat->GetElement(row, col);
      }
    this->Spacing[row] = spacing[row];
    this->Origin[row] = mat->GetElement(row,3);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetRASToIJKMatrix(vtkMatrix4x4* mat)
{
  vtkSmartPointer<vtkMatrix4x4> m = vtkSmartPointer<vtkMatrix4x4>::New();
  m->Identity();
  if (mat) 
  {
    m->DeepCopy(mat);
  }
  m->Invert();
  this->SetIJKToRASMatrix(m);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetIJKToRASMatrix(vtkMatrix4x4* mat)
{
  // this is the full matrix including the spacing and origin
  mat->Identity();
  int row, col;
  for (row=0; row<3; row++) 
    {
    for (col=0; col<3; col++) 
      {
      mat->SetElement(row, col, this->Spacing[col] * IJKToRASDirections[row][col]);
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
void vtkMRMLVolumeNode::GetIJKToRASDirections(double dirs[3][3])
{
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      dirs[i][j] = IJKToRASDirections[i][j];
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetIJKToRASDirectionMatrix(vtkMatrix4x4* ijkToRASDirectionMatrix)
{
  double dirs[3][3];
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      dirs[i][j] = ijkToRASDirectionMatrix->Element[i][j];
      }
    }
  this->SetIJKToRASDirections(dirs);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetIJKToRASDirectionMatrix(vtkMatrix4x4* ijkToRASDirectionMatrix)
{
  double dirs[3][3];
  this->GetIJKToRASDirections(dirs);
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      ijkToRASDirectionMatrix->Element[i][j] = dirs[i][j];
      }
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeNode::ComputeIJKToRASFromScanOrder(const char *order, 
                                                     const double* spacing,
                                                     const int *dims,
                                                     bool centerImage,
                                                     vtkMatrix4x4 *IJKToRAS)
{
  IJKToRAS->Identity();
  if (order == NULL) 
    {
    return false;
    }

  vtkSmartPointer<vtkMatrix4x4> scaleMat = vtkSmartPointer<vtkMatrix4x4>::New();
  scaleMat->Identity();
  scaleMat->SetElement(0,0, spacing[0]);
  scaleMat->SetElement(1,1, spacing[1]);
  scaleMat->SetElement(2,2, spacing[2]);

  vtkSmartPointer<vtkMatrix4x4> orientMat = vtkSmartPointer<vtkMatrix4x4>::New();
  orientMat->Identity();

  if (!strcmp(order,"IS") ||
      !strcmp(order,"Axial IS") ||
      !strcmp(order,  "Axial"))
    {
    double elems[] = { -1,  0,  0,  0,
                        0, -1,  0,  0, 
                        0,  0,  1,  0,
                        0,  0,  0,  1};   
    orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"SI") ||
           !strcmp(order,"Axial SI"))
    {
    double elems[] = { -1,  0,  0,  0,
                        0, -1,  0,  0, 
                        0,  0, -1,  0,
                        0,  0,  0,  1};   
    orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"RL") ||
           !strcmp(order,"Sagittal RL") ||
           !strcmp(order,  "Sagittal"))
    {
    double elems[] = {  0,  0, -1,  0,
                       -1,  0,  0,  0, 
                        0,  -1,  0,  0,
                        0,  0,  0,  1};   
    orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"LR") ||
      !strcmp(order,"Sagittal LR") )
    {
    double elems[] = {  0,  0,  1,  0,
                       -1,  0,  0,  0, 
                        0, -1,  0,  0,
                        0,  0,  0,  1};   
    orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"PA") ||
      !strcmp(order,"Coronal PA") ||
      !strcmp(order,  "Coronal"))
    {
    double elems[] = { -1,  0,  0,  0,
                        0,  0,  1,  0, 
                        0, -1,  0,  0,
                        0,  0,  0,  1};   
    orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"AP") ||
      !strcmp(order,"Coronal AP") )
    {
    double elems[] = { -1,  0,  0,  0,
                        0,  0, -1,  0, 
                        0, -1,  0,  0,
                        0,  0,  0,  1};   
    orientMat->DeepCopy(elems);
    }
  else
    {
    return false;
    }

  vtkMatrix4x4::Multiply4x4(orientMat, scaleMat, IJKToRAS);

  double pnt[] = {-dims[0]/2, -dims[1]/2, -dims[2]/2, 0};

  double *pnt1 = IJKToRAS->MultiplyDoublePoint(pnt);

  if (centerImage)
    {
    for (int j = 0; j < 3; j++)
      {
      IJKToRAS->SetElement(j, 3, pnt1[j]);
      }
    }
  return true;
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeNode::ComputeScanOrderFromIJKToRAS(vtkMatrix4x4 *ijkToRAS)
{
  double dir[4]={0,0,1,0};
  double kvec[4];
 
  ijkToRAS->MultiplyPoint(dir,kvec);
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
      vtkErrorWithObjectMacro (ijkToRAS, "Max components "<< max_comp << " not in valid range 0,1,2\n");
      return "";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetAndObserveImageData(vtkImageData *imageData)
{
  if (imageData == this->ImageData)
    {
    return;
    }
  if (this->ImageData != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->ImageData, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  for (int i=0; i<this->GetNumberOfDisplayNodes(); i++)
    {
    vtkMRMLVolumeDisplayNode *dnode = vtkMRMLVolumeDisplayNode::SafeDownCast(
      this->GetNthDisplayNode(i));
    if (dnode)
      {
      dnode->SetInputImageData(imageData);
      }
    }

  if (imageData != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      imageData, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  this->SetImageData(imageData);
  this->InvokeEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent, NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::OnNodeReferenceAdded(vtkMRMLNodeReference *reference)
{
  this->UpdateDisplayNodeImageData(vtkMRMLDisplayNode::SafeDownCast(reference->ReferencedNode));
  Superclass::OnNodeReferenceAdded(reference);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode:: OnNodeReferenceModified(vtkMRMLNodeReference *reference)
{
  this->UpdateDisplayNodeImageData(vtkMRMLDisplayNode::SafeDownCast(reference->ReferencedNode));
  Superclass::OnNodeReferenceModified(reference);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::UpdateDisplayNodeImageData(vtkMRMLDisplayNode* dNode)
{
  vtkMRMLVolumeDisplayNode* vNode = vtkMRMLVolumeDisplayNode::SafeDownCast(dNode);
  if (vNode)
    {
    vNode->SetInputImageData(this->ImageData);
    }
}

//-----------------------------------------------------------
void vtkMRMLVolumeNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  this->SetAndObserveImageData(this->GetImageData());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  // did the image data change?
  if (this->ImageData && this->ImageData == vtkImageData::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent, NULL);
    return;
    }

  return;
}

//---------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode* vtkMRMLVolumeNode::GetVolumeDisplayNode()
{
  return vtkMRMLVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetMetaDataDictionary( const itk::MetaDataDictionary& dictionary )
{
  this->Dictionary = dictionary;
  this->Modified();
}

//---------------------------------------------------------------------------
const
itk::MetaDataDictionary&
vtkMRMLVolumeNode::GetMetaDataDictionary() const
{
  return this->Dictionary;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode::ApplyTransformMatrix(vtkMatrix4x4* transformMatrix)
{
  vtkSmartPointer<vtkMatrix4x4> ijkToRASMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkSmartPointer<vtkMatrix4x4> newIJKToRASMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

  this->GetIJKToRASMatrix(ijkToRASMatrix);
  vtkMatrix4x4::Multiply4x4(transformMatrix,ijkToRASMatrix,newIJKToRASMatrix);
  
  this->SetIJKToRASMatrix(newIJKToRASMatrix);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetRASBounds(double bounds[6])
{
  Superclass::GetRASBounds( bounds);

  vtkImageData *volumeImage;
  if (! (volumeImage = this->GetImageData()) )
    {
    return;
    }

  //
  // Get the size of the volume in RAS space
  // - map the size of the volume in IJK into RAS
  // - map the middle of the volume to RAS for the center
  //   (IJK space always has origin at first pixel)
  //
  vtkMatrix4x4 *ijkToRAS = vtkMatrix4x4::New();
  this->GetIJKToRASMatrix(ijkToRAS);
  vtkMRMLTransformNode *transformNode = this->GetParentTransformNode();
  if ( transformNode )
    {
    vtkMatrix4x4 *rasToRAS = vtkMatrix4x4::New();;
    transformNode->GetMatrixTransformToWorld(rasToRAS);
    vtkMatrix4x4::Multiply4x4(rasToRAS, ijkToRAS, ijkToRAS);
    rasToRAS->Delete();
    }

  int dimensions[3];
  int i,j,k;
  volumeImage->GetDimensions(dimensions);
  double doubleDimensions[4], rasHDimensions[4];
  double minBounds[3], maxBounds[3];

  for ( i=0; i<3; i++)
    {
    minBounds[i] = 1.0e10;
    maxBounds[i] = -1.0e10;
    }
  for ( i=0; i<2; i++)
    {
    for ( j=0; j<2; j++)
      {
      for ( k=0; k<2; k++)
        {
        doubleDimensions[0] = i*(dimensions[0]) - 0.5;
        doubleDimensions[1] = j*(dimensions[1]) - 0.5 ;
        doubleDimensions[2] = k*(dimensions[2]) - 0.5;
        doubleDimensions[3] = 1;
        ijkToRAS->MultiplyPoint( doubleDimensions, rasHDimensions );
        for (int n=0; n<3; n++) {
          if (rasHDimensions[n] < minBounds[n])
            {
            minBounds[n] = rasHDimensions[n];
            }
          if (rasHDimensions[n] > maxBounds[n])
            {
            maxBounds[n] = rasHDimensions[n];
            }
          }
        }
      }
    }

   ijkToRAS->Delete();
   for ( i=0; i<3; i++)
    {
    bounds[2*i]   = minBounds[i];
    bounds[2*i+1] = maxBounds[i];
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeNode::GetModifiedSinceRead()
{
  return this->Superclass::GetModifiedSinceRead() ||
    (this->ImageData && this->ImageData->GetMTime() > this->GetStoredTime());
}
