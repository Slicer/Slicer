#include "vtkSlicerVRLabelmapHelper.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkVolumeRenderingModuleGui.h"
#include "vtkLabelMapPiecewiseFunction.h"
#include "vtkLabelMapColorTransferFunction.h"
#include "vtkFixedPointVolumeRayCastMapper.h"

vtkCxxRevisionMacro(vtkSlicerVRLabelmapHelper, "$Revision: 1.46 $");
vtkStandardNewMacro(vtkSlicerVRLabelmapHelper);
vtkSlicerVRLabelmapHelper::vtkSlicerVRLabelmapHelper(void)
{
    this->LM_OptionTree=NULL;
    this->MapperRaycast=NULL;
}

vtkSlicerVRLabelmapHelper::~vtkSlicerVRLabelmapHelper(void)
{
    //TODO TAKE CARE ABOUT ALL THE UNPACK STUFF
    if(this->LM_OptionTree)
    {
        this->LM_OptionTree->Delete();
        this->LM_OptionTree=NULL;
    }
    if(this->MapperRaycast!=NULL)
    {
        this->MapperRaycast->Delete();
        this->MapperRaycast=NULL;
    }
}
void vtkSlicerVRLabelmapHelper::Rendering(void)
{
}
void vtkSlicerVRLabelmapHelper::UpdateRendering(void)
{
}
void vtkSlicerVRLabelmapHelper::InitializePipelineNewCurrentNode()
{
  //Labelmap
    if(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetLabelMap()==1)
    {
        vtkLabelMapPiecewiseFunction *opacityNew=vtkLabelMapPiecewiseFunction::New();
        opacityNew->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()),.3,0);
        this->Gui->GetcurrentNode()->GetVolumeProperty()->SetScalarOpacity(opacityNew);
        this->Gui->GetcurrentNode()->SetIsLabelMap(1);
        vtkLabelMapColorTransferFunction *colorNew=vtkLabelMapColorTransferFunction::New();
        colorNew->Init(vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected()));
        this->Gui->GetcurrentNode()->GetVolumeProperty()->SetColor(colorNew);
        this->UpdateLM();

        //Delete
        opacityNew->Delete();
        colorNew->Delete();

    }
}
void vtkSlicerVRLabelmapHelper::UpdateLM()
{
    if(this->LM_OptionTree==NULL)
    {
        vtkErrorMacro("LM Option Tree does not exist");
        return;
    }
}

//TODO FIND a place for this
    //this->LM_OptionTree=vtkSlicerLabelMapWidget::New();
    //this->LM_OptionTree->SetParent(this->detailsFrame->GetFrame());
    //this->LM_OptionTree->Create();
    //((vtkSlicerApplication *)this->GetApplication())->Script("pack %s",this->LM_OptionTree->GetWidgetName());

    //if(this->Helper!=NULL)
    //{
    //    this->Helper->Delete();
    //    this->Helper=NULL;
    //}
