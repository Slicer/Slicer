/*=auto=========================================================================

  Portions (c) Copyright 2010 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/


#include "vtkMRMLCoreTestingMacros.h"

// to make an owner
#include "vtkMRMLModelNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkIntArray.h>

// STD includes

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
  if (caller != NULL)
    {
    vtkMRMLModelNode *callNode = vtkMRMLModelNode::SafeDownCast(caller);
    if (callNode != NULL)
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

  vtkSmartPointer< vtkObserverManager > observerManager = vtkSmartPointer< vtkObserverManager >::New();
  
  if( observerManager == NULL )
    {
    std::cerr << "This class is returning a NULL pointer from its New() method" << std::endl;
    return EXIT_FAILURE;
    }

  EXERCISE_BASIC_OBJECT_METHODS( observerManager );


  observerManager->Modified();

  // set up an owner
  vtkSmartPointer< vtkMRMLModelNode  > modelNode =  vtkSmartPointer< vtkMRMLModelNode > ::New();
  if (modelNode == NULL)
    {
    std::cerr << "Unable to create a model node."  << std::endl;
    return EXIT_FAILURE;
    }
  modelNode->SetName("Owner");
  observerManager->AssignOwner(modelNode);
  vtkObject *owner = observerManager->GetOwner();
  if (vtkMRMLModelNode::SafeDownCast(owner) != modelNode)
    {
    std::cerr << "Error getting owner."  << std::endl;
    return EXIT_FAILURE;
    }
 
  // get the call back
  vtkSmartPointer< vtkCallbackCommand> callbackCommand = observerManager->GetCallbackCommand();
  if (callbackCommand == NULL)
    {
     std::cerr << "Error getting call back command."  << std::endl;
    return EXIT_FAILURE;
    }
  callBackDataStruct *cbStruct = new callBackDataStruct;
  cbStruct->testString = std::string("Testing");
  callbackCommand->SetClientData(cbStruct);
  callbackCommand->SetCallback(TestCallback);

  // set up something to observe
  vtkSmartPointer<vtkMRMLModelNode> observed =  vtkSmartPointer<vtkMRMLModelNode>::New();
//  vtkObject *oldNode = observed;
  observerManager->SetObject(vtkObjectPointer( &(observed)), observed);

  // add some events
  vtkSmartPointer<vtkIntArray> events =  vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  observerManager->AddObjectEvents(observed, events);

  // modify the observed
  observed->SetName("Testing Model Node");
  

  // make a new node to observe
  vtkSmartPointer<vtkMRMLModelNode> observed2 =  vtkSmartPointer<vtkMRMLModelNode>::New();
  observerManager->SetAndObserveObject(vtkObjectPointer( &(observed2)), observed2);
  observerManager->AddObjectEvents(observed2, events);
  observed2->SetName("Testing a second model node");

  observerManager->RemoveObjectEvents(observed2);
  // shouldn't see call back on this one
  observed2->SetName("Don't trigger a callback");

  // another node to observe
  vtkSmartPointer<vtkMRMLModelNode> observed3 =  vtkSmartPointer<vtkMRMLModelNode>::New();
  observerManager->SetAndObserveObjectEvents(vtkObjectPointer( &(observed3)), observed3, events);
  observed3->SetName("Third node callback");

  // not using smart pointers
  vtkMRMLModelNode *observed4 = vtkMRMLModelNode::New();
  observerManager->SetAndObserveObjectEvents(vtkObjectPointer( &(observed4)), observed4, events);
  observed4->SetName("Fourth node callback");
  observerManager->SetAndObserveObject(vtkObjectPointer(&(observed4)), NULL);
  // don't need to call delete, the prior command nulled it
  //observed4->Delete();
  
  return EXIT_SUCCESS;
}
