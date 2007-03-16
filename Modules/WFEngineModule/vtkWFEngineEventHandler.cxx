#include "vtkWFEngineEventHandler.h"

#include <vtkCallbackCommand.h>
#include <vtkKWObject.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLFiducialListNode.h>

#include "vtkWFEngineHandler.h"
#include "vtkMRMLWFEngineModuleNode.h"

#include <string>
#include <sstream>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkWFEngineEventHandler );
vtkCxxRevisionMacro ( vtkWFEngineEventHandler, "$Revision: 1.0 $");


//---------------------------------------------------------------------------

vtkWFEngineEventHandler::vtkWFEngineEventHandler()
{
    this->m_workflowCB = NULL;
    this->m_mrmlScene = NULL;
    this->m_id = NULL;
    this->m_eventName = NULL;
    this->m_fiducialList = NULL;
}

vtkWFEngineEventHandler::~vtkWFEngineEventHandler()
{
    if(this->m_workflowCB)
    {
        this->m_workflowCB->Delete();
        this->m_workflowCB = NULL;
    }
    
    this->m_eventName = NULL;
    this->m_id = NULL;    
    this->m_mrmlScene = NULL;
    
    if(this->m_fiducialList)
    {
        this->m_fiducialList->Delete();
        this->m_fiducialList = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkWFEngineEventHandler::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkWFEngineEventHandler: " << this->GetClassName ( ) << "\n";    

}

void vtkWFEngineEventHandler::ProcessWorkflowLeaveEvents(vtkObject *caller, unsigned long event, void *callData, void *clientData)
{
    std::cout<<"vtkWFEngineEventHandler::ProcessWorkflowEvents"<<std::endl;
    vtkWFEngineHandler *curWFHandler = (vtkWFEngineHandler*)caller;
    vtkWFEngineEventHandler *curWFEventHandler = (vtkWFEngineEventHandler*)callData;
    
    
    if(curWFEventHandler)
    {
        curWFEventHandler->m_eventName = (const char*)clientData;
        std::cout<<"EVENT catched: "<<curWFEventHandler->m_eventName<<std::endl;
        if(curWFEventHandler->m_eventName && strcmp(curWFEventHandler->m_eventName, "addFiducial") == 0)
        {
            if(curWFEventHandler->m_mrmlScene)
            {
                // get Parameter from the m_mrmlScene
                
                vtkMRMLNode *tmpWFEngineNode = curWFEventHandler->m_mrmlScene->GetNodeByID("vtkMRMLWFEngineModuleNode1");
                vtkMRMLWFEngineModuleNode *wfEngineModuleNode = vtkMRMLWFEngineModuleNode::SafeDownCast(tmpWFEngineNode);
                if(wfEngineModuleNode)
                {
                    std::string fidNameParameter = curWFHandler->GetCurrentStepID();
                    fidNameParameter.append(".fidName");
                    std::string fidXParameter = curWFHandler->GetCurrentStepID();
                    fidXParameter.append(".fidX");
                    std::string fidYParameter = curWFHandler->GetCurrentStepID();
                    fidYParameter.append(".fidY");
                    std::string fidZParameter = curWFHandler->GetCurrentStepID();
                    fidZParameter.append(".fidZ");
                    
                    const char* fidNameValue = wfEngineModuleNode->GetAttribute(fidNameParameter.c_str());
                    const char* fidXValue = wfEngineModuleNode->GetAttribute(fidXParameter.c_str());
                    const char* fidYValue = wfEngineModuleNode->GetAttribute(fidYParameter.c_str());
                    const char* fidZValue = wfEngineModuleNode->GetAttribute(fidZParameter.c_str());
                    
                    if(curWFEventHandler->m_fiducialList == NULL)
                    {              
                        vtkMRMLNode *node = curWFEventHandler->m_mrmlScene->GetNextNodeByClass("vtkMRMLFiducialListNode");
                        if (node == NULL) {
                            std::cerr << "ERROR: No Fiducial List, adding one first!\n";
                            vtkMRMLNode
                                    *newList = curWFEventHandler->m_mrmlScene->CreateNodeByClass("vtkMRMLFiducialListNode");
                            if (newList != NULL) {
                                curWFEventHandler->m_mrmlScene->AddNode(newList);
                                curWFEventHandler->m_fiducialList = vtkMRMLFiducialListNode::SafeDownCast(newList);
                                //                        newList->Delete();
                            }
                        }
                    }
                    
                    if(curWFEventHandler->m_fiducialList)
                    {
                        int fidID;
                        fidID = curWFEventHandler->m_fiducialList->AddFiducial();
                        if(fidNameValue)
                        {
                            curWFEventHandler->m_fiducialList->SetNthFiducialLabelText(fidID, fidNameValue);   
                        }                        
                        if(fidXValue && fidYValue && fidZValue)
                        {
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
                                    
                            
                            curWFEventHandler->m_fiducialList->SetNthFiducialXYZ(fidID, x, y, z);
                        }
                    }                                
                }
                
            }        
        }
    }
}

void vtkWFEngineEventHandler::AddWorkflowObservers(vtkWFEngineHandler *curWFHandler)
{
    this->m_workflowCB = vtkCallbackCommand::New();
    this->m_workflowCB->SetClientData(this);
    this->m_workflowCB->SetCallback(&vtkWFEngineEventHandler::ProcessWorkflowLeaveEvents);
    
    if(curWFHandler)
    {
        curWFHandler->AddObserver(vtkWFEngineHandler::WorkflowStepLeaveEvent, this->m_workflowCB);
    }
    
    m_workflowCB->Delete();
    m_workflowCB = NULL;
}

void vtkWFEngineEventHandler::SetMRMLScene(vtkMRMLScene *scene)
{
    this->m_mrmlScene = scene;
}

vtkMRMLScene *vtkWFEngineEventHandler::GetMRMLScene()
{
    return this->m_mrmlScene;
}

void vtkWFEngineEventHandler::SetCurrentStepID(const char *id)
{
    this->m_id = id;
}

void vtkWFEngineEventHandler::SetEventName(const char *name)
{
    this->m_eventName = name;
}
