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
#include <vtkAlgorithmOutput.h>
#include <vtkAppendPolyData.h>
#include <vtkCallbackCommand.h>
#include <vtkEventForwarderCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkHomogeneousTransform.h>
#include <vtkImageData.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkImageReslice.h>
#include <vtkMathUtilities.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTrivialProducer.h>

#include <algorithm> // For std::min
#include <cassert>

//----------------------------------------------------------------------------

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

  this->ImageDataConnection = NULL;
  this->DataEventForwarder = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode::~vtkMRMLVolumeNode()
{
  this->SetAndObserveImageData(NULL);
  if (this->DataEventForwarder)
    {
    this->DataEventForwarder->Delete();
    }
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
      double dirs[3][3];
      for(int i=0; i<3; i++)
        {
        for(int j=0; j<3; j++)
          {
          ss >> val;
          dirs[i][j] = val;
          }
        }
      this->SetIJKToRASDirections(dirs);
      }
    if (!strcmp(attName, "spacing"))
      {
      std::stringstream ss;
      double val;
      double spacing[3];
      ss << attValue;
      for(int i=0; i<3; i++)
        {
        ss >> val;
        spacing[i] = val;
        }
      this->SetSpacing(spacing);
      }
    if (!strcmp(attName, "origin"))
      {
      std::stringstream ss;
      double val;
      double origin[3];
      ss << attValue;
      for(int i=0; i<3; i++)
        {
        ss >> val;
        origin[i] = val;
        }
      this->SetOrigin(origin);
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
    this->CopyOrientation(node);
    }

  if (node->GetImageData() != NULL)
    {
    // Only copy bulk data if it exists - this handles the case
    // of restoring from SceneViews, where the nodes will not
    // have bulk data.
    this->SetImageDataConnection(node->GetImageDataConnection());
    }

  anode->SetDisableModifiedEvent(amode);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::CopyOrientation(vtkMRMLVolumeNode *node)
{
  double dirs[3][3];
  node->GetIJKToRASDirections(dirs);

  int disabledModify = this->StartModify();
  this->SetIJKToRASDirections(dirs);
  this->SetOrigin(node->GetOrigin());
  this->SetSpacing(node->GetSpacing());
  this->EndModify(disabledModify);
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

  if (this->GetImageData() != NULL)
    {
    os << indent << "ImageData:\n";
    this->GetImageData()->PrintSelf(os, indent.GetNextIndent());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetIJKToRASDirections(double dirs[3][3])
{
  bool isModified = false;
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      if (!vtkMathUtilities::FuzzyCompare<double>(this->IJKToRASDirections[i][j], dirs[i][j]))
        {
        this->IJKToRASDirections[i][j] = dirs[i][j];
        isModified = true;
        }
      }
    }
  if (isModified)
    {
    this->StorableModifiedTime.Modified();
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetIJKToRASDirections(double ir, double jr, double kr,
                                              double ia, double ja, double ka,
                                              double is, double js, double ks)
{
  double dirs[3][3] = {{ir, jr, kr},
                       {ia, ja, ka},
                       {is, js, ks}};
  this->SetIJKToRASDirections(dirs);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetIToRASDirection(double ir, double ia, double is)
{
  double dirs[3][3] = {
    {ir, this->IJKToRASDirections[0][1], this->IJKToRASDirections[0][2]},
    {ia, this->IJKToRASDirections[1][1], this->IJKToRASDirections[1][2]},
    {is, this->IJKToRASDirections[2][1], this->IJKToRASDirections[2][2]}};
  this->SetIJKToRASDirections(dirs);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetJToRASDirection(double jr, double ja, double js)
{
  double dirs[3][3] = {
    {this->IJKToRASDirections[0][0], jr, this->IJKToRASDirections[0][2]},
    {this->IJKToRASDirections[1][0], ja, this->IJKToRASDirections[1][2]},
    {this->IJKToRASDirections[2][0], js, this->IJKToRASDirections[2][2]}};
  this->SetIJKToRASDirections(dirs);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetKToRASDirection(double kr, double ka, double ks)
{
  double dirs[3][3] = {
    {this->IJKToRASDirections[0][0], this->IJKToRASDirections[0][1], kr},
    {this->IJKToRASDirections[1][0], this->IJKToRASDirections[1][1], ka},
    {this->IJKToRASDirections[2][0], this->IJKToRASDirections[2][1], ks}};
  this->SetIJKToRASDirections(dirs);
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
void vtkMRMLVolumeNode::SetSpacing(double arg1, double arg2, double arg3)
{
  if (!vtkMathUtilities::FuzzyCompare<double>(this->Spacing[0], arg1) ||
      !vtkMathUtilities::FuzzyCompare<double>(this->Spacing[1], arg2) ||
      !vtkMathUtilities::FuzzyCompare<double>(this->Spacing[2], arg3))
    {
    this->Spacing[0] = arg1;
    this->Spacing[1] = arg2;
    this->Spacing[2] = arg3;
    this->StorableModifiedTime.Modified();
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetSpacing(double arg[3])
{
  this->SetSpacing(arg[0], arg[1], arg[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetOrigin(double arg1, double arg2, double arg3)
{
  if (!vtkMathUtilities::FuzzyCompare<double>(this->Origin[0], arg1) ||
      !vtkMathUtilities::FuzzyCompare<double>(this->Origin[1], arg2) ||
      !vtkMathUtilities::FuzzyCompare<double>(this->Origin[2], arg3))
    {
    this->Origin[0] = arg1;
    this->Origin[1] = arg2;
    this->Origin[2] = arg3;
    this->StorableModifiedTime.Modified();
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetOrigin(double arg[3])
{
  this->SetOrigin(arg[0], arg[1], arg[2]);
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
  vtkNew<vtkMatrix4x4> mat;
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

  double dirs[3][3];
  double origin[3];
  for (int row=0; row<3; row++)
    {
    for (int col=0; col<3; col++)
      {
      dirs[row][col] = mat->GetElement(row, col);
      }
    origin[row] = mat->GetElement(row, 3);
    }

  int disabledModify = this->StartModify();
  this->SetIJKToRASDirections(dirs);
  this->SetSpacing(spacing);
  this->SetOrigin(origin);
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::SetRASToIJKMatrix(vtkMatrix4x4* mat)
{
  vtkNew<vtkMatrix4x4> m;
  m->Identity();
  if (mat)
  {
    m->DeepCopy(mat);
  }
  m->Invert();
  this->SetIJKToRASMatrix(m.GetPointer());
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

  vtkNew<vtkMatrix4x4> scaleMat;
  scaleMat->Identity();
  scaleMat->SetElement(0,0, spacing[0]);
  scaleMat->SetElement(1,1, spacing[1]);
  scaleMat->SetElement(2,2, spacing[2]);

  vtkNew<vtkMatrix4x4> orientMat;
  orientMat->Identity();

  if (!strcmp(order,"IS") ||
      !strcmp(order,"Axial IS") ||
      !strcmp(order,  "Axial"))
    {
    const double elems[] = { -1,  0,  0,  0,
                        0, -1,  0,  0,
                        0,  0,  1,  0,
                        0,  0,  0,  1};
    orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"SI") ||
           !strcmp(order,"Axial SI"))
    {
    const double elems[] = { -1,  0,  0,  0,
                        0, -1,  0,  0,
                        0,  0, -1,  0,
                        0,  0,  0,  1};
    orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"RL") ||
           !strcmp(order,"Sagittal RL") ||
           !strcmp(order,  "Sagittal"))
    {
    const double elems[] = {  0,  0, -1,  0,
                       -1,  0,  0,  0,
                        0,  -1,  0,  0,
                        0,  0,  0,  1};
    orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"LR") ||
      !strcmp(order,"Sagittal LR") )
    {
    const double elems[] = {  0,  0,  1,  0,
                       -1,  0,  0,  0,
                        0, -1,  0,  0,
                        0,  0,  0,  1};
    orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"PA") ||
      !strcmp(order,"Coronal PA") ||
      !strcmp(order,  "Coronal"))
    {
    const double elems[] = { -1,  0,  0,  0,
                        0,  0,  1,  0,
                        0, -1,  0,  0,
                        0,  0,  0,  1};
    orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"AP") ||
      !strcmp(order,"Coronal AP") )
    {
    const double elems[] = { -1,  0,  0,  0,
                        0,  0, -1,  0,
                        0, -1,  0,  0,
                        0,  0,  0,  1};
    orientMat->DeepCopy(elems);
    }
  else
    {
    return false;
    }

  vtkMatrix4x4::Multiply4x4(orientMat.GetPointer(), scaleMat.GetPointer(), IJKToRAS);

  const double pnt[] = {
   static_cast<double>(-dims[0]/2),
   static_cast<double>(-dims[1]/2),
   static_cast<double>(-dims[2]/2),
   static_cast<double>( 0.0 )};

  const double * const pnt1 = IJKToRAS->MultiplyDoublePoint(pnt);

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
  if (imageData == 0)
    {
    vtkTrivialProducer* oldProducer = vtkTrivialProducer::SafeDownCast(
      this->GetImageDataConnection() ? this->GetImageDataConnection()->GetProducer() : 0);
    if (oldProducer && oldProducer->GetOutputDataObject(0))
      {
      oldProducer->GetOutputDataObject(0)->RemoveObservers(
        vtkCommand::ModifiedEvent, this->DataEventForwarder);
      }
    this->SetImageDataConnection(0);
    }
  else
    {
    vtkTrivialProducer* oldProducer = vtkTrivialProducer::SafeDownCast(
      this->GetImageDataConnection() ? this->GetImageDataConnection()->GetProducer() : 0);
    if (oldProducer && oldProducer->GetOutputDataObject(0) == imageData)
      {
      return;
      }
    else if (oldProducer && oldProducer->GetOutputDataObject(0))
      {
      oldProducer->GetOutputDataObject(0)->RemoveObservers(
        vtkCommand::ModifiedEvent, this->DataEventForwarder);
      }
    vtkNew<vtkTrivialProducer> tp;
    tp->SetOutput(imageData);
    // Propagate ModifiedEvent onto the trivial producer to make sure
    // ImageDataModifiedEvent is triggered.
    if (!this->DataEventForwarder)
      {
      this->DataEventForwarder = vtkEventForwarderCommand::New();
      }
    this->DataEventForwarder->SetTarget(tp.GetPointer());
    imageData->AddObserver(vtkCommand::ModifiedEvent, this->DataEventForwarder);
    this->SetImageDataConnection(tp->GetOutputPort());
    }
}

//---------------------------------------------------------------------------
vtkImageData* vtkMRMLVolumeNode::GetImageData()
{
  vtkAlgorithm* producer = this->ImageDataConnection ?
    this->ImageDataConnection->GetProducer() : 0;
  return vtkImageData::SafeDownCast(
    producer ? producer->GetOutputDataObject(
      this->ImageDataConnection->GetIndex()) : 0);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode
::SetImageDataConnection(vtkAlgorithmOutput *newImageDataConnection)
{
  if (newImageDataConnection == this->ImageDataConnection)
    {
    return;
    }

  vtkAlgorithm* oldImageDataAlgorithm = this->ImageDataConnection ?
    this->ImageDataConnection->GetProducer() : 0;

  this->ImageDataConnection = newImageDataConnection;

  vtkAlgorithm* imageDataAlgorithm = this->ImageDataConnection ?
    this->ImageDataConnection->GetProducer() : 0;
  if (imageDataAlgorithm != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      imageDataAlgorithm, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    imageDataAlgorithm->Register(this);
    }

  this->SetImageDataToDisplayNodes();

  if (oldImageDataAlgorithm != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations (
      oldImageDataAlgorithm, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    oldImageDataAlgorithm->UnRegister(this);
    }

  this->StorableModifiedTime.Modified();
  this->Modified();
  this->InvokeEvent( vtkMRMLVolumeNode::ImageDataModifiedEvent , this);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode
::SetImageDataToDisplayNodes()
{
  int ndisp = this->GetNumberOfDisplayNodes();
  for (int n=0; n<ndisp; n++)
    {
    vtkMRMLVolumeDisplayNode *dnode = vtkMRMLVolumeDisplayNode::SafeDownCast(
      this->GetNthDisplayNode(n));
    if (dnode)
      {
      this->SetImageDataToDisplayNode(dnode);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode
::SetImageDataToDisplayNode(vtkMRMLVolumeDisplayNode* volumeDisplayNode)
{
  assert(volumeDisplayNode);
  volumeDisplayNode->SetInputImageDataConnection(this->GetImageDataConnection());
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::OnNodeReferenceAdded(vtkMRMLNodeReference *reference)
{
  this->UpdateDisplayNodeImageData(vtkMRMLDisplayNode::SafeDownCast(reference->GetReferencedNode()));
  Superclass::OnNodeReferenceAdded(reference);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode:: OnNodeReferenceModified(vtkMRMLNodeReference *reference)
{
  this->UpdateDisplayNodeImageData(vtkMRMLDisplayNode::SafeDownCast(reference->GetReferencedNode()));
  Superclass::OnNodeReferenceModified(reference);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::UpdateDisplayNodeImageData(vtkMRMLDisplayNode* dNode)
{
  vtkMRMLVolumeDisplayNode* vNode = vtkMRMLVolumeDisplayNode::SafeDownCast(dNode);
  if (vNode)
    {
    vNode->SetInputImageDataConnection(this->ImageDataConnection);
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
  if (this->ImageDataConnection != 0 &&
      this->ImageDataConnection->GetProducer() == vtkAlgorithm::SafeDownCast(caller) &&
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
  vtkNew<vtkMatrix4x4> ijkToRASMatrix;
  vtkNew<vtkMatrix4x4> newIJKToRASMatrix;

  this->GetIJKToRASMatrix(ijkToRASMatrix.GetPointer());
  vtkMatrix4x4::Multiply4x4(transformMatrix, ijkToRASMatrix.GetPointer(), newIJKToRASMatrix.GetPointer());

  this->SetIJKToRASMatrix(newIJKToRASMatrix.GetPointer());
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

  vtkNew<vtkGeneralTransform> transform;
  transform->PostMultiply();
  transform->Identity();

  vtkNew<vtkMatrix4x4> ijkToRAS;
  this->GetIJKToRASMatrix(ijkToRAS.GetPointer());
  transform->Concatenate(ijkToRAS.GetPointer());

  vtkMRMLTransformNode *transformNode = this->GetParentTransformNode();

  if ( transformNode )
    {
    vtkNew<vtkGeneralTransform> worldTransform;
    worldTransform->Identity();
    //transformNode->GetTransformFromWorld(worldTransform);
    transformNode->GetTransformToWorld(worldTransform.GetPointer());
    transform->Concatenate(worldTransform.GetPointer());
    }

  int dimensions[3];
  int i,j,k;
  volumeImage->GetDimensions(dimensions);
  double doubleDimensions[4], *rasHDimensions;
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
        rasHDimensions = transform->TransformDoublePoint( doubleDimensions);
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
    (this->GetImageData() && this->GetImageData()->GetMTime() > this->GetStoredTime());
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//-----------------------------------------------------------
void vtkMRMLVolumeNode::ApplyTransform(vtkAbstractTransform* transform)
{
  vtkHomogeneousTransform* linearTransform = vtkHomogeneousTransform::SafeDownCast(transform);
  if (linearTransform)
    {
    this->ApplyTransformMatrix(linearTransform->GetMatrix());
    }
  else
    {
    this->ApplyNonLinearTransform(transform);
    }
  return;
}
//-----------------------------------------------------------
void vtkMRMLVolumeNode::ApplyNonLinearTransform(vtkAbstractTransform* transform)
{
  if (this->GetImageData() == 0 || !this->CanApplyNonLinearTransforms())
    {
    return;
    }
  int extent[6];
  this->GetImageData()->GetExtent(extent);

  vtkNew<vtkMatrix4x4> rasToIJK;

  /** THIS MAY BE NEEDED IF TRANSFORM ORIGIN
  // Compute extents of the output image
  // For each of 6 volume boundary planes:
  // 1. Convert the slice image to a polydata
  // 2. Transform polydata
  // Then append all poly datas and compute RAS extents

  vtkNew<vtkImageDataGeometryFilter> imageDataGeometryFilter;
  imageDataGeometryFilter->SetInput(this->GetImageData());

  vtkNew<vtkGeneralTransform> IJK2WorldTransform;
  IJK2WorldTransform->Identity();
  //IJK2WorldTransform->PostMultiply();

  IJK2WorldTransform->Concatenate(transform);

  this->GetRASToIJKMatrix(rasToIJK.GetPointer());
  rasToIJK->Invert();
  IJK2WorldTransform->Concatenate(rasToIJK.GetPointer());

  vtkNew<vtkTransformPolyDataFilter> transformFilter;
  transformFilter->SetInput(imageDataGeometryFilter->GetOutput());
  transformFilter->SetTransform(IJK2WorldTransform.GetPointer());

  vtkSmartPointer<vtkPolyData> planes[6];

  planes[0] = vtkSmartPointer<vtkPolyData>::New();
  imageDataGeometryFilter->SetExtent(extent[0],extent[0], extent[2],extent[3],extent[4],extent[5]);
  imageDataGeometryFilter->Update();
  transformFilter->Update();
  planes[0]->DeepCopy(transformFilter->GetOutput());

  planes[1] = vtkSmartPointer<vtkPolyData>::New();
  imageDataGeometryFilter->SetExtent(extent[1],extent[1], extent[2],extent[3],extent[4],extent[5]);
  imageDataGeometryFilter->Update();
  transformFilter->Update();
  planes[1]->DeepCopy(transformFilter->GetOutput());

  planes[2] = vtkSmartPointer<vtkPolyData>::New();
  imageDataGeometryFilter->SetExtent(extent[0],extent[1], extent[2],extent[2],extent[4],extent[5]);
  imageDataGeometryFilter->Update();
  transformFilter->Update();
  planes[2]->DeepCopy(transformFilter->GetOutput());

  planes[3] = vtkSmartPointer<vtkPolyData>::New();
  imageDataGeometryFilter->SetExtent(extent[0],extent[1], extent[3],extent[3],extent[4],extent[5]);
  imageDataGeometryFilter->Update();
  transformFilter->Update();
  planes[3]->DeepCopy(transformFilter->GetOutput());

  planes[4] = vtkSmartPointer<vtkPolyData>::New();
  imageDataGeometryFilter->SetExtent(extent[0],extent[1], extent[2],extent[3],extent[4],extent[4]);
  imageDataGeometryFilter->Update();
  transformFilter->Update();
  planes[4]->DeepCopy(transformFilter->GetOutput());

  planes[5] = vtkSmartPointer<vtkPolyData>::New();
  imageDataGeometryFilter->SetExtent(extent[0],extent[1], extent[2],extent[3],extent[5],extent[5]);
  imageDataGeometryFilter->Update();
  transformFilter->Update();
  planes[5]->DeepCopy(transformFilter->GetOutput());

  vtkNew<vtkAppendPolyData> appendPolyData;
  for (int i=0; i<6; i++)
    {
    appendPolyData->AddInput(planes[i]);
    }
  appendPolyData->Update();
  double bounds[6];
  appendPolyData->GetOutput()->ComputeBounds();
  appendPolyData->GetOutput()->GetBounds(bounds);

  ****/

  vtkNew<vtkImageReslice> reslice;
  reslice->GenerateStencilOutputOn();

  vtkNew<vtkGeneralTransform> resampleXform;
  resampleXform->Identity();
  resampleXform->PostMultiply();

  this->GetRASToIJKMatrix(rasToIJK.GetPointer());

  vtkNew<vtkMatrix4x4> IJKToRAS;
  IJKToRAS->DeepCopy(rasToIJK.GetPointer());
  IJKToRAS->Invert();
  transform->Inverse();

  resampleXform->Concatenate(IJKToRAS.GetPointer());
  resampleXform->Concatenate(transform);
  resampleXform->Concatenate(rasToIJK.GetPointer());

  // vtkImageReslice works faster if the input is a linear transform, so try to convert it
  // to a linear transform
  vtkNew<vtkTransform> linearResampleXform;
  if (vtkMRMLTransformNode::IsGeneralTransformLinear(resampleXform.GetPointer(), linearResampleXform.GetPointer()))
    {
    reslice->SetResliceTransform(linearResampleXform.GetPointer());
    }
  else
    {
    reslice->SetResliceTransform(resampleXform.GetPointer());
    }

  reslice->SetInputConnection(this->ImageDataConnection);
  reslice->SetInterpolationModeToLinear();
  reslice->SetBackgroundColor(0, 0, 0, 0);
  reslice->AutoCropOutputOff();
  reslice->SetOptimization(1);

  reslice->SetOutputOrigin( this->GetImageData()->GetOrigin() );
  reslice->SetOutputSpacing( this->GetImageData()->GetSpacing() );
  reslice->SetOutputDimensionality( 3 );

  /***
  double spacing[3];
  double boxBounds[6];

  int dimensions[3];
  double origin[3];

  this->GetOrigin(origin);
  this->GetSpacing(spacing);
  this->GetRASBounds(boxBounds);

  for (int i=0; i<3; i++)
    {
    dimensions[i] = (bounds[2*i+1] - bounds[2*i])/spacing[i];
    }
  reslice->SetOutputExtent( 0, dimensions[0],
                            0, dimensions[1],
                            0, dimensions[2]);
  ***/

  reslice->SetOutputExtent( extent);

  reslice->Update();

  vtkNew<vtkImageData> resampleImage;
  resampleImage->DeepCopy(reslice->GetOutput());

  this->SetAndObserveImageData(resampleImage.GetPointer());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode::ShiftImageDataExtentToZeroStart()
{
  vtkImageData* imageData = this->GetImageData();
  if (!imageData)
    {
    return;
    }

  int extent[6] = {0,-1,0,-1,0,-1};
  imageData->GetExtent(extent);

  // No need to shift if extent already starts at zeros
  if (extent[0] == 0 && extent[2] == 0 && extent[4] == 0)
    {
    return;
    }

  // Shift the origin to the extent's start
  vtkSmartPointer<vtkMatrix4x4> ijkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->GetIJKToRASMatrix(ijkToRasMatrix);
  double shiftedOrigin_IJK[4] = {
    static_cast<double>(extent[0]),
    static_cast<double>(extent[2]),
    static_cast<double>(extent[4]),
    1.0 };
  double shiftedOrigin_RAS[4] = { 0.0, 0.0, 0.0, 1.0 };
  ijkToRasMatrix->MultiplyPoint(shiftedOrigin_IJK, shiftedOrigin_RAS);
  this->SetOrigin(shiftedOrigin_RAS);

  for (int i=0; i<3; ++i)
    {
    extent[2*i+1] -= extent[2*i];
    extent[2*i] = 0;
    }
  imageData->SetExtent(extent);
}
