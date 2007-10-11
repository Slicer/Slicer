#include "vtkSlicerLabelMapWidget.h"

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWTree.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkSlicerApplication.h"

vtkStandardNewMacro (vtkSlicerLabelMapWidget );
vtkCxxRevisionMacro (vtkSlicerLabelMapWidget, "$Revision: 1.0 $");
vtkSlicerLabelMapWidget::vtkSlicerLabelMapWidget(void)
{
}

vtkSlicerLabelMapWidget::~vtkSlicerLabelMapWidget(void)
{
}

void vtkSlicerLabelMapWidget::CreateWidget(void)
{
      vtkKWTreeWithScrollbars *widgets_tree =vtkKWTreeWithScrollbars::New();
     widgets_tree->SetParent(this->GetParent());
    widgets_tree->Create();
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",widgets_tree->GetWidgetName());
    vtkKWTree *tree=widgets_tree->GetWidget();
    tree->RedrawOnIdleOn();
    tree->SelectionFillOn();
    tree->SetDeltaY(30);
    for(int i=1;i<50;i++)
    {
        std::stringstream str;
        std::stringstream str1;
        str1<<i;
        str<<"test"<<i;
        tree->AddNode("",str1.str().c_str(),str.str().c_str());
        tree->OpenNode(str1.str().c_str());
        tree->SetNodeSelectableFlag(str1.str().c_str(),0);
        tree->SetNodeFontWeightToBold(str1.str().c_str());
        vtkKWCompositeWidget *frame=vtkKWCompositeWidget::New();
        frame->SetParent(tree);
        frame->Create();
        vtkKWPushButton *pushB=vtkKWPushButton::New();
        pushB->SetParent(frame);
        pushB->SetText(str.str().c_str());
        pushB->Create();
        this->Script("pack %s -side left -anchor nw -fill x -padx 2 -pady 2",pushB->GetWidgetName());

        vtkKWLabel *label=vtkKWLabel::New();
        label->SetParent(frame);
        label->SetText(str.str().c_str());
        label->Create();
        label->SetBackgroundColor(1-i/100.,1-i/100.,1-i/100.);

        this->Script("pack %s -side right -anchor nw -fill x -padx 2 -pady 2",label->GetWidgetName());
        tree->SetNodeWindow(str1.str().c_str(),frame);
        pushB->Delete();
        label->Delete();
        frame->Delete();
        for(int j=i*1000;j<(i*1000+3);j++)
        {
            std::stringstream Cstr;
            std::stringstream Cstr1;
            Cstr1<<j;
            Cstr<<"testChild"<<j;
            tree->AddNode(str1.str().c_str(),Cstr1.str().c_str(),Cstr.str().c_str());
            tree->OpenNode(Cstr1.str().c_str());
            tree->SetNodeSelectableFlag(Cstr1.str().c_str(),0);
            tree->SetNodeFontWeightToBold(Cstr1.str().c_str());
            vtkKWCompositeWidget *frameA=vtkKWCompositeWidget::New();
            frameA->SetParent(tree);
            frameA->Create();
            vtkKWPushButton *pushBA=vtkKWPushButton::New();
            pushBA->SetParent(frameA);
            pushBA->SetText(Cstr.str().c_str());
            pushBA->Create();
            this->Script("pack %s -side left -anchor nw -fill x -padx 2 -pady 2",pushBA->GetWidgetName());

            vtkKWLabel *labelA=vtkKWLabel::New();
            labelA->SetParent(frameA);
            labelA->SetText(Cstr.str().c_str());
            labelA->Create();
            labelA->SetBackgroundColor(1-i/100.,1-i/100.,1-i/100.);

            this->Script("pack %s -side right -anchor nw -fill x -padx 2 -pady 2",labelA->GetWidgetName());
            tree->SetNodeWindow(Cstr1.str().c_str(),frameA);
            pushBA->Delete();
            labelA->Delete();
            frameA->Delete();
        }
    }
    widgets_tree->Delete();
    //end test of widgets_tree
}
