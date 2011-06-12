/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"

#include "vtkMRMLProstateNavManagerNode.h"
#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkStringArray.h"

#include "vtkMRMLScene.h"

#include "vtkProstateNavStep.h"
#include "vtkMRMLRobotNode.h"

//#include "vtkSlicerComponentGUI.h" // for vtkSetAndObserveMRMLNodeEventsMacro
#include "vtkSmartPointer.h"


//------------------------------------------------------------------------------
vtkMRMLProstateNavManagerNode* vtkMRMLProstateNavManagerNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLProstateNavManagerNode"); if(ret)
    {
      return (vtkMRMLProstateNavManagerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLProstateNavManagerNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLProstateNavManagerNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLProstateNavManagerNode");
  if(ret)
    {
      return (vtkMRMLProstateNavManagerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLProstateNavManagerNode;
}

//----------------------------------------------------------------------------
vtkMRMLProstateNavManagerNode::vtkMRMLProstateNavManagerNode()
{
  this->CurrentStep = 0;
  this->PreviousStep = 0;

  this->CurrentTargetIndex=-1;

  CalibrationVolumeNodeID=NULL;  
  TargetingVolumeNodeID=NULL;
  VerificationVolumeNodeID=NULL;
  CalibrationVolumeNodeID=NULL;
  CoverageVolumeNodeID=NULL;  

  this->CurrentNeedleIndex=-1;

  this->TargetDescriptorsVector.clear();

  TargetPlanListNodeID=NULL;
  TargetPlanListNode=NULL;

  RobotNodeID=NULL;
  RobotNode=NULL;

  this->Initialized=false;

  this->StepList=vtkStringArray::New();

  this->HideFromEditorsOff();
}

//----------------------------------------------------------------------------
vtkMRMLProstateNavManagerNode::~vtkMRMLProstateNavManagerNode()
{  
  this->StepList->Delete();
  SetCalibrationVolumeNodeID(NULL); 
  SetTargetingVolumeNodeID(NULL);
  SetVerificationVolumeNodeID(NULL);
  SetCalibrationVolumeNodeID(NULL);  
  SetCoverageVolumeNodeID(NULL);  
  
  if (this->TargetPlanListNodeID) 
  {
    SetAndObserveTargetPlanListNodeID(NULL);
  }
  if (this->RobotNodeID) 
  {
    SetAndObserveRobotNodeID(NULL);
  }

  for (std::vector<vtkProstateNavTargetDescriptor*>::iterator it=this->TargetDescriptorsVector.begin(); it!=this->TargetDescriptorsVector.end(); ++it)
    {   
    if ((*it)!=NULL)
      {
      (*it)->Delete();
      (*it)=NULL;
      }
    }
  this->TargetDescriptorsVector.clear();
}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::WriteXML(ostream& of, int nIndent)
{

  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " WorkflowSteps=\"" << GetWorkflowStepsString() << "\"";    

}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "WorkflowSteps"))
      {
      if (attValue==NULL)
        {
        vtkErrorMacro("Empty WorkflowSteps attribute value");
        }
      else if (!SetWorkflowStepsFromString(attValue))
        {
        vtkErrorMacro("Invalid WorkflowSteps attribute value: "<<attValue);
        }
      }
    }
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLProstateNavManagerNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLProstateNavManagerNode *node = (vtkMRMLProstateNavManagerNode *) anode;

  //int type = node->GetType();

  this->CurrentStep = node->CurrentStep;
  this->PreviousStep = node->PreviousStep;
  this->SetTargetPlanListNodeID(node->TargetPlanListNodeID);
  this->SetRobotNodeID(node->RobotNodeID);

  this->StepList->Reset();
  for (int i=0; i>node->StepList->GetSize(); i++)
  {
    this->StepList->SetValue(i, node->StepList->GetValue(i));
  }

  this->HideFromEditors=node->HideFromEditors;

  //copy the contents of the other
  this->NeedlesVector.assign(node->NeedlesVector.begin(), node->NeedlesVector.end() );

  // clean up old fiducial list vector
  for (std::vector<vtkProstateNavTargetDescriptor*>::iterator it=this->TargetDescriptorsVector.begin(); it!=this->TargetDescriptorsVector.end(); ++it)
    {   
    if ((*it)!=NULL)
      {
      (*it)->Delete();
      (*it)=NULL;
      }
    }
  this->TargetDescriptorsVector.clear();
  //copy the contents of the other
  vtkProstateNavTargetDescriptor* tdesc=NULL;
  for (std::vector<vtkProstateNavTargetDescriptor*>::iterator it=node->TargetDescriptorsVector.begin(); it!=node->TargetDescriptorsVector.end(); ++it)
    {    
    tdesc=vtkProstateNavTargetDescriptor::New();
    tdesc->DeepCopy(*(*it));
    this->TargetDescriptorsVector.push_back(tdesc);
    }

}

//-----------------------------------------------------------
void vtkMRMLProstateNavManagerNode::UpdateReferences()
{
   Superclass::UpdateReferences();
/*  if (this->TargetTransformNodeID != NULL && this->Scene->GetNodeByID(this->TargetTransformNodeID) == NULL)
    {
    this->SetAndObserveTargetTransformNodeID(NULL);
    }
    */
}

//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  /*if (this->TargetTransformNodeID && !strcmp(oldID, this->TargetTransformNodeID))
    {
    this->SetAndObserveTargetTransformNodeID(newID);
    }*/
}

//-----------------------------------------------------------
void vtkMRMLProstateNavManagerNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
   /*this->SetAndObserveTargetTransformNodeID(this->GetTargetTransformNodeID());*/
}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  if (this->TargetPlanListNode && this->TargetPlanListNode == vtkMRMLFiducialListNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    //this->ModifiedSinceReadOn();
    //this->InvokeEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent, NULL);
    //this->UpdateFromMRML();
    return;
    }

  return;
}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::GetNumberOfSteps()
{
  return this->StepList->GetNumberOfValues();
}


//----------------------------------------------------------------------------
const char* vtkMRMLProstateNavManagerNode::GetStepName(int i)
{
  if (i>=0 && i < this->StepList->GetNumberOfValues())
    {
      return this->StepList->GetValue(i);
    }
  else
    {
    return NULL;
    }
}
//----------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::SwitchStep(int newStep)
{
  if (newStep<0 || newStep>=GetNumberOfSteps())
  {
    return 0;
  }
  this->PreviousStep = this->CurrentStep;
  this->CurrentStep = newStep; 

  // Tentatively, this function calls vtkMRMLBrpRobotCommandNode::SwitchStep().
  // (we cannot test with the robot without sending workphase commands...)
  if (this->RobotNode)
    {
    this->RobotNode->SwitchStep(this->StepList->GetValue(newStep).c_str());
    }
  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::GetCurrentStep()
{
  return this->CurrentStep;
}


//----------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::GetPreviousStep()
{
  return this->PreviousStep;
}

//----------------------------------------------------------------------------
vtkStdString vtkMRMLProstateNavManagerNode::GetWorkflowStepsString()
{
  vtkStdString workflowSteps;
  // :TODO: generate the string from the StepList
  for (int i=0; i<this->StepList->GetNumberOfValues(); i++)
    {
    workflowSteps += this->StepList->GetValue(i);
    if (i<this->StepList->GetNumberOfValues()-1)
      {
      workflowSteps += " "; // add separator after each step but the last one
      }
    }
  return workflowSteps;
}

//----------------------------------------------------------------------------
bool vtkMRMLProstateNavManagerNode::SetWorkflowStepsFromString(const vtkStdString& workflowStepsString)
{ 
  this->StepList->Reset();

  if (workflowStepsString.empty())
    {
    return false;
    }

  vtkstd::stringstream workflowStepsStream(workflowStepsString);
  vtkstd::string stepName;
  while(workflowStepsStream>>stepName)
    {
    this->StepList->InsertNextValue(stepName);
    }
   return true;
}

//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::SetAndObserveTargetPlanListNodeID(const char *nodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->TargetPlanListNode, NULL);
  this->SetTargetPlanListNodeID(nodeID);
  vtkMRMLFiducialListNode *tnode = this->GetTargetPlanListNode();
  vtkSetAndObserveMRMLObjectMacro(this->TargetPlanListNode, tnode);
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListNode* vtkMRMLProstateNavManagerNode::GetTargetPlanListNode()
{
  vtkMRMLFiducialListNode* node = NULL;
  if (this->GetScene() && this->TargetPlanListNodeID != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->TargetPlanListNodeID);
    node = vtkMRMLFiducialListNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::SetAndObserveRobotNodeID(const char *nodeID)
{
  if (nodeID!=NULL && this->RobotNodeID!=NULL && strcmp(nodeID,this->RobotNodeID)==0)
  {
    // no change
    return;
  }
  vtkSetAndObserveMRMLObjectMacro(this->RobotNode, NULL);

  // update workflow steps from the new Robot node
  // (the Manager node is the owner of workflow steps, as it may add additional steps or modify the steps that
  // it gets from the robot)
  if (this->GetScene() && nodeID != NULL )
  {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(nodeID);
    vtkMRMLRobotNode* node = vtkMRMLRobotNode::SafeDownCast(snode);
    if (node != NULL)
    {      
      SetWorkflowStepsFromString(node->GetWorkflowStepsString());    
    }
    else
    {
      SetWorkflowStepsFromString("");
    }
  }
  else
  {
    SetWorkflowStepsFromString("");
  }
 
  this->SetRobotNodeID(nodeID);
  vtkMRMLRobotNode *tnode = this->GetRobotNode();

  vtkSetAndObserveMRMLObjectMacro(this->RobotNode, tnode);
}

//----------------------------------------------------------------------------
vtkMRMLRobotNode* vtkMRMLProstateNavManagerNode::GetRobotNode()
{
  vtkMRMLRobotNode* node = NULL;
  if (this->GetScene() && this->RobotNodeID != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->RobotNodeID);
    node = vtkMRMLRobotNode::SafeDownCast(snode);
    }
  return node;
}



//-------------------------------------------------------------------------------
unsigned int vtkMRMLProstateNavManagerNode::AddTargetDescriptor(vtkProstateNavTargetDescriptor* target)
{
  unsigned int index = this->TargetDescriptorsVector.size();
  target->Register(this);
  this->TargetDescriptorsVector.push_back(target);
  return index;
}
//-------------------------------------------------------------------------------
vtkProstateNavTargetDescriptor* vtkMRMLProstateNavManagerNode::GetTargetDescriptorAtIndex(unsigned int index)
{
  if (index < this->TargetDescriptorsVector.size())
    {
    return this->TargetDescriptorsVector[index];
    }
  else
    {
    return NULL;
    }
}
//-------------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::RemoveTargetDescriptorAtIndex(unsigned int index)
{
  if (index >= this->TargetDescriptorsVector.size())
    {
    return 0;
    }
  this->TargetDescriptorsVector.erase(this->TargetDescriptorsVector.begin()+index);
  return 1;
}
//-------------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::SetCurrentTargetIndex(int index)
{
  if (index >= (int)this->TargetDescriptorsVector.size())
    {
    // invalid index, do not change current
    return this->CurrentTargetIndex;
    }
  this->CurrentTargetIndex=index;

  if (GetRobotNode())
  {
    GetRobotNode()->ShowRobotAtTarget(GetTargetDescriptorAtIndex(this->CurrentTargetIndex));
  }

  this->Modified();
  this->InvokeEvent(vtkMRMLProstateNavManagerNode::CurrentTargetChangedEvent);
  return this->CurrentTargetIndex;
}

//-------------------------------------------------------------------------------
bool vtkMRMLProstateNavManagerNode::AddTargetToFiducialList(double targetRAS[3], unsigned int fiducialListIndex, unsigned int targetNr, int & fiducialIndex)
{
  if (fiducialListIndex >= this->NeedlesVector.size())
    {
    vtkErrorMacro("Invalid fiducial list index");
    return false;
    }
  std::string targetTypeName = this->NeedlesVector[fiducialListIndex].NeedleName;  

  vtkMRMLFiducialListNode* fidNode=this->GetTargetPlanListNode();
  if (fidNode==NULL)
    {
    vtkErrorMacro("No fiducial node is selected");
    return false;
    }

  int modifyOld=fidNode->StartModify();
  fiducialIndex = fidNode->AddFiducialWithXYZ(targetRAS[0], targetRAS[1], targetRAS[2], false);
  bool success=false;
  if (fiducialIndex==-1)
  {
    success=false;
  }
  else
  {
    std::ostrstream os;
    os << targetTypeName << "_" << targetNr << std::ends;
    fidNode->SetNthFiducialLabelText(fiducialIndex, os.str());
    fidNode->SetNthFiducialID(fiducialIndex, os.str());
    os.rdbuf()->freeze();
    fidNode->SetNthFiducialVisibility(fiducialIndex, true);    
    success=true;
  }
  fidNode->EndModify(modifyOld);
  // StartModify/EndModify discarded vtkMRMLFiducialListNode::FiducialModifiedEvent-s, so we have to resubmit them now
  fidNode->InvokeEvent(vtkMRMLFiducialListNode::FiducialModifiedEvent, NULL);
  return true;
}
//-------------------------------------------------------------------------------
bool vtkMRMLProstateNavManagerNode::GetTargetFromFiducialList(int fiducialListIndex, int fiducialIndex, double &r, double &a, double &s)
{
    if (fiducialListIndex < (int)this->NeedlesVector.size() && fiducialListIndex != -1)
      {
      vtkMRMLFiducialListNode* fidNode=NULL;
      fidNode=this->GetTargetPlanListNode();
      if (fidNode!=NULL && fiducialIndex >=0 && fiducialIndex < fidNode->GetNumberOfFiducials())
        {
        float *ras = new float[3];
        ras = fidNode->GetNthFiducialXYZ(fiducialIndex);
        r = ras[0];
        a = ras[1];
        s = ras[2];
        return true;
        }
      }
    return false;
    
}

//-------------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::SetFiducialColor(int fiducialIndex, bool selected)
{
  vtkMRMLFiducialListNode* fidNode=this->GetTargetPlanListNode();
  if (fidNode==NULL)
  {
    vtkErrorMacro("No fiducial list node is selected");
    return;
  }
  int oldModify=fidNode->StartModify();
  fidNode->SetNthFiducialSelected(fiducialIndex, selected);
  fidNode->EndModify(oldModify);
  // StartModify/EndModify discarded vtkMRMLFiducialListNode::FiducialModifiedEvent-s, so we have to resubmit them now
  fidNode->InvokeEvent(vtkMRMLFiducialListNode::FiducialModifiedEvent, NULL);
}

//------------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::SetNeedleType(unsigned int needleIndex, std::string type)
{
  if (needleIndex < this->NeedlesVector.size())
    {
    this->NeedlesVector[needleIndex].NeedleName = type;
    }
}
//----------------------------------------------------------------------------
std::string vtkMRMLProstateNavManagerNode::GetNeedleType(unsigned int needleIndex)
{
  if (needleIndex < this->NeedlesVector.size())
    {
    return this->NeedlesVector[needleIndex].NeedleName;
    }
  else
    {
    return "";
    }
}
//-----------------------------------------------------------------------------  
void vtkMRMLProstateNavManagerNode::SetNeedleDescription(unsigned int needleIndex, std::string desc)
{
  if (needleIndex < this->NeedlesVector.size())
    {
    this->NeedlesVector[needleIndex].Description = desc;
    }
}
//-----------------------------------------------------------------------------
std::string vtkMRMLProstateNavManagerNode::GetNeedleDescription(unsigned int needleIndex)
{
  if (needleIndex < this->NeedlesVector.size())
    {
    return this->NeedlesVector[needleIndex].Description;
    }
  else
    {
    return NULL;
    }
}
//------------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::SetNeedleLength(unsigned int needleIndex, float length)
{
  if (needleIndex < this->NeedlesVector.size())
    {
    this->NeedlesVector[needleIndex].NeedleLength = length;
    }
}
//------------------------------------------------------------------------------
float vtkMRMLProstateNavManagerNode::GetNeedleLength(unsigned int needleIndex)
{
  if (needleIndex < this->NeedlesVector.size())
    {
    return this->NeedlesVector[needleIndex].NeedleLength;
    }
  else
    {
    return 0;
    }
}
//------------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::SetNeedleOvershoot(unsigned int needleIndex, float overshoot)
{
  if (needleIndex < this->NeedlesVector.size())
    {
    this->NeedlesVector[needleIndex].NeedleOvershoot = overshoot;
    }
}
//------------------------------------------------------------------------------
float vtkMRMLProstateNavManagerNode::GetNeedleOvershoot(unsigned int needleIndex)
{
  if (needleIndex < this->NeedlesVector.size())
    {
    return this->NeedlesVector[needleIndex].NeedleOvershoot;
    }
  else
    {
    return 0;
    }
}

void vtkMRMLProstateNavManagerNode::Init()
{
  if (this->Initialized)
  {
    return;
  }

  // TODO: Replace the following dummy data with default read from a config file
 
  // initialize at least one needle  
  NeedleDescriptorStruct needle;

  needle.Description = "Generic";
  needle.NeedleLength = 200;
  needle.NeedleOvershoot = 0;
  needle.NeedleName = "T";
  needle.LastTargetId = 0;
  this->NeedlesVector.push_back(needle);

  needle.Description = "Biopsy";
  needle.NeedleLength = 200;
  needle.NeedleOvershoot = -13; // needle has to be inserted 13 mm short of the target
  needle.NeedleName = "B";
  needle.LastTargetId = 0;
  this->NeedlesVector.push_back(needle);

  needle.Description = "Seed";
  needle.NeedleLength = 200;
  needle.NeedleOvershoot = 1.5; // needle should overshoot the target by 1.5 mm
  needle.NeedleName = "S";
  needle.LastTargetId = 0;
  this->NeedlesVector.push_back(needle);

  this->CurrentNeedleIndex=0;

  // create corresponding targeting fiducials list
  vtkSmartPointer<vtkMRMLFiducialListNode> targetFidList = vtkSmartPointer<vtkMRMLFiducialListNode>::New();  
  targetFidList->SetLocked(true);
  targetFidList->SetName("ProstateNavTargetList");
  targetFidList->SetDescription("ProstateNav target point list");
  targetFidList->SetColor(1.0,1.0,0);
  targetFidList->SetSelectedColor(1.0, 0.0, 0.0);
  targetFidList->SetGlyphType(vtkMRMLFiducialListNode::Sphere3D);
  targetFidList->SetOpacity(0.7);
  targetFidList->SetAllFiducialsVisibility(true);
  targetFidList->SetSymbolScale(5);
  targetFidList->SetTextScale(5);    
  GetScene()->AddNode(targetFidList);
  SetAndObserveTargetPlanListNodeID(targetFidList->GetID());

  this->Initialized=true;
}

bool vtkMRMLProstateNavManagerNode::FindTargetingParams(vtkProstateNavTargetDescriptor *targetDesc)
{
  if (this->RobotNode==NULL)
    {
    return false;
    }
  return this->RobotNode->FindTargetingParams(targetDesc);
}


bool vtkMRMLProstateNavManagerNode::SetNeedle(unsigned int needleIndex, NeedleDescriptorStruct needleDesc)
{
  if (needleIndex >= this->NeedlesVector.size())
    {
    return false;
    }
  this->NeedlesVector[needleIndex]=needleDesc;
  return true;
}

bool vtkMRMLProstateNavManagerNode::GetNeedle(unsigned int needleIndex, NeedleDescriptorStruct &needleDesc)
{
  if (needleIndex >= this->NeedlesVector.size())
    {
    return false;
    }
  needleDesc=this->NeedlesVector[needleIndex];
  return true;
}
