/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkSlicerTransformLogic.h"

// MRML includes
#include "vtkCacheManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"

// VTKsys includes
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

// ITK includes
#include "itkAffineTransform.h"
#include "itkBSplineDeformableTransform.h"
#include "itkCenteredAffineTransform.h"
#include "itkCenteredEuler3DTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkCenteredSimilarity2DTransform.h"
#include "itkEuler2DTransform.h"
#include "itkEuler3DTransform.h"
#include "itkFixedCenterOfRotationAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkQuaternionRigidTransform.h"
#include "itkRigid2DTransform.h"
#include "itkRigid3DPerspectiveTransform.h"
#include "itkRigid3DTransform.h"
#include "itkScalableAffineTransform.h"
#include "itkScaleLogarithmicTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkScaleTransform.h"
#include "itkScaleVersor3DTransform.h"
#include "itkSimilarity2DTransform.h"
#include "itkTranslationTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkVersorTransform.h"
#include "itkTransformFactory.h"

vtkCxxRevisionMacro(vtkSlicerTransformLogic, "$Revision$");
vtkStandardNewMacro(vtkSlicerTransformLogic);

//----------------------------------------------------------------------------
vtkSlicerTransformLogic::vtkSlicerTransformLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTransformLogic::~vtkSlicerTransformLogic()
{
}

//-----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::hardenTransform(vtkMRMLTransformableNode* transformableNode)
{
  vtkMRMLTransformNode* transformNode =
    transformableNode ? transformableNode->GetParentTransformNode() : 0;
  if (!transformNode)
    {
    return false;
    }
  if (transformNode->IsTransformToWorldLinear())
    {
    vtkSmartPointer<vtkMatrix4x4> hardeningMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    transformNode->GetMatrixTransformToWorld(hardeningMatrix);
    transformableNode->ApplyTransformMatrix(hardeningMatrix);
    }
  else
    {
    vtkSmartPointer<vtkGeneralTransform> hardeningTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    transformNode->GetTransformToWorld(hardeningTransform);
    transformableNode->ApplyTransform(hardeningTransform);
    }

  transformableNode->SetAndObserveTransformNodeID(NULL);
  transformableNode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
  transformableNode->SetModifiedSinceRead(1);
  return true;
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkSlicerTransformLogic::AddTransform (const char* filename, vtkMRMLScene *scene)
{
  vtkMRMLTransformStorageNode *storageNode = vtkMRMLTransformStorageNode::New();

  if(scene == NULL){
    vtkErrorMacro("scene == NULL in vtkSlicerTransformLogic::AddTransform");
    return NULL;
  }

  // check for local or remote files
  int useURI = 0; // false;
  if (scene->GetCacheManager() != NULL)
    {
    useURI = scene->GetCacheManager()->IsRemoteReference(filename);
    }

  itksys_stl::string name;
  const char *localFile;
  if (useURI)
    {
    vtkDebugMacro("AddTransforn: file name is remote: " << filename);
    storageNode->SetURI(filename);
    // reset filename to the local file name
    localFile = ((scene)->GetCacheManager())->GetFilenameFromURI(filename);
    }
  else
    {
    storageNode->SetFileName(filename);
    localFile = filename;
    }

  const itksys_stl::string fname(localFile);
  // the model name is based on the file name (itksys call should work even if
  // file is not on disk yet)
  name = itksys::SystemTools::GetFilenameName(fname);

  // check to see which node can read this type of file
  vtkMRMLTransformNode *tnode = NULL;

  if (storageNode->SupportedFileType(name.c_str()))
    {
    scene->SaveStateForUndo();
    storageNode->SetScene(scene);

    // now set up the reading
    vtkMRMLGridTransformNode    *gridTfm = vtkMRMLGridTransformNode::New();
    vtkMRMLBSplineTransformNode *bsplineTfm = vtkMRMLBSplineTransformNode::New();
    vtkMRMLLinearTransformNode  *linearTfm = vtkMRMLLinearTransformNode::New();

    gridTfm->SetScene(scene);
    bsplineTfm->SetScene(scene);
    linearTfm->SetScene(scene);

    if (storageNode->ReadData(gridTfm))
      {
      tnode = gridTfm;
      }
    else if (storageNode->ReadData(bsplineTfm))
      {
      tnode = bsplineTfm;
      }
    else if (storageNode->ReadData(linearTfm))
      {
      tnode = linearTfm;
      }

    if (tnode)
      {
      std::string uname( scene->GetUniqueNameByString(name.c_str()));
      tnode->SetName(uname.c_str());
      scene->AddNode(storageNode);
      scene->AddNode(tnode);

      tnode->SetAndObserveStorageNodeID(storageNode->GetID());
      }
    gridTfm->Delete();
    bsplineTfm->Delete();
    linearTfm->Delete();
    }
  else
    {
    vtkErrorMacro("Unsupported transform file format: " << filename);
    }
  storageNode->Delete();

  return tnode;
}

int vtkSlicerTransformLogic::SaveTransform (const char* vtkNotUsed(filename), 
                                            vtkMRMLTransformNode *vtkNotUsed(transformNode))
{
  return 1;
}

  // Register transform types with ITK factory
  // This code is from Applications/CLI/BRAINSTools/BRAINSCommonLib/GenericTransformImage.cxx
  // We do this in order to register ScaleVersor3DTransform, which is not done
  // in ITK 3.18. The rest of transforms need to be re-registered when a new
  // transform is added. TODO: this must be revised in 3.20, hopefully
  // ScaleVersor3DTransform will be out of Review by then
  void vtkSlicerTransformLogic::RegisterITKTransforms(){
    //This is needed in order to read and write ScaleVersor3D TransformTypes.
    //Hopefully in ITK-3-19 this will become part of the non-review transform types.
    itk::TransformFactory<itk::ScaleVersor3DTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleVersor3DTransform<float> >::RegisterTransform ();

    itk::TransformFactory<itk::AffineTransform<double,2> >::RegisterTransform ();
    itk::TransformFactory<itk::AffineTransform<double,3> >::RegisterTransform ();
    itk::TransformFactory<itk::BSplineDeformableTransform<double,2,2> >::RegisterTransform ();
    itk::TransformFactory<itk::BSplineDeformableTransform<double,3,3> >::RegisterTransform ();
    itk::TransformFactory<itk::CenteredAffineTransform<double,2> >::RegisterTransform ();
    itk::TransformFactory<itk::CenteredAffineTransform<double,3> >::RegisterTransform ();
    itk::TransformFactory<itk::CenteredEuler3DTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::CenteredRigid2DTransform < double > >::RegisterTransform();
    itk::TransformFactory<itk::CenteredSimilarity2DTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::Similarity2DTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::Euler2DTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::Euler3DTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::FixedCenterOfRotationAffineTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::IdentityTransform<double,2> >::RegisterTransform ();
    itk::TransformFactory<itk::IdentityTransform<double,3> >::RegisterTransform ();
    itk::TransformFactory<itk::QuaternionRigidTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::Rigid2DTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::Rigid3DPerspectiveTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::Rigid3DTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::ScalableAffineTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleLogarithmicTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleSkewVersor3DTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleTransform<double,2> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleTransform<double,2> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleTransform<double,3> >::RegisterTransform ();
    itk::TransformFactory<itk::TranslationTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::VersorRigid3DTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::VersorTransform<double> >::RegisterTransform ();
    itk::TransformFactory<itk::AffineTransform<float,2> >::RegisterTransform ();
    itk::TransformFactory<itk::AffineTransform<float,3> >::RegisterTransform ();
    itk::TransformFactory<itk::BSplineDeformableTransform<float,2,2> >::RegisterTransform ();
    itk::TransformFactory<itk::BSplineDeformableTransform<float,3,3> >::RegisterTransform ();
    itk::TransformFactory<itk::CenteredAffineTransform<float,2> >::RegisterTransform ();
    itk::TransformFactory<itk::CenteredAffineTransform<float,3> >::RegisterTransform ();
    itk::TransformFactory<itk::CenteredEuler3DTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::CenteredRigid2DTransform < float > >::RegisterTransform();
    itk::TransformFactory<itk::CenteredSimilarity2DTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::Similarity2DTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::Euler2DTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::Euler3DTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::FixedCenterOfRotationAffineTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::IdentityTransform<float,2> >::RegisterTransform ();
    itk::TransformFactory<itk::IdentityTransform<float,3> >::RegisterTransform ();
    itk::TransformFactory<itk::QuaternionRigidTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::Rigid2DTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::Rigid3DPerspectiveTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::Rigid3DTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::ScalableAffineTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleLogarithmicTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleSkewVersor3DTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleTransform<float,2> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleTransform<float,2> >::RegisterTransform ();
    itk::TransformFactory<itk::ScaleTransform<float,3> >::RegisterTransform ();
    itk::TransformFactory<itk::TranslationTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::VersorRigid3DTransform<float> >::RegisterTransform ();
    itk::TransformFactory<itk::VersorTransform<float> >::RegisterTransform ();
  }
