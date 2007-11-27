#include "vtkSlicerLabelmapTree.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLColorNode.h"
#include "vtkKWTree.h"
#include "vtkSlicerLabelmapElement.h"
#include "vtkTimerLog.h"
#include "vtkKWLabel.h"
#include "vtkImageAccumulate.h"
#include "vtkKWHistogram.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkPointData.h"
#include "vtkCommand.h"
#include "vtkLabelMapPiecewiseFunction.h"
#include "vtkMRMLVolumeRenderingNode.h"
#include <sstream>
#include <vector>
vtkCxxRevisionMacro(vtkSlicerLabelmapTree, "$Revision: 0.1 $");
vtkStandardNewMacro(vtkSlicerLabelmapTree);
vtkSlicerLabelmapTree::vtkSlicerLabelmapTree(void)
{
    this->InChangeOpacityAll=0;
    this->VolumeRenderingNode=NULL;
    this->ScalarVolumeNode=NULL;
    this->RecentColorNodeID="";
}

vtkSlicerLabelmapTree::~vtkSlicerLabelmapTree(void)
{
    for(unsigned int i=0;i<this->Elements.size();i++)
    {
        this->Elements[i]->RemoveObservers(vtkCommand::AnyEvent);

        this->Elements[i]->Delete();
        this->Elements[i]=NULL;
    }
    this->Elements.clear();
}
void vtkSlicerLabelmapTree::CreateWidget(void)
{
    Superclass::CreateWidget();
    vtkKWTree *tree=this->GetWidget();
    tree->RedrawOnIdleOn();
    tree->SelectionFillOff();
    tree->SetDeltaY(30);
}

void vtkSlicerLabelmapTree::Init(vtkMRMLScalarVolumeNode *node,vtkMRMLVolumeRenderingNode *vrnode)
{
    vtkLabelMapPiecewiseFunction *piecewiseFunction=NULL;
    if(this->ScalarVolumeNode!=NULL&&!strcmp(this->RecentColorNodeID.c_str(),this->ScalarVolumeNode->GetVolumeDisplayNode()->GetColorNode()->GetID()))
    {
        vtkErrorMacro("Init already called, use UpdateGUIElementsInstead");
        return;
    }
    this->ScalarVolumeNode=node;
    this->VolumeRenderingNode=vrnode;
    if(this->VolumeRenderingNode!=NULL&&
        this->VolumeRenderingNode->GetVolumeProperty()!=NULL&&
        this->VolumeRenderingNode->GetVolumeProperty()->GetScalarOpacity()!=NULL)
    {
        piecewiseFunction=vtkLabelMapPiecewiseFunction::SafeDownCast(this->VolumeRenderingNode->GetVolumeProperty()->GetScalarOpacity());
    }
    else
    {
        vtkErrorMacro("Volume Rendering Node is not valid");
        return;
    }
    vtkLookupTable *lookup=this->ScalarVolumeNode->GetVolumeDisplayNode()->GetColorNode()->GetLookupTable();
    vtkTimerLog *timer=vtkTimerLog::New();

    vtkKWHistogram *histo=vtkKWHistogram::New();
    histo->BuildHistogram(node->GetImageData()->GetPointData()->GetScalars(),0);
    timer->StartTimer();
    int counter=0;
    //detect the max size of color Name
    int max=0;
    for(int i=(int)lookup->GetTableRange()[0];i<lookup->GetTableRange()[1];i++)
    {
        std::string colorName=this->ScalarVolumeNode->GetVolumeDisplayNode()->GetColorNode()->GetColorName(i);
        int tmp=colorName.length();
        if(tmp>max)
        {
            max=tmp;
        }
    }
    for(int i=(int)lookup->GetTableRange()[0];i<lookup->GetTableRange()[1];i++)
    {
        if(histo->GetOccurenceAtValue(i)>1)
        {
            std::stringstream streamA;
            streamA<<i;
            this->GetWidget()->AddNode("",streamA.str().c_str(),"");

            vtkSlicerLabelmapElement *element=vtkSlicerLabelmapElement::New();
            this->Elements.push_back(element);

            element->SetParent(this->GetWidget());
            element->Create();
            double rgb[3];
            lookup->GetColor(i,rgb);
            //CalculateOpacities
            int opacityLevel=(int)(piecewiseFunction->GetLabel(i)*vtkSlicerLabelmapTree::FACTOR_OPACITY_TO_STAGE);
            if(opacityLevel>5)
            {
                opacityLevel=5;
                vtkWarningMacro("OpacityLevelIsOutOfRange");
            }
            element->Init(i,this->ScalarVolumeNode->GetVolumeDisplayNode()->GetColorNode()->GetColorName(i),rgb,opacityLevel,max);
            this->Script("pack %s -side left -anchor c -expand y",element->GetWidgetName());
            element->AddObserver(vtkCommand::AnyEvent,(vtkCommand *) this->BaseTreeCallbackCommand);
            this->GetWidget()->SetNodeWindow(streamA.str().c_str(),element);
            counter++;
        }

    }
    //Take care about recent id
    this->RecentColorNodeID=this->ScalarVolumeNode->GetVolumeDisplayNode()->GetColorNode()->GetID();

    timer->StopTimer();
    vtkDebugMacro("Elapsed time:"<<timer->GetElapsedTime());
    vtkDebugMacro("NumberOfElements:"<<counter);
    histo->Delete();
    timer->Delete();

}
void vtkSlicerLabelmapTree::ProcessBaseTreeEvents(vtkObject *caller, unsigned long eid, void *callData)
{
    int *callDataInt=(int*)callData;
    vtkLabelMapPiecewiseFunction *piecewiseFunction=NULL;

    if(this->VolumeRenderingNode!=NULL&&
        this->VolumeRenderingNode->GetVolumeProperty()!=NULL&&
        this->VolumeRenderingNode->GetVolumeProperty()->GetScalarOpacity()!=NULL)
    {
        piecewiseFunction=vtkLabelMapPiecewiseFunction::SafeDownCast(this->VolumeRenderingNode->GetVolumeProperty()->GetScalarOpacity());

        piecewiseFunction->EditLabel(callDataInt[0],callDataInt[1]/(double)vtkSlicerLabelmapTree::FACTOR_OPACITY_TO_STAGE);
        if(this->InChangeOpacityAll==0)
        {
            this->InvokeEvent(vtkSlicerLabelmapTree::SingleLabelEdited);
        }
        //Now change color and label if we have a new colornode
        this->RecentColorNodeID=this->ScalarVolumeNode->GetVolumeDisplayNode()->GetColorNode()->GetID();
    }
    else
    {
        vtkErrorMacro("Volume Rendering Node is not valid");
        return;
    }

}
void vtkSlicerLabelmapTree::ChangeAllOpacities(int stage)
{
    vtkLabelMapPiecewiseFunction *piecewiseFunction=NULL;

    if(this->VolumeRenderingNode!=NULL&&
        this->VolumeRenderingNode->GetVolumeProperty()!=NULL&&
        this->VolumeRenderingNode->GetVolumeProperty()->GetScalarOpacity()!=NULL)
    {
        piecewiseFunction=vtkLabelMapPiecewiseFunction::SafeDownCast(this->VolumeRenderingNode->GetVolumeProperty()->GetScalarOpacity());

    }
    else 
    {
        vtkErrorMacro("invalid piecewisefunction");
        return;
    }
    this->InChangeOpacityAll=1;
    for(unsigned int i=0;i<this->Elements.size();i++)
    {               int id=this->Elements[i]->GetId();
    piecewiseFunction->EditLabel(id,stage/(double)vtkSlicerLabelmapTree::FACTOR_OPACITY_TO_STAGE);
    this->Elements[i]->ChangeOpacity(stage);

    }
    this->InChangeOpacityAll=0;
}

void vtkSlicerLabelmapTree::UpdateGuiElements(void)
{
    vtkLabelMapPiecewiseFunction *piecewiseFunction=NULL;

    if(this->VolumeRenderingNode!=NULL&&
        this->VolumeRenderingNode->GetVolumeProperty()!=NULL&&
        this->VolumeRenderingNode->GetVolumeProperty()->GetScalarOpacity()!=NULL)
    {

        //First of all check if we have to redraw:
        if(strcmp(this->RecentColorNodeID.c_str(),this->ScalarVolumeNode->GetVolumeDisplayNode()->GetColorNode()->GetID())!=0)
        {
            //Delete old stuff
            for(unsigned int i=0;i<this->Elements.size();i++)
            {
                this->Elements[i]->RemoveObservers(vtkCommand::AnyEvent);

                this->Elements[i]->Delete();
                this->Elements[i]=NULL;
            }
            this->Elements.clear();
            this->GetWidget()->DeleteAllNodes();
            this->Init(this->ScalarVolumeNode,this->VolumeRenderingNode);
            this->RecentColorNodeID=this->ScalarVolumeNode->GetVolumeDisplayNode()->GetColorNode()->GetID();
        }

        piecewiseFunction=vtkLabelMapPiecewiseFunction::SafeDownCast(this->VolumeRenderingNode->GetVolumeProperty()->GetScalarOpacity());

        for(unsigned int i=0; i<this->Elements.size();i++)
        {
            int id=this->Elements[i]->GetId();
            int opacityLevel=(int) (piecewiseFunction->GetLabel(id)*vtkSlicerLabelmapTree::FACTOR_OPACITY_TO_STAGE);
            this->Elements[i]->ChangeOpacity(opacityLevel);
        }

    }
    else
    {
        vtkErrorMacro("Volume Rendering Node is not valid");
        return;
    }
}
