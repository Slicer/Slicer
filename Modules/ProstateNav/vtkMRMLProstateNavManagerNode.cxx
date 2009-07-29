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

#include "vtkMRMLScene.h"

#include "vtkProstateNavStep.h"


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
  this->StepList.clear();
  this->StepTransitionMatrix.clear();
  this->CurrentStep = 0;
  this->PreviousStep = 0;
}

//----------------------------------------------------------------------------
vtkMRMLProstateNavManagerNode::~vtkMRMLProstateNavManagerNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::WriteXML(ostream& of, int nIndent)
{

  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  //switch (this->Type)
  //  {
  //  case TYPE_SERVER:
  //    of << " connectorType=\"" << "SERVER" << "\" ";
  //    break;
  //  case TYPE_CLIENT:
  //    of << " connectorType=\"" << "CLIENT" << "\" ";
  //    of << " serverHostname=\"" << this->ServerHostname << "\" ";
  //    break;
  //  default:
  //    of << " connectorType=\"" << "NOT_DEFINED" << "\" ";
  //    break;
  //  }
  //
  //of << " serverPort=\"" << this->ServerPort << "\" ";
  //of << " restrictDeviceName=\"" << this->RestrictDeviceName << "\" ";

}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;

  const char* serverHostname = "";
  int port = 0;
  int type = -1;
  int restrictDeviceName = 0;

  /*
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "connectorType"))
      {
      if (!strcmp(attValue, "SERVER"))
        {
        type = TYPE_SERVER;
        }
      else if (!strcmp(attValue, "CLIENT"))
        {
        type = TYPE_CLIENT;
        }
      else
        {
        type = TYPE_NOT_DEFINED;
        }
      }
    if (!strcmp(attName, "serverHostname"))
      {
      serverHostname = attValue;
      }
    if (!strcmp(attName, "serverPort"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> port;
      }
    if (!strcmp(attName, "restrictDeviceName"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> restrictDeviceName;;
      }
    }

  switch(type)
    {
    case TYPE_SERVER:
      this->SetTypeServer(port);
      this->SetRestrictDeviceName(restrictDeviceName);
      break;
    case TYPE_CLIENT:
      this->SetTypeClient(serverHostname, port);
      this->SetRestrictDeviceName(restrictDeviceName);
      break;
    default: // not defined
      // do nothing
      break;
    }
  */

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLProstateNavManagerNode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
  vtkMRMLProstateNavManagerNode *node = (vtkMRMLProstateNavManagerNode *) anode;

  //int type = node->GetType();
  
}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  if (this->TargetPlanList && this->TargetPlanList == vtkMRMLFiducialListNode::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    //this->ModifiedSinceReadOn();
    //this->InvokeEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent, NULL);
    //this->UpdateFromMRML();
    return;
    }

  if (this->TargetCompletedList && this->TargetCompletedList == vtkMRMLFiducialListNode::SafeDownCast(caller) &&
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
  return this->StepList.size();
}


//----------------------------------------------------------------------------
const char* vtkMRMLProstateNavManagerNode::GetStepName(int i)
{
  if (i < this->StepList.size())
    {
    return this->StepList[i].name.c_str();
    }
  else
    {
    return NULL;
    }
}


//----------------------------------------------------------------------------
vtkProstateNavStep* vtkMRMLProstateNavManagerNode::GetStepPage(int i)
{
  if (i < this->StepList.size())
    {
    return this->StepList[i].page;
    }
  else
    {
    return NULL;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::AddNewStep(const char* name, vtkProstateNavStep* page)
{

  // Add to the list
  StepInfoType step;

  step.name  = name;
  step.page  = page;

  this->StepList.push_back(step);
  
  // resize the transition matrix
  int numSteps = this->StepList.size();
  this->StepTransitionMatrix.resize(numSteps);
  
  std::vector< std::vector<int> >::iterator iter;
  for (iter = this->StepTransitionMatrix.begin();
       iter != this->StepTransitionMatrix.end(); iter ++)
    {
    iter->resize(numSteps);
    }

  // Configure the vtkProstateNavStep class
  for (int i = 0; i < numSteps; i ++)
    {
    this->StepList[i].page->SetTotalSteps(numSteps);
    this->StepList[i].page->SetStepNumber(i+1);
    this->StepList[i].page->UpdateName();
    this->StepList[i].page->SetProstateNavManager(this);
    }

}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::ClearSteps()
{

  // Clear the list of step info
  std::vector<StepInfoType>::iterator iter;
  
  for (iter = this->StepList.begin(); iter != this->StepList.end(); iter ++)
    {
    iter->page->Delete();
    }

  this->StepList.clear();

  // Clear the step transtion matrix
  std::vector< std::vector<int> >::iterator iter2;
  for (iter2 = this->StepTransitionMatrix.begin();
       iter2 != this->StepTransitionMatrix.end(); iter2 ++)
    {
    iter2->clear();
    }
  this->StepTransitionMatrix.clear();
  
}


//----------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::SwitchStep(int i)
{

  if (i >= 0 && i < this->StepList.size())
    {
    if (this->StepTransitionMatrix[this->CurrentStep][i])
      {
      this->PreviousStep = this->CurrentStep;
      this->CurrentStep = i;
      return 1;
      }
    else
      {
      return 0;
      }
    }
  else
    {
    return 0;
    }
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
void vtkMRMLProstateNavManagerNode::AllowAllTransitions()
{
  std::vector< std::vector<int> >::iterator iter;
  for (iter = this->StepTransitionMatrix.begin();
       iter != this->StepTransitionMatrix.end(); iter ++)
    {
    std::vector<int>::iterator iter2;
    for (iter2 = iter->begin(); iter2 != iter->end(); iter2 ++)
      {
      *iter2 = 1;
      }
    }

}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::ForbidAllTransitions()
{
  std::vector< std::vector<int> >::iterator iter;
  for (iter = this->StepTransitionMatrix.begin();
       iter != this->StepTransitionMatrix.end(); iter ++)
    {
    std::vector<int>::iterator iter2;
    for (iter2 = iter->begin(); iter2 != iter->end(); iter2 ++)
      {
      *iter2 = 0;
      }
    }
}


//----------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::SetStepTransitionMatrix(const int** matrix)
{
  int numSteps = this->StepTransitionMatrix.size();  

  for (int step_from = 0; step_from < numSteps; step_from ++)
    {
    for (int step_to = 0; step_to < numSteps; step_to ++)
      {
      this->StepTransitionMatrix[step_from][step_to] = matrix[step_from][step_to];
      }
    }
  return 1;
}

  
//----------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::SetAllowTransition(int step_from, int step_to)
{

  int numSteps = this->StepTransitionMatrix.size();

  // Check the range
  if (step_from >= 0 && step_from < numSteps && step_to >= 0 && step_to < numSteps)
    {
    this->StepTransitionMatrix[step_from][step_to] = 1;
    return 1;
    }
  else
    {
    return 0;
    }

}

  
//----------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::SetForbidTransition(int step_from, int step_to)
{

  int numSteps = this->StepTransitionMatrix.size();

  // Check the range
  if (step_from >= 0 && step_from < numSteps && step_to >= 0 && step_to < numSteps)
    {
    this->StepTransitionMatrix[step_from][step_to] = 0;
    return 1;
    }
  else
    {
    return 0;
    }

}


//----------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::IsTransitionable(int step_from, int step_to)
{

  int numSteps = this->StepTransitionMatrix.size();

  // Check the range
  if (step_from >= 0 && step_from < numSteps
      && step_to >= 0 && step_to < numSteps)
    {
    return this->StepTransitionMatrix[step_from][step_to];
    }
  else
    {
    return -1;
    }

}


//----------------------------------------------------------------------------
int vtkMRMLProstateNavManagerNode::IsTransitionable(int step_to)
{

  int numSteps = this->StepTransitionMatrix.size();

  if (step_to >= 0 && step_to < numSteps)  // Check the range
    {
    return this->StepTransitionMatrix[this->CurrentStep][step_to];
    }
  else
    {
    return -1;
    }

}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::SetAndObserveTargetPlanList(vtkMRMLFiducialListNode* ptr)
{

  vtkMRMLFiducialListNode *oldList = this->TargetPlanList;

  if (this->TargetPlanList != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->TargetPlanList, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  this->TargetPlanList = ptr;

  if (ptr != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      ptr, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  if ( this->TargetPlanList != ptr )
    {
    this->Modified();
    }

}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::SetAndObserveTargetCompletedList(vtkMRMLFiducialListNode* ptr)
{

  vtkMRMLFiducialListNode *oldList = this->TargetCompletedList;

  if (this->TargetCompletedList != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->TargetCompletedList, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  this->TargetCompletedList = ptr;

  if (ptr != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      ptr, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  if ( this->TargetCompletedList != ptr )
    {
    this->Modified();
    }


}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::SetAndObserveRobotConnector(vtkMRMLIGTLConnectorNode* ptr)
{

  vtkMRMLIGTLConnectorNode *oldList = this->RobotConnector;

  if (this->RobotConnector != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->RobotConnector, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  this->RobotConnector = ptr;

  if (ptr != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      ptr, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  if ( this->RobotConnector != ptr )
    {
    this->Modified();
    }

}


//----------------------------------------------------------------------------
void vtkMRMLProstateNavManagerNode::SetAndObserveScannerConnector(vtkMRMLIGTLConnectorNode* ptr)
{

  vtkMRMLIGTLConnectorNode *oldList = this->ScannerConnector;

  if (this->ScannerConnector != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->ScannerConnector, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  this->ScannerConnector = ptr;

  if (ptr != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      ptr, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  if ( this->ScannerConnector != ptr )
    {
    this->Modified();
    }

}
