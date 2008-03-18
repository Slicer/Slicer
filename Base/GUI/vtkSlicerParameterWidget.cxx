#include "vtkSlicerParameterWidget.h"

#include <vtkObjectFactory.h>
#include <vtkKWWidget.h>
#include <vtkKWCoreWidget.h>

#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include "vtkSmartPointer.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWLabelWithLabel.h"
#include "vtkKWSpinBox.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWTopLevel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWRadioButtonSetWithLabel.h"
#include "vtkKWText.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkKWMessage.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWWindowBase.h"
#include <vtkMRMLNode.h>
#include <vtkKWLabel.h>

#include "itkNumericTraits.h"

#include <ModuleDescription.h>
#include <ModuleParameterGroup.h>
#include <ModuleParameter.h>


#include <string>
#include <vector>
#include <sstream>
#include <iostream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerParameterWidget );
//vtkCxxRevisionMacro(vtkSlicerParameteWidget, "$Revision: 1.33 $");
//----------------------------------------------------------------------------

vtkSlicerParameterWidget::vtkSlicerParameterWidget()
{
    this->m_ModuleDescription = NULL;
    this->m_ModuleLogic = NULL;
    this->m_ParentWidget = NULL;
    this->m_InternalWidgetParamList = NULL;
    this->m_widgID = "";
    this->m_MRMLNode = NULL;
    
    this->m_paramToErrorMap = NULL;
    this->Initialize();
}

vtkSlicerParameterWidget::~vtkSlicerParameterWidget()
{
    if(this->m_paramToErrorMap)
    {
        this->m_paramToErrorMap->clear();
        this->m_paramToErrorMap = NULL;
    }
    
    this->DeleteInternalLists();
    
    this->m_ParentWidget = NULL;
    this->m_ModuleDescription = NULL;
    this->m_ModuleLogic = NULL;    
}

void vtkSlicerParameterWidget::Initialize()
{
    this->m_Created = false;
    this->m_CurrentIndex = -1;
    this->m_End = true;
}

int vtkSlicerParameterWidget::CreateWidgets()
{
    this->DeleteInternalLists();
    this->Initialize();
    
    if(!this->GetApplication())
    {
        std::cout<<"vtkSlicerParameterWidget: Application is not set!"<<std::endl;
        return PARAMETER_WIDGET_ERR;
    }    
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    
    if(!this->m_ModuleDescription)
    {
        std::cout<<"vtkSlicerParameterWidget: ModuleDescription is not set!"<<std::endl;
        return PARAMETER_WIDGET_FAIL;
    }
    
    if(!this->GetMRMLNode())
    {
        std::cout<<"vtkSlicerParameterWidget: MRMLNode is not set!"<<std::endl;
        return PARAMETER_WIDGET_ERR;
    }
    
    if(!this->m_ParentWidget)
    {
        std::cout<<this->GetApplication()->GetNumberOfWindows()<<std::endl;
        vtkKWWindowBase *parentWindows = this->GetApplication()->GetNthWindow(0);
        vtkKWWidget *parentWidget = parentWindows->GetViewFrame();
        if(!parentWidget)
            return PARAMETER_WIDGET_ERR;
        else
            this->m_ParentWidget = parentWidget;
    }
    
    std::string title = this->m_ModuleDescription->GetTitle();
    
    vtkSlicerParameterWidget::moduleParameterWidgetStruct *curModWidgetStruct = NULL;
    
    this->m_InternalWidgetParamList = new std::vector<vtkSlicerParameterWidget::moduleParameterWidgetStruct*>;
//    this->m_internalWidgetToParamMap = new std::map<vtkKWCoreWidget*, ModuleParameter>;
    
    // iterate over each parameter group
    std::vector<ModuleParameterGroup>::const_iterator pgbeginit
      = this->m_ModuleDescription->GetParameterGroups().begin();
    std::vector<ModuleParameterGroup>::const_iterator pgendit
      = this->m_ModuleDescription->GetParameterGroups().end();
    std::vector<ModuleParameterGroup>::const_iterator pgit;
    
    for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
        curModWidgetStruct = new vtkSlicerParameterWidget::moduleParameterWidgetStruct; 
        curModWidgetStruct->modParams = new std::vector<ModuleParameter>;
        curModWidgetStruct->paramWidget = NULL;
        
        // each parameter group is its own labeled frame
        vtkKWFrame *parameterGroupFrame = vtkKWFrame::New ( );
        parameterGroupFrame->SetParent ( this->m_ParentWidget );
        parameterGroupFrame->Create ( );
//      parameterGroupFrame->SetLabelText ((*pgit).GetLabel().c_str());
//      if ((*pgit).GetAdvanced() == "true")
//        {
//        parameterGroupFrame->CollapseFrame ( );
//        }
      
        std::string parameterGroupBalloonHelp = (*pgit).GetDescription();
        parameterGroupFrame
            ->SetBalloonHelpString(parameterGroupBalloonHelp.c_str());

//      app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
//                    parameterGroupFrame->GetWidgetName() );

      // Store the parameter group frame in a SmartPointer
//      (*this->InternalWidgetMap)[(*pgit).GetLabel()] = parameterGroupFrame;
        
        // iterate over each parameter in this group
        std::vector<ModuleParameter>::const_iterator pbeginit
            = (*pgit).GetParameters().begin();
        std::vector<ModuleParameter>::const_iterator pendit
            = (*pgit).GetParameters().end();
        std::vector<ModuleParameter>::const_iterator pit;

        int pcount;
        for (pcount = 0, pit = pbeginit; pit != pendit; ++pit, ++pcount)
        {
            // switch on the type of the parameter...
            vtkKWCoreWidget *parameter;

            if ((*pit).GetTag() == "integer")
            {
                if ((*pit).GetConstraints() == "")
                {
                    vtkKWSpinBoxWithLabel
                            *tparameter = vtkKWSpinBoxWithLabel::New();
                    tparameter->SetParent( parameterGroupFrame);
                    tparameter->Create();
                    tparameter->SetLabelText((*pit).GetLabel().c_str());
                    tparameter->GetWidget()->SetRestrictValueToInteger();
                    tparameter->GetWidget()->SetIncrement(1);
                    tparameter->GetWidget()->SetValue(atof((*pit).GetDefault().c_str()));
                    tparameter->GetWidget()
                    ->SetRange(itk::NumericTraits<int>::NonpositiveMin(),
                            itk::NumericTraits<int>::max());
                    parameter = tparameter;
                }
                else
            {
                    int min, max, step;
                    if ((*pit).GetMinimum()!= "") {
                        min = atoi((*pit).GetMinimum().c_str());
                    } else {
                        min = itk::NumericTraits<int>::NonpositiveMin();
                    }
                    if ((*pit).GetMaximum()!= "") {
                        max = atoi((*pit).GetMaximum().c_str());
                    } else {
                        max = itk::NumericTraits<int>::max();
                    }
                    if ((*pit).GetStep()!= "") {
                        step = atoi((*pit).GetStep().c_str());
                    } else {
                        step = 1;
                    }

                    vtkKWScaleWithEntry
                            *tparameter = vtkKWScaleWithEntry::New();
                    tparameter->SetParent( parameterGroupFrame);
                    tparameter->PopupModeOn();
                    tparameter->Create();
                    tparameter->SetLabelText((*pit).GetLabel().c_str());
                    tparameter->RangeVisibilityOn();
                    tparameter->SetRange(min, max);
                    tparameter->SetValue(atof((*pit).GetDefault().c_str()));
                    tparameter->SetResolution(step);
                    parameter = tparameter;
                }
            } else if ((*pit).GetTag()== "boolean") {
                vtkKWCheckButtonWithLabel
                        *tparameter = vtkKWCheckButtonWithLabel::New();
                tparameter->SetParent( parameterGroupFrame);
                tparameter->Create();
                tparameter->SetLabelText((*pit).GetLabel().c_str());
                tparameter->GetWidget()->SetSelectedState((*pit).GetDefault()== "true" ? 1 : 0);
                parameter = tparameter;
            } else if ((*pit).GetTag()== "float") {
                if ((*pit).GetConstraints()== "") {
                    vtkKWSpinBoxWithLabel
                            *tparameter = vtkKWSpinBoxWithLabel::New();
                    tparameter->SetParent( parameterGroupFrame);
                    tparameter->Create();
                    tparameter->SetLabelText((*pit).GetLabel().c_str());
                    tparameter->GetWidget()
                    ->SetRange(itk::NumericTraits<float>::NonpositiveMin(),
                            itk::NumericTraits<float>::max());
                    tparameter->GetWidget()->SetIncrement( 0.1);
                    tparameter->GetWidget()->SetValueFormat("%1.1f");
                    tparameter->GetWidget()->SetValue(atof((*pit).GetDefault().c_str()));
                    parameter = tparameter;
                } else {
                    double min, max, step;
                    if ((*pit).GetMinimum()!= "") {
                        min = atof((*pit).GetMinimum().c_str());
                    } else {
                        min = itk::NumericTraits<float>::NonpositiveMin();
                    }
                    if ((*pit).GetMaximum()!= "") {
                        max = atof((*pit).GetMaximum().c_str());
                    } else {
                        max = itk::NumericTraits<float>::max();
                    }
                    if ((*pit).GetStep()!= "") {
                        step = atof((*pit).GetStep().c_str());
                    } else {
                        step = 0.1;
                    }

                    vtkKWScaleWithEntry *tparameter= vtkKWScaleWithEntry::New();
                    tparameter->SetParent( parameterGroupFrame);
                    tparameter->PopupModeOn();
                    tparameter->Create();
                    tparameter->SetLabelText((*pit).GetLabel().c_str());
                    tparameter->RangeVisibilityOn();
                    tparameter->SetRange(min, max);
                    tparameter->SetResolution(step);
                    tparameter->SetValue(atof((*pit).GetDefault().c_str()));
                    parameter = tparameter;
                }
            } else if ((*pit).GetTag()== "double") {
                if ((*pit).GetConstraints()== "") {
                    vtkKWSpinBoxWithLabel
                            *tparameter = vtkKWSpinBoxWithLabel::New();
                    tparameter->SetParent( parameterGroupFrame);
                    tparameter->Create();
                    tparameter->SetLabelText((*pit).GetLabel().c_str());
                    tparameter->GetWidget()
                    ->SetRange(itk::NumericTraits<double>::NonpositiveMin(),
                            itk::NumericTraits<double>::max());
                    tparameter->GetWidget()->SetIncrement( 0.1);
                    tparameter->GetWidget()->SetValueFormat("%1.1f");
                    tparameter->GetWidget()->SetValue(atof((*pit).GetDefault().c_str()));
                    parameter = tparameter;
                } else {
                    double min, max, step;
                    if ((*pit).GetMinimum()!= "") {
                        min = atof((*pit).GetMinimum().c_str());
                    } else {
                        min = itk::NumericTraits<double>::NonpositiveMin();
                    }
                    if ((*pit).GetMaximum()!= "") {
                        max = atof((*pit).GetMaximum().c_str());
                    } else {
                        max = itk::NumericTraits<double>::max();
                    }
                    if ((*pit).GetStep()!= "") {
                        step = atof((*pit).GetStep().c_str());
                    } else {
                        step = 0.1;
                    }

                    vtkKWScaleWithEntry *tparameter= vtkKWScaleWithEntry::New();
                    tparameter->SetParent( parameterGroupFrame);
                    tparameter->PopupModeOn();
                    tparameter->Create();
                    tparameter->SetLabelText((*pit).GetLabel().c_str());
                    tparameter->RangeVisibilityOn();
                    tparameter->SetRange(min, max);
                    tparameter->SetResolution(step);
                    tparameter->SetValue(atof((*pit).GetDefault().c_str()));
                    parameter = tparameter;
                }
            } else if ((*pit).GetTag()== "string"|| (*pit).GetTag()== "integer-vector"|| (*pit).GetTag()== "float-vector"|| (*pit).GetTag()== "double-vector"|| (*pit).GetTag()== "string-vector") {
                vtkKWEntryWithLabel *tparameter = vtkKWEntryWithLabel::New();
                tparameter->SetParent( parameterGroupFrame);
                tparameter->Create();
                tparameter->SetLabelText((*pit).GetLabel().c_str());
                tparameter->GetWidget()->SetValue((*pit).GetDefault().c_str());
                parameter = tparameter;
            } else if ((*pit).GetTag()== "point") {
                vtkSlicerNodeSelectorWidget
                        *tparameter= vtkSlicerNodeSelectorWidget::New();

                tparameter->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL,
                        (title + " FiducialList").c_str());
                tparameter->SetNewNodeEnabled(1);
                tparameter->SetNoneEnabled(1);
                //          tparameter->SetNewNodeName((title+" output").c_str());
                tparameter->SetParent( parameterGroupFrame);
                tparameter->Create();
                tparameter->SetMRMLScene(this->m_ModuleLogic->GetMRMLScene());
                tparameter->UpdateMenu();

                tparameter->SetBorderWidth(2);
                tparameter->SetReliefToFlat();
                tparameter->SetLabelText( (*pit).GetLabel().c_str());
                parameter = tparameter;
            } else if ((*pit).GetTag()== "image" && (*pit).GetChannel()== "input") {
                vtkSlicerNodeSelectorWidget
                        *tparameter= vtkSlicerNodeSelectorWidget::New();
                std::string labelAttrName("LabelMap");
                std::string labelAttrValue("1");
                std::string nodeClass;
                const char *attrName = 0;
                const char *attrValue = 0;
                if ((*pit).GetType()== "label") {
                    nodeClass = "vtkMRMLScalarVolumeNode";
                    attrName = labelAttrName.c_str();
                    attrValue = labelAttrValue.c_str();
                } else if ((*pit).GetType()== "vector") {
                    nodeClass = "vtkMRMLVectorVolumeNode";
                } else if ((*pit).GetType()== "tensor") {
                    nodeClass = "vtkMRMLDiffusionTensorVolumeNode";
                } else if ((*pit).GetType()== "diffusion-weighted") {
                    nodeClass = "vtkMRMLDiffusionWeightedVolumeNode";
                } else {
                    nodeClass = "vtkMRMLScalarVolumeNode";
                }

                tparameter->SetNodeClass(nodeClass.c_str(), attrName,
                        attrValue, (title + " Volume").c_str());
                tparameter->SetParent( parameterGroupFrame);
                tparameter->Create();
                tparameter->SetMRMLScene(this->m_ModuleLogic->GetMRMLScene());
                tparameter->UpdateMenu();

                tparameter->SetBorderWidth(2);
                tparameter->SetReliefToFlat();
                tparameter->SetLabelText( (*pit).GetLabel().c_str());
                parameter = tparameter;
            } else if ((*pit).GetTag()== "image" && (*pit).GetChannel()== "output") {
                vtkSlicerNodeSelectorWidget
                        *tparameter= vtkSlicerNodeSelectorWidget::New();
                std::string labelAttrName("LabelMap");
                std::string labelAttrValue("1");
                std::string nodeClass;
                const char *attrName = 0;
                const char *attrValue = 0;
                if ((*pit).GetType()== "label") {
                    nodeClass = "vtkMRMLScalarVolumeNode";
                    attrName = labelAttrName.c_str();
                    attrValue = labelAttrValue.c_str();
                } else if ((*pit).GetType()== "vector") {
                    nodeClass = "vtkMRMLVectorVolumeNode";
                } else if ((*pit).GetType()== "tensor") {
                    nodeClass = "vtkMRMLDiffusionTensorVolumeNode";
                } else if ((*pit).GetType()== "diffusion-weighted") {
                    nodeClass = "vtkMRMLDiffusionWeightedVolumeNode";
                } else {
                    nodeClass = "vtkMRMLScalarVolumeNode";
                }

                tparameter->SetNodeClass(nodeClass.c_str(), attrName,
                        attrValue, (title + " Volume").c_str());
                tparameter->SetNewNodeEnabled(1);
                tparameter->SetNoneEnabled(1);
                //          tparameter->SetNewNodeName((title+" output").c_str());
                tparameter->SetParent( parameterGroupFrame);
                tparameter->Create();
                tparameter->SetMRMLScene(this->m_ModuleLogic->GetMRMLScene());
                tparameter->UpdateMenu();

                tparameter->SetBorderWidth(2);
                tparameter->SetReliefToFlat();
                tparameter->SetLabelText( (*pit).GetLabel().c_str());
                parameter = tparameter;
            } else if ((*pit).GetTag()== "geometry" && (*pit).GetChannel()== "input") {
                vtkSlicerNodeSelectorWidget
                        *tparameter= vtkSlicerNodeSelectorWidget::New();
                tparameter->SetNodeClass("vtkMRMLModelNode", NULL, NULL, (title + " Model").c_str());
                tparameter->SetParent( parameterGroupFrame);
                tparameter->Create();
                tparameter->SetMRMLScene(this->m_ModuleLogic->GetMRMLScene());
                tparameter->UpdateMenu();

                tparameter->SetBorderWidth(2);
                tparameter->SetReliefToFlat();
                tparameter->SetLabelText( (*pit).GetLabel().c_str());
                parameter = tparameter;
            } else if ((*pit).GetTag()== "geometry" && (*pit).GetChannel()=="output") {
                vtkSlicerNodeSelectorWidget
                        *tparameter= vtkSlicerNodeSelectorWidget::New();

                tparameter->SetNodeClass("vtkMRMLModelNode", NULL, NULL, (title + " Model").c_str());
                tparameter->SetNewNodeEnabled(1);
                tparameter->SetNoneEnabled(1);
                //          tparameter->SetNewNodeName((title+" output").c_str());
                tparameter->SetParent( parameterGroupFrame);
                tparameter->Create();
                tparameter->SetMRMLScene(this->m_ModuleLogic->GetMRMLScene());
                tparameter->UpdateMenu();

                tparameter->SetBorderWidth(2);
                tparameter->SetReliefToFlat();
                tparameter->SetLabelText( (*pit).GetLabel().c_str());
                parameter = tparameter;
            } else if ((*pit).GetTag()== "directory") {
                vtkKWLoadSaveButtonWithLabel
                        *tparameter= vtkKWLoadSaveButtonWithLabel::New();
                tparameter->SetParent( parameterGroupFrame);
                tparameter->Create();
                if ((*pit).GetChannel()== "output") {
                    tparameter->GetWidget()->GetLoadSaveDialog()->SaveDialogOn();
                }
                tparameter->SetLabelText( (*pit).GetLabel().c_str());
                tparameter->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
                tparameter->GetWidget()->GetLoadSaveDialog()->SetInitialFileName( (*pit).GetDefault().c_str());
                parameter = tparameter;
            } else if ((*pit).GetTag()== "file") {
                vtkKWLoadSaveButtonWithLabel
                        *tparameter= vtkKWLoadSaveButtonWithLabel::New();
                tparameter->SetParent( parameterGroupFrame);
                if ((*pit).GetChannel()== "output") {
                    tparameter->GetWidget()->GetLoadSaveDialog()->SaveDialogOn();
                }
                tparameter->Create();
                tparameter->SetLabelText( (*pit).GetLabel().c_str());
                tparameter->GetWidget()->GetLoadSaveDialog()->SetInitialFileName( (*pit).GetDefault().c_str());
                parameter = tparameter;
            } else if ((*pit).GetTag()== "string-enumeration"|| (*pit).GetTag()== "integer-enumeration"|| (*pit).GetTag()== "float-enumeration"|| (*pit).GetTag()== "double-enumeration") {
                vtkKWRadioButtonSetWithLabel
                        *tparameter= vtkKWRadioButtonSetWithLabel::New();
                tparameter->SetParent( parameterGroupFrame);
                tparameter->Create();
                tparameter->SetLabelText( (*pit).GetLabel().c_str());
                tparameter->GetWidget()->PackHorizontallyOn();
                tparameter->GetWidget()->SetMaximumNumberOfWidgetsInPackingDirection(4);
                std::vector<std::string>::const_iterator sbeginit= (*pit).GetElements().begin();
                std::vector<std::string>::const_iterator sendit= (*pit).GetElements().end();
                std::vector<std::string>::const_iterator sit;
                int id;
                for (sit = sbeginit, id=0; sit != sendit; ++sit, ++id) {
                    vtkKWRadioButton *b = tparameter->GetWidget()->AddWidget(id);
                    b->SetValue( (*sit).c_str());
                    b->SetText( (*sit).c_str());
                    b->SetAnchorToWest();
                    if (*sit == (*pit).GetDefault()) {
                        b->SetSelectedState(1);
                    } else {
                        b->SetSelectedState(0);
                    }
                }
                parameter = tparameter;
            } else {
                vtkKWLabel *tparameter = vtkKWLabel::New();
                tparameter->SetParent( parameterGroupFrame);
                tparameter->Create();
                tparameter->SetText( (*pit).GetLabel().c_str());
                parameter = tparameter;
            }

            // build the balloon help for the parameter
            std::string parameterBalloonHelp = (*pit).GetDescription();
            parameter->SetBalloonHelpString(parameterBalloonHelp.c_str());

            // pack the parameter. if the parameter has a separate label and
            // widget, then pack both side by side.
            app->Script( "pack %s -side top -anchor ne -padx 2 -pady 2",
                    parameter->GetWidgetName());
            
            // check for errors, after validation
            
            std::string error = this->GetErrorByParamName(pit->GetName());
            
            if(error != "")
            {
                vtkKWLabel *errorLabel = vtkKWLabel::New();
                errorLabel->SetParent(parameterGroupFrame);     
                errorLabel->Create();                
                errorLabel->SetText(error.c_str());
                errorLabel->SetBackgroundColor(1,0,0);
                errorLabel->SetForegroundColor(1,1,1);
                errorLabel->SetFont("times 8 bold");
                app->Script( "pack %s -side top -anchor ne -padx 2 -pady 2",
                        errorLabel->GetWidgetName());
                
                errorLabel->Delete();
                errorLabel = NULL;
            }

            curModWidgetStruct->modParams->push_back((*pit));

            //initialze/update MRML for this widget;
            this->UpdateMRMLForWidget(parameter, (*pit));
            
            this->AddParameterAndEventToWidget(parameter, (*pit));                        

            parameter->Delete();
            parameter = NULL;
        }                
        curModWidgetStruct->paramWidget = parameterGroupFrame;
        this->m_InternalWidgetParamList->push_back(curModWidgetStruct);        
    }//for
    
    //Add all change Events
    
    this->m_Created = true;
    this->m_End = false;
    
    return PARAMETER_WIDGET_SUCC;
}

vtkKWWidget *vtkSlicerParameterWidget::GetNextWidget()
{
    if(!this->IsCreated() || this->m_CurrentIndex == (this->m_InternalWidgetParamList->size() - 1))
    {
        return NULL;
    }
    
    if(this->m_CurrentIndex == -1)
    {
        this->m_CurrentIndex = 0;
        this->m_InternalIterator = this->m_InternalWidgetParamList->begin();
    }
    else if(this->m_InternalIterator != this->m_InternalWidgetParamList->end())
    {
        this->m_CurrentIndex++;
        this->m_InternalIterator++;                
    }
    else
    {
        this->m_End = true;
        return NULL;
    }        
    return (*(this->m_InternalIterator))->paramWidget;
}

bool vtkSlicerParameterWidget::IsCreated()
{
    return this->m_Created;
}

int vtkSlicerParameterWidget::size()
{
    if(IsCreated())
    {
        return this->m_InternalWidgetParamList->size();   
    }
    else
        return -1;
}

bool vtkSlicerParameterWidget::end()
{
    return this->m_End;
}

void vtkSlicerParameterWidget::reset()
{
    this->m_CurrentIndex = -1;
//    this->m_InternalIterator = NULL;
    this->m_End = false;
}

std::vector<ModuleParameter> *vtkSlicerParameterWidget::GetCurrentParameters()
{
    if(this->end())
    {
        return NULL;
    }
    else
    {
        return (*(this->m_InternalIterator))->modParams;
    }
}

int vtkSlicerParameterWidget::currentIndex()
{
    return this->m_CurrentIndex;
}

void vtkSlicerParameterWidget::SetParent(vtkKWWidget *parentWidget)
{
    this->m_ParentWidget = parentWidget;
}

void vtkSlicerParameterWidget::SetSlicerModuleLogic(vtkSlicerModuleLogic *moduleLogic)
{
    this->m_ModuleLogic = moduleLogic;
}

void vtkSlicerParameterWidget::SetModuleDescription(ModuleDescription *modDescription)
{
    this->m_ModuleDescription = modDescription;
}

void vtkSlicerParameterWidget::SetMRMLNode(vtkMRMLNode *mrmlNode)
{
    this->m_MRMLNode = mrmlNode;
}

vtkMRMLNode *vtkSlicerParameterWidget::GetMRMLNode()
{
    return this->m_MRMLNode;
}

void vtkSlicerParameterWidget::AddGUIObservers()
{

}

void vtkSlicerParameterWidget::AddParameterAndEventToWidget(vtkKWCoreWidget *parentWidget, ModuleParameter widgetParameter)
{
    
    vtkCallbackCommand *GUIChangedCallbackCommand = vtkCallbackCommand::New();
    GUIChangedCallbackCommand->SetCallback(vtkSlicerParameterWidget::GUIChangedCallback);
    
    callBackDataStruct *cbStruct = new callBackDataStruct;
    //    cbStruct->curWidgetName = NULL;
    cbStruct->parentClass = this;
    cbStruct->widgetParameter = widgetParameter;
    
    //vtkKWWidget *curWidg = this->m_ParentWidget->GetChildWidgetWithName((*mapIter).first.c_str());
    // Need to determine what type of widget we are using so we can
    // set the appropriate type of observer
    vtkKWSpinBoxWithLabel *sb = vtkKWSpinBoxWithLabel::SafeDownCast(parentWidget);
    vtkKWScaleWithEntry *se = vtkKWScaleWithEntry::SafeDownCast(parentWidget);
    vtkKWCheckButtonWithLabel *cb = vtkKWCheckButtonWithLabel::SafeDownCast(parentWidget);
    vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast(parentWidget);
    vtkSlicerNodeSelectorWidget
            *ns = vtkSlicerNodeSelectorWidget::SafeDownCast(parentWidget);
    vtkKWLoadSaveButtonWithLabel
            *lsb = vtkKWLoadSaveButtonWithLabel::SafeDownCast(parentWidget);
    vtkKWRadioButtonSetWithLabel
            *rbs = vtkKWRadioButtonSetWithLabel::SafeDownCast(parentWidget);

    if (sb) {
//        this->m_internalWidgetToParamMap->insert(std::make_pair(sb->GetWidget(), widgetParameter));
        GUIChangedCallbackCommand->SetClientData(cbStruct);
        sb->GetWidget()->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent,
                (vtkCommand *) GUIChangedCallbackCommand);
    } else if (se) {
        GUIChangedCallbackCommand->SetClientData(cbStruct);
        se->AddObserver(vtkKWScale::ScaleValueStartChangingEvent,
                (vtkCommand *) GUIChangedCallbackCommand);
        se->AddObserver(vtkKWScale::ScaleValueChangedEvent,
                (vtkCommand *) GUIChangedCallbackCommand);
    } else if (cb) {
        GUIChangedCallbackCommand->SetClientData(cbStruct);
        cb->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent,
                (vtkCommand *) GUIChangedCallbackCommand);
    } else if (e) {
        GUIChangedCallbackCommand->SetClientData(cbStruct);
        e->GetWidget()->AddObserver(vtkKWEntry::EntryValueChangedEvent,
                (vtkCommand *) GUIChangedCallbackCommand);
    } else if (ns) {
        GUIChangedCallbackCommand->SetClientData(cbStruct);
        ns->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                (vtkCommand *) GUIChangedCallbackCommand);
    } else if (lsb) {
        GUIChangedCallbackCommand->SetClientData(cbStruct);
        lsb->GetWidget()->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent,
                (vtkCommand *) GUIChangedCallbackCommand);
    } else if (rbs) {
        int num = rbs->GetWidget()->GetNumberOfWidgets();
        for (int i=0;
        i < num; ++i) {
            int id = rbs->GetWidget()->GetIdOfNthWidget(i);
            vtkKWRadioButton* rb = rbs->GetWidget()->GetWidget(id);
            GUIChangedCallbackCommand->SetClientData(cbStruct);
            rb->AddObserver(vtkKWRadioButton::SelectedStateChangedEvent,
                    (vtkCommand *) GUIChangedCallbackCommand);
        }
    }
    
    if(GUIChangedCallbackCommand)
    {
        GUIChangedCallbackCommand->Delete();
        GUIChangedCallbackCommand = NULL;
    }
}

void vtkSlicerParameterWidget::GUIChangedCallback(vtkObject *__caller, unsigned long eid, void *__clientData, void *callData)
{
    
    vtkSlicerParameterWidget::callBackDataStruct *myCBStruct = (vtkSlicerParameterWidget::callBackDataStruct*)__clientData;
    
    if(!myCBStruct)
    {
        std::cout<<"myCBStruct is NULL"<<std::endl;
        return;
    }
    
    if(myCBStruct->parentClass->GetMRMLNode())
    {
        std::string name = myCBStruct->widgetParameter.GetName();
        const char * value = myCBStruct->parentClass->GetValueFromWidget((vtkKWWidget*)__caller);
        //try to downcast to vtkMRMLWFEngineModuleNode to access their setAttribute directly
        std::cout<<"GUIChangeCallback - name: "<<name<<" value: "<<value<<std::endl;
        myCBStruct->parentClass->GetMRMLNode()->SetAttribute(myCBStruct->parentClass->GetAttributeName(name), value);
        myCBStruct->parentClass->InvokeEvent(vtkSlicerParameterWidget::ParameterWidgetChangedEvent, NULL);
    }    
}

const char *vtkSlicerParameterWidget::GetValueFromWidget(vtkKWWidget *widg)
{
    vtkKWSpinBox *sb = vtkKWSpinBox::SafeDownCast(widg);
    vtkKWScaleWithEntry *se = vtkKWScaleWithEntry::SafeDownCast(widg);
    vtkKWCheckButton *cb = vtkKWCheckButton::SafeDownCast(widg);
    vtkKWEntry *e = vtkKWEntry::SafeDownCast(widg);
    vtkSlicerNodeSelectorWidget
            *ns = vtkSlicerNodeSelectorWidget::SafeDownCast(widg);
    vtkKWLoadSaveButton
            *lsb = vtkKWLoadSaveButton::SafeDownCast(widg);
    vtkKWRadioButton
            *rb = vtkKWRadioButton::SafeDownCast(widg);
    
    // because vtkKWCheckButton is a parent of vtkKWRadioButton both return not null if a radio-button is clicked
    if(rb)
        cb = NULL;
    
    std::ostringstream strvalue;
    
    if (sb) {
        strvalue << sb->GetValue();
        strvalue << ends;
        std::cout<<strvalue.str()<<std::endl;
        return strvalue.str().c_str();
    } else if (se) {
        strvalue << se->GetValue();
        strvalue << ends;
        std::cout<<strvalue.str()<<std::endl;
        return strvalue.str().c_str();
    } else if (cb) {
        return (cb->GetSelectedState() ? "true" : "false");
    } else if (e) {
        return e->GetValue();
    } else if (ns && ns->GetSelected() != NULL) {
        return ns->GetSelected()->GetID();
    } else if (lsb) {        
        if (lsb->GetFileName())
        {
            return lsb->GetFileName();
        }
    } else if (rb) {
        vtkKWRadioButtonSetWithLabel *rbs = vtkKWRadioButtonSetWithLabel::SafeDownCast(rb->GetParent());
        if(rbs)
        {
            int num = rbs->GetWidget()->GetNumberOfWidgets();
            for (int i=0; i < num; ++i) {
                int id = rbs->GetWidget()->GetIdOfNthWidget(i);
                vtkKWRadioButton* rb = rbs->GetWidget()->GetWidget(id);
                if(rb->GetSelectedState() == 1)
                {
                    return rb->GetText();
                }
            }        
        }
        else
            return rb->GetText();
            
    } else {
        std::cout<<"WARNING: vtkSlicerParameterWidget - unsupported Widget monitored; return \"\""<<std::endl;
        std::cout<<widg->GetWidgetName()<<std::endl;
        return "";
    }
    
    return "";
//    strvalue.rdbuf()->freeze(0);
}

void vtkSlicerParameterWidget::SetWidgetID(std::string id)
{
    this->m_widgID = id;
}

const char* vtkSlicerParameterWidget::GetAttributeName(std::string name)
{
    if(this->m_widgID == "")
    {
        std::cout<<"WARNING: vtkSlicerParameterWidget - no stepID set"<<std::endl;
        return name.c_str();
    }
    
    std::string attribName = this->m_widgID + "." + name;
    return attribName.c_str();
}

void vtkSlicerParameterWidget::UpdateMRMLForWidget(vtkKWCoreWidget *parentWidget, ModuleParameter widgetParameter)
{
    const char* curValue = this->GetMRMLNode()->GetAttribute(this->GetAttributeName(widgetParameter.GetName().c_str()));
    
    if(curValue)
    {
        this->SetValueForWidget(parentWidget, curValue);
    }
    else
    {
        const char* name = this->GetAttributeName(widgetParameter.GetName().c_str());
        const char* value = widgetParameter.GetDefault().c_str();
        std::cout<<"name: "<<name<<" value: "<<value<<std::endl;
        this->GetMRMLNode()->SetAttribute(name, value);
    }
}

void vtkSlicerParameterWidget::SetValueForWidget(vtkKWCoreWidget *inputWidget, const char* value)
{
    if(!value)
    {
        return;
    }
    
    vtkKWSpinBoxWithLabel *sb = vtkKWSpinBoxWithLabel::SafeDownCast(inputWidget);
    vtkKWScaleWithEntry *se = vtkKWScaleWithEntry::SafeDownCast(inputWidget);
    vtkKWCheckButtonWithLabel *cb = vtkKWCheckButtonWithLabel::SafeDownCast(inputWidget);
    vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast(inputWidget);
    vtkSlicerNodeSelectorWidget
            *ns = vtkSlicerNodeSelectorWidget::SafeDownCast(inputWidget);
    vtkKWLoadSaveButtonWithLabel
            *lsb = vtkKWLoadSaveButtonWithLabel::SafeDownCast(inputWidget);
    vtkKWRadioButton
            *rb = vtkKWRadioButton::SafeDownCast(inputWidget);
    vtkKWRadioButtonSetWithLabel
    *rbswl = vtkKWRadioButtonSetWithLabel::SafeDownCast(inputWidget);    
    
    if (sb) {
        double d;
        std::string a_double = value;
        std::istringstream ss(a_double);
        ss >> d;
        sb->GetWidget()->SetValue(d);        
    } else if (se) {
        int i;
        std::string a_int = value;
        std::istringstream ss(a_int);
        ss >> i;
        se->SetValue(i);  
    } else if (cb) {        
        cb->GetWidget()->SetSelectedState((strcmp(value, "true") == 0) ? 1 : 0);
    } else if (e) {
        e->GetWidget()->SetValue(value);
    } else if (ns && ns->GetSelected() != NULL) {
        ns->SetSelected(this->GetMRMLNode()->GetScene()->GetNodeByID(value));
    } else if (lsb) {
       lsb->GetWidget()->GetLoadSaveDialog()->SetInitialFileName(value);
    } else if (rb) {
        vtkKWRadioButtonSetWithLabel *rbs = vtkKWRadioButtonSetWithLabel::SafeDownCast(rb->GetParent());
        if(rbs)
        {
            int num = rbs->GetWidget()->GetNumberOfWidgets();
            for (int i=0; i < num; ++i) {
                int id = rbs->GetWidget()->GetIdOfNthWidget(i);
                vtkKWRadioButton* rb = rbs->GetWidget()->GetWidget(id);
                std::cout<<rb->GetValue()<<" == "<<value<<std::endl;
                if(strcmp(rb->GetText(), value) == 0)
                {
                    rb->SelectedStateOn();
                    return;
                }
            }        
        }
        else
        {
            if(rb->GetValue() == value)
            {
                rb->SelectedStateOn();
            }
        }
    } else if(rbswl) {
        int num = rbswl->GetWidget()->GetNumberOfWidgets();
        for (int i=0; i < num; ++i) {
            int id = rbswl->GetWidget()->GetIdOfNthWidget(i);
            vtkKWRadioButton* rb = rbswl->GetWidget()->GetWidget(id);
            std::cout<<rb->GetValue()<<" == "<<value<<std::endl;
            if(strcmp(rb->GetText(), value) == 0)
            {
                rb->SelectedStateOn();
                return;
            }
        }        
    } else {
        std::cout<<"WARNING: vtkSlicerParameterWidget - try to set unsupported Widget; return \"\""<<std::endl;        
        return;
    }
}

std::string vtkSlicerParameterWidget::GetValueByName(std::string name)
{
    if(!this->GetMRMLNode())
    {
        return "";
    }
    
    const char* attrName = this->GetAttributeName(name.c_str());
    return this->GetMRMLNode()->GetAttribute(attrName);
}

void vtkSlicerParameterWidget::SetErrorMap(std::map<std::string, std::string> *errorMap)
{
    this->m_paramToErrorMap = errorMap;
}

std::string vtkSlicerParameterWidget::GetErrorByParamName(std::string name)
{
    if(this->m_paramToErrorMap)
    {
        std::map<std::string, std::string>::iterator iter = this->m_paramToErrorMap->find(name);
        
        if(iter != this->m_paramToErrorMap->end())
        {
            return iter->second;
        }
    }
    
    return "";
}

void vtkSlicerParameterWidget::DeleteInternalLists()
{
    std::cout<<"vtkSlicerParameterWidget::DeleteInternalLists"<<std::endl;
    
    if(this->m_InternalWidgetParamList)
    {
        std::vector<vtkSlicerParameterWidget::moduleParameterWidgetStruct*>::iterator iter;
        std::cout<<"m_InternalWidgetParamList.size()"<<std::endl;
        std::cout<<m_InternalWidgetParamList->size()<<std::endl;
        for(iter = this->m_InternalWidgetParamList->begin(); iter != this->m_InternalWidgetParamList->end(); iter++)
        {
            moduleParameterWidgetStruct *curModuleStruct = (moduleParameterWidgetStruct*)(*iter);
            if(curModuleStruct)
            {
                vtkKWFrame *paramFrame = vtkKWFrame::SafeDownCast(curModuleStruct->paramWidget);
                if(paramFrame)
                {
                    // iterate through all children to remove observers!
                    for(int i = 0; i < paramFrame->GetNumberOfChildren(); i++)
                    {                       
                        this->DeleteInputWidget(paramFrame->GetNthChild(i));                        
                    }
//                    paramFrame->RemoveAllChildren();
//                    paramFrame->Delete();
//                    paramFrame->RemoveAllChildren();
                    paramFrame = NULL;
                } else {
                  std::cout<<"WARNING: vtkSlicerParameterWidget - try to delete unsupported Widget; return \"\""<<std::endl;        
                  return;
                }
                curModuleStruct->paramWidget = NULL;
            }
            if(curModuleStruct->modParams)
            {
                curModuleStruct->modParams->clear();
                curModuleStruct->modParams = NULL;
            }
            
            curModuleStruct = NULL;
        }
        
        this->m_InternalWidgetParamList->clear();
        this->m_InternalWidgetParamList = NULL;
    }  
}

void vtkSlicerParameterWidget::DeleteInputWidget(vtkKWWidget *widg)
{
    vtkKWSpinBoxWithLabel *sb = vtkKWSpinBoxWithLabel::SafeDownCast(widg);
    vtkKWScaleWithEntry *se = vtkKWScaleWithEntry::SafeDownCast(widg);
    vtkKWCheckButtonWithLabel *cb = vtkKWCheckButtonWithLabel::SafeDownCast(widg);
    vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast(widg);
    vtkSlicerNodeSelectorWidget
            *ns = vtkSlicerNodeSelectorWidget::SafeDownCast(widg);
    vtkKWLoadSaveButtonWithLabel
            *lsb = vtkKWLoadSaveButtonWithLabel::SafeDownCast(widg);
    vtkKWRadioButton
            *rb = vtkKWRadioButton::SafeDownCast(widg);
    vtkKWRadioButtonSetWithLabel
            *rbswl = vtkKWRadioButtonSetWithLabel::SafeDownCast(widg);
    
    if (sb) {
        //sb->GetWidget()->RemoveAllObservers();        
    } else if (se) {
        //se->RemoveAllObservers();        
    } else if (cb) {        
        //cb->GetWidget()->RemoveAllObservers();        
    } else if (e) {
        //e->GetWidget()->RemoveAllObservers();        
    } else if (ns) {
        //ns->RemoveAllObservers();      
    } else if (lsb) {
        //lsb->GetWidget()->RemoveAllObservers();
    } else if (rbswl) {
        int num = rbswl->GetWidget()->GetNumberOfWidgets();
        for (int i=0; i < num; ++i) {
            int id = rbswl->GetWidget()->GetIdOfNthWidget(i);
            vtkKWRadioButton* rb = rbswl->GetWidget()->GetWidget(id);
            //rb->RemoveAllObservers();
        }        
    } else {
        std::cout<<"WARNING: vtkSlicerParameterWidget - try to set unsupported Widget;";
        if(widg) {
//            std::cout<<widg->GetClassName();
        } else {
            std::cout<<"NULL";
        }        
        std::cout<<" - return \"\""<<std::endl;        
        return;
    }
}
