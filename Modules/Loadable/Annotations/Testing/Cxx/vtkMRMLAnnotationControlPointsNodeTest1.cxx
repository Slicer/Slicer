#include "vtkMRMLAnnotationControlPointsNode.h"
#include "vtkMRMLAnnotationControlPointsStorageNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLScene.h"

#include <sstream>

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationControlPointsNodeTest1(int , char * [] )
{

  // ======================
  // Basic Setup
  // ======================
  vtkSmartPointer< vtkMRMLAnnotationControlPointsNode > node2 = vtkSmartPointer< vtkMRMLAnnotationControlPointsNode >::New();
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();
  node2->SetScene(mrmlScene);
  {

    vtkSmartPointer< vtkMRMLAnnotationControlPointsNode > node1 = vtkSmartPointer< vtkMRMLAnnotationControlPointsNode >::New();
    node1->SetScene(mrmlScene);
    EXERCISE_BASIC_OBJECT_METHODS( node1 );

    node1->UpdateReferences();
    node2->Copy( node1 );

    mrmlScene->RegisterNodeClass(node1);
    mrmlScene->AddNode(node2);
  }

  vtkMRMLAnnotationStorageNode *storNode = dynamic_cast <vtkMRMLAnnotationStorageNode *> (node2->CreateDefaultStorageNode());

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
  node2->CreateAnnotationPointDisplayNode();
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = node2->GetAnnotationPointDisplayNode();
  if (!pointDisplayNode)
    {
    std::cerr << "Error in AnnotationPointDisplayNode() " << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    // register with scene
    mrmlScene->RegisterNodeClass(pointDisplayNode);
    }

  std::cout << "Passed DisplayNode" << std::endl;

  node2->SetName("AnnotationNodeTest") ;

  std::string nodeTagName = node2->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  {
    double ctp[3] = { 1, 1, 1};
    node2->AddControlPoint(ctp,1,1);
   // double* _ctp =node2->GetControlPointCoordinates(0);
   // if (!_ctp)
   //   {
   //     std::cerr << "Error in AddControlPoint() " << std::endl;
   //     return EXIT_FAILURE;
   //   }
   //
   // if ((ctp[0] != _ctp[0]) || (ctp[1] != _ctp[1]) ||(ctp[2] != _ctp[2]) )
   //   {
   //     std::cerr << "Error in AddControlPoint(): Coordinates are not correct " << std::endl;
   //     return EXIT_FAILURE;
   //   }

    if (!node2->GetAnnotationAttribute(0, vtkMRMLAnnotationControlPointsNode::CP_VISIBLE)
        || !node2->GetAnnotationAttribute(0, vtkMRMLAnnotationControlPointsNode::CP_SELECTED))
      {
    std::cerr << "Error in AddControlPoint(): Attributes are not correct " << std::endl;
    return EXIT_FAILURE;
      }

  }
  {
    double ctp[3] = { 2, 2, 2};
    node2->AddControlPoint(ctp,0,1);
  }
  {
    double ctp[3] = { 1, 2, 3};
    node2->SetControlPoint(3,ctp,0,0);
    //double* _ctp =node2->GetControlPointCoordinates(2);
    //if (!_ctp)
    //  {
    //    cout << "Got zero" << endl;
    //  }
    //cout << "Got " << _ctp[0] << " " << _ctp[1] << " " << _ctp[2] << endl;

    node2->SetControlPoint(2,ctp,0,0);
  }

  if (node2->GetNumberOfControlPoints() != 4)
    {
       std::cerr << "Error in AddControlPoint() " << std::endl;
       return EXIT_FAILURE;
    }

  node2->DeleteControlPoint(3);
  if (node2->GetNumberOfControlPoints() != 3)
    {
       std::cerr << "Error in DeleteControlPoint() " << std::endl;
       return EXIT_FAILURE;
    }

  cout << "Passed ControlPoint" << endl;

  node2->AddText("TESTING",1,1);

  if ((node2->GetNumberOfTexts() != 1) || node2->GetText(0).compare("TESTING") )
    {
       std::cerr << "Error in AddText() " << std::endl;
       return EXIT_FAILURE;
    }

  cout << "Passed Text" << endl;

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  // mrmlScene->SetURL("/home/pohl/Slicer3/Slicer3/QTModules/Reporting/Testing/AnnotationControlPointNodeTest.mrml");
  mrmlScene->SetURL("AnnotationControlPointNodeTest.mrml");
  mrmlScene->Commit();
  // Now Read in File to see if ReadXML works - it first disconnects from node2 !
  mrmlScene->Connect();

  if (mrmlScene->GetNumberOfNodesByClass("vtkMRMLAnnotationControlPointsNode") != 1)
    {
        std::cerr << "Error in ReadXML() or WriteXML()" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLAnnotationControlPointsNode *node3 = dynamic_cast < vtkMRMLAnnotationControlPointsNode *> (mrmlScene->GetNthNodeByClass(0,"vtkMRMLAnnotationControlPointsNode"));
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


