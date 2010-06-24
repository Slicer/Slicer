#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationRulerStorageNode.h"


#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>

#include "TestingMacros.h"

int vtkMRMLAnnotationRulerNodeTest1(int , char * [] )
{

  // ======================
  // Basic Setup 
  // ======================

  vtkSmartPointer< vtkMRMLAnnotationRulerNode > node2 = vtkSmartPointer< vtkMRMLAnnotationRulerNode >::New();
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();
  // node2->Initialize(mrmlScene);

  {

    vtkSmartPointer< vtkMRMLAnnotationRulerNode > node1 = vtkSmartPointer< vtkMRMLAnnotationRulerNode >::New();  
    // node1->Initialize(mrmlScene);
    EXERCISE_BASIC_OBJECT_METHODS( node1 );

    node1->UpdateReferences();
    node2->Copy( node1 );

    mrmlScene->RegisterNodeClass(node1);
    mrmlScene->AddNode(node2);
  }
  vtkMRMLAnnotationRulerStorageNode *storNode = dynamic_cast <vtkMRMLAnnotationRulerStorageNode *> (node2->CreateDefaultStorageNode());

  if( !storNode )
    {
      std::cerr << "Error in CreateDefaultStorageNode()" << std::endl;
      return EXIT_FAILURE;
    }
  storNode->Delete();

  std::cout << "Passed StorageNode" << std::endl;
  
  // ======================
  // Modify Properties
  // ======================
  node2->Reset();
  node2->StartModify();
  // node2->Initialize(mrmlScene);


  node2->SetName("AnnotationRulerNodeTest") ;
  
  std::string nodeTagName = node2->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  {
    double ctp[3] = { 1, 1, 1};
    node2->SetPosition1(ctp);
  }
  {
    double ctp[3] = { 1, 2, 3};
    node2->SetPosition2(ctp);
  }
 
   double *ctrlPointID = node2->GetPosition2();

   if (ctrlPointID[0]!= 1 || (ctrlPointID[1] != 2) || (ctrlPointID[2] != 3))
    {
      std::cerr << "Error in Array Attributes: "  << ctrlPointID[0] << "!=1, " << ctrlPointID[1]<< "!=2, " << ctrlPointID[2]<< "!=3, " << std::endl;
      return EXIT_FAILURE;
    }

  vtkIndent ind;
  cout << endl << "===== After Adding Points ====" << endl;
  node2->PrintAnnotationInfo(cout,ind);
  cout << "=========" << endl << endl;

  cout << "Passed Adding and Deleting Data" << endl;

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML 
  // ======================

  mrmlScene->SetURL("AnnotationRulerNodeTest.mrml");
  mrmlScene->Commit();

  // Now Read in File to see if ReadXML works - it first disconnects from node2 ! 
  mrmlScene->Connect();
  vtkIndent ij;

  if (mrmlScene->GetNumberOfNodesByClass("vtkMRMLAnnotationRulerNode") != 1) 
    {
        std::cerr << "Error in ReadXML() or WriteXML() - Did not create a class called vtkMRMLAnnotationRulerNode" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLAnnotationRulerNode *node3 = dynamic_cast < vtkMRMLAnnotationRulerNode *> (mrmlScene->GetNthNodeByClass(0,"vtkMRMLAnnotationRulerNode"));
  if (!node3) 
      {
    std::cerr << "Error in ReadXML() or WriteXML(): could not find vtkMRMLAnnotationRulerNode" << std::endl;
    return EXIT_FAILURE;
      }

  std::stringstream initialAnnotation, afterAnnotation;
  
  //cout << " ================= " << endl;
  //mrmlScene->PrintSelf(cout,ind);
  //return 0;
  // node2->PrintSelf(cout,ind);

  node2->PrintAnnotationInfo(initialAnnotation,ind);

  node3->PrintAnnotationInfo(afterAnnotation,ind);
  if (initialAnnotation.str().compare(afterAnnotation.str())) 
  {
    std::cerr << "Error in ReadXML() or WriteXML()" << std::endl;
    std::cerr << "Before:" << std::endl << initialAnnotation.str() <<std::endl;
    std::cerr << "After:" << std::endl << afterAnnotation.str() <<std::endl;
    return EXIT_FAILURE;
  }
  cout << "Passed XML" << endl;

  return EXIT_SUCCESS;
  
}


