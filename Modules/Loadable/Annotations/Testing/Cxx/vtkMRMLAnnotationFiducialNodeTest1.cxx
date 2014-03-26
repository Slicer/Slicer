#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLScene.h"

#include <sstream>

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationFiducialNodeTest1(int , char * [] )
{

  // ======================
  // Basic Setup
  // ======================
  vtkSmartPointer< vtkMRMLAnnotationFiducialNode > node2 = vtkSmartPointer< vtkMRMLAnnotationFiducialNode >::New();
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();
  node2->SetScene(mrmlScene);


  vtkSmartPointer< vtkMRMLAnnotationFiducialNode > node1 = vtkSmartPointer< vtkMRMLAnnotationFiducialNode >::New();
  node1->SetScene(mrmlScene);
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  node1->UpdateReferences();
  node2->Copy( node1 );

  mrmlScene->RegisterNodeClass(node1);
  mrmlScene->AddNode(node2);


  std::cout << "Passed Basic" << std::endl;

  // ======================
  // Modify Properties
  // ======================
  node2->Reset();
  node2->StartModify();

  node2->SetScene(mrmlScene);
  node2->CreateAnnotationPointDisplayNode();
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = node2->GetAnnotationPointDisplayNode();
  if (!pointDisplayNode)
    {
    std::cerr << "Error in AnnotationPointDisplayNode() " << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    // register with the scene
    mrmlScene->RegisterNodeClass(pointDisplayNode);
    }
  std::cout << "Passed DisplayNode" << std::endl;

  node2->SetName("AnnotationFidcucialNodeTest") ;

  std::string nodeTagName = node2->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  double ctp[3] = { 1, 2, 3};
  const char* text = "Test 1 2";
  node2->SetFiducialLabel(text);
  if (!node2->SetFiducial(ctp,1,0))
    {
    vtkIndent f;
    node2->PrintSelf(cout,f);
    std::cerr << "Error: Could not define Fiducial " << std::endl;
    return EXIT_FAILURE;
    }
  node2->SetSelected(1);
  node2->SetDisplayVisibility(0);

  double *_ctp =  node2->GetFiducialCoordinates();

  if ( !node2->GetNumberOfTexts() || node2->GetFiducialLabel().compare(text))
    {
    std::cerr << "Error in SetFiducial: Label is not set correct " << std::endl;
    return EXIT_FAILURE;
      }

  if (_ctp[0] != ctp[0] ||_ctp[1] != ctp[1] ||_ctp[2] != ctp[2])
    {
    std::cerr << "Error in SetFiducial: coordinates are not set correct " << std::endl;
    return EXIT_FAILURE;
    }

  if (!node2->GetSelected() || node2->GetDisplayVisibility())
    {
    std::cerr << "Error in SetFiducial: attributes are not set correct " << std::endl;
    return EXIT_FAILURE;
    }

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  // mrmlScene->SetURL("/home/pohl/Slicer3/Slicer3/QTModules/Reporting/Testing/AnnotationControlPointNodeTest.mrml");
  mrmlScene->SetURL("AnnotationFiducialNodeTest.mrml");
  mrmlScene->Commit();
  // Now Read in File to see if ReadXML works - it first disconnects from node2 !
  mrmlScene->Connect();

  if (mrmlScene->GetNumberOfNodesByClass("vtkMRMLAnnotationFiducialNode") != 1)
    {
    std::cerr << "Error in ReadXML() or WriteXML()" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLAnnotationFiducialNode *node3 = dynamic_cast < vtkMRMLAnnotationFiducialNode *> (mrmlScene->GetNthNodeByClass(0,"vtkMRMLAnnotationFiducialNode"));
  if (!node3)
    {
    std::cerr << "Error in ReadXML() or WriteXML()" << std::endl;
    return EXIT_FAILURE;
    }

  vtkIndent ind;
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


  // std::stringstream ss;
  // node2->WriteXML(ss,5);
  // std::string writeXML = ss.str();
  // std::vector<std::string> tmpVec;
  //
  // size_t pos = writeXML.find("     ");
  // while (pos != std::string::npos)
  //   {
  //     pos += 6;
  //     size_t fix = writeXML.find('=',pos);
  //     tmpVec.push_back(writeXML.substr(pos,fix - pos));
  //     fix +=2;
  //     pos = writeXML.find("\"     ",fix);
  //
  //     if (pos == std::string::npos)
  //     {
  //       std::string tmp = writeXML.substr(fix);
  //       std::replace(tmp.begin(), tmp.end(), '\"', ' ');
  //       // tmp.erase(remove_if(tmp.begin(), tmp.end(), isspace), tmp.end());
  //       tmpVec.push_back(tmp);
  //     }
  //     else
  //     {
  //       tmpVec.push_back(writeXML.substr(fix,pos-fix));
  //       pos ++;
  //     }
  //   }
  //
  //
  // const char **readXML = new const char*[tmpVec.size()+1];
  // for (int i= 0 ; i < int(tmpVec.size()); i++)
  //   {
  //     readXML[i] =  tmpVec[i].c_str();
  //   }
  // readXML[tmpVec.size()]= NULL;
  // node2->ReadXMLAttributes(readXML);
  // delete[] readXML;


