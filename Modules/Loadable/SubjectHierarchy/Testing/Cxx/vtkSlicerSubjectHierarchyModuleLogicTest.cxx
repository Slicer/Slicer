/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Subject Hierarchy includes
#include "vtkSlicerSubjectHierarchyModuleLogic.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"

// STD includes
#include <cassert>
#include <sstream>

namespace
{
  bool PopulateScene(vtkMRMLScene* scene);

  bool TestExpand();
  bool TestNodeAccess();
  bool TestNodeAssociations();
  bool TestTreeOperations();
  bool TestInsertDicomSeriesEmptyScene();
  bool TestInsertDicomSeriesPopulatedScene();
  bool TestVisibilityOperations();
  bool TestTransformBranch();

  const char* STUDY_ATTRIBUTE_NAME = "TestStudyAttribute";
  const char* STUDY_ATTRIBUTE_VALUE = "1";
  const char* UID_NAME = vtkMRMLSubjectHierarchyConstants::GetDICOMUIDName();
  const char* PATIENT_UID_VALUE = "PATIENT";
  const char* STUDY1_UID_VALUE = "STUDY1";
  const char* STUDY2_UID_VALUE = "STUDY2";
  const char* VOLUME1_UID_VALUE = "VOLUME1";
  const char* MODEL1_UID_VALUE = "MODEL1";
  const char* VOLUME2_UID_VALUE = "VOLUME2";
  const char* MODEL21_UID_VALUE = "MODEL21";
  const char* MODEL22_UID_VALUE = "MODEL22";

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkSlicerSubjectHierarchyModuleLogicTest(int vtkNotUsed(argc),
                                              char * vtkNotUsed(argv)[] )
{
  if (!TestExpand())
    {
    std::cerr << "'TestExpand' call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

namespace
{
  //---------------------------------------------------------------------------
  // Populate a sample subject hierarchy scene
  // Scene
  //  + SubjectHierarchyNode
  //     |    (Patient)
  //     +- SubjectHierarchyNode
  //     |   |    (Study)
  //     |   +- SubjectHierarchyNode -- ScalarVolumeNode (volume1)
  //     |   |        (Series)            > DisplayNode
  //     |   +- SubjectHierarchyNode -- ModelNode (model1)
  //     |            (Series)            > DisplayNode
  //     +- SubjectHierarchyNode
  //         |    (Study)
  //         +- SubjectHierarchyNode -- ScalarVolumeNode (volume2)
  //         |        (Series)                  > DisplayNode
  //         +- SubjectHierarchyNode -- ModelHierarchyNode -- ModelNode (model21)
  //         |       (Series)          (nested association)       > DisplayNode
  //         +- SubjectHierarchyNode -- ModelHierarchyNode -- ModelNode (model22)
  //                 (Series)          (nested association)       > DisplayNode
  //
  bool PopulateScene(vtkMRMLScene* scene)
    {
    // Create subject and studies
    vtkMRMLSubjectHierarchyNode* patientShNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
      scene, NULL, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient(), "Patient");
    patientShNode->AddUID(UID_NAME, PATIENT_UID_VALUE);

    vtkMRMLSubjectHierarchyNode* study1ShNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
      scene, patientShNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy(), "Study1");
    study1ShNode->AddUID(UID_NAME, STUDY1_UID_VALUE);

    vtkMRMLSubjectHierarchyNode* study2ShNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
      scene, patientShNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy(), "Study2");
    study2ShNode->AddUID(UID_NAME, STUDY2_UID_VALUE);
    study2ShNode->SetAttribute(STUDY_ATTRIBUTE_NAME, STUDY_ATTRIBUTE_VALUE);

    // Create volume series in study 1
    vtkNew<vtkMRMLScalarVolumeNode> volume1Node;
    volume1Node->SetName("Volume1");
    scene->AddNode(volume1Node.GetPointer());

    vtkNew<vtkMRMLScalarVolumeDisplayNode> volume1DisplayNode;
    scene->AddNode(volume1DisplayNode.GetPointer());
    volume1Node->SetAndObserveDisplayNodeID(volume1DisplayNode->GetID());

    vtkMRMLSubjectHierarchyNode* volume1SeriesShNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
      scene, study1ShNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries(),
      volume1Node->GetName(), volume1Node.GetPointer());
    volume1SeriesShNode->AddUID(UID_NAME, VOLUME1_UID_VALUE);

    // Create model1 series in study 1
    vtkNew<vtkMRMLModelNode> model1Node;
    model1Node->SetName("Model1");
    scene->AddNode(model1Node.GetPointer());

    vtkNew<vtkMRMLModelDisplayNode> model1DisplayNode;
    scene->AddNode(model1DisplayNode.GetPointer());
    model1Node->SetAndObserveDisplayNodeID(model1DisplayNode->GetID());

    vtkMRMLSubjectHierarchyNode* model1SeriesShNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
      scene, study1ShNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries(),
      model1Node->GetName(), model1Node.GetPointer());
    model1SeriesShNode->AddUID(UID_NAME, MODEL1_UID_VALUE);

    // Create volume series in study 2
    vtkNew<vtkMRMLScalarVolumeNode> volume2Node;
    volume2Node->SetName("Volume2");
    scene->AddNode(volume2Node.GetPointer());

    vtkNew<vtkMRMLScalarVolumeDisplayNode> volume2DisplayNode;
    scene->AddNode(volume2DisplayNode.GetPointer());
    volume2Node->SetAndObserveDisplayNodeID(volume2DisplayNode->GetID());

    vtkMRMLSubjectHierarchyNode* volume2SeriesShNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
      scene, study2ShNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries(),
      volume2Node->GetName(), volume2Node.GetPointer());
    volume2SeriesShNode->AddUID(UID_NAME, VOLUME2_UID_VALUE);

    // Create model21 series in study 2 with nested association
    vtkNew<vtkMRMLModelNode> model21Node;
    model21Node->SetName("Model21");
    scene->AddNode(model21Node.GetPointer());

    vtkNew<vtkMRMLModelDisplayNode> model21DisplayNode;
    scene->AddNode(model21DisplayNode.GetPointer());
    model21Node->SetAndObserveDisplayNodeID(model21DisplayNode->GetID());

    vtkNew<vtkMRMLModelHierarchyNode> model21ModelHierarchyNode;
    model21ModelHierarchyNode->SetName("Model21_Hierarchy");
    model21ModelHierarchyNode->SetDisplayableNodeID(model21Node->GetID());
    //model21ModelHierarchyNode->SetParentNodeID(modelHierarchyRootNode->GetID()); // No parent node needed to test nested associations
    scene->AddNode(model21ModelHierarchyNode.GetPointer());

    vtkMRMLSubjectHierarchyNode* model21SeriesShNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
      scene, study2ShNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries(),
      model21Node->GetName(), model21ModelHierarchyNode.GetPointer());
    model21SeriesShNode->AddUID(UID_NAME, MODEL21_UID_VALUE);

    // Create model22 series in study 2 with nested association
    vtkNew<vtkMRMLModelNode> model22Node;
    model22Node->SetName("Model22");
    scene->AddNode(model22Node.GetPointer());

    vtkNew<vtkMRMLModelDisplayNode> model22DisplayNode;
    scene->AddNode(model22DisplayNode.GetPointer());
    model22Node->SetAndObserveDisplayNodeID(model22DisplayNode->GetID());

    vtkNew<vtkMRMLModelHierarchyNode> model22ModelHierarchyNode;
    model22ModelHierarchyNode->SetName("Model22_Hierarchy");
    model22ModelHierarchyNode->SetDisplayableNodeID(model22Node->GetID());
    //model22ModelHierarchyNode->SetParentNodeID(modelHierarchyRootNode->GetID()); // No parent node needed to test nested associations
    scene->AddNode(model22ModelHierarchyNode.GetPointer());

    vtkMRMLSubjectHierarchyNode* model22SeriesShNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
      scene, study2ShNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries(),
      model22Node->GetName(), model22ModelHierarchyNode.GetPointer());
    model22SeriesShNode->AddUID(UID_NAME, MODEL22_UID_VALUE);

    int currentNodeCount = scene->GetNumberOfNodes();
    int expectedNodeCount = 20;
    if (expectedNodeCount != currentNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with PopulateScene\n"
                << "\tcurrentNodeCount: " << currentNodeCount << "\n"
                << "\texpectedNodeCount: " << expectedNodeCount
                << std::endl;
      return false;
      }

    currentNodeCount = scene->GetNumberOfNodesByClass("vtkMRMLScalarVolumeNode");
    expectedNodeCount = 2;
    if (expectedNodeCount != currentNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with PopulateScene\n"
                << "\tcurrentNodeCount: " << currentNodeCount << "\n"
                << "\texpectedNodeCount: " << expectedNodeCount
                << std::endl;
      return false;
      }

    currentNodeCount = scene->GetNumberOfNodesByClass("vtkMRMLModelNode");
    expectedNodeCount = 3;
    if (expectedNodeCount != currentNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with PopulateScene\n"
                << "\tcurrentNodeCount: " << currentNodeCount << "\n"
                << "\texpectedNodeCount: " << expectedNodeCount
                << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestExpand()
    {
    if (!TestNodeAccess())
      {
      std::cerr << "'TestNodeAccess' call not successful." << std::endl;
      return false;
      }
    if (!TestNodeAssociations())
      {
      std::cerr << "'TestNodeAssociations' call not successful." << std::endl;
      return false;
      }
    if (!TestTreeOperations())
      {
      std::cerr << "'TestTreeOperations' call not successful." << std::endl;
      return false;
      }
    if (!TestInsertDicomSeriesEmptyScene())
      {
      std::cerr << "'TestInsertDicomSeriesEmptyScene' call not successful." << std::endl;
      return false;
      }
    if (!TestInsertDicomSeriesPopulatedScene())
      {
      std::cerr << "'TestInsertDicomSeriesPopulatedScene' call not successful." << std::endl;
      return false;
      }
    if (!TestVisibilityOperations())
      {
      std::cerr << "'TestVisibilityOperations' call not successful." << std::endl;
      return false;
      }
    if (!TestTransformBranch())
      {
      std::cerr << "'TestTransformBranch' call not successful." << std::endl;
      return false;
      }
    return true;
    }

  //---------------------------------------------------------------------------
  bool TestNodeAccess()
    {
    vtkNew<vtkMRMLScene> scene;
    if (!PopulateScene(scene.GetPointer()))
      {
      return false;
      }

    // Get node by UID
    vtkMRMLSubjectHierarchyNode* patientNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene.GetPointer(), UID_NAME, PATIENT_UID_VALUE);
    if (!patientNode)
      {
      std::cout << "Failed to get subject by UID" << std::endl;
      return false;
      }
    // Check level
    if (strcmp(patientNode->GetLevel(), vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
      {
      std::cout << "Wrong level of found node!" << std::endl;
      return false;
      }
    // Check name
    std::string patientNodeName = std::string(patientNode->GetNameWithoutPostfix()) +
      vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNodeNamePostfix();
    if (patientNodeName.compare(patientNode->GetName()))
      {
      std::cout << "Failed to get correct name without postfix!" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestNodeAssociations()
    {
    vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
    if (!PopulateScene(scene))
      {
      return false;
      }

    // Get volume node for testing simple association
    vtkMRMLSubjectHierarchyNode* volume1ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, VOLUME1_UID_VALUE);
    if (!volume1ShNode)
      {
      std::cout << "Failed to get volume by UID" << std::endl;
      return false;
      }

    // Get associated data node simple case
    vtkMRMLScalarVolumeNode* volume1Node = vtkMRMLScalarVolumeNode::SafeDownCast(
      volume1ShNode->GetAssociatedNode() );
    if (!volume1Node)
      {
      std::cout << "Failed to get associated volume node (simple association)" << std::endl;
      return false;
      }

    // Get associated subject hierarchy node simple case
    vtkMRMLSubjectHierarchyNode* volume1ShNodeTest =
      vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(volume1Node);
    if (volume1ShNodeTest != volume1ShNode)
      {
      std::cout << "Failed to get associated subject hierarchy node for volume (simple association)" << std::endl;
      return false;
      }

    // Get model node for testing nested association
    vtkMRMLSubjectHierarchyNode* model21ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, MODEL21_UID_VALUE);
    if (!model21ShNode)
      {
      std::cout << "Failed to get model by UID" << std::endl;
      return false;
      }

    // Get associated data node nested case
    vtkMRMLModelNode* model21Node = vtkMRMLModelNode::SafeDownCast(
      model21ShNode->GetAssociatedNode() );
    if (!model21Node)
      {
      std::cout << "Failed to get associated model node (nested association)" << std::endl;
      return false;
      }

    // Get associated subject hierarchy node nested case
    vtkMRMLSubjectHierarchyNode* model21ShNodeTest =
      vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(model21Node);
    if (model21ShNodeTest != model21ShNode)
      {
      std::cout << "Failed to get associated subject hierarchy node for model (nested association)" << std::endl;
      return false;
      }

    // Get simple hierarchy node for nested association
    vtkMRMLHierarchyNode* model21ModelHierarchyNode =
      vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene, model21Node->GetID());
    if (!model21ModelHierarchyNode || !model21ModelHierarchyNode->IsA("vtkMRMLModelHierarchyNode"))
      {
      std::cout << "Failed to get associated model hierarchy node for model (directly associated hierarchy node in nested association)" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestTreeOperations()
    {
    vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
    if (!PopulateScene(scene))
      {
      return false;
      }

    // Get nodes used in this test case
    vtkMRMLSubjectHierarchyNode* patientShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, PATIENT_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* study1ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, STUDY1_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* volume1ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, VOLUME1_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* volume2ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, VOLUME2_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* model21ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, MODEL21_UID_VALUE);
    if (!patientShNode || !study1ShNode || !volume1ShNode || !volume2ShNode || !model21ShNode)
      {
      std::cout << "Failed to get nodes by UID" << std::endl;
      return false;
      }

    // AreNodesInSameBranch utility method in logic class
    vtkMRMLSubjectHierarchyNode* commonAncestor = NULL;

    commonAncestor = vtkSlicerSubjectHierarchyModuleLogic::AreNodesInSameBranch(
      volume1ShNode, volume2ShNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy());
    if (commonAncestor)
      {
      std::cout << "Common parent check failed (volume1 and volume2 are not in the same study)" << std::endl;
      return false;
      }

    commonAncestor = vtkSlicerSubjectHierarchyModuleLogic::AreNodesInSameBranch(
      volume2ShNode, model21ShNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy());
    if ( !commonAncestor
      || strcmp(commonAncestor->GetLevel(), vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()) )
      {
      std::cout << "Common parent check failed (volume2 and model21 are in the same study)" << std::endl;
      return false;
      }

    commonAncestor = vtkSlicerSubjectHierarchyModuleLogic::AreNodesInSameBranch(
      volume1ShNode, volume2ShNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient());
    if ( !commonAncestor
      || strcmp(commonAncestor->GetLevel(), vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()) )
      {
      std::cout << "Common parent check failed (volume1 and volume2 are in the same patient)" << std::endl;
      return false;
      }

    // GetChildWithName utility method in node class
    vtkMRMLSubjectHierarchyNode* foundChild = NULL;

    foundChild = vtkMRMLSubjectHierarchyNode::GetChildWithName(
      NULL, patientShNode->GetNameWithoutPostfix().c_str(), scene );
    if ( !foundChild
      || strcmp(foundChild->GetLevel(), vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()) )
      {
      std::cout << "Failed to find top-level subject hierarchy node by name" << std::endl;
      return false;
      }

    foundChild = vtkMRMLSubjectHierarchyNode::GetChildWithName(
      patientShNode, study1ShNode->GetNameWithoutPostfix().c_str() );
    if ( !foundChild
      || strcmp(foundChild->GetLevel(), vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()) )
      {
      std::cout << "Failed to find child subject hierarchy node by name" << std::endl;
      return false;
      }

    // GetAssociatedChildrenNodes utility method in node class
    vtkNew<vtkCollection> childNodes;

    patientShNode->GetAssociatedChildrenNodes(childNodes.GetPointer());
    int expectedChildCount = 5;
    int currentChildCount = childNodes->GetNumberOfItems();
    if (currentChildCount != expectedChildCount)
      {
      std::cout << "Line " << __LINE__ << " - Failed to find associated children nodes:\n"
                << "\tcurrent count: " << currentChildCount << "\n"
                << "\texpected count: " << expectedChildCount
                << std::endl;
      return false;
      }

    childNodes->RemoveAllItems();
    patientShNode->GetAssociatedChildrenNodes(childNodes.GetPointer(), "vtkMRMLScalarVolumeNode");
    expectedChildCount = 2;
    currentChildCount = childNodes->GetNumberOfItems();
    if (currentChildCount != expectedChildCount)
      {
      std::cout << "Line " << __LINE__ << " - Failed to find associated children volume nodes:\n"
                << "\tcurrent count: " << currentChildCount << "\n"
                << "\texpected count: " << expectedChildCount
                << std::endl;
      return false;
      }

    // GetAttributeFromAncestor utility method in node class
    const char* attributeValueAnyLevel =
      volume2ShNode->GetAttributeFromAncestor(STUDY_ATTRIBUTE_NAME);
    if (!attributeValueAnyLevel || strcmp(attributeValueAnyLevel, STUDY_ATTRIBUTE_VALUE))
      {
      std::cout << "Failed to find attribute from ancestor in any level" << std::endl;
      return false;
      }

    const char* attributeValueStudyLevel =
      volume2ShNode->GetAttributeFromAncestor(STUDY_ATTRIBUTE_NAME, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy());
    if (!attributeValueStudyLevel || strcmp(attributeValueStudyLevel, STUDY_ATTRIBUTE_VALUE))
      {
      std::cout << "Failed to find attribute from ancestor in study level" << std::endl;
      return false;
      }

    const char* attributeValueWrongLevel =
      volume2ShNode->GetAttributeFromAncestor(STUDY_ATTRIBUTE_NAME, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient());
    if (attributeValueWrongLevel)
      {
      std::cout << "Found attribute from ancestor in wrong level" << std::endl;
      return false;
      }

    // GetAncestorAtLevel utility method in node class
    vtkMRMLSubjectHierarchyNode* ancestorAtStudyLevel =
      volume1ShNode->GetAncestorAtLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy());
    if ( !ancestorAtStudyLevel
      || strcmp(ancestorAtStudyLevel->GetLevel(), vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()) )
      {
      std::cout << "Failed to find ancestor at study level" << std::endl;
      return false;
      }

    vtkMRMLSubjectHierarchyNode* ancestorAtPatientLevel =
      volume1ShNode->GetAncestorAtLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient());
    if ( !ancestorAtPatientLevel
      || strcmp(ancestorAtPatientLevel->GetLevel(), vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()) )
      {
      std::cout << "Failed to find ancestor at patient level" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestInsertDicomSeriesEmptyScene()
    {
    vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

    // Create series node to insert
    const char* seriesUid = "NEW_SERIES";
    vtkMRMLSubjectHierarchyNode* seriesShNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
      scene, NULL, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries(), "Series");
    seriesShNode->AddUID(UID_NAME, seriesUid);

    vtkMRMLSubjectHierarchyNode* insertedSeriesNode = vtkSlicerSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy(
      scene, PATIENT_UID_VALUE, STUDY1_UID_VALUE, seriesUid );
    if (insertedSeriesNode != seriesShNode)
      {
      std::cout << "Failed to insert DICOM series node" << std::endl;
      return false;
      }

    // Check newly created study and patient nodes
    vtkMRMLSubjectHierarchyNode* patientShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, PATIENT_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* studyShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, STUDY1_UID_VALUE);
    if (!patientShNode || !studyShNode)
      {
      std::cout << "Patient and study nodes not created for DICOM series" << std::endl;
      return false;
      }

    // Check if series is indeed the child of the newly created study
    if (seriesShNode->GetParentNode() != studyShNode)
      {
      std::cout << "DICOM series node not correctly inserted under study node" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestInsertDicomSeriesPopulatedScene()
    {
    vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
    if (!PopulateScene(scene))
      {
      return false;
      }

    // Create series node to insert
    const char* seriesUid = "NEW_SERIES";
    vtkMRMLSubjectHierarchyNode* seriesShNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
      scene, NULL, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries(), "Series");
    seriesShNode->AddUID(UID_NAME, seriesUid);

    vtkMRMLSubjectHierarchyNode* insertedSeriesNode = vtkSlicerSubjectHierarchyModuleLogic::InsertDicomSeriesInHierarchy(
      scene, PATIENT_UID_VALUE, STUDY1_UID_VALUE, seriesUid );

    // Check if series was inserted under the desired study
    vtkMRMLSubjectHierarchyNode* studyShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, STUDY1_UID_VALUE);
    if (!studyShNode)
      {
      std::cout << "Study node not found" << std::endl;
      return false;
      }
    if (seriesShNode->GetParentNode() != studyShNode)
      {
      std::cout << "DICOM series node not correctly inserted under study node" << std::endl;
      return false;
      }
    if (insertedSeriesNode != seriesShNode)
      {
      std::cout << "Inserted series does not match retrieved series" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestVisibilityOperations()
    {
    vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
    if (!PopulateScene(scene))
      {
      return false;
      }

    // Get nodes used in this test case
    vtkMRMLSubjectHierarchyNode* patientShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, PATIENT_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* study1ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, STUDY1_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* study2ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, STUDY2_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* volume1ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, VOLUME1_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* model1ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, MODEL1_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* volume2ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, VOLUME2_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* model21ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, MODEL21_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* model22ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, MODEL22_UID_VALUE);
    if ( !patientShNode || !study1ShNode || !study2ShNode || !volume1ShNode
      || !model1ShNode || !volume2ShNode || !model21ShNode || !model22ShNode)
      {
      std::cout << "Failed to get nodes by UID" << std::endl;
      return false;
      }
    if ( !volume1ShNode->GetAssociatedNode() || !volume1ShNode->GetAssociatedNode()->IsA("vtkMRMLScalarVolumeNode")
      || !model1ShNode->GetAssociatedNode() || !model1ShNode->GetAssociatedNode()->IsA("vtkMRMLModelNode")
      || !volume2ShNode->GetAssociatedNode() || !volume2ShNode->GetAssociatedNode()->IsA("vtkMRMLScalarVolumeNode")
      || !model21ShNode->GetAssociatedNode() || !model21ShNode->GetAssociatedNode()->IsA("vtkMRMLModelNode")
      || !model22ShNode->GetAssociatedNode() || !model22ShNode->GetAssociatedNode()->IsA("vtkMRMLModelNode") )
      {
      std::cout << "Unable to get data nodes" << std::endl;
      return false;
      }
    vtkMRMLScalarVolumeDisplayNode* volume1DisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      vtkMRMLScalarVolumeNode::SafeDownCast(volume1ShNode->GetAssociatedNode())->GetDisplayNode() );
    vtkMRMLModelDisplayNode* model1DisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(
      vtkMRMLModelNode::SafeDownCast(model1ShNode->GetAssociatedNode())->GetDisplayNode() );
    vtkMRMLScalarVolumeDisplayNode* volume2DisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      vtkMRMLScalarVolumeNode::SafeDownCast(volume2ShNode->GetAssociatedNode())->GetDisplayNode() );
    vtkMRMLModelDisplayNode* model21DisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(
      vtkMRMLModelNode::SafeDownCast(model21ShNode->GetAssociatedNode())->GetDisplayNode() );
    vtkMRMLModelDisplayNode* model22DisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(
      vtkMRMLModelNode::SafeDownCast(model22ShNode->GetAssociatedNode())->GetDisplayNode() );
    if (!volume1DisplayNode || !model1DisplayNode || !volume2DisplayNode || !model21DisplayNode || !model22DisplayNode)
      {
      std::cout << "Unable to get display nodes" << std::endl;
      return false;
      }

    // By default Visibility is on, SliceIntersectionVisibility is off. Check this
    if (model1DisplayNode->GetVisibility() != 1 || model1DisplayNode->GetSliceIntersectionVisibility() != 0)
      {
      std::cout << "Default visibility settings changed! Test application needs to be reviewed" << std::endl;
      return false;
      }

    if (patientShNode->GetDisplayVisibilityForBranch() != 1)
      {
      std::cout << "Wrong display visibility value for patient" << std::endl;
      return false;
      }

    // Check display visibility value propagation down a branch
    study2ShNode->SetDisplayVisibilityForBranch(0);
    if ( model21DisplayNode->GetVisibility() != 0
      || model22DisplayNode->GetVisibility() != 0 )
      {
      std::cout << "Failed to set display visibility on study" << std::endl;
      return false;
      }

    // Check partial visibility
    if (patientShNode->GetDisplayVisibilityForBranch() != 2)
      {
      std::cout << "Wrong partial display visibility value for patient" << std::endl;
      return false;
      }

    model22ShNode->SetDisplayVisibilityForBranch(1);
    if (study2ShNode->GetDisplayVisibilityForBranch() != 2)
      {
      std::cout << "Wrong partial display visibility value for study" << std::endl;
      return false;
      }
    if (patientShNode->GetDisplayVisibilityForBranch() != 2)
      {
      std::cout << "Wrong partial display visibility value for patient" << std::endl;
      return false;
      }

    // Show everything again
    patientShNode->SetDisplayVisibilityForBranch(1);
    if ( study1ShNode->GetDisplayVisibilityForBranch() != 1
      || study2ShNode->GetDisplayVisibilityForBranch() != 1
      || model1DisplayNode->GetVisibility() != 1
      || model21DisplayNode->GetVisibility() != 1
      || model22DisplayNode->GetVisibility() != 1 )
      {
      std::cout << "Wrong display visibility values after showing everything again" << std::endl;
      return false;
      }

    return true;
    }

  //---------------------------------------------------------------------------
  bool TestTransformBranch()
    {
    vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
    if (!PopulateScene(scene))
      {
      return false;
      }

    // Get nodes used in this test case
    vtkMRMLSubjectHierarchyNode* study2ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, STUDY2_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* model21ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, MODEL21_UID_VALUE);
    vtkMRMLSubjectHierarchyNode* model22ShNode =
      vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNodeByUID(scene, UID_NAME, MODEL22_UID_VALUE);
    if (!study2ShNode || !model21ShNode || !model22ShNode)
      {
      std::cout << "Failed to get nodes by UID" << std::endl;
      return false;
      }

    // Set non-empty poly data to model21 and model22 to be able to check transform hardening
    vtkNew<vtkPoints> model21Points;
    model21Points->InsertNextPoint(0.0, 0.0, 0.0);
    vtkNew<vtkPolyData> model21PolyData;
    model21PolyData->SetPoints(model21Points.GetPointer());

    vtkNew<vtkPoints> model22Points;
    model22Points->InsertNextPoint(0.0, 0.0, 0.0);
    vtkNew<vtkPolyData> model22PolyData;
    model22PolyData->SetPoints(model22Points.GetPointer());

    vtkMRMLModelNode* model21Node = vtkMRMLModelNode::SafeDownCast(model21ShNode->GetAssociatedNode());
    vtkMRMLModelNode* model22Node = vtkMRMLModelNode::SafeDownCast(model22ShNode->GetAssociatedNode());
    if (!model21Node || !model22Node)
      {
      std::cout << "Failed to get model data nodes" << std::endl;
      return false;
      }
    model21Node->SetAndObservePolyData(model21PolyData.GetPointer());
    model22Node->SetAndObservePolyData(model22PolyData.GetPointer());

    // Create transforms to use in this test case
    vtkNew<vtkMatrix4x4> transformMatrix1;
    transformMatrix1->SetElement(0,3,100.0); // Translation along X by 100 mm
    vtkNew<vtkMRMLLinearTransformNode> transformNode1;
    transformNode1->SetMatrixTransformToParent(transformMatrix1.GetPointer());
    scene->AddNode(transformNode1.GetPointer());

    vtkNew<vtkMatrix4x4> transformMatrix2;
    transformMatrix2->SetElement(1,3,50.0); // Translation along Y by 50 mm
    vtkNew<vtkMRMLLinearTransformNode> transformNode2; // Second transform is identity
    transformNode2->SetMatrixTransformToParent(transformMatrix2.GetPointer());
    scene->AddNode(transformNode2.GetPointer());

    // Check transformed nodes
    if (study2ShNode->IsAnyNodeInBranchTransformed())
      {
      std::cout << "Wrong inquiry about transformed nodes in study2" << std::endl;
      return false;
      }

    // Transform model21 using transform1
    vtkSlicerSubjectHierarchyModuleLogic::TransformBranch(model21ShNode, transformNode1.GetPointer());
    vtkMRMLTransformNode * currentTransformNode = model21Node->GetParentTransformNode();
    vtkMRMLTransformNode * expectedTransformNode = transformNode1.GetPointer();
    if (currentTransformNode != expectedTransformNode)
      {
      std::cout << "Line " << __LINE__ << " - Failed to apply transform1 on model1:\n"
                << "\tcurrentTransformNode: "
                << (currentTransformNode ? currentTransformNode->GetID() : "<null>") << "\n"
                << "\texpectedTransformNode: "
                << (expectedTransformNode ? expectedTransformNode->GetID() : "<null>")
                << std::endl;
      return false;
      }
    if (!study2ShNode->IsAnyNodeInBranchTransformed())
      {
      std::cout << "Wrong inquiry about transformed nodes in study2" << std::endl;
      return false;
      }
    if (study2ShNode->IsAnyNodeInBranchTransformed(transformNode1.GetPointer()))
      {
      std::cout << "Wrong inquiry about transformed nodes in study2 with exception transform set" << std::endl;
      return false;
      }

    // Transform study2 using transform2, harden transform on model21
    vtkSlicerSubjectHierarchyModuleLogic::TransformBranch(study2ShNode, transformNode2.GetPointer());
    if ( model21Node->GetParentTransformNode() != transformNode2.GetPointer()
      || model22Node->GetParentTransformNode() != transformNode2.GetPointer() )
      {
      std::cout << "Failed to apply transform2 on study2" << std::endl;
      return false;
      }

    // Check whether transform1 was hardened on model21
    double model21PointsArray[3] = {0.0, 0.0, 0.0};
    model21PolyData->GetPoint(0, model21PointsArray);
    if (model21PointsArray[0] != 100.0 || model21PointsArray[1] != 0.0 || model21PointsArray[2] != 0.0)
      {
      std::cout << transformNode2->GetName()
                << " was not hardened on model21 when applying "
                << transformNode2->GetName() << " on " << study2ShNode->GetName() << std::endl;
      return false;
      }

    // Set transform1 on study2 without hardening
    vtkSlicerSubjectHierarchyModuleLogic::TransformBranch(study2ShNode, transformNode1.GetPointer(), false);
    if ( model21Node->GetParentTransformNode() != transformNode1.GetPointer()
      || model22Node->GetParentTransformNode() != transformNode1.GetPointer() )
      {
      std::cout << "Failed to apply transform1 on study2" << std::endl;
      return false;
      }
    model21PolyData->GetPoint(0, model21PointsArray);
    if (model21PointsArray[0] != 100.0 || model21PointsArray[1] != 0.0 || model21PointsArray[2] != 0.0)
      {
      std::cout << "Transform2 was hardened on model21 when applying transform1 on study2 without hardening" << std::endl;
      return false;
      }

    // Harden transform2 on model22
    vtkSlicerSubjectHierarchyModuleLogic::TransformBranch(study2ShNode, transformNode2.GetPointer(), false);
    vtkSlicerSubjectHierarchyModuleLogic::HardenTransformOnBranch(study2ShNode);

    model21PolyData->GetPoint(0, model21PointsArray);
    if (model21PointsArray[0] != 100.0 || model21PointsArray[1] != 50.0 || model21PointsArray[2] != 0.0)
      {
      std::cout << "Failed to harden transform on study2" << std::endl;
      return false;
      }

    double model22PointsArray[3] =   {0.0, 0.0, 0.0};
    model22PolyData->GetPoint(0, model22PointsArray);
    if (model22PointsArray[0] != 0.0 || model22PointsArray[1] != 50.0 || model22PointsArray[2] != 0.0)
      {
      std::cout << "Failed to harden transform on study2" << std::endl;
      return false;
      }

    // Remove transforms from study
    vtkSlicerSubjectHierarchyModuleLogic::TransformBranch(study2ShNode, transformNode1.GetPointer());
    vtkSlicerSubjectHierarchyModuleLogic::TransformBranch(study2ShNode, NULL);
    if (study2ShNode->IsAnyNodeInBranchTransformed())
      {
      std::cout << "Wrong inquiry about transformed nodes in study2" << std::endl;
      return false;
      }

    return true;
    }

} // end of anonymous namespace
