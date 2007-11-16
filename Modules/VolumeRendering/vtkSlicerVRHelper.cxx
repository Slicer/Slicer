#include "vtkSlicerVRHelper.h"
#include "vtkVolumeRenderingModuleGUI.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkVolume.h"
#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkCallbackCommand.h"
#include "vtkMatrix4x4.h"
#include "vtkMRMLTransformNode.h"


vtkCxxRevisionMacro(vtkSlicerVRHelper, "$Revision: 1.46 $");
vtkStandardNewMacro(vtkSlicerVRHelper);

vtkSlicerVRHelper::vtkSlicerVRHelper(void)
{
    this->VolumeRenderingCallbackCommand=vtkCallbackCommand::New();
    this->VolumeRenderingCallbackCommand->SetClientData(reinterpret_cast<void *>(this));
    this->VolumeRenderingCallbackCommand->SetCallback(vtkSlicerVRHelper::VolumeRenderingCallback);
    this->Volume=NULL;
}

vtkSlicerVRHelper::~vtkSlicerVRHelper(void)
{
    if(this->Volume!=NULL)
    {
        vtkKWRenderWidget *renderWidget= this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer();
        renderWidget->RemoveViewProp(this->Volume);
        renderWidget->Render();
        this->Volume->Delete();
        this->Volume=NULL;
    }
    if(this->VolumeRenderingCallbackCommand!=NULL)
    {
        this->VolumeRenderingCallbackCommand->Delete();
        this->VolumeRenderingCallbackCommand=NULL;
    }
}

void vtkSlicerVRHelper::InitializePipelineNewCurrentNode()
{
    
   
}
void vtkSlicerVRHelper::Rendering()
{
   
}

void vtkSlicerVRHelper::UpdateRendering()
{
}

void vtkSlicerVRHelper::ShutdownPipeline()
{
   
}


void vtkSlicerVRHelper::VolumeRenderingCallback( vtkObject *caller, unsigned long eid, void *clientData, void *callData )
{
    vtkSlicerVRHelper *self = reinterpret_cast<vtkSlicerVRHelper *>(clientData);


    if (self->GetInVolumeRenderingCallbackFlag()==1)
    {
        //Commented as we us recursion on purpose
        //#ifdef _DEBUG
        //     vtkErrorWithObjectMacro(self,"In vtkVOlumeRendering *********GUICallback called recursively?");
        //#endif

    }

    vtkDebugWithObjectMacro(self, "In vtkVolumeREndering GUICallback");

    self->SetInVolumeRenderingCallbackFlag(1);
    self->ProcessVolumeRenderingEvents(caller, eid, callData);
    self->SetInVolumeRenderingCallbackFlag(0);

}

void vtkSlicerVRHelper::UpdateGUIElements(void)
{
}
void vtkSlicerVRHelper::ProcessVolumeRenderingEvents(vtkObject *caller, unsigned long eid, void *callData)
{
}
void vtkSlicerVRHelper::Init(vtkVolumeRenderingModuleGUI *gui)
{
    this->Gui=gui;
    this->SetApplication(this->Gui->GetApplication());
}

void vtkSlicerVRHelper::CalculateMatrix(vtkMatrix4x4 *output)
{
        //Update matrix
    //Check for NUll Pointer
    if(this->Gui!=NULL&&this->Gui->GetNS_ImageData()!=NULL&&this->Gui->GetNS_ImageData()->GetSelected()!=NULL)
    {
        //IJK to ras
         vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
         vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetIJKToRASMatrix(matrix);

         // Parent transforms
         vtkMRMLTransformNode *tmp=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetParentTransformNode();
         vtkMatrix4x4   *transform=vtkMatrix4x4::New();
         
         tmp->GetMatrixTransformToWorld(transform);
         //Transform world to ras
         vtkMatrix4x4::Multiply4x4(transform,matrix,output);
         matrix->Delete();
         transform->Delete();
    

         //while(tmp!=NULL)
         //{
         //    root=tmp;
         //    tmp=tmp->GetParentTransformNode();
         //}
         ////We don't have a tansform->We only do ijk to ras
         //if(root==NULL)
         //{
         //   vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetIJKToRASMatrix(output);
         //   return;
         //}
         ////We have to take car about all the parent transform nodes
         //else
         //{
         //    vtkMatrix4x4 *transform=vtkMatrix4x4::New();
         //    vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetParentTransformNode()->GetMatrixTransformToNode(root,transfrom);
         //    
         //}
         ////get matrix to root of parent transforms

    }
    else
    {
        vtkErrorMacro("invalid data");
    }
}
