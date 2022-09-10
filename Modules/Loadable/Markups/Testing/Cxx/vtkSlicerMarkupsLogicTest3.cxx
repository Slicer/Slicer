/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Annotations includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// Markups includes
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkSlicerMarkupsLogic.h"

// MRML includes
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"


// VTK includes
#include <vtkNew.h>


int vtkSlicerMarkupsLogicTest3(int , char * [] )
{
  vtkNew<vtkSlicerMarkupsLogic> logic1;
  vtkNew<vtkMRMLScene> scene;
  // add a selection node
  vtkMRMLApplicationLogic* applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene.GetPointer());

  // Test converting annotations in the scene to markups

  // no scene
  logic1->ConvertAnnotationFiducialsToMarkups();

  // empty scene
  logic1->SetMRMLScene(scene.GetPointer());
  logic1->ConvertAnnotationFiducialsToMarkups();

  // set up the annotation module logic
  vtkNew<vtkSlicerAnnotationModuleLogic> annotLogic;
  annotLogic->SetMRMLScene(scene.GetPointer());


  // add some annotations
  vtkMRMLAnnotationHierarchyNode* fid1Parent = vtkMRMLAnnotationHierarchyNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLAnnotationHierarchyNode"));
  for (int n = 0; n < 10; n++)
    {
    vtkNew<vtkMRMLAnnotationFiducialNode> annotFid;
    double p1[3] = {1.1, -2.2, 3.3};
    p1[0] = static_cast<double>(n);
    annotFid->SetFiducialCoordinates(p1);
    annotFid->Initialize(scene.GetPointer());
    vtkMRMLAnnotationHierarchyNode* fid1 = vtkMRMLAnnotationHierarchyNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLAnnotationHierarchyNode"));
    fid1->SetAssociatedNodeID(annotFid->GetID());
    fid1->SetParentNodeID(fid1Parent->GetID());
    }

  // add some more annotations
  vtkMRMLAnnotationHierarchyNode* fid2Parent = vtkMRMLAnnotationHierarchyNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLAnnotationHierarchyNode"));
  for (int n = 0; n < 5; n++)
    {
    vtkNew<vtkMRMLAnnotationFiducialNode> annotFid;
    double p1[3] = {5.5, -6.6, 0.0};
    p1[2] = static_cast<double>(n);
    annotFid->SetFiducialCoordinates(p1);
    annotFid->Initialize(scene.GetPointer());
    if (n == 3)
      {
      annotFid->SetDescription("testing description");
      }
    if (n == 4)
      {
      annotFid->SetAttribute("AssociatedNodeID", "vtkMRMLScalarVolumeNode4");
      }
    vtkMRMLAnnotationHierarchyNode* fid2 = vtkMRMLAnnotationHierarchyNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLAnnotationHierarchyNode"));
    fid2->SetAssociatedNodeID(annotFid->GetID());
    fid2->SetParentNodeID(fid2Parent->GetID());
    }

  // convert and test
  logic1->ConvertAnnotationFiducialsToMarkups();

  int numAnnotationFiducials = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationFiducialNode");
  int numMarkupsFiducials = scene->GetNumberOfNodesByClass("vtkMRMLMarkupsFiducialNode");
  if (numAnnotationFiducials != 0 ||
      numMarkupsFiducials != 2)
    {
    std::cerr << "Failed to convert 15 annotation fiducials in two hierarchies "
    << " to 2 markup lists, have " << numAnnotationFiducials
    << " annotation fiduicals and " << numMarkupsFiducials
    << " markups fiducial lists" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Converted annotation fiducials to " << numMarkupsFiducials
              << " markups fiducial lists" << std::endl;
    }
//  vtkIndent indent;
//  for (int n = 0; n < numMarkupsFiducials; ++n)
//    {
//    vtkMRMLNode *mrmlNode = scene->GetNthNodeByClass(n, "vtkMRMLMarkupsFiducialNode");
//    std::cout << "\nConverted Markups node " << n << ":" << std::endl;
//    mrmlNode->PrintSelf(std::cout, indent);
//    }

  // clean up before testing
  applicationLogic->SetMRMLScene(nullptr);
  logic1->SetMRMLScene(nullptr);
  annotLogic->SetMRMLScene(nullptr);
  applicationLogic->Delete();

  // check the second list
  vtkMRMLNode *mrmlNode = scene->GetNthNodeByClass(1, "vtkMRMLMarkupsFiducialNode");
  if (mrmlNode)
    {
    vtkMRMLMarkupsFiducialNode *markupsFid = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
    if (markupsFid)
      {
      std::string desc = markupsFid->GetNthControlPointDescription(3);
      if (desc.compare("testing description") != 0)
        {
        std::cerr << "Failed to get the expected description on markup 3, got: "
                  << desc.c_str() << std::endl;
        return EXIT_FAILURE;
        }
      std::string assocNodeID = markupsFid->GetNthControlPointAssociatedNodeID(4);
      if (assocNodeID.compare("vtkMRMLScalarVolumeNode4") != 0)
        {
        std::cerr << "Failed to get the expected associated node id on markup 4, got: "
                  << assocNodeID.c_str() << std::endl;
        return EXIT_FAILURE;
        }
      vtkVector3d posVector = markupsFid->GetNthControlPointPositionVector(0);
      double* pos = posVector.GetData();
      double expectedPos[3] = {5.5, -6.6, 0.0};
      if (vtkMath::Distance2BetweenPoints(pos, expectedPos) > 0.01)
        {
        std::cerr << "Expected 0th position of 5.5, -6.6, 0.0, but got: "
                  << pos[0] << "," << pos[1] << "," << pos[2] << std::endl;
        return EXIT_FAILURE;
        }
      vtkMRMLMarkupsDisplayNode *dispNode = markupsFid->GetMarkupsDisplayNode();
      if (dispNode)
        {
        double col[3];
        dispNode->GetColor(col);
        double annotCol[3];
        vtkNew<vtkMRMLAnnotationPointDisplayNode> pointDispNode;
        pointDispNode->GetColor(annotCol);
        if (vtkMath::Distance2BetweenPoints(col, annotCol) > 0.01)
          {
          std::cerr << "Failed to set color, expected "
                    << annotCol[0] << ","
                    << annotCol[1] << ","
                    << annotCol[2] << ", but got: "
                    << col[0] << ","
                    << col[1] << ","
                    << col[2] << std::endl;
          return EXIT_FAILURE;
          }
        }
      else
        {
        std::cerr << "Second markups node doesn't have a display node!"
                  << std::endl;
        return EXIT_FAILURE;
        }
      }
    else
      {
      std::cerr << "Unable to get second markups fiducial node for testing!" << std::endl;
      return EXIT_FAILURE;
      }
    }
  // cleanup
  scene->Clear(1);

  return EXIT_SUCCESS;
}
