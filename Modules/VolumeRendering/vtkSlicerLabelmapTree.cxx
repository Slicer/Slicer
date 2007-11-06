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
#include <sstream>
#include <vector>
vtkCxxRevisionMacro(vtkSlicerLabelmapTree, "$Revision: 0.1 $");
vtkStandardNewMacro(vtkSlicerLabelmapTree);
vtkSlicerLabelmapTree::vtkSlicerLabelmapTree(void)
{
}

vtkSlicerLabelmapTree::~vtkSlicerLabelmapTree(void)
{
}
void vtkSlicerLabelmapTree::CreateWidget(void)
{
    Superclass::CreateWidget();
    vtkKWTree *tree=this->GetWidget();
    tree->RedrawOnIdleOn();
    tree->SelectionFillOn();
    tree->SetDeltaY(30);
}

void vtkSlicerLabelmapTree::Init(vtkMRMLScalarVolumeNode *node)
{
    this->Node=node;
    vtkLookupTable *lookup=this->Node->GetVolumeDisplayNode()->GetColorNode()->GetLookupTable();
    vtkTimerLog *timer=vtkTimerLog::New();

    vtkKWHistogram *histo=vtkKWHistogram::New();
    histo->BuildHistogram(node->GetImageData()->GetPointData()->GetScalars(),0);
    timer->StartTimer();
    int counter=0;
    //detect the max size of color Name
    int max=0;
    for(int i=lookup->GetTableRange()[0];i<lookup->GetTableRange()[1];i++)
    {
        std::string colorName=this->Node->GetVolumeDisplayNode()->GetColorNode()->GetColorName(i);
        int tmp=colorName.length();
        if(tmp>max)
        {
            max=tmp;
        }
    }
    for(int i=lookup->GetTableRange()[0];i<lookup->GetTableRange()[1];i++)
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
            element->Init(this->Node->GetVolumeDisplayNode()->GetColorNode()->GetColorName(i),rgb,1,max);
            this->Script("pack %s -side left -anchor c -expand y",element->GetWidgetName());
            element->AddObserver(vtkCommand::AnyEvent,(vtkCommand *) this->BaseTreeCallbackCommand);
            this->GetWidget()->SetNodeWindow(streamA.str().c_str(),element);
            counter++;
        }

    }
    timer->StopTimer();
    vtkErrorMacro("Elapsed time:"<<timer->GetElapsedTime());
    vtkErrorMacro("NumberOfElements:"<<counter);

}
void vtkSlicerLabelmapTree::ProcessBaseTreeEvents(vtkObject *caller, unsigned long eid, void *callData)
{
    this->Script("puts \" event\"");

}
void vtkSlicerLabelmapTree::ChangeAllOpacities(int stage)
{
    for(int i=0;i<this->Elements.size();i++)
    {
        this->Elements[i]->ChangeOpacity(stage);
    }
}
