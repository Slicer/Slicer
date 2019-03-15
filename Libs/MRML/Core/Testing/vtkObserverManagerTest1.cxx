/*=auto=========================================================================

  Portions (c) Copyright 2010 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

struct callBackDataStruct{
  std::string testString;
};

void TestCallback( vtkObject *caller,
                   unsigned long vtkNotUsed(eid),
                   void *clientData, void *vtkNotUsed(callData) )
{
  std::cout << "TestCallback" << std::endl;
  callBackDataStruct *myCBStruct = (callBackDataStruct*)clientData;
  if (myCBStruct)
    {
    std::cout << "testString = " << myCBStruct->testString << std::endl;
    }
  if (caller != nullptr)
    {
    vtkMRMLModelNode *callNode = vtkMRMLModelNode::SafeDownCast(caller);
    if (callNode != nullptr)
      {
      std::cout << "Caller name = " << callNode->GetName();
      }
    else
      {
      std::cerr << "Null caller node" << std::endl;
      }
    }
}

int vtkObserverManagerTest1(int , char * [] )
{
  vtkNew<vtkObserverManager> observerManager;
  EXERCISE_BASIC_OBJECT_METHODS(observerManager.GetPointer());

  observerManager->Modified();

  // set up an owner
  vtkNew<vtkMRMLModelNode> modelNode;
  modelNode->SetName("Owner");
  observerManager->AssignOwner(modelNode.GetPointer());
  vtkObject *owner = observerManager->GetOwner();
  if (vtkMRMLModelNode::SafeDownCast(owner) != modelNode.GetPointer())
    {
    std::cerr << "Error getting owner."  << std::endl;
    return EXIT_FAILURE;
    }

  // get the call back
  vtkCallbackCommand* callbackCommand = observerManager->GetCallbackCommand();
  if (callbackCommand == nullptr)
    {
     std::cerr << "Error getting call back command."  << std::endl;
    return EXIT_FAILURE;
    }
  callBackDataStruct *cbStruct = new callBackDataStruct;
  cbStruct->testString = std::string("Testing");
  callbackCommand->SetClientData(cbStruct);
  callbackCommand->SetCallback(TestCallback);

  // set up something to observe
  vtkSmartPointer<vtkMRMLModelNode> observed = vtkSmartPointer<vtkMRMLModelNode>::New();
  observerManager->SetObject(vtkObjectPointer( &(observed)), observed);

  // add some events
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  observerManager->AddObjectEvents(observed.GetPointer(), events.GetPointer());

  // modify the observed
  observed->SetName("Testing Model Node");


  // make a new node to observe
  vtkSmartPointer<vtkMRMLModelNode> observed2 = vtkSmartPointer<vtkMRMLModelNode>::New();
  observerManager->SetAndObserveObjectEvents(vtkObjectPointer( &(observed2)), observed2, events.GetPointer());
  observed2->SetName("Testing a second model node");

  observerManager->RemoveObjectEvents(observed2);
  // shouldn't see call back on this one
  observed2->SetName("Don't trigger a callback");

  // another node to observe
  vtkSmartPointer<vtkMRMLModelNode> observed3 = vtkSmartPointer<vtkMRMLModelNode>::New();
  observerManager->SetAndObserveObjectEvents(vtkObjectPointer( &(observed3)), observed3, events.GetPointer());
  observed3->SetName("Third node callback");

  // not using smart pointers
  vtkMRMLModelNode *observed4 = vtkMRMLModelNode::New();
  observerManager->SetAndObserveObjectEvents(vtkObjectPointer( &(observed4)), observed4, events.GetPointer());
  observed4->SetName("Fourth node callback");
  observerManager->SetAndObserveObject(vtkObjectPointer(&(observed4)), nullptr);
  // don't need to call delete, the prior command nulled it
  //observed4->Delete();

  return EXIT_SUCCESS;
}
