/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkSlicerTransformLogic.h"
#include "vtkSlicerTransformsReader.h"
#include <vtkMRMLNodeWriter.h>
#include <vtkNew.h>

#include "vtkTransformVisualizerGlyph3D.h"

// MRML includes
#include "vtkCacheManager.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLTransformDisplayNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeNode.h"

// vtkAddon includes
#include <vtkScatteredBSplineTransform.h>

// ITKsys includes
#include <itksys/SystemTools.hxx>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkCollection.h>
#include <vtkArrowSource.h>
#include <vtkBoundingBox.h>
#include <vtkBSplineTransform.h>
#include <vtkConeSource.h>
#include <vtkContourFilter.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkGlyphSource2D.h>
#include <vtkImageData.h>
#include <vtkLine.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkTransform.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOrientedBSplineTransform.h>
#include <vtkOrientedGridTransform.h>
#include <vtkPoints.h>
#include <vtkPointSet.h>
#include <vtkPointData.h>
#include <vtkSphereSource.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTubeFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVectorNorm.h>
#include <vtkWarpVector.h>

// ITK includes
#include "itkBSplineDeformableTransform.h"
#include "itkCenteredAffineTransform.h"
#include "itkCenteredEuler3DTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkCenteredSimilarity2DTransform.h"
#include "itkEuler2DTransform.h"
#include "itkFixedCenterOfRotationAffineTransform.h"
#include "itkQuaternionRigidTransform.h"
#include "itkRigid3DPerspectiveTransform.h"
#include "itkScaleLogarithmicTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkScaleVersor3DTransform.h"
#include "itkTranslationTransform.h"
#include "itkTransformFactory.h"

vtkStandardNewMacro(vtkSlicerTransformLogic);

//----------------------------------------------------------------------------
vtkSlicerTransformLogic::vtkSlicerTransformLogic() = default;

//----------------------------------------------------------------------------
vtkSlicerTransformLogic::~vtkSlicerTransformLogic() = default;

//-----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::hardenTransform(vtkMRMLTransformableNode* transformableNode)
{
  if (!transformableNode)
  {
    return false;
  }
  return transformableNode->HardenTransform();
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkSlicerTransformLogic::AddTransform(const char* filename, vtkMRMLScene* scene, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  vtkNew<vtkMRMLTransformStorageNode> storageNode;

  if (scene == nullptr)
  {
    vtkErrorMacro("scene == nullptr in vtkSlicerTransformLogic::AddTransform");
    return nullptr;
  }

  // check for local or remote files
  int useURI = 0; // false;
  if (scene->GetCacheManager() != nullptr)
  {
    useURI = scene->GetCacheManager()->IsRemoteReference(filename);
  }

  std::string localFile;
  if (useURI)
  {
    vtkDebugMacro("AddTransforn: file name is remote: " << filename);
    storageNode->SetURI(filename);
    // reset filename to the local file name
    localFile = scene->GetCacheManager()->GetFilenameFromURI(filename);
  }
  else
  {
    storageNode->SetFileName(filename);
    localFile = filename;
  }

  // the model name is based on the file name (itksys call should work even if
  // file is not on disk yet)
  const std::string name = itksys::SystemTools::GetFilenameName(localFile);

  if (!storageNode->SupportedFileType(name.c_str()))
  {
    vtkErrorToMessageCollectionMacro(userMessages, "vtkSlicerTransformLogic::AddTransform", "Unsupported transform file format: " << filename);
    return nullptr;
  }

  // check to see which node can read this type of file
  vtkSmartPointer<vtkMRMLTransformNode> tnode;

  storageNode->SetScene(scene);

  vtkNew<vtkMRMLTransformNode> generalTransform;
  generalTransform->SetScene(scene);
  if (!storageNode->ReadData(generalTransform.GetPointer()))
  {
    if (userMessages)
    {
      userMessages->AddMessages(storageNode->GetUserMessages());
    }
    vtkErrorToMessageCollectionMacro(userMessages, "vtkSlicerTransformLogic::AddTransform", "Failed to read transform from file: " << filename);
    return nullptr;
  }

  // If we can create a specialized transform type then do it, otherwise just use
  // the generic transform node.
  // Specialized transforms are mainly used for filtering for transform type in node
  // selectors but that could be achieved by using node attributes. So, in the long
  // term we should get rid of specialized transform node classes and just use
  // vtkMRMLTransformNode (anyway, transform classes are not meaningful when transforms
  // are composited by hardening and we have mixed transforms).
  storageNode->GetUserMessages()->ClearMessages();
  switch (GetTransformKind(generalTransform.GetPointer()))
  {
    case TRANSFORM_LINEAR:
      tnode = vtkSmartPointer<vtkMRMLTransformNode>::Take(vtkMRMLLinearTransformNode::New());
      storageNode->ReadData(tnode.GetPointer());
      break;
    case TRANSFORM_BSPLINE:
      tnode = vtkSmartPointer<vtkMRMLTransformNode>::Take(vtkMRMLBSplineTransformNode::New());
      storageNode->ReadData(tnode.GetPointer());
      break;
    case TRANSFORM_GRID:
      tnode = vtkSmartPointer<vtkMRMLTransformNode>::Take(vtkMRMLGridTransformNode::New());
      storageNode->ReadData(tnode.GetPointer());
      break;
    default:
      // No specialized classes for thin plate spline, general, and other transforms
      tnode = generalTransform.GetPointer();
  }

  if (userMessages)
  {
    userMessages->AddMessages(storageNode->GetUserMessages());
  }

  const std::string basename(storageNode->GetFileNameWithoutExtension(localFile));
  const std::string uname(scene->GetUniqueNameByString(basename.c_str()));
  tnode->SetName(uname.c_str());
  scene->AddNode(storageNode.GetPointer());
  scene->AddNode(tnode);

  tnode->SetAndObserveStorageNodeID(storageNode->GetID());

  return tnode;
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::GetTransformedPointSamplesAsMagnitudeImage(vtkImageData* magnitudeImage,
                                                                         vtkMRMLTransformNode* inputTransformNode,
                                                                         vtkMatrix4x4* ijkToRAS,
                                                                         bool transformToWorld /* = true */)
{
  if (!magnitudeImage || !inputTransformNode || !ijkToRAS)
  {
    vtkGenericWarningMacro("vtkSlicerTransformLogic::GetTransformedPointSamplesAsMagnitudeImage failed: invalid input");
    return false;
  }

  vtkNew<vtkGeneralTransform> inputTransform;
  if (transformToWorld)
  {
    inputTransformNode->GetTransformToWorld(inputTransform.GetPointer());
  }
  else
  {
    inputTransformNode->GetTransformFromWorld(inputTransform.GetPointer());
  }

  // The orientation of the volume cannot be set in the image
  // therefore the volume will not appear in the correct position
  // if the direction matrix is not identity.
  magnitudeImage->AllocateScalars(VTK_FLOAT, 1);

  double point_RAS[4] = { 0, 0, 0, 1 };
  double transformedPoint_RAS[4] = { 0, 0, 0, 1 };
  double pointDislocationVector_RAS[4] = { 0, 0, 0, 1 };
  double point_IJK[4] = { 0, 0, 0, 1 };
  float* voxelPtr = static_cast<float*>(magnitudeImage->GetScalarPointer());
  int* extent = magnitudeImage->GetExtent();
  for (point_IJK[2] = extent[4]; point_IJK[2] <= extent[5]; point_IJK[2]++)
  {
    for (point_IJK[1] = extent[2]; point_IJK[1] <= extent[3]; point_IJK[1]++)
    {
      for (point_IJK[0] = extent[0]; point_IJK[0] <= extent[1]; point_IJK[0]++)
      {
        ijkToRAS->MultiplyPoint(point_IJK, point_RAS);

        inputTransform->TransformPoint(point_RAS, transformedPoint_RAS);

        pointDislocationVector_RAS[0] = transformedPoint_RAS[0] - point_RAS[0];
        pointDislocationVector_RAS[1] = transformedPoint_RAS[1] - point_RAS[1];
        pointDislocationVector_RAS[2] = transformedPoint_RAS[2] - point_RAS[2];

        float mag = sqrt(pointDislocationVector_RAS[0] * pointDislocationVector_RAS[0] + pointDislocationVector_RAS[1] * pointDislocationVector_RAS[1]
                         + pointDislocationVector_RAS[2] * pointDislocationVector_RAS[2]);

        *(voxelPtr++) = mag;
      }
    }
  }

  return true;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerTransformLogic::CreateDisplacementVolumeFromTransform(vtkMRMLTransformNode* inputTransformNode,
                                                                                  vtkMRMLVolumeNode* referenceVolumeNode,
                                                                                  bool magnitude /*=true*/,
                                                                                  vtkMRMLVolumeNode* existingOutputVolumeNode /* = nullptr */)
{
  if (inputTransformNode == nullptr)
  {
    vtkErrorMacro("vtkSlicerTransformLogic::CreateDisplacementVolumeFromTransform failed: inputs are invalid");
    return nullptr;
  }
  if ((referenceVolumeNode == nullptr || referenceVolumeNode->GetImageData() == nullptr) && (existingOutputVolumeNode == nullptr))
  {
    vtkErrorMacro("vtkSlicerTransformLogic::ConvertToGridTransform failed: either a valid \
                        reference volume node or an existing output volume node has to be specified");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (scene == nullptr)
  {
    vtkErrorMacro("vtkSlicerTransformLogic::CreateDisplacementVolumeFromTransform failed: scene invalid");
    return nullptr;
  }

  // Create/get a volume node
  vtkSmartPointer<vtkMRMLVolumeNode> outputVolumeNode;
  if (existingOutputVolumeNode != nullptr)
  {
    outputVolumeNode = existingOutputVolumeNode;
    // We always compute transform to world. Set the parent transform to world, to be consistent.
    outputVolumeNode->SetAndObserveTransformNodeID(nullptr);
  }
  else
  {
    // Displacement vectors are a vector volume, so that they are shown and selectable as one
    if (magnitude)
    {
      outputVolumeNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
    }
    else
    {
      outputVolumeNode = vtkSmartPointer<vtkMRMLVectorVolumeNode>::New();
    }
    std::string nodeName = inputTransformNode->GetName();
    if (magnitude)
    {
      nodeName += " displacement magnitude";
    }
    else
    {
      nodeName += " displacement vectors";
    }
    nodeName = scene->GenerateUniqueName(nodeName);
    outputVolumeNode->SetName(nodeName.c_str());
    scene->AddNode(outputVolumeNode);
  }

  // Create/get displacement field image
  vtkImageData* outputVolume = outputVolumeNode->GetImageData();
  if (outputVolume == nullptr)
  {
    vtkNew<vtkImageData> newOutputVolume;
    outputVolume = newOutputVolume.GetPointer();
    outputVolumeNode->SetAndObserveImageData(outputVolume);
  }

  vtkNew<vtkMatrix4x4> ijkToRas; // RAS refers to world
  if (referenceVolumeNode != nullptr)
  {
    referenceVolumeNode->GetIJKToRASMatrix(ijkToRas.GetPointer());
    vtkNew<vtkMatrix4x4> rasToWorld;
    if (vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(referenceVolumeNode->GetParentTransformNode(), nullptr /* world */, rasToWorld.GetPointer()))
    {
      vtkMatrix4x4::Multiply4x4(rasToWorld.GetPointer(), ijkToRas.GetPointer(), ijkToRas.GetPointer());
    }
    else
    {
      vtkErrorMacro("vtkSlicerTransformLogic::CreateDisplacementVolumeFromTransform: non-linearly transformed reference volume is not supported."
                    " Harden or remove the transform from of the reference volume.");
      if (outputVolumeNode != existingOutputVolumeNode)
      {
        scene->RemoveNode(outputVolumeNode);
      }
      return nullptr;
    }
    outputVolumeNode->SetIJKToRASMatrix(ijkToRas.GetPointer());
    outputVolume->SetExtent(referenceVolumeNode->GetImageData()->GetExtent());
    outputVolumeNode->SetAndObserveImageData(outputVolume);
  }
  else
  {
    outputVolumeNode->GetIJKToRASMatrix(ijkToRas.GetPointer());
  }

  // Fill the volume
  if (magnitude)
  {
    vtkSlicerTransformLogic::GetTransformedPointSamplesAsMagnitudeImage(outputVolume, inputTransformNode, ijkToRas.GetPointer());
    outputVolumeNode->SetVoxelVectorType(vtkMRMLVolumeNode::VoxelVectorTypeUndefined);
  }
  else
  {
    vtkSlicerTransformLogic::GetTransformedPointSamplesAsVectorImage(outputVolume, inputTransformNode, ijkToRas.GetPointer());
    // This indicates that the voxel values should be transformed to LPS when written to file
    outputVolumeNode->SetVoxelVectorType(vtkMRMLVolumeNode::VoxelVectorTypeSpatial);
  }

  if (outputVolumeNode->GetDisplayNode() == nullptr)
  {
    if (magnitude)
    {
      vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
      scene->AddNode(displayNode.GetPointer());
      displayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeFileViridis.txt");
      outputVolumeNode->SetAndObserveNthDisplayNodeID(0, displayNode->GetID());
    }
    else
    {
      vtkNew<vtkMRMLVectorVolumeDisplayNode> displayNode;
      scene->AddNode(displayNode.GetPointer());
      displayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeFileViridis.txt");
      outputVolumeNode->SetAndObserveNthDisplayNodeID(0, displayNode->GetID());
    }
  }

  return outputVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkSlicerTransformLogic::ConvertToGridTransform(vtkMRMLTransformNode* inputTransformNode,
                                                                      vtkMRMLVolumeNode* referenceVolumeNode /* = nullptr */,
                                                                      vtkMRMLTransformNode* existingOutputTransformNode /* = nullptr */)
{
  if (inputTransformNode == nullptr)
  {
    vtkErrorMacro("vtkSlicerTransformLogic::ConvertToGridTransform failed: inputTransformNode is invalid");
    return nullptr;
  }
  if ((referenceVolumeNode == nullptr || referenceVolumeNode->GetImageData() == nullptr) && (existingOutputTransformNode == nullptr))
  {
    vtkErrorMacro("vtkSlicerTransformLogic::ConvertToGridTransform failed: either a valid reference volume node"
                  " or an existing output transform node has to be specified");
    return nullptr;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (scene == nullptr)
  {
    vtkErrorMacro("vtkSlicerTransformLogic::ConvertToGridTransform failed: scene invalid");
    return nullptr;
  }

  // Create/get a grid transform
  vtkSmartPointer<vtkMRMLTransformNode> outputGridTransformNode;
  if (existingOutputTransformNode != nullptr)
  {
    outputGridTransformNode = existingOutputTransformNode;
    // We always compute transform to world. Set the parent transform to world, to be consistent.
    outputGridTransformNode->SetAndObserveTransformNodeID(nullptr);
  }
  else
  {
    outputGridTransformNode = vtkSmartPointer<vtkMRMLTransformNode>::New();
    std::string nodeName = std::string(inputTransformNode->GetName()) + " grid transform";
    outputGridTransformNode->SetName(scene->GenerateUniqueName(nodeName).c_str());
    scene->AddNode(outputGridTransformNode);
  }
  // Create/get grid transform
  vtkOrientedGridTransform* outputGridTransform = vtkOrientedGridTransform::SafeDownCast(
    outputGridTransformNode->GetTransformToParentAs("vtkOrientedGridTransform", false /* don't report conversion error */, true /* we would like to modify the transform */));
  if (outputGridTransform == nullptr)
  {
    // we cannot reuse the existing transform, create a new one
    vtkNew<vtkOrientedGridTransform> newOutputGridTransform;
    outputGridTransform = newOutputGridTransform.GetPointer();
    outputGridTransformNode->SetAndObserveTransformFromParent(outputGridTransform);
  }
  // Create/get displacement field image
  vtkImageData* outputVolume = outputGridTransform->GetDisplacementGrid();
  if (outputVolume == nullptr)
  {
    vtkNew<vtkImageData> newOutputVolume;
    outputVolume = newOutputVolume.GetPointer();
    outputGridTransform->SetDisplacementGridData(outputVolume);
  }
  // Update geometry based on reference image (if specified)
  vtkNew<vtkMatrix4x4> ijkToRas; // RAS refers to world
  if (referenceVolumeNode != nullptr)
  {
    referenceVolumeNode->GetIJKToRASMatrix(ijkToRas.GetPointer());
    vtkNew<vtkMatrix4x4> rasToWorld;
    if (vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(referenceVolumeNode->GetParentTransformNode(), nullptr /* world */, rasToWorld.GetPointer()))
    {
      vtkMatrix4x4::Multiply4x4(rasToWorld.GetPointer(), ijkToRas.GetPointer(), ijkToRas.GetPointer());
    }
    else
    {
      vtkErrorMacro("vtkSlicerTransformLogic::ConvertToGridTransform: non-linearly transformed reference volume"
                    " is not supported. Harden or remove the transform from of the reference volume.");
      if (outputGridTransformNode != existingOutputTransformNode)
      {
        scene->RemoveNode(outputGridTransformNode);
      }
      return nullptr;
    }
    double origin[3] = { 0, 0, 0 };
    double spacing[3] = { 1, 1, 1 };
    vtkNew<vtkMatrix4x4> ijkToRasDirection; // normalized direction matrix
    for (int c = 0; c < 3; c++)
    {
      origin[c] = ijkToRas->GetElement(c, 3);
      spacing[c] = sqrt(ijkToRas->Element[0][c] * ijkToRas->Element[0][c] + ijkToRas->Element[1][c] * ijkToRas->Element[1][c] + ijkToRas->Element[2][c] * ijkToRas->Element[2][c]);
      if (spacing[c] == 0)
      {
        // Prevent division by zero in case there is a projection matrix is in the transform chain
        spacing[c] = 1.0;
      }
      for (int r = 0; r < 3; r++)
      {
        ijkToRasDirection->SetElement(r, c, ijkToRas->GetElement(r, c) / spacing[c]);
      }
    }
    outputVolume->SetExtent(referenceVolumeNode->GetImageData()->GetExtent());
    outputVolume->SetOrigin(origin);
    outputVolume->SetSpacing(spacing);
    // vtkImageData cannot store directions, therefore that has to be set in the grid transform
    outputGridTransform->SetGridDirectionMatrix(ijkToRasDirection.GetPointer());
  }
  else
  {
    // Use existing geometry in grid transform
    double* origin = outputVolume->GetOrigin();
    double* spacing = outputVolume->GetSpacing();
    vtkMatrix4x4* gridDirection = outputGridTransform->GetGridDirectionMatrix();
    for (unsigned int row = 0; row < 3; row++)
    {
      for (unsigned int col = 0; col < 3; col++)
      {
        if (gridDirection != nullptr)
        {
          ijkToRas->SetElement(row, col, spacing[col] * gridDirection->GetElement(row, col));
        }
        else
        {
          ijkToRas->SetElement(row, col, spacing[col] * ijkToRas->GetElement(row, col));
        }
      }
      ijkToRas->SetElement(row, 3, origin[row]);
    }
  }

  // Fill the volume with displacement values
  bool transformToWorld = false; // usually grid transform is defined as transform from parent
  vtkSlicerTransformLogic::GetTransformedPointSamplesAsVectorImage(outputVolume, inputTransformNode, ijkToRas.GetPointer(), transformToWorld);

  return outputGridTransformNode.GetPointer();
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::ImportTransformFromDisplacementVolume(vtkMRMLTransformNode* transformNode, vtkMRMLVectorVolumeNode* displacementVolumeNode)
{
  if (!transformNode || !displacementVolumeNode)
  {
    vtkErrorMacro("vtkSlicerTransformLogic::ImportTransformFromDisplacementVolume failed: invalid transform or volume node");
    return false;
  }
  vtkImageData* displacementImage = displacementVolumeNode->GetImageData();
  vtkDataArray* displacements = (displacementImage ? displacementImage->GetPointData()->GetScalars() : nullptr);
  if (!displacements || displacements->GetNumberOfComponents() != 3)
  {
    vtkErrorMacro("vtkSlicerTransformLogic::ImportTransformFromDisplacementVolume failed: the volume has to have 3 components, a displacement vector in each voxel");
    return false;
  }

  // Geometry of the volume in world coordinates
  vtkNew<vtkMatrix4x4> ijkToRas;
  displacementVolumeNode->GetIJKToRASMatrix(ijkToRas);
  vtkNew<vtkMatrix4x4> rasToWorld;
  if (!vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(displacementVolumeNode->GetParentTransformNode(), nullptr /* world */, rasToWorld))
  {
    vtkErrorMacro("vtkSlicerTransformLogic::ImportTransformFromDisplacementVolume failed: non-linearly transformed volume"
                  " is not supported. Harden or remove the transform of the volume.");
    return false;
  }
  vtkMatrix4x4::Multiply4x4(rasToWorld, ijkToRas, ijkToRas);
  double origin[3] = { 0.0, 0.0, 0.0 };
  double spacing[3] = { 1.0, 1.0, 1.0 };
  vtkNew<vtkMatrix4x4> ijkToRasDirection; // normalized direction matrix
  for (int c = 0; c < 3; c++)
  {
    origin[c] = ijkToRas->GetElement(c, 3);
    spacing[c] = sqrt(ijkToRas->Element[0][c] * ijkToRas->Element[0][c] + ijkToRas->Element[1][c] * ijkToRas->Element[1][c] + ijkToRas->Element[2][c] * ijkToRas->Element[2][c]);
    if (spacing[c] == 0)
    {
      spacing[c] = 1.0;
    }
    for (int r = 0; r < 3; r++)
    {
      ijkToRasDirection->SetElement(r, c, ijkToRas->GetElement(r, c) / spacing[c]);
    }
  }

  // A copy of the displacements, so that the transform does not change with the volume
  vtkNew<vtkImageData> displacementGrid;
  displacementGrid->SetExtent(displacementImage->GetExtent());
  displacementGrid->SetOrigin(origin);
  displacementGrid->SetSpacing(spacing);
  vtkNew<vtkDoubleArray> displacementsCopy;
  displacementsCopy->DeepCopy(displacements);
  // Under a linear transform M, p displaced by d goes to M*(p+d) = M*p + M*d,
  // so the world displacement is the linear part of M applied to d.
  for (vtkIdType tupleIndex = 0; tupleIndex < displacementsCopy->GetNumberOfTuples(); ++tupleIndex)
  {
    double displacement_RAS[4] = { 0.0, 0.0, 0.0, 0.0 };
    displacementsCopy->GetTuple(tupleIndex, displacement_RAS);
    double displacement_World[4] = { 0.0, 0.0, 0.0, 0.0 };
    rasToWorld->MultiplyPoint(displacement_RAS, displacement_World);
    displacementsCopy->SetTuple(tupleIndex, displacement_World);
  }
  displacementGrid->GetPointData()->SetScalars(displacementsCopy);

  vtkNew<vtkOrientedGridTransform> gridTransform;
  gridTransform->SetDisplacementGridData(displacementGrid);
  // vtkImageData cannot store directions, therefore that has to be set in the grid transform
  gridTransform->SetGridDirectionMatrix(ijkToRasDirection);
  // The displacement volume holds the displacement of the transform to world
  transformNode->SetAndObserveTransformToParent(gridTransform);
  return true;
}

//----------------------------------------------------------------------------
vtkDataArray* vtkSlicerTransformLogic::GetDisplacementArray(vtkPointSet* mesh, const char* displacementArrayName)
{
  if (!mesh || mesh->GetNumberOfPoints() == 0)
  {
    return nullptr;
  }
  vtkDataArray* displacements = (displacementArrayName && displacementArrayName[0] != '\0' ? mesh->GetPointData()->GetArray(displacementArrayName) //
                                                                                           : mesh->GetPointData()->GetVectors());
  return (displacements && displacements->GetNumberOfComponents() == 3 ? displacements : nullptr);
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::SetScatteredBSplineTransformDefaults(vtkScatteredBSplineTransform* transform, vtkPointSet* mesh, const char* displacementArrayName)
{
  if (!transform)
  {
    return;
  }
  vtkDataArray* displacements = vtkSlicerTransformLogic::GetDisplacementArray(mesh, displacementArrayName);
  transform->SetDisplacementArrayName(displacements ? displacements->GetName() : displacementArrayName);
  transform->UseLinearApproximationOn();
  transform->AutomaticGridBoundsOn();
  transform->SetControlPointSpacing(0.0, 0.0, 0.0);
  transform->SetMaximumNumberOfLevels(8);
  // Beyond the fitted region the displacement fades to zero, so that a transform of a model does not move positions far from it
  transform->SetBorderMode(VTK_BSPLINE_ZERO_AT_BORDER);
  // The displacement falls to zero at twice the largest displacement from the points
  transform->AutomaticBorderFalloffDistanceOn();
  if (!displacements)
  {
    return;
  }
  // The fit covers the points and a margin around them
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  mesh->GetBounds(bounds);
  double diagonal = std::sqrt((bounds[1] - bounds[0]) * (bounds[1] - bounds[0]) //
                              + (bounds[3] - bounds[2]) * (bounds[3] - bounds[2]) + (bounds[5] - bounds[4]) * (bounds[5] - bounds[4]));
  double margin = 0.1 * diagonal;
  transform->SetGridBoundsMargin(margin);
  double magnitudeRange[2] = { 0.0, -1.0 };
  displacements->GetRange(magnitudeRange, -1); // -1: L2 norm over all components
  // The fitted region also holds the falloff: 1.25 times the automatic falloff distance beyond the points
  double fittedMargin = std::max(margin, 1.25 * 2.0 * std::max(0.0, magnitudeRange[1]));
  double largestSize = std::max(std::max(bounds[1] - bounds[0], bounds[3] - bounds[2]), bounds[5] - bounds[4]) + 2.0 * fittedMargin;
  transform->SetMinimumControlPointSpacing(largestSize > 0.0 ? largestSize / 100.0 : 0.1);
  transform->SetTolerance(magnitudeRange[1] > 0.0 ? 0.001 * magnitudeRange[1] : 1e-6);
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::ImportTransformFromModel(vtkMRMLTransformNode* transformNode,
                                                       vtkMRMLModelNode* modelNode,
                                                       const char* displacementArrayName /*=nullptr*/,
                                                       bool keepLinked /*=true*/)
{
  if (!transformNode || !modelNode)
  {
    vtkErrorMacro("vtkSlicerTransformLogic::ImportTransformFromModel failed: invalid transform or model node");
    return false;
  }
  vtkPointSet* mesh = modelNode->GetMesh();
  if (!vtkSlicerTransformLogic::GetDisplacementArray(mesh, displacementArrayName))
  {
    vtkErrorMacro("vtkSlicerTransformLogic::ImportTransformFromModel failed: the mesh of the model has no 3-component point data array "
                  << (displacementArrayName && displacementArrayName[0] ? displacementArrayName : "(active vectors)"));
    return false;
  }

  vtkNew<vtkScatteredBSplineTransform> fit;
  vtkSlicerTransformLogic::SetScatteredBSplineTransformDefaults(fit, mesh, displacementArrayName);

  // A model-based transform that follows the model
  if (keepLinked)
  {
    transformNode->SetAndObserveModelBasedTransformToParent(fit, modelNode);
    return true;
  }

  // A copy of the fit, as a B-spline transform that can be saved
  fit->SetInputPointSet(mesh);
  fit->Update();
  vtkImageData* coefficients = fit->GetCoefficientData();
  if (!coefficients)
  {
    vtkErrorMacro("vtkSlicerTransformLogic::ImportTransformFromModel failed: the displacements could not be fitted");
    return false;
  }
  vtkNew<vtkImageData> coefficientsCopy;
  coefficientsCopy->DeepCopy(coefficients);
  vtkNew<vtkOrientedBSplineTransform> bsplineTransform;
  bsplineTransform->SetCoefficientData(coefficientsCopy);
  bsplineTransform->SetBorderMode(fit->GetBorderMode());
  bsplineTransform->SetDisplacementScale(fit->GetDisplacementScale());
  transformNode->SetAndObserveTransformToParent(bsplineTransform);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::GetTransformedPointSamplesAsVectorImage(vtkImageData* vectorImage,
                                                                      vtkMRMLTransformNode* inputTransformNode,
                                                                      vtkMatrix4x4* ijkToRAS,
                                                                      bool transformToWorld /* = true */)
{
  if (!vectorImage || !inputTransformNode || !ijkToRAS)
  {
    vtkGenericWarningMacro("vtkSlicerTransformLogic::GetTransformedPointSamplesAsVectorImage failed: invalid input");
    return false;
  }
  vtkNew<vtkGeneralTransform> inputTransform;
  if (transformToWorld)
  {
    inputTransformNode->GetTransformToWorld(inputTransform.GetPointer());
  }
  else
  {
    inputTransformNode->GetTransformFromWorld(inputTransform.GetPointer());
  }

  // The orientation of the volume cannot be set in the image
  // therefore the volume will not appear in the correct position
  // if the direction matrix is not identity.
  vectorImage->AllocateScalars(VTK_FLOAT, 3);

  double point_RAS[4] = { 0, 0, 0, 1 };
  double transformedPoint_RAS[4] = { 0, 0, 0, 1 };
  double point_IJK[4] = { 0, 0, 0, 1 };
  float* voxelPtr = static_cast<float*>(vectorImage->GetScalarPointer());
  int* extent = vectorImage->GetExtent();
  for (point_IJK[2] = extent[4]; point_IJK[2] <= extent[5]; point_IJK[2]++)
  {
    for (point_IJK[1] = extent[2]; point_IJK[1] <= extent[3]; point_IJK[1]++)
    {
      for (point_IJK[0] = extent[0]; point_IJK[0] <= extent[1]; point_IJK[0]++)
      {
        ijkToRAS->MultiplyPoint(point_IJK, point_RAS);

        inputTransform->TransformPoint(point_RAS, transformedPoint_RAS);

        // store the pointDislocationVector_RAS components in the image
        *(voxelPtr++) = static_cast<float>(transformedPoint_RAS[0] - point_RAS[0]);
        *(voxelPtr++) = static_cast<float>(transformedPoint_RAS[1] - point_RAS[1]);
        *(voxelPtr++) = static_cast<float>(transformedPoint_RAS[2] - point_RAS[2]);
      }
    }
  }

  return true;
}

//----------------------------------------------------------------------------
vtkSlicerTransformLogic::TransformKind vtkSlicerTransformLogic::GetTransformKind(vtkMRMLTransformNode* transformNode)
{
  if (transformNode == nullptr)
  {
    return TRANSFORM_OTHER;
  }

  // Check if it is a simple transform
  vtkAbstractTransform* inputTransform = transformNode->GetTransformToParent();
  if (vtkTransform::SafeDownCast(inputTransform))
  {
    return TRANSFORM_LINEAR;
  }
  else if (vtkOrientedBSplineTransform::SafeDownCast(inputTransform))
  {
    return TRANSFORM_BSPLINE;
  }
  else if (vtkOrientedGridTransform::SafeDownCast(inputTransform))
  {
    return TRANSFORM_GRID;
  }
  else if (vtkThinPlateSplineTransform::SafeDownCast(inputTransform))
  {
    return TRANSFORM_THINPLATESPLINE;
  }

  // Check if it is a simple transform embedded into a general transform
  vtkGeneralTransform* generalTransform = vtkGeneralTransform::SafeDownCast(inputTransform);
  if (generalTransform == nullptr)
  {
    // it's not a general transform, so we cannot decompose it
    return TRANSFORM_OTHER;
  }
  vtkNew<vtkCollection> transformList;
  vtkMRMLTransformNode::FlattenGeneralTransform(transformList.GetPointer(), generalTransform);
  if (transformList->GetNumberOfItems() == 1)
  {
    if (vtkTransform::SafeDownCast(transformList->GetItemAsObject(0)))
    {
      return TRANSFORM_LINEAR;
    }
    else if (vtkOrientedBSplineTransform::SafeDownCast(transformList->GetItemAsObject(0)))
    {
      return TRANSFORM_BSPLINE;
    }
    else if (vtkOrientedGridTransform::SafeDownCast(transformList->GetItemAsObject(0)))
    {
      return TRANSFORM_GRID;
    }
    else if (vtkThinPlateSplineTransform::SafeDownCast(transformList->GetItemAsObject(0)))
    {
      return TRANSFORM_THINPLATESPLINE;
    }
  }
  else if (transformList->GetNumberOfItems() == 2)
  {
    if (vtkTransform::SafeDownCast(transformList->GetItemAsObject(1)) //
        && vtkOrientedBSplineTransform::SafeDownCast(transformList->GetItemAsObject(0)))
    {
      // B-spline and bulk transform embedded in a general transform
      // BRAINS returns bspline+bulk transform this way, so we consider it as a bspline transform
      return TRANSFORM_BSPLINE;
    }
  }
  return TRANSFORM_OTHER;
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetTransformedNodes(vtkMRMLScene* scene, vtkMRMLTransformNode* transformNode, std::vector<vtkMRMLDisplayableNode*>& transformedNodes, bool recursive)
{
  if (!scene || !transformNode || !transformNode->GetID())
  {
    return;
  }

  std::vector<vtkMRMLNode*> nodes;
  scene->GetNodesByClass("vtkMRMLDisplayableNode", nodes);
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    vtkMRMLDisplayableNode* node = vtkMRMLDisplayableNode::SafeDownCast(nodes[i]);
    if (node->GetTransformNodeID() && //
        strcmp(transformNode->GetID(), node->GetTransformNodeID()) == 0)
    {
      transformedNodes.push_back(node);

      vtkMRMLTransformNode* childTransformNode = vtkMRMLTransformNode::SafeDownCast(node);
      if (recursive && childTransformNode)
      {
        vtkSlicerTransformLogic::GetTransformedNodes(scene, childTransformNode, transformedNodes, recursive);
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetNodesRASBounds(const std::vector<vtkMRMLDisplayableNode*>& nodes, double bounds[6])
{
  vtkBoundingBox box;
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    double nodeBounds[6];
    nodes[i]->GetRASBounds(nodeBounds);
    if (nodeBounds[0] <= nodeBounds[1] || //
        nodeBounds[2] <= nodeBounds[3] || //
        nodeBounds[4] <= nodeBounds[5])
    {
      box.AddBounds(nodeBounds);
    }
  }
  box.GetBounds(bounds);
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetNodesBounds(const std::vector<vtkMRMLDisplayableNode*>& nodes, double bounds[6])
{
  vtkBoundingBox box;
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    double nodeBounds[6];
    nodes[i]->GetBounds(nodeBounds);
    if (nodeBounds[0] <= nodeBounds[1] || //
        nodeBounds[2] <= nodeBounds[3] || //
        nodeBounds[4] <= nodeBounds[5])
    {
      box.AddBounds(nodeBounds);
    }
  }
  box.GetBounds(bounds);
}

//----------------------------------------------------------------------------
std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> vtkSlicerTransformLogic::CreateFileIOHandlers()
{
  std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> handlers;
  vtkNew<vtkSlicerTransformsReader> transformsReader;
  transformsReader->SetTransformLogic(this);
  handlers.emplace_back(transformsReader);
  handlers.emplace_back(vtkMRMLNodeWriter::CreateNodeWriter("Transforms", "TransformFile", { "vtkMRMLTransformNode" }, true));
  return handlers;
}
