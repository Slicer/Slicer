/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkSlicerTransformLogic.h"

#include "vtkTransformVisualizerGlyph3D.h"

// MRML includes
#include "vtkCacheManager.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLTransformDisplayNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"

// ITKsys includes
#include <itksys/SystemTools.hxx>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkCollection.h>
#include <vtkArrowSource.h>
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
vtkSlicerTransformLogic::vtkSlicerTransformLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTransformLogic::~vtkSlicerTransformLogic()
{
}

//-----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::hardenTransform(vtkMRMLTransformableNode* transformableNode)
{
  vtkMRMLTransformNode* transformNode =
    transformableNode ? transformableNode->GetParentTransformNode() : 0;
  if (!transformNode)
    {
    return false;
    }
  if (transformNode->IsTransformToWorldLinear())
    {
    vtkNew<vtkMatrix4x4> hardeningMatrix;
    transformNode->GetMatrixTransformToWorld(hardeningMatrix.GetPointer());
    transformableNode->ApplyTransformMatrix(hardeningMatrix.GetPointer());
    }
  else
    {
    vtkNew<vtkGeneralTransform> hardeningTransform;
    transformNode->GetTransformToWorld(hardeningTransform.GetPointer());
    transformableNode->ApplyTransform(hardeningTransform.GetPointer());
    }

  transformableNode->SetAndObserveTransformNodeID(NULL);
  return true;
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkSlicerTransformLogic::AddTransform (const char* filename, vtkMRMLScene *scene)
{
  vtkNew<vtkMRMLTransformStorageNode> storageNode;

  if(scene == NULL)
    {
    vtkErrorMacro("scene == NULL in vtkSlicerTransformLogic::AddTransform");
    return NULL;
    }

  // check for local or remote files
  int useURI = 0; // false;
  if (scene->GetCacheManager() != NULL)
    {
    useURI = scene->GetCacheManager()->IsRemoteReference(filename);
    }

  itksys_stl::string name;
  const char *localFile;
  if (useURI)
    {
    vtkDebugMacro("AddTransforn: file name is remote: " << filename);
    storageNode->SetURI(filename);
    // reset filename to the local file name
    localFile = ((scene)->GetCacheManager())->GetFilenameFromURI(filename);
    }
  else
    {
    storageNode->SetFileName(filename);
    localFile = filename;
    }

  const itksys_stl::string fname(localFile);
  // the model name is based on the file name (itksys call should work even if
  // file is not on disk yet)
  name = itksys::SystemTools::GetFilenameName(fname);

  if (!storageNode->SupportedFileType(name.c_str()))
    {
    vtkErrorMacro("Unsupported transform file format: " << filename);
    return NULL;
    }

  // check to see which node can read this type of file
  vtkSmartPointer<vtkMRMLTransformNode> tnode;

  scene->SaveStateForUndo();
  storageNode->SetScene(scene);

  vtkNew<vtkMRMLTransformNode> generalTransform;
  generalTransform->SetScene(scene);
  if (!storageNode->ReadData(generalTransform.GetPointer()))
    {
    vtkErrorMacro("Failed to read transform from file: " << filename);
    return NULL;
    }

  // If we can create a specialized transform type then do it, otherwise just use
  // the generic transform node.
  // Specialized transforms are mainly used for filtering for transform type in node
  // selectors but that could be achieved by using node attributes. So, in the long
  // term we should get rid of specialized transform node classes and just use
  // vtkMRMLTransformNode (anyway, transform classes are not meaningful when transforms
  // are composited by hardening and we have mixed transforms).
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
      tnode =  generalTransform.GetPointer();
    }

  const itksys_stl::string basename(itksys::SystemTools::GetFilenameWithoutExtension(fname));
  const std::string uname( scene->GetUniqueNameByString(basename.c_str()));
  tnode->SetName(uname.c_str());
  scene->AddNode(storageNode.GetPointer());
  scene->AddNode(tnode);

  tnode->SetAndObserveStorageNodeID(storageNode->GetID());

  return tnode;
}

int vtkSlicerTransformLogic::SaveTransform (const char* vtkNotUsed(filename),
                                            vtkMRMLTransformNode *vtkNotUsed(transformNode))
{
  return 1;
}

//----------------------------------------------------------------------------
const char* vtkSlicerTransformLogic::GetVisualizationDisplacementMagnitudeScalarName()
{
  return "DisplacementMagnitude";
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetTransformedPointSamples(vtkPointSet* outputPointSet, vtkMRMLTransformNode* inputTransformNode, vtkMatrix4x4* gridToRAS, int* gridSize)
{
  if (!inputTransformNode)
    {
    return;
    }

  int numOfSamples=gridSize[0]*gridSize[1]*gridSize[2];

  //Will contain all the points that are to be rendered
  vtkNew<vtkPoints> samplePositions_RAS;
  samplePositions_RAS->SetNumberOfPoints(numOfSamples);

  //Will contain the corresponding vectors for outputPointSet
  vtkNew<vtkDoubleArray> sampleVectors_RAS;
  sampleVectors_RAS->Initialize();
  sampleVectors_RAS->SetNumberOfComponents(3);
  sampleVectors_RAS->SetNumberOfTuples(numOfSamples);
  sampleVectors_RAS->SetName("DisplacementVector");

  vtkNew<vtkGeneralTransform> inputTransform;
  inputTransformNode->GetTransformToWorld(inputTransform.GetPointer());

  double point_RAS[4] = {0,0,0,1};
  double transformedPoint_RAS[4] = {0,0,0,1};
  double pointDislocationVector_RAS[4] = {0,0,0,1};
  double point_Grid[4]={0,0,0,1};
  int sampleIndex=0;
  for (point_Grid[2]=0; point_Grid[2]<gridSize[2]; point_Grid[2]++)
    {
    for (point_Grid[1]=0; point_Grid[1]<gridSize[1]; point_Grid[1]++)
      {
      for (point_Grid[0]=0; point_Grid[0]<gridSize[0]; point_Grid[0]++)
        {
        gridToRAS->MultiplyPoint(point_Grid, point_RAS);

        inputTransform->TransformPoint(point_RAS, transformedPoint_RAS);

        pointDislocationVector_RAS[0] = transformedPoint_RAS[0] - point_RAS[0];
        pointDislocationVector_RAS[1] = transformedPoint_RAS[1] - point_RAS[1];
        pointDislocationVector_RAS[2] = transformedPoint_RAS[2] - point_RAS[2];

        samplePositions_RAS->SetPoint(sampleIndex, point_RAS[0], point_RAS[1], point_RAS[2]);
        sampleVectors_RAS->SetTuple3(sampleIndex,pointDislocationVector_RAS[0], pointDislocationVector_RAS[1], pointDislocationVector_RAS[2]);
        sampleIndex++;
        }
      }
    }

  outputPointSet->SetPoints(samplePositions_RAS.GetPointer());
  vtkPointData* pointData = outputPointSet->GetPointData();
  pointData->SetVectors(sampleVectors_RAS.GetPointer());

  // Compute vector magnitude and add to the data set
  vtkNew<vtkVectorNorm> norm;
#if VTK_MAJOR_VERSION <= 5
  norm->SetInput(outputPointSet);
#else
  norm->SetInputData(outputPointSet);
#endif
  norm->Update();
  vtkDataArray* vectorMagnitude = norm->GetOutput()->GetPointData()->GetScalars();
  vectorMagnitude->SetName(GetVisualizationDisplacementMagnitudeScalarName());
  int idx=pointData->AddArray(vectorMagnitude);
  pointData->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
  }

/// Takes samples from the displacement field specified by the transformation on a slice
/// and stores it in an unstructured grid.
/// pointGroupSize: the number of points will be N*pointGroupSize (the actual number will be returned in numGridPoints[3])
//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetTransformedPointSamplesOnSlice(vtkPointSet* outputPointSet, vtkMRMLTransformNode* inputTransformNode, vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize, double pointSpacing, int pointGroupSize/*=1*/, int* numGridPoints/*=0*/)
{
  int numOfPointsX=floor(fieldOfViewSize[0]/(pointSpacing*pointGroupSize))*pointGroupSize;
  int numOfPointsY=floor(fieldOfViewSize[1]/(pointSpacing*pointGroupSize))*pointGroupSize;
  double xOfs = (fieldOfViewSize[0]-(numOfPointsX*pointSpacing))/2 -fieldOfViewSize[0]/2+fieldOfViewOrigin[0];
  double yOfs = (fieldOfViewSize[1]-(numOfPointsY*pointSpacing))/2 -fieldOfViewSize[1]/2+fieldOfViewOrigin[1];

  int gridSize[3]={numOfPointsX+1,numOfPointsY+1,1};

  vtkNew<vtkMatrix4x4> gridToRAS;
  gridToRAS->DeepCopy(sliceToRAS);
  vtkNew<vtkMatrix4x4> gridOffset;
  gridOffset->Element[0][3]=xOfs;
  gridOffset->Element[1][3]=yOfs;
  vtkMatrix4x4::Multiply4x4(gridToRAS.GetPointer(),gridOffset.GetPointer(),gridToRAS.GetPointer());
  vtkNew<vtkMatrix4x4> gridScaling;
  gridScaling->Element[0][0]=pointSpacing;
  gridScaling->Element[1][1]=pointSpacing;
  gridScaling->Element[2][2]=pointSpacing;
  vtkMatrix4x4::Multiply4x4(gridToRAS.GetPointer(),gridScaling.GetPointer(),gridToRAS.GetPointer());

  if (numGridPoints)
    {
    numGridPoints[0]=gridSize[0];
    numGridPoints[1]=gridSize[1];
    numGridPoints[2]=1;
    }

  GetTransformedPointSamples(outputPointSet, inputTransformNode, gridToRAS.GetPointer(), gridSize);

  float sliceNormal_RAS[3] = {0,0,0};
  sliceNormal_RAS[0] = sliceToRAS->GetElement(0,2);
  sliceNormal_RAS[1] = sliceToRAS->GetElement(1,2);
  sliceNormal_RAS[2] = sliceToRAS->GetElement(2,2);

  // Project vectors to the slice plane
  float dot = 0;
  vtkDataArray* projectedVectors = outputPointSet->GetPointData()->GetVectors();
  double* chosenVector = NULL;
  int numOfTuples=projectedVectors->GetNumberOfTuples();
  for(int i = 0; i < numOfTuples; i++)
    {
    chosenVector = projectedVectors->GetTuple3(i);
    dot = chosenVector[0]*sliceNormal_RAS[0] + chosenVector[1]*sliceNormal_RAS[1] + chosenVector[2]*sliceNormal_RAS[2];
    projectedVectors->SetTuple3(i, chosenVector[0]-dot*sliceNormal_RAS[0], chosenVector[1]-dot*sliceNormal_RAS[1], chosenVector[2]-dot*sliceNormal_RAS[2]);
    }
  projectedVectors->SetName("ProjectedDisplacementVector");
  outputPointSet->GetPointData()->SetActiveAttribute(GetVisualizationDisplacementMagnitudeScalarName(), vtkDataSetAttributes::SCALARS);
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetTransformedPointSamplesAsMagnitudeImage(vtkImageData* magnitudeImage, vtkMRMLTransformNode* inputTransformNode, vtkMatrix4x4* ijkToRAS)
{
  if (!inputTransformNode)
    {
    return;
    }
  vtkNew<vtkGeneralTransform> inputTransform;
  inputTransformNode->GetTransformToWorld(inputTransform.GetPointer());

  int* extent=magnitudeImage->GetExtent();
  int imageSize[3]={extent[1]-extent[0]+1, extent[3]-extent[2]+1, extent[5]-extent[4]+1};

  // The orientation of the volume cannot be set in the image
  // therefore the volume will not appear in the correct position
  // if the direction matrix is not identity.
#if VTK_MAJOR_VERSION <= 5
  magnitudeImage->SetScalarTypeToFloat();
  magnitudeImage->SetNumberOfScalarComponents(1);
  magnitudeImage->AllocateScalars();
#else
  magnitudeImage->AllocateScalars(VTK_FLOAT, 1);
#endif

  double point_RAS[4] = {0,0,0,1};
  double transformedPoint_RAS[4] = {0,0,0,1};
  double pointDislocationVector_RAS[4] = {0,0,0,1};
  double point_IJK[4]={0,0,0,1};
  float* voxelPtr=static_cast<float*>(magnitudeImage->GetScalarPointer());
  for (point_IJK[2]=0; point_IJK[2]<imageSize[2]; point_IJK[2]++)
    {
    for (point_IJK[1]=0; point_IJK[1]<imageSize[1]; point_IJK[1]++)
      {
      for (point_IJK[0]=0; point_IJK[0]<imageSize[0]; point_IJK[0]++)
        {
        ijkToRAS->MultiplyPoint(point_IJK, point_RAS);

        inputTransform->TransformPoint(point_RAS, transformedPoint_RAS);

        pointDislocationVector_RAS[0] = transformedPoint_RAS[0] - point_RAS[0];
        pointDislocationVector_RAS[1] = transformedPoint_RAS[1] - point_RAS[1];
        pointDislocationVector_RAS[2] = transformedPoint_RAS[2] - point_RAS[2];

        float mag=sqrt(
          pointDislocationVector_RAS[0]*pointDislocationVector_RAS[0]+
          pointDislocationVector_RAS[1]*pointDislocationVector_RAS[1]+
          pointDislocationVector_RAS[2]*pointDislocationVector_RAS[2]);

        *(voxelPtr++)=mag;
        }
      }
    }
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerTransformLogic::CreateDisplacementVolumeFromTransform(vtkMRMLTransformNode* inputTransformNode, vtkMRMLVolumeNode* referenceVolumeNode, bool magnitude/*=true*/)
{
  if (inputTransformNode==NULL || referenceVolumeNode==NULL || referenceVolumeNode->GetImageData()==NULL)
    {
    vtkErrorMacro("vtkSlicerTransformLogic::CreateDisplacementMagnitudeVolumeFromTransform failed: inputs are invalid");
    return NULL;
    }
  vtkMRMLScene* scene=this->GetMRMLScene();
  if (scene==NULL)
    {
    vtkErrorMacro("vtkSlicerTransformLogic::CreateDisplacementMagnitudeVolumeFromTransform failed: scene invalid");
    return NULL;
    }

  // Fill the volume
  vtkNew<vtkImageData> outputVolume;
  outputVolume->SetExtent(referenceVolumeNode->GetImageData()->GetExtent());
  vtkNew<vtkMatrix4x4> ijkToRas;
  referenceVolumeNode->GetIJKToRASMatrix(ijkToRas.GetPointer());
  std::string nodeName=inputTransformNode->GetName();
  if (magnitude)
    {
    vtkSlicerTransformLogic::GetTransformedPointSamplesAsMagnitudeImage(outputVolume.GetPointer(), inputTransformNode, ijkToRas.GetPointer());
    nodeName+=" displacement magnitude";
    }
  else
    {
    vtkSlicerTransformLogic::GetTransformedPointSamplesAsVectorImage(outputVolume.GetPointer(), inputTransformNode, ijkToRas.GetPointer());
    nodeName+=" displacement vectors";
    }

  // Create a volume node
  vtkNew<vtkMRMLScalarVolumeNode> outputVolumeNode;
  nodeName=scene->GenerateUniqueName(nodeName);
  outputVolumeNode->SetName(nodeName.c_str());
  outputVolumeNode->SetIJKToRASMatrix(ijkToRas.GetPointer());
  outputVolumeNode->SetAndObserveImageData(outputVolume.GetPointer());
  scene->AddNode(outputVolumeNode.GetPointer());

  if (magnitude)
    {
    vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
    scene->AddNode(displayNode.GetPointer());
    displayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
    outputVolumeNode->SetAndObserveNthDisplayNodeID(0, displayNode->GetID());
    }
  else
    {
    vtkNew<vtkMRMLVectorVolumeDisplayNode> displayNode;
    scene->AddNode(displayNode.GetPointer());
    displayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
    outputVolumeNode->SetAndObserveNthDisplayNodeID(0, displayNode->GetID());
    }

  return outputVolumeNode.GetPointer();
}


//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetTransformedPointSamplesAsVectorImage(vtkImageData* vectorImage, vtkMRMLTransformNode* inputTransformNode, vtkMatrix4x4* ijkToRAS)
{
  if (!inputTransformNode)
    {
    return;
    }
  vtkNew<vtkGeneralTransform> inputTransform;
  inputTransformNode->GetTransformToWorld(inputTransform.GetPointer());

  int* extent=vectorImage->GetExtent();
  int imageSize[3]={extent[1]-extent[0]+1, extent[3]-extent[2]+1, extent[5]-extent[4]+1};

  // The orientation of the volume cannot be set in the image
  // therefore the volume will not appear in the correct position
  // if the direction matrix is not identity.
#if VTK_MAJOR_VERSION <= 5
  vectorImage->SetScalarTypeToFloat();
  vectorImage->SetNumberOfScalarComponents(3);
  vectorImage->AllocateScalars();
#else
  vectorImage->AllocateScalars(VTK_FLOAT, 3);
#endif

  double point_RAS[4] = {0,0,0,1};
  double transformedPoint_RAS[4] = {0,0,0,1};
  double point_IJK[4]={0,0,0,1};
  float* voxelPtr=static_cast<float*>(vectorImage->GetScalarPointer());
  for (point_IJK[2]=0; point_IJK[2]<imageSize[2]; point_IJK[2]++)
    {
    for (point_IJK[1]=0; point_IJK[1]<imageSize[1]; point_IJK[1]++)
      {
      for (point_IJK[0]=0; point_IJK[0]<imageSize[0]; point_IJK[0]++)
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
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetTransformedPointSamplesOnRoi(vtkPointSet* pointSet, vtkMRMLTransformNode* inputTransformNode, vtkMatrix4x4* roiToRAS, int* roiSize, double pointSpacingMm, int pointGroupSize/*=1*/, int* numGridPoints/*=0*/)
{
  double roiSpacing[3]=
    {
    sqrt(roiToRAS->Element[0][0]*roiToRAS->Element[0][0]+roiToRAS->Element[1][0]*roiToRAS->Element[1][0]+roiToRAS->Element[2][0]*roiToRAS->Element[2][0]),
    sqrt(roiToRAS->Element[0][1]*roiToRAS->Element[0][1]+roiToRAS->Element[1][1]*roiToRAS->Element[1][1]+roiToRAS->Element[2][1]*roiToRAS->Element[2][1]),
    sqrt(roiToRAS->Element[0][2]*roiToRAS->Element[0][2]+roiToRAS->Element[1][2]*roiToRAS->Element[1][2]+roiToRAS->Element[2][2]*roiToRAS->Element[2][2])
    };

  double roiSizeMm[3]={roiSize[0]*roiSpacing[0], roiSize[1]*roiSpacing[1], roiSize[2]*roiSpacing[2]};

  int numOfPointsX=floor(roiSizeMm[0]/(pointSpacingMm*pointGroupSize)+0.5)*pointGroupSize;
  int numOfPointsY=floor(roiSizeMm[1]/(pointSpacingMm*pointGroupSize)+0.5)*pointGroupSize;
  int numOfPointsZ=floor(roiSizeMm[2]/(pointSpacingMm*pointGroupSize)+0.5)*pointGroupSize;

  double xOfs = (roiSizeMm[0]-(numOfPointsX*pointSpacingMm))/2.0/roiSpacing[0];
  double yOfs = (roiSizeMm[1]-(numOfPointsY*pointSpacingMm))/2.0/roiSpacing[1];
  double zOfs = (roiSizeMm[2]-(numOfPointsZ*pointSpacingMm))/2.0/roiSpacing[2];

  int gridSize[3]={numOfPointsX+1,numOfPointsY+1,numOfPointsZ+1};

  vtkNew<vtkMatrix4x4> gridToRAS;
  gridToRAS->DeepCopy(roiToRAS);
  vtkNew<vtkMatrix4x4> gridOffset;
  gridOffset->Element[0][3]=xOfs;
  gridOffset->Element[1][3]=yOfs;
  gridOffset->Element[2][3]=zOfs;
  vtkMatrix4x4::Multiply4x4(gridToRAS.GetPointer(),gridOffset.GetPointer(),gridToRAS.GetPointer());
  vtkNew<vtkMatrix4x4> gridScaling;
  gridScaling->Element[0][0]=pointSpacingMm/roiSpacing[0];
  gridScaling->Element[1][1]=pointSpacingMm/roiSpacing[1];
  gridScaling->Element[2][2]=pointSpacingMm/roiSpacing[2];
  vtkMatrix4x4::Multiply4x4(gridToRAS.GetPointer(),gridScaling.GetPointer(),gridToRAS.GetPointer());

  // We need rounding (floor(...+0.5)) because otherwise due to minor numerical differences
  // we could have one more or one less grid size when the roiSpacing does not match exactly the
  // glyph spacing.

  GetTransformedPointSamples(pointSet, inputTransformNode, gridToRAS.GetPointer(), gridSize);

  if (numGridPoints!=NULL)
    {
    numGridPoints[0]=gridSize[0];
    numGridPoints[1]=gridSize[1];
    numGridPoints[2]=gridSize[2];
    }
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetGlyphVisualization3d(vtkPolyData* output, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS, int* roiSize)
{
  vtkNew<vtkUnstructuredGrid> pointSet;
  vtkMRMLTransformNode* inputTransformNode=vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  GetTransformedPointSamplesOnRoi(pointSet.GetPointer(), inputTransformNode, roiToRAS, roiSize, displayNode->GetGlyphSpacingMm());
  vtkNew<vtkTransformVisualizerGlyph3D> glyphFilter;
  glyphFilter->SetScaleFactor(displayNode->GetGlyphScalePercent()*0.01);
  glyphFilter->SetColorModeToColorByScalar();
  glyphFilter->OrientOn();
#if VTK_MAJOR_VERSION <= 5
  glyphFilter->SetInput(pointSet.GetPointer());
#else
  glyphFilter->SetInputData(pointSet.GetPointer());
#endif

  glyphFilter->SetColorArray(GetVisualizationDisplacementMagnitudeScalarName());

  glyphFilter->SetMagnitudeThresholdLower(displayNode->GetGlyphDisplayRangeMinMm());
  glyphFilter->SetMagnitudeThresholdUpper(displayNode->GetGlyphDisplayRangeMaxMm());
  glyphFilter->SetMagnitudeThresholding(true);

  switch (displayNode->GetGlyphType())
    {
    //Arrows
    case vtkMRMLTransformDisplayNode::GLYPH_TYPE_ARROW:
      {
      vtkNew<vtkArrowSource> arrowSource;
      arrowSource->SetTipLength(displayNode->GetGlyphTipLengthPercent()*0.01);
      arrowSource->SetTipRadius(displayNode->GetGlyphDiameterMm()*0.5);
      arrowSource->SetTipResolution(displayNode->GetGlyphResolution());
      arrowSource->SetShaftRadius(arrowSource->GetTipRadius()*0.01*displayNode->GetGlyphShaftDiameterPercent());
      arrowSource->SetShaftResolution(displayNode->GetGlyphResolution());
      glyphFilter->SetScaleDirectional(true);
      glyphFilter->SetScaleModeToScaleByVector();
      glyphFilter->SetSourceConnection(arrowSource->GetOutputPort());
      break;
      }
    //Cones
    case vtkMRMLTransformDisplayNode::GLYPH_TYPE_CONE:
      {
      vtkNew<vtkConeSource> coneSource;
      coneSource->SetHeight(1.0);
      coneSource->SetRadius(displayNode->GetGlyphDiameterMm()*0.5);
      coneSource->SetResolution(displayNode->GetGlyphResolution());
      glyphFilter->SetScaleDirectional(true);
      glyphFilter->SetScaleModeToScaleByVector();
      glyphFilter->SetSourceConnection(coneSource->GetOutputPort());
      break;
     }
    //Spheres
    case vtkMRMLTransformDisplayNode::GLYPH_TYPE_SPHERE:
      {
      vtkNew<vtkSphereSource> sphereSource;
      sphereSource->SetRadius(0.5);
      sphereSource->SetThetaResolution(displayNode->GetGlyphResolution());
      sphereSource->SetPhiResolution(displayNode->GetGlyphResolution());
      glyphFilter->SetScaleDirectional(false);
      glyphFilter->SetScaleModeToScaleByScalar();
      glyphFilter->SetSourceConnection(sphereSource->GetOutputPort());
      break;
      }
    }

  glyphFilter->Update();
  output->ShallowCopy(glyphFilter->GetOutput());
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetGlyphVisualization2d(vtkPolyData* output, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize)
{
  //Pre-processing
  vtkNew<vtkUnstructuredGrid> pointSet;
  pointSet->Initialize();

  vtkMRMLTransformNode* inputTransformNode=vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  GetTransformedPointSamplesOnSlice(pointSet.GetPointer(), inputTransformNode, sliceToRAS, fieldOfViewOrigin, fieldOfViewSize, displayNode->GetGlyphSpacingMm());

  vtkNew<vtkTransformVisualizerGlyph3D> glyphFilter;
  vtkNew<vtkTransform> rotateArrow;
  vtkNew<vtkGlyphSource2D> glyph2DSource;
  switch (displayNode->GetGlyphType())
    {
    case vtkMRMLTransformDisplayNode::GLYPH_TYPE_ARROW:
      glyph2DSource->SetGlyphTypeToArrow();
      glyphFilter->SetScaleModeToScaleByVector();
      // move the origin from the middle of the arrow to the base of the arrow
      rotateArrow->Translate(0.5,0,0);
      break;
    case vtkMRMLTransformDisplayNode::GLYPH_TYPE_CONE:
      glyph2DSource->SetGlyphTypeToEdgeArrow();
      glyphFilter->SetScaleModeToScaleByVector();
      // move the origin from the base of the cone to the middle of the cone
      rotateArrow->Translate(0.5,0,0);
      break;
    case vtkMRMLTransformDisplayNode::GLYPH_TYPE_SPHERE:
      glyph2DSource->SetGlyphTypeToCircle();
      glyphFilter->SetScaleModeToScaleByScalar();
      break;
    default: glyph2DSource->SetGlyphTypeToNone();
    }

  float sliceNormal_RAS[3] = {0,0,0};
  sliceNormal_RAS[0] = sliceToRAS->GetElement(0,2);
  sliceNormal_RAS[1] = sliceToRAS->GetElement(1,2);
  sliceNormal_RAS[2] = sliceToRAS->GetElement(2,2);

  bool useNewMethod=false;
  if (useNewMethod)
    {
    // the arrow tips are oriented correctly, but it seems that the direction of the shaft is not always correct
    vtkNew<vtkMatrix4x4> glyphOrientation;
    glyphOrientation->DeepCopy(sliceToRAS);
    glyphOrientation->Element[0][3]=0;
    glyphOrientation->Element[1][3]=0;
    glyphOrientation->Element[2][3]=0;
    rotateArrow->SetMatrix(glyphOrientation.GetPointer());
    rotateArrow->Inverse();
    }
  else
    {
    // the arrow tips are note always oriented correctly, but the direction of the shaft looks correct
    rotateArrow->RotateX(vtkMath::DegreesFromRadians(acos(abs(sliceNormal_RAS[2])))); // TODO: check this, it might not be correct for an arbitrarily oriented slice normal
    }

  glyph2DSource->SetScale(1);
  glyph2DSource->SetFilled(0);

  glyphFilter->SetScaleFactor(displayNode->GetGlyphScalePercent()*0.01);
  glyphFilter->SetScaleDirectional(false);
  glyphFilter->SetColorModeToColorByScalar();
  glyphFilter->SetSourceTransform(rotateArrow.GetPointer());
  glyphFilter->SetSourceConnection(glyph2DSource->GetOutputPort());
#if VTK_MAJOR_VERSION <= 5
  glyphFilter->SetInput(pointSet.GetPointer());
#else
  glyphFilter->SetInputData(pointSet.GetPointer());
#endif

  glyphFilter->SetColorArray(GetVisualizationDisplacementMagnitudeScalarName());

  glyphFilter->SetMagnitudeThresholdLower(displayNode->GetGlyphDisplayRangeMinMm());
  glyphFilter->SetMagnitudeThresholdUpper(displayNode->GetGlyphDisplayRangeMaxMm());
  glyphFilter->SetMagnitudeThresholding(true);

  glyphFilter->Update();

  output->ShallowCopy(glyphFilter->GetOutput());
}

//----------------------------------------------------------------------------
int vtkSlicerTransformLogic::GetGridSubdivision(vtkMRMLTransformDisplayNode* displayNode)
{
  int subdivision=floor(displayNode->GetGridSpacingMm()/displayNode->GetGridResolutionMm()+0.5);
  if (subdivision<1)
    {
    // avoid infinite loops and division by zero errors
    subdivision=1;
   }
  return subdivision;
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::CreateGrid(vtkPolyData* gridPolyData, vtkMRMLTransformDisplayNode* displayNode, int numGridPoints[3], vtkPolyData* warpedGrid/*=NULL*/)
{
  vtkNew<vtkCellArray> grid;
  vtkNew<vtkLine> line;

  int gridSubdivision=GetGridSubdivision(displayNode);

  // Create lines along i
  for (int k = 0; k < numGridPoints[2]; k+=gridSubdivision)
    {
    for (int j = 0; j < numGridPoints[1]; j+=gridSubdivision)
     {
      for (int i = 0; i < numGridPoints[0]-1; i++)
        {
        line->GetPointIds()->SetId(0, (i) + (j*numGridPoints[0]) + (k*numGridPoints[0]*numGridPoints[1]));
        line->GetPointIds()->SetId(1, (i+1) + (j*numGridPoints[0]) + (k*numGridPoints[0]*numGridPoints[1]));
        grid->InsertNextCell(line.GetPointer());
        }
      }
    }

  // Create lines along j
  for (int k = 0; k < numGridPoints[2]; k+=gridSubdivision)
    {
    for (int j = 0; j < numGridPoints[1]-1; j++)
      {
      for (int i = 0; i < numGridPoints[0]; i+=gridSubdivision)
       {
        line->GetPointIds()->SetId(0, (i) + ((j)*numGridPoints[0]) + (k*numGridPoints[0]*numGridPoints[1]));
        line->GetPointIds()->SetId(1, (i) + ((j+1)*numGridPoints[0]) + (k*numGridPoints[0]*numGridPoints[1]));
        grid->InsertNextCell(line.GetPointer());
        }
      }
    }

  // Create lines along k
  for (int k = 0; k < numGridPoints[2]-1; k++)
   {
    for (int j = 0; j < numGridPoints[1]; j+=gridSubdivision)
     {
      for (int i = 0; i < numGridPoints[0]; i+=gridSubdivision)
       {
        line->GetPointIds()->SetId(0, (i) + ((j)*numGridPoints[0]) + ((k)*numGridPoints[0]*numGridPoints[1]));
        line->GetPointIds()->SetId(1, (i) + ((j)*numGridPoints[0]) + ((k+1)*numGridPoints[0]*numGridPoints[1]));
        grid->InsertNextCell(line.GetPointer());
        }
      }
    }

  gridPolyData->SetLines(grid.GetPointer());

  if (warpedGrid)
    {
    vtkNew<vtkWarpVector> warp;
#if VTK_MAJOR_VERSION <= 5
    warp->SetInput(gridPolyData);
#else
    warp->SetInputData(gridPolyData);
#endif
    warp->SetScaleFactor(displayNode->GetGridScalePercent()*0.01);
    warp->Update();
    vtkPolyData* polyoutput = warp->GetPolyDataOutput();

    int idx=polyoutput->GetPointData()->AddArray(gridPolyData->GetPointData()->GetArray(GetVisualizationDisplacementMagnitudeScalarName()));
    polyoutput->GetPointData()->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
    warpedGrid->ShallowCopy(polyoutput);
    //warpedGrid->Update();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetGridVisualization2d(vtkPolyData* output, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize)
{
  double pointSpacing=displayNode->GetGridSpacingMm()/GetGridSubdivision(displayNode);
  int numGridPoints[3]={0};

  vtkNew<vtkPolyData> gridPolyData;
  vtkMRMLTransformNode* transformNode=vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  GetTransformedPointSamplesOnSlice(gridPolyData.GetPointer(), transformNode, sliceToRAS, fieldOfViewOrigin, fieldOfViewSize, pointSpacing, GetGridSubdivision(displayNode), numGridPoints);

  if (displayNode->GetGridShowNonWarped())
    {
    // Show both the original (non-warped) and the warped grid

    // Create the grids
    vtkNew<vtkPolyData> warpedGridPolyData;
    CreateGrid(gridPolyData.GetPointer(), displayNode, numGridPoints, warpedGridPolyData.GetPointer());

    // Set the displacement magnitude DataArray in the non-warped grid data to zero.
    // Create a new DataArray, because the same array is used by both the warped
    // and the non-warped grid.
    vtkDataArray* displacementMagnitudeScalars=gridPolyData->GetPointData()->GetArray(GetVisualizationDisplacementMagnitudeScalarName());
    vtkSmartPointer<vtkDataArray> zeroDisplacementMagnitudeScalars=vtkSmartPointer<vtkDataArray>::Take(displacementMagnitudeScalars->NewInstance());
    zeroDisplacementMagnitudeScalars->SetName(GetVisualizationDisplacementMagnitudeScalarName());
    zeroDisplacementMagnitudeScalars->SetNumberOfTuples(displacementMagnitudeScalars->GetNumberOfTuples());
    zeroDisplacementMagnitudeScalars->FillComponent(0,0.0);

    // Replace the DataArray in the non-warped grid
    gridPolyData->GetPointData()->RemoveArray(GetVisualizationDisplacementMagnitudeScalarName());
    int idx=gridPolyData->GetPointData()->AddArray(zeroDisplacementMagnitudeScalars);
    gridPolyData->GetPointData()->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);

    // Create the output by combining the warped and non-warped grid
    vtkNew<vtkAppendPolyData> appender;
#if VTK_MAJOR_VERSION <= 5
    appender->AddInput(gridPolyData.GetPointer());
    appender->AddInput(warpedGridPolyData.GetPointer());
#else
    appender->AddInputData(gridPolyData.GetPointer());
    appender->AddInputData(warpedGridPolyData.GetPointer());
#endif
    appender->Update();
    output->ShallowCopy(appender->GetOutput());
    output->GetPointData()->SetActiveAttribute(GetVisualizationDisplacementMagnitudeScalarName(), vtkDataSetAttributes::SCALARS);
    }
  else
    {
    // The output is the warped grid
    CreateGrid(gridPolyData.GetPointer(), displayNode, numGridPoints, output);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetGridVisualization3d(vtkPolyData* output, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS, int* roiSize)
{
  double pointSpacing=displayNode->GetGridSpacingMm()/GetGridSubdivision(displayNode);
  int numGridPoints[3]={0};

  vtkNew<vtkPolyData> gridPolyData;
  vtkMRMLTransformNode* inputTransformNode=vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  GetTransformedPointSamplesOnRoi(gridPolyData.GetPointer(), inputTransformNode, roiToRAS, roiSize, pointSpacing, GetGridSubdivision(displayNode), numGridPoints);

  vtkNew<vtkPolyData> warpedGridPolyData;
  CreateGrid(gridPolyData.GetPointer(), displayNode, numGridPoints, warpedGridPolyData.GetPointer());

  vtkNew<vtkTubeFilter> tubeFilter;
#if VTK_MAJOR_VERSION <= 5
  tubeFilter->SetInput(warpedGridPolyData.GetPointer());
#else
  tubeFilter->SetInputData(warpedGridPolyData.GetPointer());
#endif
  tubeFilter->SetRadius(displayNode->GetGridLineDiameterMm()*0.5);
  tubeFilter->SetNumberOfSides(8);
  tubeFilter->Update();
  output->ShallowCopy(tubeFilter->GetOutput());
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetContourVisualization2d(vtkPolyData* output, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize)
{
  vtkNew<vtkImageData> magnitudeImage;
  double pointSpacing=displayNode->GetContourResolutionMm();

  vtkNew<vtkMatrix4x4> ijkToRAS;

  int numOfPointsX=ceil(fieldOfViewSize[0]/pointSpacing);
  int numOfPointsY=ceil(fieldOfViewSize[1]/pointSpacing);
  double xOfs = -fieldOfViewSize[0]/2+fieldOfViewOrigin[0];
  double yOfs = -fieldOfViewSize[1]/2+fieldOfViewOrigin[1];

  ijkToRAS->DeepCopy(sliceToRAS);
  vtkNew<vtkMatrix4x4> ijkOffset;
  ijkOffset->Element[0][3]=xOfs;
  ijkOffset->Element[1][3]=yOfs;
  vtkMatrix4x4::Multiply4x4(ijkToRAS.GetPointer(),ijkOffset.GetPointer(),ijkToRAS.GetPointer());
  vtkNew<vtkMatrix4x4> voxelSpacing;
  voxelSpacing->Element[0][0]=pointSpacing;
  voxelSpacing->Element[1][1]=pointSpacing;
  voxelSpacing->Element[2][2]=pointSpacing;
  vtkMatrix4x4::Multiply4x4(ijkToRAS.GetPointer(),voxelSpacing.GetPointer(),ijkToRAS.GetPointer());

  int imageSize[3]={numOfPointsX, numOfPointsY,1};

  vtkMRMLTransformNode* inputTransformNode=vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  magnitudeImage->SetExtent(0,imageSize[0]-1,0,imageSize[1]-1,0,imageSize[2]-1);
  GetTransformedPointSamplesAsMagnitudeImage(magnitudeImage.GetPointer(), inputTransformNode, ijkToRAS.GetPointer());

  vtkNew<vtkContourFilter> contourFilter;
  double* levels=displayNode->GetContourLevelsMm();
  for (unsigned int i=0; i<displayNode->GetNumberOfContourLevels(); i++)
   {
    contourFilter->SetValue(i, levels[i]);
   }
#if VTK_MAJOR_VERSION <= 5
  contourFilter->SetInput(magnitudeImage.GetPointer());
#else
  contourFilter->SetInputData(magnitudeImage.GetPointer());
#endif
  contourFilter->Update();

  vtkNew<vtkTransformPolyDataFilter> transformSliceToRas;
  vtkNew<vtkTransform> sliceToRasTransform;
  sliceToRasTransform->SetMatrix(ijkToRAS.GetPointer());
  transformSliceToRas->SetTransform(sliceToRasTransform.GetPointer());
  transformSliceToRas->SetInputConnection(contourFilter->GetOutputPort());
  transformSliceToRas->Update();
  output->ShallowCopy(transformSliceToRas->GetOutput());
}

//----------------------------------------------------------------------------
void vtkSlicerTransformLogic::GetContourVisualization3d(vtkPolyData* output, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS, int* roiSize)
{
  // Compute the sampling image grid position, orientation, and spacing
  double pointSpacingMm=displayNode->GetContourResolutionMm();
  double roiSpacingMm[3]=
    {
    sqrt(roiToRAS->Element[0][0]*roiToRAS->Element[0][0]+roiToRAS->Element[1][0]*roiToRAS->Element[1][0]+roiToRAS->Element[2][0]*roiToRAS->Element[2][0]),
    sqrt(roiToRAS->Element[0][1]*roiToRAS->Element[0][1]+roiToRAS->Element[1][1]*roiToRAS->Element[1][1]+roiToRAS->Element[2][1]*roiToRAS->Element[2][1]),
    sqrt(roiToRAS->Element[0][2]*roiToRAS->Element[0][2]+roiToRAS->Element[1][2]*roiToRAS->Element[1][2]+roiToRAS->Element[2][2]*roiToRAS->Element[2][2])
    };
  double pointSpacingX_ROI=pointSpacingMm/roiSpacingMm[0];
  double pointSpacingY_ROI=pointSpacingMm/roiSpacingMm[1];
  double pointSpacingZ_ROI=pointSpacingMm/roiSpacingMm[2];
  int numOfPointsX=ceil(double(roiSize[0])/pointSpacingX_ROI);
  int numOfPointsY=ceil(double(roiSize[1])/pointSpacingY_ROI);
  int numOfPointsZ=ceil(double(roiSize[2])/pointSpacingZ_ROI);
  if (numOfPointsZ==0)
    {
    numOfPointsZ=1;
    }

  double xOfs = 0;
  double yOfs = 0;
  double zOfs = 0;
  vtkNew<vtkMatrix4x4> ijkToRAS;
  ijkToRAS->DeepCopy(roiToRAS);
  vtkNew<vtkMatrix4x4> ijkOffset;
  ijkOffset->Element[0][3]=xOfs;
  ijkOffset->Element[1][3]=yOfs;
  ijkOffset->Element[2][3]=zOfs;
  vtkMatrix4x4::Multiply4x4(ijkToRAS.GetPointer(),ijkOffset.GetPointer(),ijkToRAS.GetPointer());
  vtkNew<vtkMatrix4x4> voxelSpacing;
  voxelSpacing->Element[0][0]=pointSpacingX_ROI;
  voxelSpacing->Element[1][1]=pointSpacingY_ROI;
  voxelSpacing->Element[2][2]=pointSpacingZ_ROI;
  vtkMatrix4x4::Multiply4x4(ijkToRAS.GetPointer(),voxelSpacing.GetPointer(),ijkToRAS.GetPointer());

  // Sample on an image
  vtkNew<vtkImageData> magnitudeImage;
  int imageSize[3]={numOfPointsX, numOfPointsY, numOfPointsZ};
  vtkMRMLTransformNode* transformNode=vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  magnitudeImage->SetExtent(0,imageSize[0]-1,0,imageSize[1]-1,0,imageSize[2]-1);
  GetTransformedPointSamplesAsMagnitudeImage(magnitudeImage.GetPointer(), transformNode, ijkToRAS.GetPointer());

  // Contput contours
  vtkNew<vtkContourFilter> contourFilter;
  double* levels=displayNode->GetContourLevelsMm();
  for (unsigned int i=0; i<displayNode->GetNumberOfContourLevels(); i++)
    {
    contourFilter->SetValue(i, levels[i]);
    }
#if VTK_MAJOR_VERSION <= 5
  contourFilter->SetInput(magnitudeImage.GetPointer());
#else
  contourFilter->SetInputData(magnitudeImage.GetPointer());
#endif
  contourFilter->Update();

  //  Transform contours to RAS
  vtkNew<vtkTransformPolyDataFilter> transformSliceToRas;
  vtkNew<vtkTransform> sliceToRasTransform;
  sliceToRasTransform->SetMatrix(ijkToRAS.GetPointer());
  transformSliceToRas->SetTransform(sliceToRasTransform.GetPointer());
  transformSliceToRas->SetInputConnection(contourFilter->GetOutputPort());
  transformSliceToRas->Update();
  output->ShallowCopy(transformSliceToRas->GetOutput());
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::GetVisualization2d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMRMLSliceNode* sliceNode)
{
  if (displayNode==NULL || output_RAS==NULL || sliceNode==NULL)
    {
    return false;
    }

  vtkMatrix4x4* sliceToRAS=sliceNode->GetSliceToRAS();
  double* fieldOfViewOrigin=sliceNode->GetXYZOrigin();
  double* fieldOfViewSize=sliceNode->GetFieldOfView();
  GetVisualization2d(output_RAS, displayNode, sliceToRAS, fieldOfViewOrigin, fieldOfViewSize);

  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::GetVisualization2d(vtkPolyData* output, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize)
{
  if (displayNode==NULL || output==NULL || sliceToRAS==NULL || fieldOfViewOrigin==NULL || fieldOfViewSize==NULL)
    {
    return false;
    }

  // Use the color exactly as defined in the colormap
  displayNode->AutoScalarRangeOff();
  if (displayNode->GetColorNode() && displayNode->GetColorNode()->GetLookupTable())
    {
    double* range = displayNode->GetColorNode()->GetLookupTable()->GetRange();
    displayNode->SetScalarRange(range[0], range[1]);
    }

  switch (displayNode->GetVisualizationMode())
    {
  case vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH:
    GetGlyphVisualization2d(output, displayNode, sliceToRAS, fieldOfViewOrigin, fieldOfViewSize);
    break;
  case vtkMRMLTransformDisplayNode::VIS_MODE_GRID:
    GetGridVisualization2d(output, displayNode, sliceToRAS, fieldOfViewOrigin, fieldOfViewSize);
    break;
  case vtkMRMLTransformDisplayNode::VIS_MODE_CONTOUR:
    GetContourVisualization2d(output, displayNode, sliceToRAS, fieldOfViewOrigin, fieldOfViewSize);
    break;
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::GetVisualization3d(vtkPolyData* output, vtkMRMLTransformDisplayNode* displayNode, vtkMRMLNode* regionNode)
{
  if (displayNode==NULL || output==NULL || regionNode==NULL)
    {
    return false;
    }

  vtkNew<vtkMatrix4x4> ijkToRAS;
  int regionSize_IJK[3]={0};
  vtkMRMLSliceNode* sliceNode=vtkMRMLSliceNode::SafeDownCast(regionNode);
  vtkMRMLDisplayableNode* displayableNode=vtkMRMLDisplayableNode::SafeDownCast(regionNode);
  if (sliceNode!=NULL)
   {
    double pointSpacing=displayNode->GetGlyphSpacingMm();

    vtkMatrix4x4* sliceToRAS=sliceNode->GetSliceToRAS();
    double* fieldOfViewSize=sliceNode->GetFieldOfView();
    double* fieldOfViewOrigin=sliceNode->GetXYZOrigin();

    int numOfPointsX=floor(fieldOfViewSize[0]/pointSpacing+0.5);
    int numOfPointsY=floor(fieldOfViewSize[1]/pointSpacing+0.5);
    double xOfs = -fieldOfViewSize[0]/2+fieldOfViewOrigin[0];
    double yOfs = -fieldOfViewSize[1]/2+fieldOfViewOrigin[1];

    ijkToRAS->DeepCopy(sliceToRAS);
    vtkNew<vtkMatrix4x4> ijkOffset;
    ijkOffset->Element[0][3]=xOfs;
    ijkOffset->Element[1][3]=yOfs;
    vtkMatrix4x4::Multiply4x4(ijkToRAS.GetPointer(),ijkOffset.GetPointer(),ijkToRAS.GetPointer());
    vtkNew<vtkMatrix4x4> voxelSpacing;
    voxelSpacing->Element[0][0]=pointSpacing;
    voxelSpacing->Element[1][1]=pointSpacing;
    voxelSpacing->Element[2][2]=pointSpacing;
    vtkMatrix4x4::Multiply4x4(ijkToRAS.GetPointer(),voxelSpacing.GetPointer(),ijkToRAS.GetPointer());

    regionSize_IJK[0]=numOfPointsX;
    regionSize_IJK[1]=numOfPointsY;
    regionSize_IJK[2]=0;
    }
  else if (displayableNode!=NULL)
    {
    double bounds_RAS[6]={0};
    displayableNode->GetRASBounds(bounds_RAS);
    ijkToRAS->SetElement(0,3,bounds_RAS[0]);
    ijkToRAS->SetElement(1,3,bounds_RAS[2]);
    ijkToRAS->SetElement(2,3,bounds_RAS[4]);
    regionSize_IJK[0]=floor(bounds_RAS[1]-bounds_RAS[0]);
    regionSize_IJK[1]=floor(bounds_RAS[3]-bounds_RAS[2]);
    regionSize_IJK[2]=floor(bounds_RAS[5]-bounds_RAS[4]);
    }
  else
   {
    vtkWarningWithObjectMacro(displayNode, "Failed to get transform visualization in 3D: unsupported ROI type");
    return false;
    }

  vtkSlicerTransformLogic::GetVisualization3d(output, displayNode, ijkToRAS.GetPointer(), regionSize_IJK);

  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::GetVisualization3d(vtkPolyData* output, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS, int* roiSize)
{
  if (displayNode==NULL || output==NULL || roiToRAS==NULL || roiSize==NULL)
    {
    return false;
    }

  int oldModify=displayNode->StartModify();

  // Use the color exactly as defined in the colormap
  displayNode->AutoScalarRangeOff();
  if (displayNode->GetColorNode() && displayNode->GetColorNode()->GetLookupTable())
    {
    double* range = displayNode->GetColorNode()->GetLookupTable()->GetRange();
    displayNode->SetScalarRange(range[0], range[1]);
    }

  displayNode->SetScalarVisibility(1);
  switch (displayNode->GetVisualizationMode())
    {
  case vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH:
    displayNode->SetBackfaceCulling(1);
    displayNode->SetOpacity(1);
    GetGlyphVisualization3d(output, displayNode, roiToRAS, roiSize);
    break;
  case vtkMRMLTransformDisplayNode::VIS_MODE_GRID:
    displayNode->SetBackfaceCulling(1);
    displayNode->SetOpacity(1);
    GetGridVisualization3d(output, displayNode, roiToRAS, roiSize);
    break;
  case vtkMRMLTransformDisplayNode::VIS_MODE_CONTOUR:
    displayNode->SetBackfaceCulling(0);
    displayNode->SetOpacity(displayNode->GetContourOpacity());
    GetContourVisualization3d(output, displayNode, roiToRAS, roiSize);
    break;
    }

  displayNode->EndModify(oldModify);

  return true;
}

//----------------------------------------------------------------------------
vtkSlicerTransformLogic::TransformKind vtkSlicerTransformLogic::GetTransformKind(vtkMRMLTransformNode *transformNode)
{
  if (transformNode==NULL)
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

  // Check if it is a simple transform embedded into a general transform
  vtkGeneralTransform* generalTransform = vtkGeneralTransform::SafeDownCast(inputTransform);
  if (generalTransform==NULL)
    {
    // it's not a general transform, so we cannot decompose it
    return TRANSFORM_OTHER;
    }
  vtkNew<vtkCollection> transformList;
  vtkMRMLTransformNode::FlattenGeneralTransform(transformList.GetPointer(), generalTransform);
  if (transformList->GetNumberOfItems()==1)
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
    }
  else if (transformList->GetNumberOfItems()==2)
    {
    if (vtkTransform::SafeDownCast(transformList->GetItemAsObject(1))
      && vtkOrientedBSplineTransform::SafeDownCast(transformList->GetItemAsObject(0)))
      {
      // B-spline and bulk transform embedded in a general transform
      // BRAINS returns bspline+bulk transform this way, so we consider it as a bspline transform
      return TRANSFORM_BSPLINE;
      }
    }
  return TRANSFORM_OTHER;
}
