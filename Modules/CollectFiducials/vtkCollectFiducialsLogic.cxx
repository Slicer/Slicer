/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

//#include "vtkMRMLModelDisplayNode.h"
//#include "vtkMRMLScalarVolumeNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkCollectFiducialsLogic.h"

vtkCxxRevisionMacro(vtkCollectFiducialsLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkCollectFiducialsLogic);

//---------------------------------------------------------------------------
vtkCollectFiducialsLogic::vtkCollectFiducialsLogic()
{

  // Timer Handling

  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkCollectFiducialsLogic::DataCallback);

}


//---------------------------------------------------------------------------
vtkCollectFiducialsLogic::~vtkCollectFiducialsLogic()
{

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

}


//---------------------------------------------------------------------------
void vtkCollectFiducialsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkCollectFiducialsLogic:             " << this->GetClassName() << "\n";

}


//---------------------------------------------------------------------------
void vtkCollectFiducialsLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
  vtkCollectFiducialsLogic *self = reinterpret_cast<vtkCollectFiducialsLogic *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkCollectFiducialsLogic DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkCollectFiducialsLogic::UpdateAll()
{

}

void vtkCollectFiducialsLogic::ResetAllFiducials(const char *ID)
{
  //std::cerr << "DEBUG: Attempting to reset the fiducials." << std::endl;
  if (ID)
  {
    vtkMRMLFiducialListNode *flist =
        vtkMRMLFiducialListNode::SafeDownCast(this->GetApplicationLogic()->GetMRMLScene()->GetNodeByID(ID));
    if (flist == NULL)
      {
      vtkErrorMacro("CollectFiducialsLogic: The fiducial list node doesn't exist.");
      return;
      }
    flist->RemoveAllFiducials();
    //std::cerr << "DEBUG: Fiducial removal successful." << std::endl;
  }
}

void vtkCollectFiducialsLogic::AddFiducial(const char *ID, char* name, float x, float y, float z)
{
  //std::cerr << "DEBUG: Attempting to add fiducial " << name << " to the list." << std::endl;
  if (ID)
  {
    vtkMRMLFiducialListNode *flist =
        vtkMRMLFiducialListNode::SafeDownCast(this->GetApplicationLogic()->GetMRMLScene()->GetNodeByID(ID));
    if (flist == NULL)
      {
      vtkErrorMacro("CollectFiducialsLogic: The fiducial list node doesn't exist.");
      return;
      }
    //flist->AddFiducialWithXYZ(x, y, z, 0);
    flist->AddFiducialWithLabelXYZSelectedVisibility(name, x, y, z, 0, 1);
    //std::cerr << "DEBUG: Fiducial addition successful." << std::endl;
    //flist->PrintSelf(std::cout, vtkIndent(0));
  }
}

void vtkCollectFiducialsLogic::GetNewFiducialMeasure(const char *ID, float *px, float *py, float *pz)
{
  if(ID)
  {
    vtkMRMLLinearTransformNode *xfrm =
        vtkMRMLLinearTransformNode::SafeDownCast(this->GetApplicationLogic()->GetMRMLScene()->GetNodeByID(ID));
    if (xfrm == NULL)
    {
      vtkErrorMacro("CollectFiducialsLogic: The tracker transform node doesn't exist.");
      return;
    }
    *px = xfrm->GetMatrixTransformToParent()->GetElement(0, 3);
    *py = xfrm->GetMatrixTransformToParent()->GetElement(1, 3);
    *pz = xfrm->GetMatrixTransformToParent()->GetElement(2, 3);
  }
}





