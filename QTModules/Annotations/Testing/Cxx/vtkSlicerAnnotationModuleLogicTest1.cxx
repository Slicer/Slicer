#include "vtkSlicerAnnotationModuleLogic.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include "TestingMacros.h"
#include <time.h>

int WaitForTestReceivedMessage(vtkSlicerAnnotationModuleLogic* node2) 
{  int index = 0;
  // Under windows you can also use Sleep;
  while (index < 10 && !node2->TestReceivedMessage) 
    {
      index ++;
      clock_t wait;
      wait = clock() + 100;
      while( clock() < wait) ; // do nothing but loop...
    }
  if (node2->TestReceivedMessage == 0) 
    {
      return 0;
    }
  // Reset Value 
  node2->TestReceivedMessage = 0;
  return 1;
}

void CheckTextColor(vtkMRMLAnnotationRulerNode *rnode)
{
  cout << "=== ColorCheck: " << rnode->GetAnnotationTextDisplayNode() << " " << rnode->GetAnnotationTextDisplayNode()->GetColor()[0] << " " << rnode->GetAnnotationTextDisplayNode()->GetColor()[1] << " " << rnode->GetAnnotationTextDisplayNode()->GetColor()[2] << endl;

}

int colorTest(double *incolor, double *outcolor)
{
  if (!memcmp(incolor,outcolor,sizeof(double)*3))
    {
      return 0;
    }
  else {
    cout << "incolor  " << incolor[0] << " " << incolor[1] << " " << incolor[2] << endl;
    cout << "outcolor " << outcolor[0] << " " << outcolor[1] << " " << outcolor[2] << endl;
    return EXIT_FAILURE;  
  }

} 

int vtkSlicerAnnotationModuleLogicTest1(int , char * [] )
{

  // ======================
  // Basic Setup 
  // ======================
  vtkSmartPointer<vtkSlicerAnnotationModuleLogic > node2 = vtkSmartPointer< vtkSlicerAnnotationModuleLogic >::New();
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();
  {
    vtkSmartPointer< vtkSlicerAnnotationModuleLogic > node1 = vtkSmartPointer< vtkSlicerAnnotationModuleLogic >::New();  
    EXERCISE_BASIC_OBJECT_METHODS( node1 );
  }

  vtkSmartPointer<vtkMRMLAnnotationRulerNode> rnode = vtkSmartPointer<vtkMRMLAnnotationRulerNode>::New();
  mrmlScene->RegisterNodeClass(rnode);
  mrmlScene->AddNode(rnode);

  double incolor1[3] = { 147.0/255.0, 0 , 0 };
  double incolor2[3] = { 0, 147.0/255.0, 0 };
  double incolor3[3] = { 0, 0, 147.0/255.0};

  node2->SetAnnotationLinesProperties((vtkMRMLAnnotationLinesNode*)rnode, vtkSlicerAnnotationModuleLogic::TEXT_COLOR, incolor1);
  double *outcolor1 = rnode->GetAnnotationTextDisplayNode()->GetColor();
  if ( colorTest(incolor1,outcolor1))
    {
      std::cout << "Setting Text Color Failed!" << std::endl;
      return EXIT_FAILURE;  
    }
  std::cout << "Setting Text Color Ok!" << std::endl;

  node2->SetAnnotationLinesProperties(rnode, vtkSlicerAnnotationModuleLogic::POINT_COLOR, incolor2);
  double *outcolor2 = rnode->GetAnnotationPointDisplayNode()->GetColor();
  if ( colorTest(incolor2,outcolor2))
    {
      std::cout << "Setting Point Color Failed!" << std::endl;
      return EXIT_FAILURE;  
    }
  std::cout << "Setting Point Color Ok!" << std::endl;

  node2->SetAnnotationLinesProperties(rnode, vtkSlicerAnnotationModuleLogic::LINE_COLOR, incolor3);
  double *outcolor3 = rnode->GetAnnotationLineDisplayNode()->GetColor();
  if ( colorTest(incolor3,outcolor3))
    {
      std::cout << "Setting Line Color Failed!" << std::endl;
      return EXIT_FAILURE;  
    }
  std::cout << "Setting Line Color Ok!" << std::endl;
  
  double* getcolor1 = node2->GetAnnotationLinesPropertiesColor(rnode, vtkSlicerAnnotationModuleLogic::TEXT_COLOR);

  if (colorTest(incolor1, getcolor1))
    {
      cout << "Correct DisplayNode: " <<rnode->GetAnnotationTextDisplayNode() << " " <<   rnode->GetAnnotationLineDisplayNode() << endl;
      std::cout << "Getting Text Color Failed!" << std::endl;
      return EXIT_FAILURE;  
    }
  std::cout << "Getting Text Color Ok!" << std::endl;
    
  double* getcolor2 = node2->GetAnnotationLinesPropertiesColor(rnode, vtkSlicerAnnotationModuleLogic::POINT_COLOR);
  if (colorTest(incolor2, getcolor2))
    {
    std::cout << "Getting Point Color Failed!" << std::endl;
    return EXIT_FAILURE;  
    }
  std::cout << "Getting Point Color Ok!" << std::endl;

  double* getcolor3 = node2->GetAnnotationLinesPropertiesColor(rnode, vtkSlicerAnnotationModuleLogic::LINE_COLOR);
  if (colorTest(incolor3, getcolor3))
    {
      std::cout << "Getting Line Color Failed!" << std::endl;
      return EXIT_FAILURE;  
    }

  std::cout << "Getting Line Color Ok!" << std::endl;
  

  // Check if we receive message
  node2->AddRulerNodeObserver(rnode);
  // Just has to be done once 
  node2->TestReceivedMessage = 0;
  if (WaitForTestReceivedMessage(node2))
    {
      cout << "WaitForTestReceivedMessage does not work" << endl;
      return EXIT_FAILURE; 
    }
  cout << "WaitForTestReceivedMessage works" << endl;


  // 1.) Test
  double ctp[3] = { 1, 1, 1};

  rnode->SetPosition1(ctp);
  if (!WaitForTestReceivedMessage(node2))
    {
      cout << "Problem with SetPosition1" << endl;
      return EXIT_FAILURE; 
    }
  cout << "SetPosition1 OK" << endl;

  rnode->SetPosition2(ctp);
  if (!WaitForTestReceivedMessage(node2))
    {
      cout << "Problem with SetPosition2" << endl;
      return EXIT_FAILURE; 
    }
  cout << "SetPosition2 OK" << endl;

 
  // 2.) Text Display Node Test..
  double scale = 20;
  rnode->GetAnnotationTextDisplayNode()->SetTextScale( scale );
  if (!WaitForTestReceivedMessage(node2))
    {
      cout << "Problem with SetTextScale" << endl;
      return EXIT_FAILURE; 
    }
  cout << "SetTextScale OK" << endl;
  

  return EXIT_SUCCESS;  
}



