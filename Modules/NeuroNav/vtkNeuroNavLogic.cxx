/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $HeadURL: $
Date:      $Date: $
Version:   $Revision: $

==========================================================================*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkNeuroNavLogic.h"

#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerColorLogic.h"

#include "vtkCylinderSource.h"
#include "vtkSphereSource.h"
#include "vtkAppendPolyData.h"
#include "vtkMultiThreader.h"

#include "vnl/vnl_float_3.h"


vtkCxxRevisionMacro(vtkNeuroNavLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkNeuroNavLogic);

//---------------------------------------------------------------------------
vtkNeuroNavLogic::vtkNeuroNavLogic()
{

  this->SliceDriver[0] = vtkNeuroNavLogic::SLICE_DRIVER_USER;
  this->SliceDriver[1] = vtkNeuroNavLogic::SLICE_DRIVER_USER;
  this->SliceDriver[2] = vtkNeuroNavLogic::SLICE_DRIVER_USER;

  // If the following code doesn't work, slice nodes should be obtained from application GUI
  this->SliceNode[0] = NULL;
  this->SliceNode[1] = NULL;
  this->SliceNode[2] = NULL;

  this->NeedRealtimeImageUpdate0 = 0;
  this->NeedRealtimeImageUpdate1 = 0;
  this->NeedRealtimeImageUpdate2 = 0;
  this->ImagingControl = 0;

  this->EnableOblique = false;
  this->TransformNodeName = NULL;
  this->SliceNo1Last = 1;
  this->SliceNo2Last = 1;
  this->SliceNo3Last = 1;
  this->OriginalTrackerNode = NULL;
  this->UseRegistration = false;

  this->Pat2ImgReg = vtkIGTPat2ImgRegistration::New();
  this->UpdatedTrackerNode = NULL;
}


//---------------------------------------------------------------------------
vtkNeuroNavLogic::~vtkNeuroNavLogic()
{
  if (this->Pat2ImgReg)
    {
    this->Pat2ImgReg->Delete();
    this->Pat2ImgReg = NULL;
    }

  if (this->UpdatedTrackerNode)
    {
    this->UpdatedTrackerNode->Delete();
    this->UpdatedTrackerNode = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkNeuroNavLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkNeuroNavLogic:             " << this->GetClassName() << "\n";

}


//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkNeuroNavLogic::SetVisibilityOfLocatorModel(const char* nodeName, int v)
{
  vtkMRMLModelNode*   locatorModel;
  vtkMRMLDisplayNode* locatorDisp;

  // Check if any node with the specified name exists
  vtkMRMLScene*  scene = this->GetApplicationLogic()->GetMRMLScene();
  vtkCollection* collection = scene->GetNodesByName(nodeName);

  if (collection != NULL && collection->GetNumberOfItems() == 0)
    {
    // if a node doesn't exist
    locatorModel = AddLocatorModel(nodeName, 0.0, 1.0, 1.0);
    }
  else
    {
    locatorModel = vtkMRMLModelNode::SafeDownCast(collection->GetItemAsObject(0));
    }

  if (locatorModel)
    {
    locatorDisp = locatorModel->GetDisplayNode();
    locatorDisp->SetVisibility(v);
    locatorModel->Modified();
    this->GetApplicationLogic()->GetMRMLScene()->Modified();
    }

  return locatorModel;
}


//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkNeuroNavLogic::AddLocatorModel(const char* nodeName, double r, double g, double b)
{

  vtkMRMLModelNode           *locatorModel;
  vtkMRMLModelDisplayNode    *locatorDisp;

  locatorModel = vtkMRMLModelNode::New();
  locatorDisp = vtkMRMLModelDisplayNode::New();

  GetMRMLScene()->SaveStateForUndo();
  GetMRMLScene()->AddNode(locatorDisp);
  GetMRMLScene()->AddNode(locatorModel);  

  locatorDisp->SetScene(this->GetMRMLScene());

  locatorModel->SetName(nodeName);
  locatorModel->SetScene(this->GetMRMLScene());
  locatorModel->SetAndObserveDisplayNodeID(locatorDisp->GetID());
  locatorModel->SetHideFromEditors(0);

  // Cylinder represents the locator stick
  vtkCylinderSource *cylinder = vtkCylinderSource::New();
  cylinder->SetRadius(1.5);
  cylinder->SetHeight(100);
  cylinder->SetCenter(0, 50, 0);
  cylinder->Update();

  // Sphere represents the locator tip 
  vtkSphereSource *sphere = vtkSphereSource::New();
  sphere->SetRadius(3.0);
  sphere->SetCenter(0, 0, 0);
  sphere->Update();

  vtkAppendPolyData *apd = vtkAppendPolyData::New();
  apd->AddInput(sphere->GetOutput());
  apd->AddInput(cylinder->GetOutput());
  apd->Update();

  locatorModel->SetAndObservePolyData(apd->GetOutput());

  double color[3];
  color[0] = r;
  color[1] = g;
  color[2] = b;
  locatorDisp->SetPolyData(locatorModel->GetPolyData());
  locatorDisp->SetColor(color);

  cylinder->Delete();
  sphere->Delete();
  apd->Delete();

  //locatorModel->Delete();
  locatorDisp->Delete();

  return locatorModel;
}



void vtkNeuroNavLogic::GetCurrentPosition(double *px, double *py, double *pz)
{
  *px = 0.0;
  *py = 0.0;
  *pz = 0.0;

  if (! this->OriginalTrackerNode)
    {
    return;
    }

  vtkMatrix4x4* transform;
  //transform = transformNode->GetMatrixTransformToParent();
  transform = this->OriginalTrackerNode->GetMatrixTransformToParent();

  if (transform)
    {
    // set volume orientation
    *px = transform->GetElement(0, 3);
    *py = transform->GetElement(1, 3);
    *pz = transform->GetElement(2, 3);
    }
}



void vtkNeuroNavLogic::UpdateTransformNodeByName(const char *name)
{
  if (name)
    {
    this->SetTransformNodeName(name);

    vtkMRMLScene* scene = this->GetApplicationLogic()->GetMRMLScene();
    vtkCollection* collection = scene->GetNodesByName(this->TransformNodeName);

    if (collection != NULL && collection->GetNumberOfItems() == 0)
      {
      // the node name does not exist in the MRML tree
      return;
      }

    this->OriginalTrackerNode = vtkMRMLLinearTransformNode::SafeDownCast(collection->GetItemAsObject(0));
    if (this->Pat2ImgReg && this->UseRegistration)
      {
      this->UpdateLocatorTransform();
      }
    }
}


void vtkNeuroNavLogic::UpdateFiducialSeeding(const char *name, double offset)
{
  if (name)
    {
    // The following line causes memory leaking.
    // this->GetApplicationLogic()->GetMRMLScene()->SaveStateForUndo();

    vtkMRMLScene* scene = this->GetApplicationLogic()->GetMRMLScene();
    vtkCollection* collection = scene->GetNodesByName(name);

    if (collection != NULL && collection->GetNumberOfItems() == 0)
      {
      vtkErrorMacro("NeuroNavLogic: The node name: (" << name << ") does not exist in the MRML tree");
      return;
      }

    vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(collection->GetItemAsObject(0));
    if (flist == NULL)
      {
      vtkErrorMacro("NeuroNavLogic: The fiducial list node doesn't exist.");
      return;
      }

    double x, y, z;
    this->GetCurrentPosition(&x, &y, &z);
    if (offset != 0.0)
      {
      double len = sqrt(x*x + y*y + z*z);
      double r = (len + offset) / len;
      x = r*x;
      y = r*y;
      z = r*z;
      }

    flist->SetNthFiducialXYZ(0, x, y, z); 
    }
}


void vtkNeuroNavLogic::UpdateDisplay(int sliceNo1, int sliceNo2, int sliceNo3)
{
  if (! this->OriginalTrackerNode)
    {
    return;
    }

  vtkMatrix4x4* transform;
  //transform = transformNode->GetMatrixTransformToParent();
  transform = this->OriginalTrackerNode->GetMatrixTransformToParent();

  if (transform)
    {
    // set volume orientation
    float tx = transform->GetElement(0, 0);
    float ty = transform->GetElement(1, 0);
    float tz = transform->GetElement(2, 0);
    float nx = transform->GetElement(0, 2);
    float ny = transform->GetElement(1, 2);
    float nz = transform->GetElement(2, 2);
    float px = transform->GetElement(0, 3);
    float py = transform->GetElement(1, 3);
    float pz = transform->GetElement(2, 3);

    UpdateSliceNode(sliceNo1, sliceNo2, sliceNo3, 
                    nx, ny, nz, 
                    tx, ty, tz, 
                    px, py, pz);
    }
}


void vtkNeuroNavLogic::UpdateSliceNode(int sliceNo1, int sliceNo2, int sliceNo3,
                                       float nx, float ny, float nz,
                                       float tx, float ty, float tz,
                                       float px, float py, float pz)
{
  CheckSliceNodes();

  if (sliceNo1) // axial driven by User 
    {
    if (sliceNo1 != this->SliceNo1Last)
      {
      this->SliceNode[0]->SetOrientationToAxial();
      this->SliceNode[0]->JumpSlice(0, 0, 0);
      this->SliceNode[0]->UpdateMatrices();
      this->SliceNo1Last = sliceNo1;
      }
    }
  else
    {
    this->SliceNo1Last = sliceNo1;
    if (this->EnableOblique) // perpendicular
      {
      this->SliceNode[0]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 0);
      this->SliceNode[0]->UpdateMatrices();
      }
    else
      {
      this->SliceNode[0]->SetOrientationToAxial();
      this->SliceNode[0]->JumpSlice(px, py, pz);
      this->SliceNode[0]->UpdateMatrices();
      }
    }

  if (sliceNo2) // sagittal driven by User 
    {
    if (sliceNo2 != this->SliceNo2Last)
      {
      this->SliceNode[1]->SetOrientationToSagittal();
      this->SliceNode[1]->JumpSlice(0, 0, 0);
      this->SliceNode[1]->UpdateMatrices();
      this->SliceNo2Last = sliceNo2;
      }
    }
  else
    {
    this->SliceNo2Last = sliceNo2;
    if (this->EnableOblique) // In-Plane
      {
      this->SliceNode[1]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 1);
      this->SliceNode[1]->UpdateMatrices();
      }
    else
      {
      this->SliceNode[1]->SetOrientationToSagittal();
      this->SliceNode[1]->JumpSlice(px, py, pz);
      this->SliceNode[1]->UpdateMatrices();
      }
    }


  if (sliceNo3) // coronal driven by User 
    {
    if (sliceNo3 != this->SliceNo3Last)
      {
      this->SliceNode[2]->SetOrientationToCoronal();
      this->SliceNode[2]->JumpSlice(0, 0, 0);
      this->SliceNode[2]->UpdateMatrices();
      this->SliceNo3Last = sliceNo3;
      }
    }
  else
    {
    this->SliceNo3Last = sliceNo3;
    if (this->EnableOblique)  // In-Plane 90
      {
      this->SliceNode[2]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 2);
      this->SliceNode[2]->UpdateMatrices();
      }
    else
      {
      this->SliceNode[2]->SetOrientationToCoronal();
      this->SliceNode[2]->JumpSlice(px, py, pz);
      this->SliceNode[2]->UpdateMatrices();
      }
    }
}



void vtkNeuroNavLogic::CheckSliceNodes()
{
  if (this->SliceNode[0] == NULL)
    {
    this->SliceNode[0] = this->GetApplicationLogic()
      ->GetSliceLogic("Red")->GetSliceNode();
    }
  if (this->SliceNode[1] == NULL)
    {
    this->SliceNode[1] = this->GetApplicationLogic()
      ->GetSliceLogic("Yellow")->GetSliceNode();
    }
  if (this->SliceNode[2] == NULL)
    {
    this->SliceNode[2] = this->GetApplicationLogic()
      ->GetSliceLogic("Green")->GetSliceNode();
    }
}



void vtkNeuroNavLogic::ApplyTransform(float *position, float *norm, float *transnorm)
{
    // Transform position, norm and transnorm
    // ---------------------------------------------------------
    float p[4];
    float n[4];
    float tn[4];

    for (int i = 0; i < 3; i++)
    {
        p[i] = position[i];
        n[i] = norm[i];
        tn[i] = transnorm[i];
    }
    p[3] = 1;     // translation affects a poistion
    n[3] = 0;     // translation doesn't affect an orientation
    tn[3] = 0;    // translation doesn't affect an orientation

    this->Pat2ImgReg->GetLandmarkTransformMatrix()->MultiplyPoint(p, p);    // transform a position
    this->Pat2ImgReg->GetLandmarkTransformMatrix()->MultiplyPoint(n, n);    // transform an orientation
    this->Pat2ImgReg->GetLandmarkTransformMatrix()->MultiplyPoint(tn, tn);  // transform an orientation

    for (int i = 0; i < 3; i++)
    {
        position[i] = p[i];
        norm[i] = n[i];
        transnorm[i] = tn[i];
    }
}



void vtkNeuroNavLogic::UpdateLocatorTransform()
{

  if (! this->OriginalTrackerNode)
    {
    return;
    }


  vtkMatrix4x4* transform;
  transform = this->OriginalTrackerNode->GetMatrixTransformToParent();
  if (transform)
    {
    // Get locator matrix
    vnl_float_3 p, n, t, c;
    float tt[3], nn[3], pp[3];

    // set volume orientation
    tt[0] = transform->GetElement(0, 0);
    tt[1] = transform->GetElement(1, 0);
    tt[2] = transform->GetElement(2, 0);
    nn[0] = transform->GetElement(0, 2);
    nn[1] = transform->GetElement(1, 2);
    nn[2] = transform->GetElement(2, 2);
    pp[0] = transform->GetElement(0, 3);
    pp[1] = transform->GetElement(1, 3);
    pp[2] = transform->GetElement(2, 3);

    this->ApplyTransform(pp, nn, tt);
    for (int i = 0; i < 3; i++)
      {
      t[i] = tt[i];
      n[i] = nn[i];
      p[i] = pp[i];
      }

    // Ensure N, T orthogonal:
    //    C = N x T
    //    T = C x N
    c = vnl_cross_3d(n, t);
    t = vnl_cross_3d(c, n);

    // Ensure vectors are normalized
    n.normalize();
    t.normalize();
    c.normalize(); 

    /*
# Find transform, N, that brings the locator coordinate frame 
# into the scanner frame.  Then invert N to M and set it to the locator's
# userMatrix to position the locator within the world space.
#
# 1.) Concatenate a translation, T, TO the origin which is (-x,-y,-z)
#     where the locator's position is (x,y,z).
# 2.) Concatenate the R matrix.  If the locator's reference frame has
#     axis Ux, Uy, Uz, then Ux is the TOP ROW of R, Uy is the second, etc.
# 3.) Translate the cylinder so its tip is at the origin instead
#     of the center of its tube.  Call this matrix C.
# Then: N = C*R*T, M = Inv(N)
#
# (See page 419 and 429 of "Computer Graphics", Hearn & Baker, 1997,
#  ISBN 0-13-530924-7)
# 
# The alternative approach used here is to find the transform, M, that
# moves the scanner coordinate frame to the locator's.  
# 
# 1.) Translate the cylinder so its tip is at the origin instead
#     of the center of its tube.  Call this matrix C.
# 2.) Concatenate the R matrix.  If the locator's reference frame has
#     axis Ux, Uy, Uz, then Ux is the LEFT COL of R, Uy is the second,etc.
# 3.) Concatenate a translation, T, FROM the origin which is (x,y,z)
#     where the locator's position is (x,y,z).
# Then: M = T*R*C
*/
    vtkMatrix4x4 *locator_matrix = vtkMatrix4x4::New();
    vtkTransform *locator_transform = vtkTransform::New();

    // Locator's offset: p[0], p[1], p[2]
    float x0 = p[0];
    float y0 = p[1];
    float z0 = p[2];


    // Locator's coordinate axis:
    // Ux = T
    float Uxx = t[0];
    float Uxy = t[1];
    float Uxz = t[2];

    // Uy = -N
    float Uyx = -n[0];
    float Uyy = -n[1];
    float Uyz = -n[2];

    // Uz = Ux x Uy
    float Uzx = Uxy*Uyz - Uyy*Uxz;
    float Uzy = Uyx*Uxz - Uxx*Uyz;
    float Uzz = Uxx*Uyy - Uyx*Uxy;

    // Ux
    locator_matrix->SetElement(0, 0, Uxx);
    locator_matrix->SetElement(1, 0, Uxy);
    locator_matrix->SetElement(2, 0, Uxz);
    locator_matrix->SetElement(3, 0, 0);
    // Uy
    locator_matrix->SetElement(0, 1, Uyx);
    locator_matrix->SetElement(1, 1, Uyy);
    locator_matrix->SetElement(2, 1, Uyz);
    locator_matrix->SetElement(3, 1, 0);
    // Uz
    locator_matrix->SetElement(0, 2, Uzx);
    locator_matrix->SetElement(1, 2, Uzy);
    locator_matrix->SetElement(2, 2, Uzz);
    locator_matrix->SetElement(3, 2, 0);
    // Bottom row
    locator_matrix->SetElement(0, 3, 0);
    locator_matrix->SetElement(1, 3, 0);
    locator_matrix->SetElement(2, 3, 0);
    locator_matrix->SetElement(3, 3, 1);

    // Set the vtkTransform to PostMultiply so a concatenated matrix, C,
    // is multiplied by the existing matrix, M: C*M (not M*C)
    locator_transform->PostMultiply();
    // M = T*R*C


    // NORMAL PART

    locator_transform->Identity();
    // C:
    locator_transform->Translate(0, 0, 0);
    // R:
    locator_transform->Concatenate(locator_matrix);
    // T:
    locator_transform->Translate(x0, y0, z0);

    if (! this->UpdatedTrackerNode)
      {
      this->UpdatedTrackerNode = vtkMRMLLinearTransformNode::New();
      this->UpdatedTrackerNode->SetName("NeuroNavTracker");
      this->UpdatedTrackerNode->SetDescription("Tracker after patient to image registration.");
      GetMRMLScene()->AddNode(this->UpdatedTrackerNode);
      }

    vtkMatrix4x4 *matrix = this->UpdatedTrackerNode->GetMatrixTransformToParent();
//    matrix->DeepCopy(locator_transform->GetMatrix());
    matrix->DeepCopy(this->Pat2ImgReg->GetLandmarkTransformMatrix());

    this->UpdatedTrackerNode->Modified();

    locator_matrix->Delete();
    locator_transform->Delete();
    }
}

int vtkNeuroNavLogic::PerformPatientToImageRegistration()
{
  int error = this->GetPat2ImgReg()->DoRegistration();
  if (error)
    {
    this->SetUseRegistration(0);
    return error;
    }

  this->SetUseRegistration(1);
  return 0;
}


