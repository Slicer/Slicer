#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationStorageNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLScene.h"

#include <sstream>

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationNodeTest1(int , char * [] )
{

  // ======================
  // Basic Setup
  // ======================
  vtkNew<vtkMRMLAnnotationNode> node2;
  // This was a bug - used to crash
  node2->GetText(0);

  vtkNew<vtkMRMLScene> mrmlScene;
  vtkNew<vtkMRMLAnnotationNode> node1;
  mrmlScene->AddNode(node1.GetPointer());

  node1->UpdateReferences();
  node2->Copy(node1.GetPointer());

  mrmlScene->RegisterNodeClass(node1.GetPointer());
  mrmlScene->AddNode(node2.GetPointer());

  vtkNew<vtkMRMLAnnotationStorageNode> prototypeStorageNode;
  mrmlScene->RegisterNodeClass(prototypeStorageNode.GetPointer());
  vtkMRMLAnnotationStorageNode *storNode = vtkMRMLAnnotationStorageNode::SafeDownCast(
    node2->CreateDefaultStorageNode());

  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  mrmlScene->RemoveNode(node1.GetPointer());

  if( !storNode )
    {
    std::cerr << "Error in CreateDefaultStorageNode()" << std::endl;
    return EXIT_FAILURE;
    }
  storNode->Delete();

  std::cout << "Passed StorageNode" << std::endl;

  node2->CreateAnnotationTextDisplayNode();
  vtkMRMLAnnotationTextDisplayNode *textDisplayNode = node2->GetAnnotationTextDisplayNode();
  if (!textDisplayNode)
    {
    std::cerr << "Error in AnnotationTextDisplayNode() " << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    // register the node type to the scene
    mrmlScene->RegisterNodeClass(textDisplayNode);
    }
  std::cout << "Passed DisplayNode" << std::endl;


  // ======================
  // Modify Properties
  // ======================
  node2->Reset(nullptr);
  node2->StartModify();

  node2->SetName("AnnotationNodeTest") ;

  std::string nodeTagName = node2->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  const char* text = "Test 1";
  int id = node2->AddText(text, 1, 0);
  if (id < 0 || node2->GetText(id).compare(text))
    {
      std::cerr << "Error in AddText(): did not add text correctly " << id << " '" << node2->GetText(id) << "'" <<  std::endl;
      return EXIT_FAILURE;
    }
  if (node2->GetAnnotationAttribute(id, vtkMRMLAnnotationNode::TEXT_VISIBLE) || !node2->GetAnnotationAttribute(id, vtkMRMLAnnotationNode::TEXT_SELECTED))
    {
      std::cerr << "Error in AddText(): Attributes are not correct " << std::endl;
      return EXIT_FAILURE;
    }

  node2->AddText("Test ,2"  , 0, 1);
  node2->AddText("Test3"  , 1, 1);
  id = node2->AddText("Test3"  , 1, 1);

  if (node2->GetNumberOfTexts() != 4)
    {
       std::cerr << "Error in AddText() " << std::endl;
       return EXIT_FAILURE;
    }

  node2->DeleteText(id);

  if (node2->GetNumberOfTexts() != 3)
    {
       std::cerr << "Error in DeleteText() " << std::endl;
       return EXIT_FAILURE;
    }

  const char* text2 = "Test New";
  node2->SetText(0,text2, 1, 1);
  if (node2->GetText(0).compare(text2))
    {
      std::cerr << "Error in SetText(): did not set text correctly " << 0 << " '" << node2->GetText(0) << "'" <<  std::endl;
      return EXIT_FAILURE;
    }


  cout << "Passed Text" << endl;

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML
  // ======================
  mrmlScene->SetURL("./AnnotationNodeTest.mrml");
  mrmlScene->Commit();
  // Now Read in File to see if ReadXML works - it first disconnects from node2 !
  mrmlScene->Connect();

  if (mrmlScene->GetNumberOfNodesByClass("vtkMRMLAnnotationNode") != 1)
    {
        std::cerr << "Error in ReadXML() or WriteXML()" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLAnnotationNode *node3 = vtkMRMLAnnotationNode::SafeDownCast(mrmlScene->GetFirstNodeByClass("vtkMRMLAnnotationNode"));
  if (!node3)
    {
    std::cerr << "Error in ReadXML() or WriteXML()" << std::endl;
    return EXIT_FAILURE;
    }

  vtkIndent ind;
  std::stringstream initialAnnotation, afterAnnotation;

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
