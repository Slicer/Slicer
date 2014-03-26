#include "vtkMRMLAnnotationDisplayNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLScene.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationDisplayNodeTest2(int , char * [] )
{
  // define test colors
  double testColor1[3] = { 0.1,0.2,0.3 };
  double testColor2[3] = { 0.4,0.5,0.6 };
  double testColor3[3] = { 0.7,0.7,0.7 };
  double specular1 = 0.3;
  double specular2 = 0.4;
  double specular3 = 0.9;


  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

  vtkSmartPointer< vtkMRMLAnnotationDisplayNode > displayNode1 = vtkSmartPointer< vtkMRMLAnnotationDisplayNode >::New();
  scene->AddNode(displayNode1);

  vtkSmartPointer< vtkMRMLAnnotationDisplayNode > displayNode2 = vtkSmartPointer< vtkMRMLAnnotationDisplayNode >::New();
  scene->AddNode(displayNode2);

  vtkSmartPointer< vtkMRMLAnnotationDisplayNode > displayNode3 = vtkSmartPointer< vtkMRMLAnnotationDisplayNode >::New();
  scene->AddNode(displayNode3);

  vtkSmartPointer< vtkMRMLAnnotationNode > annotationNode = vtkSmartPointer<vtkMRMLAnnotationNode>::New();
  scene->AddNode(annotationNode);

  annotationNode->SetAndObserveNthDisplayNodeID(0,displayNode1->GetID());
  annotationNode->SetAndObserveNthDisplayNodeID(1,displayNode2->GetID());
  annotationNode->SetAndObserveNthDisplayNodeID(2,displayNode3->GetID());

  // now set custom colors
  displayNode2->SetColor(testColor1[0], testColor1[1], testColor1[2]);

  double* readColor = displayNode2->GetColor();
  if ((readColor[0] != testColor1[0]) || (readColor[1] != testColor1[1]) || (readColor[2] != testColor1[2]))
    {
    std::cout << "ERROR: Color was not propagated correctly! Is: "<< readColor[0] << ", " << readColor[1] << ", " << readColor[2] << " and should be " << testColor1[0] << ", " << testColor1[1] << ", " << testColor1[2] << "!" << std::endl;
    return EXIT_FAILURE;
    }

  displayNode2->SetSpecular(specular1);

  displayNode3->SetColor(testColor2[0], testColor2[1], testColor2[2]);

  readColor = displayNode3->GetColor();
  if ((readColor[0] != testColor2[0]) || (readColor[1] != testColor2[1]) || (readColor[2] != testColor2[2]))
    {
    std::cout << "ERROR: Color was not propagated correctly! Is: "<< readColor[0] << ", " << readColor[1] << ", " << readColor[2] << " and should be " << testColor2[0] << ", " << testColor2[1] << ", " << testColor2[2] << "!" << std::endl;
    return EXIT_FAILURE;
    }

  displayNode3->SetSpecular(specular2);

  // now try to propagate a supercolor to displaynode2 and 3
  displayNode1->SetAndPropagateSuperColor(testColor3[0], testColor3[1], testColor3[2]);

  // also propagate the specular
  displayNode1->SetAndPropagateSuperSpecular(specular3);

  // now check if all displayNodes of the annotationNode have the testColor3
  for(int i=0; i< annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkMRMLDisplayNode *displayNode = annotationNode->GetNthDisplayNode(i);

    if (!displayNode)
      {
      // something went wrong
      std::cout << "ERROR: Could not get associated displayNodes!" << std::endl;
      return EXIT_FAILURE;
      }

    // check if color propagation worked
    double* readColor = displayNode->GetColor();
    if ((readColor[0] != testColor3[0]) || (readColor[1] != testColor3[1]) || (readColor[2] != testColor3[2]))
      {
      std::cout << "ERROR: Color was not propagated correctly! Is: "<< readColor[0] << ", " << readColor[1] << ", " << readColor[2] << " and should be " << testColor3[0] << ", " << testColor3[1] << ", " << testColor3[2] << "!" << std::endl;
      return EXIT_FAILURE;
      }

    // check if specular propagation worked
    if (displayNode->GetSpecular() != specular3)
      {
      std::cout << "ERROR: Specular was not propagated correctly! Is: "<< displayNode->GetSpecular() << " and should be: " << specular3 << "!" << std::endl;
      return EXIT_FAILURE;
      }

    }

  return EXIT_SUCCESS;
}
