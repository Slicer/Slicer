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
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAppendPolyData.h>
#include <vtkBoundingBox.h>
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
#include <vector>

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

  this->ImageDataConnection = nullptr;
  this->DataEventForwarder = nullptr;

  this->ContentModifiedEvents->InsertNextValue(vtkMRMLVolumeNode::ImageDataModifiedEvent);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode::~vtkMRMLVolumeNode()
{
  this->SetAndObserveImageData(nullptr);
  if (this->DataEventForwarder)
    {
    this->DataEventForwarder->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

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
  of << " ijkToRASDirections=\"" << ss.str() << "\"";

  of << " spacing=\""
    << this->Spacing[0] << " " << this->Spacing[1] << " " << this->Spacing[2] << "\"";

  of << " origin=\""
    << this->Origin[0] << " " << this->Origin[1] << " " << this->Origin[2] << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
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
void vtkMRMLVolumeNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLVolumeNode* node = vtkMRMLVolumeNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }
  if (deepCopy)
    {
    vtkSmartPointer<vtkImageData> targetImageData = node->GetImageData();
    if (targetImageData.GetPointer() != nullptr)
      {
      targetImageData = vtkSmartPointer<vtkImageData>::Take(node->GetImageData()->NewInstance());
      targetImageData->DeepCopy(node->GetImageData());
      }
    this->SetAndObserveImageData(targetImageData); // invokes vtkMRMLVolumeNode::ImageDataModifiedEvent, which is not masked by StartModify
    }
  else
    {
    // shallow-copy
    this->SetAndObserveImageData(node->GetImageData()); // invokes vtkMRMLVolumeNode::ImageDataModifiedEvent, which is not masked by StartModify
    }

  // targetScalarVolumeNode->SetAndObserveTransformNodeID is not called, as we want to keep the currently applied transform
  this->CopyOrientation(node);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::CopyOrientation(vtkMRMLVolumeNode *node)
{
  MRMLNodeModifyBlocker blocker(this);
  vtkSmartPointer<vtkMatrix4x4> ijkToRasmatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  node->GetIJKToRASMatrix(ijkToRasmatrix);
  this->SetIJKToRASMatrix(ijkToRasmatrix);
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

  if (this->GetImageData() != nullptr)
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
  if (this->GetSpacing() == nullptr)
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
  if (this->GetSpacing() == nullptr)
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
  if (argMat == nullptr)
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
  if (order == nullptr)
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
  if (imageData == nullptr)
    {
    vtkTrivialProducer* oldProducer = vtkTrivialProducer::SafeDownCast(
      this->GetImageDataConnection() ? this->GetImageDataConnection()->GetProducer() : nullptr);
    if (oldProducer && oldProducer->GetOutputDataObject(0))
      {
      oldProducer->GetOutputDataObject(0)->RemoveObservers(
        vtkCommand::ModifiedEvent, this->DataEventForwarder);
      }
    this->SetImageDataConnection(nullptr);
    }
  else
    {
    vtkTrivialProducer* oldProducer = vtkTrivialProducer::SafeDownCast(
      this->GetImageDataConnection() ? this->GetImageDataConnection()->GetProducer() : nullptr);
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
    this->ImageDataConnection->GetProducer() : nullptr;
  return vtkImageData::SafeDownCast(
    producer ? producer->GetOutputDataObject(
      this->ImageDataConnection->GetIndex()) : nullptr);
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
    this->ImageDataConnection->GetProducer() : nullptr;

  this->ImageDataConnection = newImageDataConnection;

  vtkAlgorithm* imageDataAlgorithm = this->ImageDataConnection ?
    this->ImageDataConnection->GetProducer() : nullptr;
  if (imageDataAlgorithm != nullptr)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      imageDataAlgorithm, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    imageDataAlgorithm->Register(this);
    }

  this->SetImageDataToDisplayNodes();

  if (oldImageDataAlgorithm != nullptr)
    {
    vtkEventBroker::GetInstance()->RemoveObservations (
      oldImageDataAlgorithm, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    oldImageDataAlgorithm->UnRegister(this);
    }

  this->StorableModifiedTime.Modified();
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent);
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
  if (this->ImageDataConnection != nullptr &&
      this->ImageDataConnection->GetProducer() == vtkAlgorithm::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent);
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
  this->GetSliceBounds(bounds, nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetSliceBounds(double bounds[6],
  vtkMatrix4x4* rasToSlice, bool useVoxelCenter/*=false*/)
{
  vtkMRMLVolumeNode::GetBoundsInternal(bounds, rasToSlice, true, useVoxelCenter);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetBounds(double bounds[6])
{
  vtkMRMLVolumeNode::GetBoundsInternal(bounds, nullptr, false);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetBoundsInternal(double bounds[6],
  vtkMatrix4x4* rasToSlice, bool useTransform, bool useVoxelCenter/*=false*/)
{
  vtkMath::UninitializeBounds(bounds);
  vtkImageData *volumeImage = this->GetImageData();
  if (!volumeImage)
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

  if ( useTransform && transformNode )
    {
    vtkNew<vtkGeneralTransform> worldTransform;
    worldTransform->Identity();
    transformNode->GetTransformToWorld(worldTransform.GetPointer());
    transform->Concatenate(worldTransform.GetPointer());
    }
  if (rasToSlice)
    {
    transform->Concatenate(rasToSlice);
    }

  int dimensions[3] = { 0 };
  volumeImage->GetDimensions(dimensions);
  double doubleDimensions[4] = { 0, 0, 0, 1 };
  vtkBoundingBox boundingBox;
  for (int i=0; i<2; i++)
    {
    for (int j=0; j<2; j++)
      {
      for (int k=0; k<2; k++)
        {
        if (useVoxelCenter)
          {
          doubleDimensions[0] = i * (dimensions[0] - 1);
          doubleDimensions[1] = j * (dimensions[1] - 1);
          doubleDimensions[2] = k * (dimensions[2] - 1);
          double* rasHDimensions = transform->TransformDoublePoint(doubleDimensions);
          boundingBox.AddPoint(rasHDimensions);
          }
        else
          {
          doubleDimensions[0] = i * dimensions[0] - 0.5;
          doubleDimensions[1] = j * dimensions[1] - 0.5;
          doubleDimensions[2] = k * dimensions[2] - 0.5;
          double* rasHDimensions = transform->TransformDoublePoint(doubleDimensions);
          boundingBox.AddPoint(rasHDimensions);
          }
        }
      }
    }
  boundingBox.GetBounds(bounds);
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
  if (this->GetImageData() == nullptr || !this->CanApplyNonLinearTransforms())
    {
    return;
    }
  int extent[6];
  this->GetImageData()->GetExtent(extent);

  vtkNew<vtkMatrix4x4> rasToIJK;

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
  double backgroundColor[4] = { 0, 0, 0, 0 };
  for (int i = 0; i < 4; i++)
    {
    backgroundColor[i] = this->GetImageBackgroundScalarComponentAsDouble(i);
    }
  reslice->SetBackgroundColor(backgroundColor);

  // Enable AutoCropOutput to compute extent, origin, and spacing to avoid clipping.
  // This requires that extent, origin, and spacing are not set manually
  // (SetOutputOrigin, SetOutputSpacing, SetOutputExtent must not be called).
  reslice->AutoCropOutputOn();

  // Keep output spacing (1,1,1)
  reslice->TransformInputSamplingOff();

  reslice->SetOptimization(1);
  reslice->SetOutputDimensionality(3);

  reslice->Update();

  vtkNew<vtkImageData> resampleImage;
  resampleImage->DeepCopy(reslice->GetOutput());

  // Perform image data and origin update in one step
  int wasModified = this->StartModify();

  // Origin is stored in image node therefore origin in
  // image data object must be set to (0,0,0).
  double resampledOrigin_IJK[4] = { 0, 0, 0, 0 };
  resampleImage->GetOrigin(resampledOrigin_IJK);
  double resampledOrigin_RAS[4] = { 0, 0, 0, 0 };
  IJKToRAS->MultiplyPoint(resampledOrigin_IJK, resampledOrigin_RAS);
  double* origin = this->GetOrigin();
  this->SetOrigin(
    origin[0] + resampledOrigin_RAS[0],
    origin[1] + resampledOrigin_RAS[1],
    origin[2] + resampledOrigin_RAS[2]);
  resampleImage->SetOrigin(0, 0, 0);

  this->SetAndObserveImageData(resampleImage.GetPointer());
  this->EndModify(wasModified);
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

//---------------------------------------------------------------------------
double vtkMRMLVolumeNode::GetImageBackgroundScalarComponentAsDouble(int component)
{
  vtkImageData* imageData = this->GetImageData();
  if (!imageData)
  {
    return 0.0;
  }

  int extent[6] = { 0,-1,0,-1,0,-1 };
  imageData->GetExtent(extent);

  if (extent[0] > extent[1] || extent[2] > extent[3] || extent[4] > extent[5])
  {
    return 0.0;
  }

  std::vector<double> scalarValues;
  scalarValues.push_back(imageData->GetScalarComponentAsDouble(extent[0], extent[2], extent[4], component));
  scalarValues.push_back(imageData->GetScalarComponentAsDouble(extent[0], extent[2], extent[5], component));
  scalarValues.push_back(imageData->GetScalarComponentAsDouble(extent[0], extent[3], extent[4], component));
  scalarValues.push_back(imageData->GetScalarComponentAsDouble(extent[0], extent[3], extent[5], component));
  scalarValues.push_back(imageData->GetScalarComponentAsDouble(extent[1], extent[2], extent[4], component));
  scalarValues.push_back(imageData->GetScalarComponentAsDouble(extent[1], extent[2], extent[5], component));
  scalarValues.push_back(imageData->GetScalarComponentAsDouble(extent[1], extent[3], extent[4], component));
  scalarValues.push_back(imageData->GetScalarComponentAsDouble(extent[1], extent[3], extent[5], component));

  // Get the median value (nth_element performs partial sorting until nth largest element is found)
  const int medianElementIndex = 3;
  std::nth_element(scalarValues.begin(), scalarValues.begin() + medianElementIndex, scalarValues.end());
  double medianValue = scalarValues[medianElementIndex];

  return medianValue;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeNode::CreateDefaultSequenceDisplayNodes()
{
  if (this->GetDisplayNode())
    {
    // there is a display node already
    return;
    }
  this->CreateDefaultDisplayNodes();

  // Turn off auto window/level for scalar volumes (it is costly to compute recommended ww/wl and image would appear to be flickering)
  vtkMRMLScalarVolumeDisplayNode* scalarVolumeDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
  if (scalarVolumeDisplayNode)
    {
    scalarVolumeDisplayNode->AutoWindowLevelOff();
    }
}

//------------------------------------------------------------------------------
bool vtkMRMLVolumeNode::IsCentered()
{
  double centerPosition[3] = { 0.0, 0.0, 0.0 };
  this->GetCenterPositionRAS(centerPosition);
  double tolerance = this->GetMaxSpacing() * 0.1;
  bool centered = (fabs(centerPosition[0]) <= tolerance
    && fabs(centerPosition[1]) <= tolerance
    && fabs(centerPosition[2]) <= tolerance);
  return centered;
}

//------------------------------------------------------------------------------
void vtkMRMLVolumeNode::GetCenterPositionRAS(double* centerPositionRAS, bool useParentTransform/*=true*/)
{
  vtkImageData* imageData = this->GetImageData();
  if (!imageData)
    {
    centerPositionRAS[0] = 0.0;
    centerPositionRAS[1] = 0.0;
    centerPositionRAS[2] = 0.0;
    return;
    }

  int* extent = imageData->GetExtent();
  double centerPositionIJK[4] =
    {
    double(extent[0] + extent[1]) / 2.0,
    double(extent[2] + extent[3]) / 2.0,
    double(extent[4] + extent[5]) / 2.0,
    1.0
    };

  vtkNew<vtkMatrix4x4> ijkToRasMatrix;
  this->GetIJKToRASMatrix(ijkToRasMatrix);
  double centerPositionRAS1[4] = { 0.0, 0.0, 0.0, 1.0 };
  ijkToRasMatrix->MultiplyPoint(centerPositionIJK, centerPositionRAS1);

  if (useParentTransform)
    {
    vtkNew<vtkGeneralTransform> volumeRasToRasTransform;
    vtkMRMLTransformNode::GetTransformBetweenNodes(this->GetParentTransformNode(), nullptr, volumeRasToRasTransform);
    volumeRasToRasTransform->TransformPoint(centerPositionRAS1, centerPositionRAS1);
    }

  centerPositionRAS[0] = centerPositionRAS1[0];
  centerPositionRAS[1] = centerPositionRAS1[1];
  centerPositionRAS[2] = centerPositionRAS1[2];
}

//------------------------------------------------------------------------------
bool vtkMRMLVolumeNode::AddCenteringTransform()
{
  if (this->IsCentered())
    {
    // already centered (without changing parent transform)
    return false;
    }
  this->SetAndObserveTransformNodeID(nullptr);
  if (this->IsCentered())
    {
    // already centered (without adding a transform), no need to add a parent transform
    return true;
    }
  double centerPositionRAS[3];
  this->GetCenterPositionRAS(centerPositionRAS, false);
  vtkMRMLScene* scene = this->GetScene();
  if (!scene)
    {
    vtkErrorMacro("vtkMRMLVolumeNode::AddCenteringTransform failed: invalid scene");
    return false;
    }
  std::string transformName = this->GetName();
  transformName.append(" centering transform");
  vtkMRMLLinearTransformNode* centeringTransform = vtkMRMLLinearTransformNode::SafeDownCast(
    scene->AddNewNodeByClass("vtkMRMLLinearTransformNode", transformName));
  vtkNew<vtkMatrix4x4> centeringTransformMatrix;
  centeringTransformMatrix->SetElement(0, 3, -centerPositionRAS[0]);
  centeringTransformMatrix->SetElement(1, 3, -centerPositionRAS[1]);
  centeringTransformMatrix->SetElement(2, 3, -centerPositionRAS[2]);
  centeringTransform->SetMatrixTransformToParent(centeringTransformMatrix);
  this->SetAndObserveTransformNodeID(centeringTransform->GetID());

  // Place transform in the same subject hierarchy folder as the volume node
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
  if (shNode)
    {
    vtkIdType volumeParentItemId = shNode->GetItemParent(shNode->GetItemByDataNode(this));
    shNode->SetItemParent(shNode->GetItemByDataNode(centeringTransform), volumeParentItemId);
    }
  return true;
}
