#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationLinesNode.h"
#include "vtkMRMLAnnotationLinesStorageNode.h"
#include "vtkMRMLScene.h"

#include <sstream>

#include "vtkMRMLCoreTestingMacros.h"

void SetControlPointsAndText(vtkMRMLAnnotationLinesNode* node2)  {
  node2->AddText("TESTING 1",1,1);
  node2->AddText("TESTING 2",1,1);

  {
    double ctp[3] = { 1, 1, 1};
    node2->AddControlPoint(ctp,1,1);
  }
  {
    double ctp[3] = { 2, 2, 2};
    node2->AddControlPoint(ctp,0,1);
  }
  {
    double ctp[3] = { 1, 2, 3};
    node2->AddControlPoint(ctp,0,0);
  }


}
int vtkMRMLAnnotationLinesNodeTest1(int , char * [] )
{

  // ======================
  // Basic Setup
  // ======================
  vtkSmartPointer< vtkMRMLAnnotationLinesNode > node2 = vtkSmartPointer< vtkMRMLAnnotationLinesNode >::New();
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();
  node2->SetScene(mrmlScene);
  {

    vtkSmartPointer< vtkMRMLAnnotationLinesNode > node1 = vtkSmartPointer< vtkMRMLAnnotationLinesNode >::New();
    node1->SetScene(mrmlScene);
    EXERCISE_BASIC_OBJECT_METHODS( node1 );

    node1->UpdateReferences();
    node2->Copy( node1 );

    mrmlScene->RegisterNodeClass(node1);
    mrmlScene->AddNode(node2);
  }

  vtkMRMLAnnotationLinesStorageNode *storNode = dynamic_cast <vtkMRMLAnnotationLinesStorageNode *> (node2->CreateDefaultStorageNode());

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

  node2->SetScene(mrmlScene);

  node2->CreateAnnotationLineDisplayNode();
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = node2->GetAnnotationLineDisplayNode();
  if (!lineDisplayNode)
    {
    std::cerr << "Error in AnnotationLineDisplayNode() " << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    // register the node type with the scene
    mrmlScene->RegisterNodeClass(lineDisplayNode);
    }

  std::cout << "Passed DisplayNode" << std::endl;

  node2->SetName("AnnotationNodeTest") ;

  std::string nodeTagName = node2->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  SetControlPointsAndText(node2);

  if (node2->AddLine(0,1,1,0) < 0)
    {
      std::cerr << "Error in AddLine" << std::endl;
       return EXIT_FAILURE;
    }

  node2->ResetAnnotations();
  if (node2->GetNumberOfLines())
    {
      std::cerr << "Error in ResetAnnotations" << std::endl;
       return EXIT_FAILURE;
    }

  SetControlPointsAndText(node2);
  node2->AddLine(0,1,1,0);


  int sel = 0;
  int vis = 1;

  node2->AddLine(0,2,sel,vis);

  vtkIdType ctrlPointID[2];

  int flag = node2->GetEndPointsId(1,ctrlPointID);
  if (!flag || ctrlPointID[0] || (ctrlPointID[1] != 2) || (node2->GetAnnotationAttribute(1, vtkMRMLAnnotationLinesNode::LINE_SELECTED)!= sel ) ||  (node2->GetAnnotationAttribute(1, vtkMRMLAnnotationLinesNode::LINE_VISIBLE) != vis))
    {
      std::cerr << "Error in Line Attributes: " << flag << "!=1, " << ctrlPointID[0] << "!=0, " << ctrlPointID[1]<< "!=2" << node2->GetAnnotationAttribute(1, vtkMRMLAnnotationLinesNode::LINE_SELECTED) << "!=" << sel<< ", " << node2->GetAnnotationAttribute(1, vtkMRMLAnnotationLinesNode::LINE_VISIBLE) <<"!="<< vis << std::endl;
      return EXIT_FAILURE;
    }

  node2->AddLine(1,2,0,0);

  if (node2->GetNumberOfLines() != 3)
    {
      std::cerr << "Error in Number of Lines" << std::endl;
      return EXIT_FAILURE;
    }

  node2->DeleteLine(1);
  if (node2->GetNumberOfLines() != 2)
    {
      std::cerr << "Error in Deleting lines" << std::endl;
      return EXIT_FAILURE;
    }

  vtkIndent ind;
  node2->PrintAnnotationInfo(cout,ind);


  cout << "Passed Adding and Deleting Data" << endl;

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  // mrmlScene->SetURL("/home/pohl/Slicer3/Slicer3/QTModules/Reporting/Testing/AnnotationLineNodeTest.mrml");
  mrmlScene->SetURL("AnnotationLineNodeTest.mrml");
  mrmlScene->Commit();
  // Now Read in File to see if ReadXML works - it first disconnects from node2 !
  mrmlScene->Connect();

  if (mrmlScene->GetNumberOfNodesByClass("vtkMRMLAnnotationLinesNode") != 1)
    {
        std::cerr << "Error in ReadXML() or WriteXML()" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLAnnotationLinesNode *node3 = dynamic_cast < vtkMRMLAnnotationLinesNode *> (mrmlScene->GetNthNodeByClass(0,"vtkMRMLAnnotationLinesNode"));
  if (!node3)
      {
    std::cerr << "Error in ReadXML() or WriteXML()" << std::endl;
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

  return EXIT_SUCCESS;

}


