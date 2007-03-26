#include "vtkWFEngineEventHandler.h"

#include <vtkCallbackCommand.h>
#include <vtkKWObject.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLFiducialListNode.h>
#include <vtkSlicerApplication.h>

#include "vtkWFEngineHandler.h"
#include "vtkMRMLWFEngineModuleNode.h"

#include <vtkKWMultiColumnListWithScrollbars.h>
#include <vtkKWMultiColumnList.h>
#include <vtkSlicerFiducialsLogic.h>
#include <vtkSlicerFiducialsGUI.h>

#include <string>
#include <sstream>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkWFEngineEventHandler );
vtkCxxRevisionMacro ( vtkWFEngineEventHandler, "$Revision: 1.0 $");


//---------------------------------------------------------------------------

vtkWFEngineEventHandler::vtkWFEngineEventHandler()
{
    this->m_id = NULL;
    this->m_eventName = NULL;
    this->m_fiducialList = NULL;    
}

vtkWFEngineEventHandler::~vtkWFEngineEventHandler()
{
    this->m_eventName = NULL;
    this->m_id = NULL;      
    
    if(this->m_fiducialList)
    {
        this->m_fiducialList->Delete();
        this->m_fiducialList = NULL;
    }        
    
    this->RemoveAllObservers();
}

//---------------------------------------------------------------------------
void vtkWFEngineEventHandler::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkWFEngineEventHandler: " << this->GetClassName ( ) << "\n";    

}

void vtkWFEngineEventHandler::ProcessWorkflowLeaveEvents(vtkObject *caller, unsigned long event, void *callData, void *clientData)
{
    std::cout<<"vtkWFEngineEventHandler::ProcessWorkflowLeaveEvents"<<std::endl;
    vtkWFEngineHandler *curWFHandler = (vtkWFEngineHandler*)caller;
    vtkWFEngineEventHandler *curWFEventHandler = (vtkWFEngineEventHandler*)callData;

    if (curWFEventHandler) {
        curWFEventHandler->m_eventName = (const char*)clientData;
        std::cout<<"EVENT catched: "<<curWFEventHandler->m_eventName<<std::endl;
        if (curWFEventHandler->m_eventName && strcmp(
                curWFEventHandler->m_eventName, "addFiducial")== 0) {

            vtkMRMLNode *curNode= NULL;

            if (curWFHandler) {
                curNode = curWFHandler->GetWFMRMLNode();
            }

            if (curNode) {
                // get Parameter from the m_mrmlScene

                vtkMRMLWFEngineModuleNode
                        *wfEngineModuleNode = vtkMRMLWFEngineModuleNode::SafeDownCast(curNode);
                if (wfEngineModuleNode) {
                    vtkMRMLScene *curScene = wfEngineModuleNode->GetScene();
                    std::string
                            fidNameParameter = curWFHandler->GetCurrentStepID();
                    fidNameParameter.append(".fidName");
                    std::string
                            fidXParameter = curWFHandler->GetCurrentStepID();
                    fidXParameter.append(".fidX");
                    std::string
                            fidYParameter = curWFHandler->GetCurrentStepID();
                    fidYParameter.append(".fidY");
                    std::string
                            fidZParameter = curWFHandler->GetCurrentStepID();
                    fidZParameter.append(".fidZ");

                    const char
                            * fidNameValue = wfEngineModuleNode->GetAttribute(fidNameParameter.c_str());
                    const char
                            * fidXValue = wfEngineModuleNode->GetAttribute(fidXParameter.c_str());
                    const char
                            * fidYValue = wfEngineModuleNode->GetAttribute(fidYParameter.c_str());
                    const char
                            * fidZValue = wfEngineModuleNode->GetAttribute(fidZParameter.c_str());

                    if (curWFEventHandler->m_fiducialList == NULL) {
                        vtkMRMLNode
                                *node = curScene->GetNextNodeByClass("vtkMRMLFiducialListNode");
                        if (node == NULL) {
                            std::cerr << "ERROR: No Fiducial List, adding one first!\n";
                            vtkMRMLNode
                                    *newList = curScene->CreateNodeByClass("vtkMRMLFiducialListNode");
                            if (newList != NULL) {
                                curScene->AddNode(newList);
                                curWFEventHandler->m_fiducialList = vtkMRMLFiducialListNode::SafeDownCast(newList);
                                //                        newList->Delete();
                            }
                        }
                    }

                    if (curWFEventHandler->m_fiducialList) {
                        int fidID;
                        fidID = curWFEventHandler->m_fiducialList->AddFiducial();
                        if (fidNameValue) {
                            curWFEventHandler->m_fiducialList->SetNthFiducialLabelText(
                                    fidID, fidNameValue);
                        }
                        if (fidXValue && fidYValue && fidZValue) {
                            std::ostringstream strvalue;
                            std::string a_double;
                            double x;
                            a_double = fidXValue;
                            std::istringstream sx(a_double);
                            sx >> x;

                            double y;
                            a_double = fidYValue;
                            std::istringstream sy(a_double);
                            sy >> y;

                            double z;
                            a_double = fidZValue;
                            std::istringstream sz(a_double);
                            sz >> z;

                            curWFEventHandler->m_fiducialList->SetNthFiducialXYZ(
                                    fidID, x, y, z);
                        }//fi
                    }//fi                                
                }//fi
            }
        }
    }
}

void vtkWFEngineEventHandler::ProcessWorkflowEnterEvents(vtkObject *caller, unsigned long event, void *callData, void *clientData)
{
    std::cout<<"vtkWFEngineEventHandler::ProcessWorkflowEnterEvents"<<std::endl;
    vtkWFEngineHandler *curWFHandler = (vtkWFEngineHandler*)caller;
    vtkWFEngineEventHandler *curWFEventHandler = (vtkWFEngineEventHandler*)callData;
        
    if(curWFEventHandler)
    {
        curWFEventHandler->m_eventName = (const char*)clientData;
        if(curWFEventHandler->m_eventName && strcmp(curWFEventHandler->m_eventName, "showFiducialList") == 0)
        {
            if(curWFHandler != NULL && curWFHandler->GetWizardClientArea() != NULL)
            {
                vtkMRMLNode *curNode = NULL;
                vtkMRMLScene *curScene = NULL;
                if(curWFHandler)
                {
                    curNode = curWFHandler->GetWFMRMLNode();
                }
                  
                if(curNode)
                {
                    curScene = curNode->GetScene();
                }
                vtkKWMultiColumnListWithScrollbars *mclw = vtkKWMultiColumnListWithScrollbars::New();
                mclw->SetParent(curWFHandler->GetWizardClientArea());
                mclw->Create();
                
                mclw->GetWidget()->AddColumn("Name");
                mclw->GetWidget()->AddColumn("X");
                mclw->GetWidget()->AddColumn("Y");
                mclw->GetWidget()->AddColumn("Z");
                
                if(curWFEventHandler->m_fiducialList == NULL)
                {              
                    vtkMRMLNode *node = curScene->GetNextNodeByClass("vtkMRMLFiducialListNode");
                    vtkMRMLFiducialListNode *fidListNode = vtkMRMLFiducialListNode::SafeDownCast(node);
                    if (fidListNode == NULL) {
                        std::cerr << "ERROR: No Fiducial List, adding one first!\n";                    
                    }
                    else
                        curWFEventHandler->m_fiducialList = fidListNode;
                }
                
                if(curWFEventHandler->m_fiducialList)
                {                                        
                    for(int i = 0; i < curWFEventHandler->m_fiducialList->GetNumberOfFiducials(); i++)
                    {
                        mclw->GetWidget()->AddRow();
                        mclw->GetWidget()->SetCellText(mclw->GetWidget()->GetNumberOfRows() - 1, 0, curWFEventHandler->m_fiducialList->GetNthFiducialLabelText(i));
                        mclw->GetWidget()->SetCellTextAsDouble(mclw->GetWidget()->GetNumberOfRows() - 1, 1, curWFEventHandler->m_fiducialList->GetNthFiducialXYZ(i)[0]);
                        mclw->GetWidget()->SetCellTextAsDouble(mclw->GetWidget()->GetNumberOfRows() - 1, 2, curWFEventHandler->m_fiducialList->GetNthFiducialXYZ(i)[1]);
                        mclw->GetWidget()->SetCellTextAsDouble(mclw->GetWidget()->GetNumberOfRows() - 1, 3, curWFEventHandler->m_fiducialList->GetNthFiducialXYZ(i)[2]);
                    }                      
                }
                
                curWFEventHandler->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  mclw->GetWidgetName()); 
                
                mclw->Delete();
                mclw = NULL;
            }
        }
    }
}

void vtkWFEngineEventHandler::AddWorkflowObservers(vtkWFEngineHandler *curWFHandler)
{
    if(curWFHandler)
    {
        vtkCallbackCommand *curWorkflowCB = vtkCallbackCommand::New();
        curWorkflowCB->SetClientData(this);
        curWorkflowCB->SetCallback(&vtkWFEngineEventHandler::ProcessWorkflowLeaveEvents);
       
        curWFHandler->AddObserver(vtkWFEngineHandler::WorkflowStepLeaveEvent, curWorkflowCB);
        
        curWorkflowCB->Delete();
        curWorkflowCB = NULL;
        
        curWorkflowCB = vtkCallbackCommand::New();
        curWorkflowCB->SetClientData(this);
        curWorkflowCB->SetCallback(&vtkWFEngineEventHandler::ProcessWorkflowEnterEvents);
        
        curWFHandler->AddObserver(vtkWFEngineHandler::WorkflowStepEnterEvent, curWorkflowCB);
        
        curWorkflowCB->Delete();
        curWorkflowCB = NULL;        
    }   
}

void vtkWFEngineEventHandler::SetCurrentStepID(const char *id)
{
    this->m_id = id;
}

void vtkWFEngineEventHandler::SetEventName(const char *name)
{
    this->m_eventName = name;
}
