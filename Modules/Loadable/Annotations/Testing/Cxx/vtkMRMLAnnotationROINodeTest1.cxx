#include "vtkMRMLAnnotationROINode.h"
#include <vtkMRMLScene.h>


#include "vtkMRMLCoreTestingMacros.h"

/* this test has been adopted from vtkMRMLAnnotationAngleTest1 by
 * Andrey Fedorov to demonstrate some of the problems observed with the ROI
 * annotation node
 */

int vtkMRMLAnnotationROINodeTest1(int , char * [] )
{

  // ======================
  // Basic Setup
  // ======================

  vtkSmartPointer< vtkMRMLAnnotationROINode > node2 = vtkSmartPointer< vtkMRMLAnnotationROINode >::New();
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();
  // node2->Initialize(mrmlScene);

  {

    vtkSmartPointer< vtkMRMLAnnotationROINode > node1 = vtkSmartPointer< vtkMRMLAnnotationROINode >::New();
    // node1->Initialize(mrmlScene);

    EXERCISE_BASIC_OBJECT_METHODS( node1 );

    node1->UpdateReferences();
    node2->Copy( node1 );

    mrmlScene->RegisterNodeClass(node1);
    mrmlScene->AddNode(node2);
  }
/*
 * No ROI storage node exists yet
  vtkMRMLAnnotationROIStorageNode *storNode = dynamic_cast <vtkMRMLAnnotationROIStorageNode *> (node2->CreateDefaultStorageNode());

  if( !storNode )
    {
      std::cerr << "Error in CreateDefaultStorageNode()" << std::endl;
      return EXIT_FAILURE;
    }
  storNode->Delete();

  std::cout << "Passed StorageNode" << std::endl;
*/
  // ======================
  // Modify Properties
  // ======================
  node2->Reset();
  node2->StartModify();
  //node2->Initialize(mrmlScene);


  node2->SetName("AnnotationROINodeTest") ;

  std::string nodeTagName = node2->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  /*
  {
    double ctp[3] = { 1, 1, 1};
    node2->SetPosition1(ctp);
  }
  {
    double ctp[3] = { 2, 2, 2};
    node2->SetPosition2(ctp);
  }
  {
    double ctp[3] = { 1, 2, 3};
    node2->SetPositionCenter(ctp);
  }


  int vis = 1;
  node2->SetRay1Visibility(vis);


  double *ctrlPointID = node2->GetPositionCenter();

  if (ctrlPointID[0]!= 1 || (ctrlPointID[1] != 2) || (ctrlPointID[2] != 3) ||  node2->GetRay1Visibility() != vis)
    {
      std::cerr << "Error in Array Attributes: "  << ctrlPointID[0] << "!=1, " << ctrlPointID[1]<< "!=2" << ctrlPointID[2]<< "!=3, "
        << node2->GetRay1Visibility() <<"!="<< vis << std::endl;
      return EXIT_FAILURE;
    }
  */

  vtkIndent ind;
  int retval;
  retval = node2->SetXYZ(3,-5,0);
  if (!retval)
    {
    std::cerr << "ERROR: got " << retval << " when trying to call SetXYZ" << std::endl;
    return EXIT_FAILURE;
    }
  retval = node2->SetRadiusXYZ(100,200,-10);
  if  (!retval)
    {
    std::cerr << "ERROR: got " << retval << " when trying to call SetRadiusXYZ" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "PrintAnnotationInfo:" << std::endl;
  node2->PrintAnnotationInfo(cout,ind);

  double center[3];
  node2->GetXYZ(center);
  double radius[3];
  node2->GetRadiusXYZ(radius);

  cout << "Center: " << center[0] << ", " << center[1] << ", " << center[2] << std::endl;
  cout << "Radius: " << radius[0] << ", " << radius[1] << ", " << radius[2] << std::endl;

  if(center[0]!=3 || center[1]!=-5 || center[2]!=0 ||
     radius[0]!=100 || radius[1]!=200 || radius[2]!=-10)
    {
    std::cerr << "Error: Center and/or radius not as expected, should be 3,-5,0 and 100,200,-10" << std::endl;
    return EXIT_FAILURE;
    }
  cout << "Passed Adding and Deleting Data" << endl;

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  /*
  mrmlScene->SetURL("AnnotationROINodeTest.mrml");
  mrmlScene->Commit();

  // Now Read in File to see if ReadXML works - it first disconnects from node2 !
  mrmlScene->Connect();
  vtkIndent ij;

  if (mrmlScene->GetNumberOfNodesByClass("vtkMRMLAnnotationROINode") != 1)
    {
        std::cerr << "Error in ReadXML() or WriteXML() - Did not create a class called vtkMRMLAnnotationROINode" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLAnnotationROINode *node3 = dynamic_cast < vtkMRMLAnnotationROINode *> (mrmlScene->GetNthNodeByClass(0,"vtkMRMLAnnotationROINode"));
  if (!node3)
      {
    std::cerr << "Error in ReadXML() or WriteXML(): could not find vtkMRMLAnnotationROINode" << std::endl;
    return EXIT_FAILURE;
      }

  std::stringstream initialAnnotation, afterAnnotation;


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
  */

  return EXIT_SUCCESS;

}


