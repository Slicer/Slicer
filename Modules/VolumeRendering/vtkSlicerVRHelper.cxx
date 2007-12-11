#include "vtkSlicerVRHelper.h"
#include "vtkVolumeRenderingModuleGUI.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkVolume.h"
#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkCallbackCommand.h"
#include "vtkMatrix4x4.h"
#include "vtkMRMLTransformNode.h"
#include "vtkKWProgressDialog.h"


vtkCxxRevisionMacro(vtkSlicerVRHelper, "$Revision: 1.46 $");
vtkStandardNewMacro(vtkSlicerVRHelper);

vtkSlicerVRHelper::vtkSlicerVRHelper(void)
{
    this->VolumeRenderingCallbackCommand=vtkCallbackCommand::New();
    this->VolumeRenderingCallbackCommand->SetClientData(reinterpret_cast<void *>(this));
    this->VolumeRenderingCallbackCommand->SetCallback(vtkSlicerVRHelper::VolumeRenderingCallback);
    this->Volume=NULL;
    this->GradientDialog=NULL;
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
    this->DisplayProgressDialog("Preparation: Please stand by");
}

void vtkSlicerVRHelper::CalculateMatrix(vtkMatrix4x4 *output)
{
        //Update matrix
    //Check for NUll Pointer
    if(this->Gui!=NULL&&this->Gui->GetNS_ImageData()!=NULL&&this->Gui->GetNS_ImageData()->GetSelected()!=NULL)
    {
        vtkMRMLTransformNode *tmp=vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetParentTransformNode();
        //check if we have a TransformNode
        if(tmp==NULL)
        {
            vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetIJKToRASMatrix(output);
            return;
        }

        //IJK to ras
         vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
         vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetIJKToRASMatrix(matrix);
         
         // Parent transforms
         vtkMatrix4x4   *transform=vtkMatrix4x4::New();        
         tmp->GetMatrixTransformToWorld(transform);

 
         //Transform world to ras
         vtkMatrix4x4::Multiply4x4(transform,matrix,output);


         matrix->Delete();
         transform->Delete();

    }
    else
    {
        vtkErrorMacro("invalid data");
    }
}

void vtkSlicerVRHelper::DisplayProgressDialog(const char* message)
{
    if(this->GradientDialog!=NULL)
    {
                    //this->GradientDialog->SetMessageText(message);
        return;
    }
                this->GradientDialog = vtkKWProgressDialog::New();
            this->GradientDialog->SetParent (  this->Gui->GetApplicationGUI()->GetMainSlicerWindow());
            this->GradientDialog->SetDisplayPositionToMasterWindowCenter();
            this->GradientDialog->Create ( );
            this->GradientDialog->SetMessageText(message);
            this->GradientDialog->Display();
}

void vtkSlicerVRHelper::WithdrawProgressDialog()
{
    if(this->GradientDialog==NULL)
    {
        return;
    }
        this->GradientDialog->Withdraw();
        this->GradientDialog->SetParent(NULL);
        this->GradientDialog->Delete();
        this->GradientDialog=NULL;
}
