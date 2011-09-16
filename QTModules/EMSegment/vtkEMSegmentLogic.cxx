#include <algorithm>
#include <sstream>

#include "vtkEMSegmentLogic.h"
#include "vtkObjectFactory.h"
#include "vtkDirectory.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkGridTransform.h"
#include "vtkImageEMLocalSegmenter.h"
#include "vtkImageEMLocalSuperClass.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkTransformToGrid.h"
#include "vtkIdentityTransform.h"
#include "vtkMRMLEMSAtlasNode.h"
#include "vtkMRMLEMSGlobalParametersNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLEMSTemplateNode.h"
#include "vtkImageIslandFilter.h"
#include "vtkDataIOManagerLogic.h"
#include "vtkMath.h"
#include "vtkImageLevelSets.h"
#include "vtkImageMultiLevelSets.h"
#include "vtkImageLogOdds.h"
#include "vtkMultiThreader.h"
#include "vtkImageThreshold.h"
#include "vtkImageMathematics.h"
#include "vtkImageAppend.h"
#include "vtkImageClip.h"
#include "vtkImageTranslateExtent.h"
#include "vtkITKImageWriter.h" 
#include "vtkImageEllipsoidSource.h"
#include <vtksys/SystemTools.hxx>

#ifdef Slicer3_USE_KWWIDGETS
#include <vtkMRMLAtlasCreatorNode.h>
#endif

#ifdef _WIN32
//for _mktemp
#include <io.h>
#endif

// A helper class to compare two maps
template<class T>
class MapCompare
{
public:
  static bool map_value_comparer(typename std::map<T, unsigned int>::value_type &i1, typename std::map<
      T, unsigned int>::value_type &i2)
  {
    return i1.second < i2.second;
  }
};

//----------------------------------------------------------------------------
vtkEMSegmentLogic* vtkEMSegmentLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkEMSegmentLogic");
  if (ret)
    {
    return (vtkEMSegmentLogic*) ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkEMSegmentLogic;
}

//----------------------------------------------------------------------------
vtkEMSegmentLogic::vtkEMSegmentLogic()
{
  this->ModuleName = NULL;
  this->CurrentTmpFileName = NULL;

  this->ProgressCurrentAction = NULL;
  this->ProgressGlobalFractionCompleted = 0.0;
  this->ProgressCurrentFractionCompleted = 0.0;

  //this->DebugOn();

  this->MRMLManager = NULL; // NB: must be set before SetMRMLManager is called
  vtkEMSegmentMRMLManager* manager = vtkEMSegmentMRMLManager::New();
  this->SetMRMLManager(manager);
  manager->Delete();

  this->SlicerCommonInterface = NULL;

}

//----------------------------------------------------------------------------
vtkEMSegmentLogic::~vtkEMSegmentLogic()
{
  this->SetMRMLManager(NULL);
  this->SetProgressCurrentAction(NULL);
  this->SetModuleName(NULL);

  if (this->SlicerCommonInterface)
    {
    this->SlicerCommonInterface->Delete();
    this->SlicerCommonInterface = NULL;
    }

}

//----------------------------------------------------------------------------
vtkSlicerCommonInterface* vtkEMSegmentLogic::GetSlicerCommonInterface()
{

  if (!this->SlicerCommonInterface)
    {
    this->SlicerCommonInterface = vtkSlicerCommonInterface::New();
    }

  return this->SlicerCommonInterface;

}

//----------------------------------------------------------------------------
// This function is part of the EMSegmenter's stable API
vtkMRMLScalarVolumeNode*
vtkEMSegmentLogic::AddArchetypeScalarVolume(const char* filename, const char* volname, vtkSlicerApplicationLogic* appLogic, vtkMRMLScene* mrmlScene)
{
  return this->AddArchetypeScalarVolume(filename, volname, appLogic, mrmlScene,
      false);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkEMSegmentLogic::AddArchetypeScalarVolume(const char* filename, const char* volname, vtkSlicerApplicationLogic* appLogic, vtkMRMLScene* mrmlScene, bool centered)
{
  vtkSlicerVolumesLogic* volLogic = vtkSlicerVolumesLogic::New();
  volLogic->SetMRMLScene(mrmlScene);
  volLogic->SetApplicationLogic(appLogic);
  vtkMRMLScalarVolumeNode* volNode = NULL;
  if (centered)
    {
    volNode = volLogic->AddArchetypeScalarVolume(filename, volname, 2);
    }
  else
    {
    volNode = volLogic->AddArchetypeScalarVolume(filename, volname, 0);
    }
  volLogic->Delete();
  return volNode;
}

char*
vtkEMSegmentLogic::mktemp_file(const char* postfix)
{
  char *ptr = NULL;
  char filename[256];
  FILE *fp, *fp2;

  std::ostringstream mytemplate;
  mytemplate << this->GetTemporaryDirectory() << "/fn" << rand() << "XXXXXX";
  std::ostringstream s;

#if _WIN32
  // _mktemp alone is unusable because of it's limitation to 26 files
  strcpy_s( filename, sizeof(filename), mytemplate.str().c_str() );
  ptr = _mktemp( filename );
  if ( fopen_s( &fp, ptr, "w" ) != 0 )
  std::cout << "Cannot create file " << ptr << std::endl;
  s << ptr << postfix;
  if ( fopen_s( &fp2, s.str().c_str(), "w" ) != 0 )
  std::cout << "Cannot create file " << ptr << std::endl;
#else
  strcpy(filename, mytemplate.str().c_str());
  ptr = mktemp(filename);
  if ((fp = fopen(ptr, "w")) == NULL)
    std::cout << "Cannot create file " << ptr << std::endl;
  s << ptr << postfix;
  if ((fp2 = fopen(s.str().c_str(), "w")) == NULL)
    std::cout << "Cannot create file " << ptr << std::endl;
#endif
  fclose(fp);
  fclose(fp2);

  this->SetCurrentTmpFileName(s.str().c_str());
  return this->GetCurrentTmpFileName();
}

char*
vtkEMSegmentLogic::mktemp_dir()
{
  char *ptr;
  char filename[256];

  std::ostringstream mytemplate;
  mytemplate << this->GetTemporaryDirectory() << "/dn" << rand() << "XXXXXX";

#if _WIN32
  //todo, on windows _mkdtemp is not available
  strcpy_s( filename, sizeof(filename), mytemplate.str().c_str() );
  ptr = _mktemp(filename);
#else
  strcpy(filename, mytemplate.str().c_str());
  ptr = mkdtemp(filename);
#endif

  return ptr;
}

//----------------------------------------------------------------------------
bool vtkEMSegmentLogic::StartPreprocessingInitializeInputData()
{
  this->MRMLManager->GetWorkingDataNode()->SetInputTargetNodeIsValid(1);
  this->MRMLManager->GetWorkingDataNode()->SetInputAtlasNodeIsValid(1);
  this->MRMLManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);
  this->MRMLManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(0);

  return true;
}

//----------------------------------------------------------------------------
void vtkEMSegmentLogic::PrintImageInfo(vtkMRMLVolumeNode* volumeNode)
{
  if (volumeNode == NULL || volumeNode->GetImageData() == NULL)
    {
    std::cout << "Volume node or image data is null" << std::endl;
    return;
    }

  // extent
  int extent[6];
  volumeNode->GetImageData()->GetExtent(extent);
  std::cout << "Extent: " << std::endl;
  std::cout << extent[0] << " " << extent[1] << " " << extent[2] << " "
      << extent[3] << " " << extent[4] << " " << extent[5] << std::endl;

  // ijkToRAS
  vtkMatrix4x4* matrix = vtkMatrix4x4::New();
  volumeNode->GetIJKToRASMatrix(matrix);
  std::cout << "IJKtoRAS Matrix: " << std::endl;
  for (unsigned int r = 0; r < 4; ++r)
    {
    std::cout << "   ";
    for (unsigned int c = 0; c < 4; ++c)
      {
      std::cout << matrix->GetElement(r, c) << "   ";
      }
    std::cout << std::endl;
    }
  matrix->Delete();
}

// a utility to print out a vtk image origin, spacing, and extent
//----------------------------------------------------------------------------
void vtkEMSegmentLogic::PrintImageInfo(vtkImageData* image)
{
  double spacing[3];
  double origin[3];
  int extent[6];

  image->GetSpacing(spacing);
  image->GetOrigin(origin);
  image->GetExtent(extent);

  std::cout << "Spacing: " << spacing[0] << " " << spacing[1] << " "
      << spacing[2] << std::endl;
  std::cout << "Origin: " << origin[0] << " " << origin[1] << " " << origin[2]
      << std::endl;
  std::cout << "Extent: " << extent[0] << " " << extent[1] << " " << extent[2]
      << " " << extent[3] << " " << extent[4] << " " << extent[5] << std::endl;
}

//----------------------------------------------------------------------------
bool vtkEMSegmentLogic::IsVolumeGeometryEqual(vtkMRMLVolumeNode* lhs, vtkMRMLVolumeNode* rhs)
{
  if (lhs == NULL || rhs == NULL || lhs->GetImageData() == NULL
      || rhs->GetImageData() == NULL)
    {
    return false;
    }

  // check extent
  int extentLHS[6];
  lhs->GetImageData()->GetExtent(extentLHS);
  int extentRHS[6];
  rhs->GetImageData()->GetExtent(extentRHS);
  bool equalExent = std::equal(extentLHS, extentLHS + 6, extentRHS);

  // check ijkToRAS
  vtkMatrix4x4* matrixLHS = vtkMatrix4x4::New();
  lhs->GetIJKToRASMatrix(matrixLHS);
  vtkMatrix4x4* matrixRHS = vtkMatrix4x4::New();
  rhs->GetIJKToRASMatrix(matrixRHS);
  bool equalMatrix = true;
  for (int r = 0; r < 4; ++r)
    {
    for (int c = 0; c < 4; ++c)
      {
      // Otherwise small errors will cause that they are not equal but should be ignored !
      if (double(int((*matrixLHS)[r][c] * 100000) / 100000.0) != double(int(
          (*matrixRHS)[r][c] * 100000) / 100000.0))
        {
        equalMatrix = false;
        break;
        }
      }
    }

  matrixLHS->Delete();
  matrixRHS->Delete();
  return equalExent && equalMatrix;
}

// loops through the faces of the image bounding box and counts all the different image values and stores them in a map
// T represents the image data type
template<class T>
T vtkEMSegmentLogic::GuessRegistrationBackgroundLevel(vtkImageData* imageData)
{
  int borderWidth = 5;
  T inLevel;
  typedef std::map<T, unsigned int> MapType;
  MapType m;
  long totalVoxelsCounted = 0;

  T* inData = static_cast<T*> (imageData->GetScalarPointer());
  int dim[3];
  imageData->GetDimensions(dim);

  vtkIdType inc[3];
  vtkIdType iInc, jInc, kInc;
  imageData->GetIncrements(inc);

  // k first slice
  for (int k = 0; k < borderWidth; ++k)
    {
    kInc = k * inc[2];
    for (int j = 0; j < dim[1]; ++j)
      {
      jInc = j * inc[1];
      for (int i = 0; i < dim[0]; ++i)
        {
        iInc = i * inc[0];
        inLevel = inData[iInc + jInc + kInc];
        if (m.count(inLevel))
          {
          ++m[inLevel];
          }
        else
          {
          m[inLevel] = 1;
          }
        ++totalVoxelsCounted;
        }
      }
    }

  // k last slice
  for (int k = dim[2] - borderWidth; k < dim[2]; ++k)
    {
    kInc = k * inc[2];
    for (int j = 0; j < dim[1]; ++j)
      {
      jInc = j * inc[1];
      for (int i = 0; i < dim[0]; ++i)
        {
        iInc = i * inc[0];
        inLevel = inData[iInc + jInc + kInc];
        if (m.count(inLevel))
          {
          ++m[inLevel];
          }
        else
          {
          m[inLevel] = 1;
          }
        ++totalVoxelsCounted;
        }
      }
    }

  // j first slice
  for (int j = 0; j < borderWidth; ++j)
    {
    jInc = j * inc[1];
    for (int k = 0; k < dim[2]; ++k)
      {
      kInc = k * inc[2];
      for (int i = 0; i < dim[0]; ++i)
        {
        iInc = i * inc[0];
        inLevel = inData[iInc + jInc + kInc];
        if (m.count(inLevel))
          {
          ++m[inLevel];
          }
        else
          {
          m[inLevel] = 1;
          }
        ++totalVoxelsCounted;
        }
      }
    }

  // j last slice
  for (int j = dim[1] - borderWidth; j < dim[1]; ++j)
    {
    jInc = j * inc[1];
    for (int k = 0; k < dim[2]; ++k)
      {
      kInc = k * inc[2];
      for (int i = 0; i < dim[0]; ++i)
        {
        iInc = i * inc[0];
        inLevel = inData[iInc + jInc + kInc];
        if (m.count(inLevel))
          {
          ++m[inLevel];
          }
        else
          {
          m[inLevel] = 1;
          }
        ++totalVoxelsCounted;
        }
      }
    }

  // i first slice
  for (int i = 0; i < borderWidth; ++i)
    {
    iInc = i * inc[0];
    for (int k = 0; k < dim[2]; ++k)
      {
      kInc = k * inc[2];
      for (int j = 0; j < dim[1]; ++j)
        {
        jInc = j * inc[1];
        inLevel = inData[iInc + jInc + kInc];
        if (m.count(inLevel))
          {
          ++m[inLevel];
          }
        else
          {
          m[inLevel] = 1;
          }
        ++totalVoxelsCounted;
        }
      }
    }

  // i last slice
  for (int i = dim[0] - borderWidth; i < dim[0]; ++i)
    {
    iInc = i * inc[0];
    for (int k = 0; k < dim[2]; ++k)
      {
      kInc = k * inc[2];
      for (int j = 0; j < dim[1]; ++j)
        {
        jInc = j * inc[1];
        inLevel = inData[iInc + jInc + kInc];
        if (m.count(inLevel))
          {
          ++m[inLevel];
          }
        else
          {
          m[inLevel] = 1;
          }
        ++totalVoxelsCounted;
        }
      }
    }

  // all the information is stored in map m :  std::map<T, unsigned int>

  if (m.empty())
    {
    // no image data provided?
    return 0;
    }
  else if (m.size() == 1)
    {
    // Homogeneous background
    return m.begin()->first;
    }
  else
    {
    // search for the largest element
    typename MapType::iterator itor = std::max_element(m.begin(), m.end(),
        MapCompare<T>::map_value_comparer);

    // the iterator is pointing to the element with the largest value in the range [m.begin(), m.end()]
    T backgroundLevel = itor->first;

    // how many counts?
    double percentageOfVoxels = 100.0 * static_cast<double> (itor->second)
        / totalVoxelsCounted;

    std::cout << "   Background level guess : " << std::endl
        << "   first place: " << static_cast<int> (backgroundLevel) << " ("
        << percentageOfVoxels << "%) " << std::endl;

    // erase largest element
    m.erase(itor);

    // again, search for the largest element (second place)
    typename MapType::iterator itor2 = std::max_element(m.begin(), m.end(),
        MapCompare<T>::map_value_comparer);

    T backgroundLevel_second_place = itor2->first;

    double percentageOfVoxels_secondplace = 100.0
        * static_cast<double> (itor2->second) / totalVoxelsCounted;

    std::cout << "   second place: "
        << static_cast<int> (backgroundLevel_second_place) << " ("
        << percentageOfVoxels_secondplace << "%)" << std::endl;

    return backgroundLevel;
    }
}

//
// A Slicer3 wrapper around vtkImageReslice.  Reslice the image data
// from inputVolumeNode into outputVolumeNode with the output image
// geometry specified by outputVolumeGeometryNode.  Optionally specify
// a transform.  The reslice transform will be:
//
// outputIJK->outputRAS->(outputRASToInputRASTransform)->inputRAS->inputIJK
//
//----------------------------------------------------------------------------
void vtkEMSegmentLogic::SlicerImageReslice(vtkMRMLVolumeNode* inputVolumeNode, vtkMRMLVolumeNode* outputVolumeNode, vtkMRMLVolumeNode* outputVolumeGeometryNode, vtkTransform* outputRASToInputRASTransform, int interpolationType, double backgroundLevel)
{
  vtkImageData* inputImageData = inputVolumeNode->GetImageData();
  vtkImageData* outputImageData = outputVolumeNode->GetImageData();
  vtkImageData* outputGeometryData = NULL;
  if (outputVolumeGeometryNode != NULL)
    {
    outputGeometryData = outputVolumeGeometryNode->GetImageData();
    }

  vtkImageReslice* resliceFilter = vtkImageReslice::New();

  //
  // set inputs
  resliceFilter->SetInput(inputImageData);

  //
  // set geometry
  if (outputGeometryData != NULL)
    {
    resliceFilter->SetInformationInput(outputGeometryData);
    outputVolumeNode->CopyOrientation(outputVolumeGeometryNode);
    }

  //
  // setup total transform
  // ijk of output -> RAS -> XFORM -> RAS -> ijk of input
  vtkTransform* totalTransform = vtkTransform::New();
  if (outputRASToInputRASTransform != NULL)
    {
    totalTransform->DeepCopy(outputRASToInputRASTransform);
    }

  vtkMatrix4x4* outputIJKToRAS = vtkMatrix4x4::New();
  outputVolumeNode->GetIJKToRASMatrix(outputIJKToRAS);
  vtkMatrix4x4* inputRASToIJK = vtkMatrix4x4::New();
  inputVolumeNode->GetRASToIJKMatrix(inputRASToIJK);

  totalTransform->PreMultiply();
  totalTransform->Concatenate(outputIJKToRAS);
  totalTransform->PostMultiply();
  totalTransform->Concatenate(inputRASToIJK);
  resliceFilter->SetResliceTransform(totalTransform);

  //
  // resample the image
  resliceFilter->SetBackgroundLevel(backgroundLevel);
  resliceFilter->OptimizationOn();

  switch (interpolationType)
    {
    case vtkEMSegmentMRMLManager::InterpolationNearestNeighbor:
      resliceFilter->SetInterpolationModeToNearestNeighbor();
      break;
    case vtkEMSegmentMRMLManager::InterpolationCubic:
      resliceFilter->SetInterpolationModeToCubic();
      break;
    case vtkEMSegmentMRMLManager::InterpolationLinear:
    default:
      resliceFilter->SetInterpolationModeToLinear();
    }

  resliceFilter->Update();
  outputImageData->ShallowCopy(resliceFilter->GetOutput());

  //
  // clean up
  outputIJKToRAS->Delete();
  inputRASToIJK->Delete();
  resliceFilter->Delete();
  totalTransform->Delete();
}

// Assume geometry is already specified, create
// outGrid(p) = postMultiply \circ inGrid \circ preMultiply (p)
//
// right now simplicity over speed.  Optimize later?
//----------------------------------------------------------------------------
void vtkEMSegmentLogic::ComposeGridTransform(vtkGridTransform* inGrid, vtkMatrix4x4* preMultiply, vtkMatrix4x4* postMultiply, vtkGridTransform* outGrid)
{
  // iterate over output grid
  double inPt[4] =
  { 0, 0, 0, 1 };
  double pt[4] =
  { 0, 0, 0, 1 };
  double* outDataPtr =
      static_cast<double*> (outGrid->GetDisplacementGrid()->GetScalarPointer());
  vtkIdType numOutputVoxels =
      outGrid->GetDisplacementGrid()-> GetNumberOfPoints();

  for (vtkIdType i = 0; i < numOutputVoxels; ++i)
    {
    outGrid->GetDisplacementGrid()->GetPoint(i, inPt);
    preMultiply->MultiplyPoint(inPt, pt);
    inGrid->TransformPoint(pt, pt);
    postMultiply->MultiplyPoint(pt, pt);

    *outDataPtr++ = pt[0] - inPt[0];
    *outDataPtr++ = pt[1] - inPt[1];
    *outDataPtr++ = pt[2] - inPt[2];
    }
}

//
// A Slicer3 wrapper around vtkImageReslice.  Reslice the image data
// from inputVolumeNode into outputVolumeNode with the output image
// geometry specified by outputVolumeGeometryNode.  Optionally specify
// a transform.  The reslice transorm will be:
//
// outputIJK->outputRAS->(outputRASToInputRASTransform)->inputRAS->inputIJK
//
//----------------------------------------------------------------------------
void vtkEMSegmentLogic::SlicerImageResliceWithGrid(vtkMRMLVolumeNode* inputVolumeNode, vtkMRMLVolumeNode* outputVolumeNode, vtkMRMLVolumeNode* outputVolumeGeometryNode, vtkGridTransform* outputRASToInputRASTransform, int interpolationType, double backgroundLevel)
{
  vtkImageData* inputImageData = inputVolumeNode->GetImageData();
  vtkImageData* outputImageData = outputVolumeNode->GetImageData();
  vtkImageData* outputGeometryData = NULL;
  if (outputVolumeGeometryNode != NULL)
    {
    outputGeometryData = outputVolumeGeometryNode->GetImageData();
    }

  vtkImageReslice* resliceFilter = vtkImageReslice::New();

  //
  // set inputs
  resliceFilter->SetInput(inputImageData);

  //
  // create total transform
  vtkTransformToGrid* gridSource = vtkTransformToGrid::New();
  vtkIdentityTransform* idTransform = vtkIdentityTransform::New();
  gridSource->SetInput(idTransform);
  //gridSource->SetGridScalarType(VTK_FLOAT);
  idTransform->Delete();

  //
  // set geometry
  if (outputGeometryData != NULL)
    {
    resliceFilter->SetInformationInput(outputGeometryData);
    outputVolumeNode->CopyOrientation(outputVolumeGeometryNode);

    gridSource->SetGridExtent(outputGeometryData->GetExtent());
    gridSource->SetGridSpacing(outputGeometryData->GetSpacing());
    gridSource->SetGridOrigin(outputGeometryData->GetOrigin());
    }
  else
    {
    gridSource->SetGridExtent(outputImageData->GetExtent());
    gridSource->SetGridSpacing(outputImageData->GetSpacing());
    gridSource->SetGridOrigin(outputImageData->GetOrigin());
    }
  gridSource->Update();
  vtkGridTransform* totalTransform = vtkGridTransform::New();
  totalTransform->SetDisplacementGrid(gridSource->GetOutput());
  //  totalTransform->SetInterpolationModeToCubic();
  gridSource->Delete();

  //
  // fill in total transform
  // ijk of output -> RAS -> XFORM -> RAS -> ijk of input
  vtkMatrix4x4* outputIJKToRAS = vtkMatrix4x4::New();
  outputVolumeNode->GetIJKToRASMatrix(outputIJKToRAS);
  vtkMatrix4x4* inputRASToIJK = vtkMatrix4x4::New();
  inputVolumeNode->GetRASToIJKMatrix(inputRASToIJK);
  vtkEMSegmentLogic::ComposeGridTransform(outputRASToInputRASTransform,
      outputIJKToRAS, inputRASToIJK, totalTransform);
  resliceFilter->SetResliceTransform(totalTransform);

  //
  // resample the image
  resliceFilter->SetBackgroundLevel(backgroundLevel);
  resliceFilter->OptimizationOn();

  switch (interpolationType)
    {
    case vtkEMSegmentMRMLManager::InterpolationNearestNeighbor:
      resliceFilter->SetInterpolationModeToNearestNeighbor();
      break;
    case vtkEMSegmentMRMLManager::InterpolationCubic:
      resliceFilter->SetInterpolationModeToCubic();
      break;
    case vtkEMSegmentMRMLManager::InterpolationLinear:
    default:
      resliceFilter->SetInterpolationModeToLinear();
    }

  resliceFilter->Update();
  outputImageData->ShallowCopy(resliceFilter->GetOutput());

  //
  // clean up
  outputIJKToRAS->Delete();
  inputRASToIJK->Delete();
  resliceFilter->Delete();
  totalTransform->Delete();
}

//----------------------------------------------------------------------------
void vtkEMSegmentLogic::StartPreprocessingResampleAndCastToTarget(vtkMRMLVolumeNode* movingVolumeNode, vtkMRMLVolumeNode* fixedVolumeNode, vtkMRMLVolumeNode* outputVolumeNode)
{
  if (!vtkEMSegmentLogic::IsVolumeGeometryEqual(fixedVolumeNode,
      outputVolumeNode))
    {

    std::cout << "Warning: Target-to-target registration skipped but "
        << "target images have differenent geometries. " << std::endl
        << "Suggestion: If you are not positive that your images are "
        << "aligned, you should enable target-to-target registration."
        << std::endl;

    std::cout << "Fixed Volume Node: " << std::endl;
    PrintImageInfo(fixedVolumeNode);
    std::cout << "Output Volume Node: " << std::endl;
    PrintImageInfo(outputVolumeNode);

    // std::cout << "Resampling target image " << i << "...";
    double backgroundLevel = 0;
    switch (movingVolumeNode->GetImageData()->GetScalarType())
      {
      vtkTemplateMacro(backgroundLevel = (GuessRegistrationBackgroundLevel<VTK_TT>(movingVolumeNode->GetImageData()));)
        ;
      }
    std::cout << "   Guessed background level: " << backgroundLevel
        << std::endl;

    vtkEMSegmentLogic::SlicerImageReslice(movingVolumeNode, outputVolumeNode,
        fixedVolumeNode, NULL, vtkEMSegmentMRMLManager::InterpolationLinear,
        backgroundLevel);
    }

  if (fixedVolumeNode->GetImageData()->GetScalarType()
      != movingVolumeNode->GetImageData()->GetScalarType())
    {
    //cast
    vtkImageCast* cast = vtkImageCast::New();
    cast->SetInput(outputVolumeNode->GetImageData());
    cast->SetOutputScalarType(fixedVolumeNode->GetImageData()->GetScalarType());
    cast->Update();
    outputVolumeNode->GetImageData()->DeepCopy(cast->GetOutput());
    cast->Delete();
    }
  std::cout << "Resampling and casting output volume \""
      << outputVolumeNode->GetName() << "\" to reference target \""
      << fixedVolumeNode->GetName() << "\" DONE" << std::endl;
}

//----------------------------------------------------------------------------
double vtkEMSegmentLogic::GuessRegistrationBackgroundLevel(vtkMRMLVolumeNode* volumeNode)
{
  if (!volumeNode || !volumeNode->GetImageData())
    {
    std::cerr
        << "double vtkEMSegmentLogic::GuessRegistrationBackgroundLevel(vtkMRMLVolumeNode* volumeNode) : volumeNode or volumeNode->GetImageData is null"
        << std::endl;
    return -1;
    }

  // guess background level    
  double backgroundLevel = 0;
  switch (volumeNode->GetImageData()->GetScalarType())
    {
    vtkTemplateMacro(backgroundLevel = (GuessRegistrationBackgroundLevel<VTK_TT>(volumeNode->GetImageData()));)
      ;
    }
  std::cout << "   Guessed background level: " << backgroundLevel << std::endl;
  return backgroundLevel;
}

//-----------------------------------------------------------------------------
vtkIntArray*
vtkEMSegmentLogic::NewObservableEvents()
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  return events;
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::CopyDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  //
  // copy atlas related parameters to algorithm
  //
  vtkstd::cout << "atlas data...";
  this->CopyAtlasDataToSegmenter(segmenter);

  //
  // copy target related parameters to algorithm
  //
  vtkstd::cout << "target data...";
  this->CopyTargetDataToSegmenter(segmenter);

  //
  // copy global parameters to algorithm 
  //
  vtkstd::cout << "global data...";
  this->CopyGlobalDataToSegmenter(segmenter);

  //
  // copy tree base parameters to algorithm
  //
  vtkstd::cout << "tree data...";
  vtkImageEMLocalSuperClass* rootNode = vtkImageEMLocalSuperClass::New();
  this->CopyTreeDataToSegmenter(rootNode,
      this->MRMLManager->GetTreeRootNodeID());
  segmenter->SetHeadClass(rootNode);

  //cout << "====  vtkEMSegmentLogic::CopyDataToSegmenter: Print out  entire tree " << endl;
  // vtkIndent indent;
  // rootNode->PrintSelf(cout , indent);

  rootNode->Delete();
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::CopyAtlasDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  segmenter-> SetNumberOfTrainingSamples(
      this->MRMLManager-> GetAtlasNumberOfTrainingSamples());
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::CopyTargetDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  // !!! todo: TESTING HERE!!!
  vtkMRMLEMSVolumeCollectionNode* workingTarget =
      this->MRMLManager->GetWorkingDataNode()->GetAlignedTargetNode();

  if (workingTarget == NULL)
  {
    vtkErrorMacro("TargetNode is null");
    return;
  }

  unsigned int numTargetImages = workingTarget->GetNumberOfVolumes();
  std::cout << "Setting number of target images: " << numTargetImages
      << std::endl;
  segmenter->SetNumInputImages(numTargetImages);

  for (unsigned int i = 0; i < numTargetImages; ++i)
    {
    std::string mrmlID = workingTarget->GetNthVolumeNodeID(i);
    vtkDebugMacro("Setting target image " << i << " mrmlID="
        << mrmlID.c_str());

    vtkImageData* imageData =
        workingTarget->GetNthVolumeNode(i)->GetImageData();

    std::cout << "AddingTargetImage..." << std::endl;
    this->PrintImageInfo(imageData);
    imageData->Update();
    this->PrintImageInfo(imageData);

    segmenter->SetImageInput(i, imageData);
    }
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::CopyGlobalDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  if (this->MRMLManager->GetEnableMultithreading())
    {
    segmenter-> SetDisableMultiThreading(0);
    }
  else
    {
    segmenter-> SetDisableMultiThreading(1);
    }
  segmenter->SetPrintDir(this->MRMLManager->GetSaveWorkingDirectory());

  //
  // NB: In the algorithm code smoothing width and sigma parameters
  // are defined globally.  In this logic, they are defined for each
  // parent node.  For now copy parameters from the root tree
  // node. !!!todo!!!
  //
  vtkIdType rootNodeID = this->MRMLManager->GetTreeRootNodeID();
  segmenter-> SetSmoothingWidth(
      this->MRMLManager-> GetTreeNodeSmoothingKernelWidth(rootNodeID));

  // type mismatch between logic and algorithm !!!todo!!!
  int intSigma = vtkMath::Round(
      this->MRMLManager-> GetTreeNodeSmoothingKernelSigma(rootNodeID));
  segmenter->SetSmoothingSigma(intSigma);

  int biasType = this->MRMLManager-> GetBiasCorrectionType(rootNodeID);
  segmenter->SetBiasCorrectionType(biasType);

  //
  // registration parameters
  //
  int algType = this->ConvertGUIEnumToAlgorithmEnumInterpolationType(
      this->MRMLManager->GetRegistrationInterpolationType());
  segmenter->SetRegistrationInterpolationType(algType);
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::CopyTreeDataToSegmenter(vtkImageEMLocalSuperClass* node, vtkIdType nodeID)
{
  // need this here because the vtkImageEM* classes don't use
  // virtual functions and so failed initializations lead to
  // memory errors
  node->SetNumInputImages(this->MRMLManager->GetTargetNumberOfSelectedVolumes());

  // copy generic tree node data to segmenter
  this->CopyTreeGenericDataToSegmenter(node, nodeID);

  // add children
  unsigned int numChildren = this->MRMLManager->GetTreeNodeNumberOfChildren(
      nodeID);
  double totalProbability = 0.0;
  for (unsigned int i = 0; i < numChildren; ++i)
    {
    vtkIdType childID = this->MRMLManager->GetTreeNodeChildNodeID(nodeID, i);
    bool isLeaf = this->MRMLManager->GetTreeNodeIsLeaf(childID);

    if (isLeaf)
      {
      vtkImageEMLocalClass* childNode = vtkImageEMLocalClass::New();
      // need this here because the vtkImageEM* classes don't use
      // virtual functions and so failed initializations lead to
      // memory errors
      childNode->SetNumInputImages(
          this->MRMLManager-> GetTargetNumberOfSelectedVolumes());
      this->CopyTreeGenericDataToSegmenter(childNode, childID);
      this->CopyTreeLeafDataToSegmenter(childNode, childID);
      node->AddSubClass(childNode, i);
      childNode->Delete();
      }
    else
      {
      vtkImageEMLocalSuperClass* childNode = vtkImageEMLocalSuperClass::New();
      this->CopyTreeDataToSegmenter(childNode, childID);
      node->AddSubClass(childNode, i);
      childNode->Delete();
      }

    totalProbability += this->MRMLManager->GetTreeNodeClassProbability(childID);
    }

  // check if totalProbability != 1.0
  if (abs(totalProbability - 1.0) > 0.000001)
    {
    vtkWarningMacro("Warning: child probabilities don't sum to unity for node "
        << this->MRMLManager->GetTreeNodeName(nodeID)
        << " they sum to " << totalProbability);
    }

  // copy other parent specific tree node data to segmenter
  // Do it after all classes are added so that number of children are set correctly - Important for mrf 
  this->CopyTreeParentDataToSegmenter(node, nodeID);

  node->Update();
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::DefineValidSegmentationBoundary()
{
  //
  // Setup ROI.  If if looks bogus then use the default (entire image)
  bool useDefaultBoundary = false;
  int boundMin[3];
  int boundMax[3];

  // get dimensions of target image
  int targetImageDimensions[3];
  this->MRMLManager->GetTargetInputNode()->GetNthVolumeNode(0)-> GetImageData()->GetDimensions(
      targetImageDimensions);

  this->MRMLManager->GetSegmentationBoundaryMin(boundMin);
  this->MRMLManager->GetSegmentationBoundaryMax(boundMax);
  // Specify boundary in 1-based, NOT 0-based as you might expect
  for (unsigned int i = 0; i < 3; ++i)
    {
    if (boundMin[i] < 1 || boundMin[i] > targetImageDimensions[i]
        || boundMax[i] < 1 || boundMax[i] > targetImageDimensions[i]
        || boundMax[i] < boundMin[i])
      {
      useDefaultBoundary = true;
      break;
      }
    }
  if (useDefaultBoundary)
    {
    std::cout << std::endl
        << "===================================================================="
        << std::endl
        << "Warning: the segmentation ROI was bogus, setting ROI to entire image"
        << std::endl << "Axis 0 -  Image Min: 1 <= RoiMin(" << boundMin[0]
        << ") <= ROIMax(" << boundMax[0] << ") <=  Image Max:"
        << targetImageDimensions[0] << std::endl
        << "Axis 1 -  Image Min: 1 <= RoiMin(" << boundMin[1] << ") <= ROIMax("
        << boundMax[1] << ") <=  Image Max:" << targetImageDimensions[1]
        << std::endl << "Axis 2 -  Image Min: 1 <= RoiMin(" << boundMin[2]
        << ") <= ROIMax(" << boundMax[2] << ") <=  Image Max:"
        << targetImageDimensions[2] << std::endl
        << "NOTE: The above warning about ROI should not lead to poor segmentation results;  the entire image should be segmented.  It only indicates an error if you intended to segment a subregion of the image."
        << std::endl << "Define Boundary as: ";
    for (unsigned int i = 0; i < 3; ++i)
      {
      boundMin[i] = 1;
      boundMax[i] = targetImageDimensions[i];
      std::cout << boundMin[i] << ", " << boundMax[i] << ",   ";
      }
    std::cout << std::endl
        << "===================================================================="
        << std::endl;

    this->MRMLManager->SetSegmentationBoundaryMin(boundMin);
    this->MRMLManager->SetSegmentationBoundaryMax(boundMax);
    }
}

void vtkEMSegmentLogic::CopyTreeGenericDataToSegmenter(vtkImageEMLocalGenericClass* node, vtkIdType nodeID)
{
  unsigned int numTargetImages =
      this->MRMLManager->GetTargetNumberOfSelectedVolumes();

  this->DefineValidSegmentationBoundary();
  int boundMin[3];
  int boundMax[3];
  this->MRMLManager->GetSegmentationBoundaryMin(boundMin);
  this->MRMLManager->GetSegmentationBoundaryMax(boundMax);
  node->SetSegmentationBoundaryMin(boundMin[0], boundMin[1], boundMin[2]);
  node->SetSegmentationBoundaryMax(boundMax[0], boundMax[1], boundMax[2]);

  node->SetProbDataWeight(this->MRMLManager-> GetTreeNodeSpatialPriorWeight(
      nodeID));

  node->SetTissueProbability(this->MRMLManager-> GetTreeNodeClassProbability(
      nodeID));

  node->SetPrintWeights(this->MRMLManager->GetTreeNodePrintWeight(nodeID));

  // set target input channel weights
  for (unsigned int i = 0; i < numTargetImages; ++i)
    {
    node->SetInputChannelWeights(
        this->MRMLManager-> GetTreeNodeInputChannelWeight(nodeID, i), i);
    }

  if (this->GetMRMLManager()->GetGlobalParametersNode()->GetAMFSmoothing()
      && (nodeID != this->GetMRMLManager()->GetTreeRootNodeID())
      && (this->GetMRMLManager()->GetTreeNodeParentNodeID(nodeID)
          == this->GetMRMLManager()->GetTreeRootNodeID()))
    {
    // Set up posterior image data !
    if (!node->GetPosteriorImageData())
      {
      vtkImageData* imgData = vtkImageData::New();
      node->SetPosteriorImageData(imgData);
      imgData->Delete();
      }

    }

  //
  // registration related data
  //
  //!!!bcd!!!

  //
  // set probability data
  //

  // get working atlas
  // !!! error checking!
  vtkMRMLVolumeNode* atlasNode =
      this->MRMLManager->GetAlignedSpatialPriorFromTreeNodeID(nodeID);
  if (atlasNode)
    {
    vtkDebugMacro("Setting spatial prior: node="
        << this->MRMLManager->GetTreeNodeName(nodeID));
    vtkImageData* imageData = atlasNode->GetImageData();
    node->SetProbDataPtr(imageData);
    }

  int exclude =
      this->MRMLManager->GetTreeNodeExcludeFromIncompleteEStep(nodeID);
  node->SetExcludeFromIncompleteEStepFlag(exclude);
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::CopyTreeParentDataToSegmenter(vtkImageEMLocalSuperClass* node, vtkIdType nodeID)
{
  node->SetPrintFrequency(this->MRMLManager-> GetTreeNodePrintFrequency(nodeID));
  node->SetPrintBias(this->MRMLManager-> GetTreeNodePrintBias(nodeID));
  node->SetPrintLabelMap(this->MRMLManager-> GetTreeNodePrintLabelMap(nodeID));

  node->SetPrintEMLabelMapConvergence(
      this->MRMLManager->GetTreeNodePrintEMLabelMapConvergence(nodeID));
  node->SetPrintEMWeightsConvergence(
      this->MRMLManager->GetTreeNodePrintEMWeightsConvergence(nodeID));
  node->SetStopEMType(this->ConvertGUIEnumToAlgorithmEnumStoppingConditionType(
      this->MRMLManager-> GetTreeNodeStoppingConditionEMType(nodeID)));
  node->SetStopEMValue(this->MRMLManager-> GetTreeNodeStoppingConditionEMValue(
      nodeID));
  node->SetStopEMMaxIter(
      this->MRMLManager->GetTreeNodeStoppingConditionEMIterations(nodeID));

  node->SetPrintMFALabelMapConvergence(
      this->MRMLManager->GetTreeNodePrintMFALabelMapConvergence(nodeID));
  node->SetPrintMFAWeightsConvergence(
      this->MRMLManager->GetTreeNodePrintMFAWeightsConvergence(nodeID));
  node->SetStopMFAType(
      this->ConvertGUIEnumToAlgorithmEnumStoppingConditionType(
          this->MRMLManager-> GetTreeNodeStoppingConditionMFAType(nodeID)));
  node->SetStopMFAValue(
      this->MRMLManager-> GetTreeNodeStoppingConditionMFAValue(nodeID));
  node->SetStopMFAMaxIter(
      this->MRMLManager->GetTreeNodeStoppingConditionMFAIterations(nodeID));

  node->SetStopBiasCalculation(
      this->MRMLManager->GetTreeNodeBiasCalculationMaxIterations(nodeID));

  node->SetPrintShapeSimularityMeasure(0); // !!!bcd!!!

  node->SetPCAShapeModelType(0); // !!!bcd!!!

  node->SetRegistrationIndependentSubClassFlag(0); // !!!bcd!!!
  node->SetRegistrationType(0); // !!!bcd!!!

  node->SetGenerateBackgroundProbability(
      this->MRMLManager->GetTreeNodeGenerateBackgroundProbability(nodeID));

  // New in 3.6. : Alpha now reflects user interface and is now correctly set for each parent node
  // cout << "Alpha setting for " << this->MRMLManager->GetTreeNodeName(nodeID) << " " << this->MRMLManager->GetTreeNodeAlpha(nodeID) << endl;
  node->SetAlpha(this->MRMLManager->GetTreeNodeAlpha(nodeID));

  // Set Markov matrices
  // this should already be set correctly 
  int numChildren = node->GetNumClasses();
  if (this->MRMLManager->GetTreeNodeNumberOfChildren(nodeID) != numChildren)
    {
    vtkErrorMacro("Please sync number of classes of this node with the entry in MRMLManager before calling this function");
    return;
    }

  const int numDirections = 6;
  int MFA2DFlag = this->MRMLManager->GetTreeNodeInteractionMatrices2DFlag(
      nodeID);
  if (MFA2DFlag)
    {
    cout << "MFA 2D Interaction Flag is activated for node "
        << this->MRMLManager->GetTreeNodeName(nodeID) << endl;
    }
  for (int d = 0; d < numDirections; ++d)
    {
    for (int r = 0; r < numChildren; ++r)
      {
      for (int c = 0; c < numChildren; ++c)
        {
        double val = 0;
        // Depending if we want to have 2D or 3D neighborhood the flag is set differently
        if (r == c)
          {
          // in 2D do not set up and down
          if (!MFA2DFlag || ((d % 3) != 2))
            {
            val = 1;
            }
          }
        node->SetMarkovMatrix(val, d, c, r);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::CopyTreeLeafDataToSegmenter(vtkImageEMLocalClass* node, vtkIdType nodeID)
{
  unsigned int numTargetImages =
      this->MRMLManager->GetTargetNumberOfSelectedVolumes();

  // this label describes the output intensity value for this class in
  // the segmentation result
  node->SetLabel(this->MRMLManager->GetTreeNodeIntensityLabel(nodeID));

  // set log mean and log covariance
  for (unsigned int r = 0; r < numTargetImages; ++r)
    {
    node->SetLogMu(
        this->MRMLManager-> GetTreeNodeDistributionLogMeanWithCorrection(
            nodeID, r), r);

    for (unsigned int c = 0; c < numTargetImages; ++c)
      {
      node->SetLogCovariance(
          this->MRMLManager-> GetTreeNodeDistributionLogCovarianceWithCorrection(
              nodeID, r, c), r, c);
      }
    }

  node->SetPrintQuality(this->MRMLManager->GetTreeNodePrintQuality(nodeID));
}

//-----------------------------------------------------------------------------
int vtkEMSegmentLogic::ConvertGUIEnumToAlgorithmEnumStoppingConditionType(int guiEnumValue)
{
  switch (guiEnumValue)
    {
    case (vtkEMSegmentMRMLManager::StoppingConditionIterations):
      return EMSEGMENT_STOP_FIXED;
    case (vtkEMSegmentMRMLManager::StoppingConditionLabelMapMeasure):
      return EMSEGMENT_STOP_LABELMAP;
    case (vtkEMSegmentMRMLManager::StoppingConditionWeightsMeasure):
      return EMSEGMENT_STOP_WEIGHTS;
    default:
      vtkErrorMacro("Unknown stopping condition type: " << guiEnumValue)
      ;
      return -1;
    }
}

//-----------------------------------------------------------------------------
int vtkEMSegmentLogic::ConvertGUIEnumToAlgorithmEnumInterpolationType(int guiEnumValue)
{
  switch (guiEnumValue)
    {
    case (vtkEMSegmentMRMLManager::InterpolationLinear):
      return EMSEGMENT_REGISTRATION_INTERPOLATION_LINEAR;
    case (vtkEMSegmentMRMLManager::InterpolationNearestNeighbor):
      return EMSEGMENT_REGISTRATION_INTERPOLATION_NEIGHBOUR;
    case (vtkEMSegmentMRMLManager::InterpolationCubic):
      // !!! not implemented
      vtkErrorMacro("Cubic interpolation not implemented: " << guiEnumValue)
      ;
      return -1;
    default:
      vtkErrorMacro("Unknown interpolation type: " << guiEnumValue)
      ;
      return -1;
    }
}

//----------------------------------------------------------------------------
vtkstd::string vtkEMSegmentLogic::GetTclGeneralDirectory()
{
  // Later do automatically
  vtkstd::string file_path = this->GetModuleShareDirectory() + vtkstd::string(
      "/Tcl");
  return vtksys::SystemTools::ConvertToOutputPath(file_path.c_str());
}

//----------------------------------------------------------------------------
void vtkEMSegmentLogic::TransferIJKToRAS(vtkMRMLVolumeNode* volumeNode, int ijk[3], double ras[3])
{
  vtkMatrix4x4* matrix = vtkMatrix4x4::New();
  volumeNode->GetIJKToRASMatrix(matrix);
  float input[4] =
  { ijk[0], ijk[1], ijk[2], 1 };
  float output[4];
  matrix->MultiplyPoint(input, output);
  ras[0] = output[0];
  ras[1] = output[1];
  ras[2] = output[2];
}

//----------------------------------------------------------------------------
void vtkEMSegmentLogic::TransferRASToIJK(vtkMRMLVolumeNode* volumeNode, double ras[3], int ijk[3])
{
  vtkMatrix4x4* matrix = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(matrix);
  double input[4] =
  { ras[0], ras[1], ras[2], 1 };
  double output[4];
  matrix->MultiplyPoint(input, output);
  ijk[0] = int(output[0]);
  ijk[1] = int(output[1]);
  ijk[2] = int(output[2]);
}

//----------------------------------------------------------------------------
// works for running stuff in TCL so that you do not need to look in two windows 
void vtkEMSegmentLogic::PrintText(char *TEXT)
{
  cout << TEXT << endl;
}

//----------------------------------------------------------------------------
void vtkEMSegmentLogic::PrintTextNoNewLine(char *TEXT)
{
  cout << TEXT;
  cout.flush();
}

//----------------------------------------------------------------------------
void vtkEMSegmentLogic::AutoCorrectSpatialPriorWeight(vtkIdType nodeID)
{
  unsigned int numChildren = this->MRMLManager->GetTreeNodeNumberOfChildren(
      nodeID);
  for (unsigned int i = 0; i < numChildren; ++i)
    {
    vtkIdType childID = this->MRMLManager->GetTreeNodeChildNodeID(nodeID, i);
    bool isLeaf = this->MRMLManager->GetTreeNodeIsLeaf(childID);
    if (isLeaf)
      {
      if ((this->MRMLManager->GetTreeNodeSpatialPriorWeight(childID) > 0.0)
          && (!this->MRMLManager->GetAlignedSpatialPriorFromTreeNodeID(childID)))
        {
        vtkWarningMacro("Class with ID " << childID << " is set to 0 bc no atlas assigned to class!" );
        this->MRMLManager->SetTreeNodeSpatialPriorWeight(childID, 0.0);
        }
      }
    else
      {
      this->AutoCorrectSpatialPriorWeight(childID);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::CreatePackageFilenames(vtkMRMLScene* scene, const char* vtkNotUsed(packageDirectoryName))
{
  //
  // set up mrml manager for this new scene
  vtkEMSegmentMRMLManager* newSceneManager = vtkEMSegmentMRMLManager::New();
  newSceneManager->SetMRMLScene(scene);
  vtkMRMLEMSTemplateNode* newEMSTemplateNode =
      dynamic_cast<vtkMRMLEMSTemplateNode*> (scene->GetNthNodeByClass(0,
          "vtkMRMLEMSTemplateNode"));
  if (newEMSTemplateNode == NULL)
    {
    vtkWarningMacro("CreatePackageFilenames: no EMSSegmenter node!");
    newSceneManager->Delete();
    return;
    }
  if (newSceneManager->SetNodeWithCheck(newEMSTemplateNode))
    {
    vtkWarningMacro("CreatePackageFilenames: not a valid template node!");
    newSceneManager->Delete();
    return;
    }

  vtkMRMLEMSWorkingDataNode* workingDataNode =
      newSceneManager->GetWorkingDataNode();

  //
  // We might be creating volume storage nodes.  We must decide if the
  // images should be automatically centered when they are read.  Look
  // at the original input target node zero to decide if we will use
  // centering.
  bool centerImages = false;
  if (workingDataNode && workingDataNode->GetInputTargetNode())
    {
    if (workingDataNode->GetInputTargetNode()->GetNumberOfVolumes() > 0)
      {
      if (!workingDataNode->GetInputTargetNode()->GetNthVolumeNode(0))
        {
        vtkErrorMacro("CreatePackageFilenames: the first InputTagetNode is not defined!");
        vtkIndent ind;
        workingDataNode->GetInputTargetNode()->PrintSelf(cerr, ind);
        cout << endl;
        }
      else
        {
        vtkMRMLStorageNode
            * firstTargetStorageNode =
                workingDataNode->GetInputTargetNode()->GetNthVolumeNode(0)->GetStorageNode();
        vtkMRMLVolumeArchetypeStorageNode
            * firstTargetVolumeStorageNode =
                dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*> (firstTargetStorageNode);
        if (firstTargetVolumeStorageNode != NULL)
          {
          centerImages = firstTargetVolumeStorageNode->GetCenterImage();
          }
        }
      }
    }

  // get the full path to the scene
  std::vector < std::string > scenePathComponents;
  vtkstd::string rootDir = newSceneManager->GetMRMLScene()->GetRootDirectory();
  if (rootDir.find_last_of("/") == rootDir.length() - 1)
    {
    vtkDebugMacro("em seg: found trailing slash in : " << rootDir);
    rootDir = rootDir.substr(0, rootDir.length() - 1);
    }
  vtkDebugMacro("em seg scene manager root dir = " << rootDir);
  vtksys::SystemTools::SplitPath(rootDir.c_str(), scenePathComponents);

  // change the storage file for the segmentation result
    {
    vtkMRMLVolumeNode* volumeNode = newSceneManager->GetOutputVolumeNode();
    if (volumeNode != NULL)
      {
      vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
      vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode =
          dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*> (storageNode);
      if (volumeStorageNode == NULL)
        {
        // create a new storage node for this volume
        volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
        scene->AddNodeNoNotify(volumeStorageNode);
        volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
        std::cout << "Added storage node : " << volumeStorageNode->GetID()
            << std::endl;
        volumeStorageNode->Delete();
        storageNode = volumeStorageNode;
        }
      volumeStorageNode->SetCenterImage(centerImages);

      // create new filename
      std::string oldFilename =
          (storageNode->GetFileName() ? storageNode->GetFileName()
              : "SegmentationResult.mhd");
      std::string oldFilenameNoPath = vtksys::SystemTools::GetFilenameName(
          oldFilename);

      scenePathComponents.push_back("Segmentation");
      scenePathComponents.push_back(oldFilenameNoPath);

      std::string newFilename = vtksys::SystemTools::JoinPath(
          scenePathComponents);
      storageNode->SetFileName(newFilename.c_str());
      scenePathComponents.pop_back();
      scenePathComponents.pop_back();

      }
    }

  //
  // change the storage file for the targets
  int numTargets = newSceneManager->GetTargetNumberOfSelectedVolumes();

  // input target volumes
  if (workingDataNode->GetInputTargetNode())
    {
    for (int i = 0; i < numTargets; ++i)
      {
      vtkMRMLVolumeNode* volumeNode =
          workingDataNode->GetInputTargetNode()->GetNthVolumeNode(i);
      if (volumeNode != NULL)
        {
        vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
        vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode =
            dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*> (storageNode);
        if (volumeStorageNode == NULL)
          {
          // create a new storage node for this volume
          volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
          scene->AddNodeNoNotify(volumeStorageNode);
          volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
          std::cout << "Added storage node : " << volumeStorageNode->GetID()
              << std::endl;
          volumeStorageNode->Delete();
          storageNode = volumeStorageNode;
          }
        volumeStorageNode->SetCenterImage(centerImages);
        // create new filename
        vtkstd::stringstream defaultFilename;
        defaultFilename << "Target" << i << "_Input.mhd";
        std::string oldFilename =
            (storageNode->GetFileName() ? storageNode->GetFileName()
                : defaultFilename.str().c_str());
        std::string oldFilenameNoPath = vtksys::SystemTools::GetFilenameName(
            oldFilename);
        scenePathComponents.push_back("Target");
        scenePathComponents.push_back("Input");
        scenePathComponents.push_back(oldFilenameNoPath);
        std::string newFilename = vtksys::SystemTools::JoinPath(
            scenePathComponents);

        storageNode->SetFileName(newFilename.c_str());
        scenePathComponents.pop_back();
        scenePathComponents.pop_back();
        scenePathComponents.pop_back();
        }
      }
    }

  // aligned target volumes
  if (workingDataNode->GetAlignedTargetNode())
    {
    for (int i = 0; i < numTargets; ++i)
      {
      vtkMRMLVolumeNode* volumeNode =
          workingDataNode->GetAlignedTargetNode()->GetNthVolumeNode(i);
      if (volumeNode != NULL)
        {
        vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
        vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode =
            dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*> (storageNode);
        if (volumeStorageNode == NULL)
          {
          // create a new storage node for this volume
          volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
          scene->AddNodeNoNotify(volumeStorageNode);
          volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
          std::cout << "Added storage node : " << volumeStorageNode->GetID()
              << std::endl;
          volumeStorageNode->Delete();
          storageNode = volumeStorageNode;
          }
        volumeStorageNode->SetCenterImage(centerImages);
        // create new filename
        vtkstd::stringstream defaultFilename;
        defaultFilename << "Target" << i << "_Aligned.mhd";
        std::string oldFilename =
            (storageNode->GetFileName() ? storageNode->GetFileName()
                : defaultFilename.str().c_str());
        std::string oldFilenameNoPath = vtksys::SystemTools::GetFilenameName(
            oldFilename);
        scenePathComponents.push_back("Target");
        scenePathComponents.push_back("Aligned");
        scenePathComponents.push_back(oldFilenameNoPath);
        std::string newFilename = vtksys::SystemTools::JoinPath(
            scenePathComponents);

        storageNode->SetFileName(newFilename.c_str());
        scenePathComponents.pop_back();
        scenePathComponents.pop_back();
        scenePathComponents.pop_back();
        }
      }
    }

  //
  // change the storage file for the atlas
  int numAtlasVolumes =
      newSceneManager->GetAtlasInputNode()-> GetNumberOfVolumes();

  // input atlas volumes
  if (newSceneManager->GetAtlasInputNode())
    {
    for (int i = 0; i < numAtlasVolumes; ++i)
      {
      vtkMRMLVolumeNode* volumeNode =
          newSceneManager->GetAtlasInputNode()->GetNthVolumeNode(i);
      if (volumeNode != NULL)
        {
        vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
        vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode =
            dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*> (storageNode);
        if (volumeStorageNode == NULL)
          {
          // create a new storage node for this volume
          volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
          scene->AddNodeNoNotify(volumeStorageNode);
          volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
          std::cout << "Added storage node : " << volumeStorageNode->GetID()
              << std::endl;
          volumeStorageNode->Delete();
          storageNode = volumeStorageNode;
          }
        volumeStorageNode->SetCenterImage(centerImages);
        // create new filename
        vtkstd::stringstream defaultFilename;
        defaultFilename << "Atlas" << i << "_Input.mhd";
        std::string oldFilename =
            (storageNode->GetFileName() ? storageNode->GetFileName()
                : defaultFilename.str().c_str());
        std::string oldFilenameNoPath = vtksys::SystemTools::GetFilenameName(
            oldFilename);
        scenePathComponents.push_back("Atlas");
        scenePathComponents.push_back("Input");
        scenePathComponents.push_back(oldFilenameNoPath);
        std::string newFilename = vtksys::SystemTools::JoinPath(
            scenePathComponents);

        storageNode->SetFileName(newFilename.c_str());
        scenePathComponents.pop_back();
        scenePathComponents.pop_back();
        scenePathComponents.pop_back();
        }
      }
    }

  // aligned atlas volumes
  if (workingDataNode->GetAlignedAtlasNode())
    {
    for (int i = 0; i < numAtlasVolumes; ++i)
      {
      vtkMRMLVolumeNode* volumeNode =
          workingDataNode->GetAlignedAtlasNode()->GetNthVolumeNode(i);
      if (volumeNode != NULL)
        {
        vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
        vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode =
            dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*> (storageNode);
        if (volumeStorageNode == NULL)
          {
          // create a new storage node for this volume
          volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
          scene->AddNodeNoNotify(volumeStorageNode);
          volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
          std::cout << "Added storage node : " << volumeStorageNode->GetID()
              << std::endl;
          volumeStorageNode->Delete();
          storageNode = volumeStorageNode;
          }
        volumeStorageNode->SetCenterImage(centerImages);
        // create new filename
        vtkstd::stringstream defaultFilename;
        defaultFilename << "Atlas" << i << "_Aligned.mhd";
        std::string oldFilename =
            (storageNode->GetFileName() ? storageNode->GetFileName()
                : defaultFilename.str().c_str());
        std::string oldFilenameNoPath = vtksys::SystemTools::GetFilenameName(
            oldFilename);
        scenePathComponents.push_back("Atlas");
        scenePathComponents.push_back("Aligned");
        scenePathComponents.push_back(oldFilenameNoPath);
        std::string newFilename = vtksys::SystemTools::JoinPath(
            scenePathComponents);

        storageNode->SetFileName(newFilename.c_str());
        scenePathComponents.pop_back();
        scenePathComponents.pop_back();
        scenePathComponents.pop_back();
        }
      }
    }

  // clean up
  newSceneManager->Delete();
}

//-----------------------------------------------------------------------------
bool vtkEMSegmentLogic::CreatePackageDirectories(const char* packageDirectoryName)
{
  vtkstd::string packageDirectory(packageDirectoryName);

  // check that parent directory exists
  std::string parentDirectory = vtksys::SystemTools::GetParentDirectory(
      packageDirectory.c_str());
  if (!vtksys::SystemTools::FileExists(parentDirectory.c_str()))
    {
    vtkWarningMacro
    ("CreatePackageDirectories: Parent directory does not exist!");
    return false;
    }

  // create package directories
  bool createdOK = true;
  std::string newDir = packageDirectory + "/Atlas/Input";
  createdOK = createdOK && vtksys::SystemTools::MakeDirectory(newDir.c_str());
  newDir = packageDirectory + "/Atlas/Aligned";
  createdOK = createdOK && vtksys::SystemTools::MakeDirectory(newDir.c_str());
  newDir = packageDirectory + "/Target/Input";
  createdOK = createdOK && vtksys::SystemTools::MakeDirectory(newDir.c_str());
  newDir = packageDirectory + "/Target/Normalized";
  createdOK = createdOK && vtksys::SystemTools::MakeDirectory(newDir.c_str());
  newDir = packageDirectory + "/Target/Aligned";
  createdOK = createdOK && vtksys::SystemTools::MakeDirectory(newDir.c_str());
  newDir = packageDirectory + "/Segmentation";
  createdOK = createdOK && vtksys::SystemTools::MakeDirectory(newDir.c_str());

  if (!createdOK)
    {
    vtkWarningMacro("CreatePackageDirectories: Could not create directories!");
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkEMSegmentLogic::WritePackagedScene(vtkMRMLScene* scene)
{
  //
  // write the volumes
  scene->InitTraversal();
  vtkMRMLNode* currentNode;
  bool allOK = true;
  while ((currentNode = scene->GetNextNodeByClass("vtkMRMLVolumeNode"))
      && (currentNode != NULL))
    {
    vtkMRMLVolumeNode* volumeNode =
        dynamic_cast<vtkMRMLVolumeNode*> (currentNode);

    if (volumeNode == NULL)
      {
      vtkWarningMacro("Volume node is null for node: "
          << currentNode->GetID());
      scene->RemoveNode(currentNode);
      allOK = false;
      continue;
      }
    if (volumeNode->GetImageData() == NULL)
      {
      vtkWarningMacro("Volume data is null for volume node: " << currentNode->GetID() << " Name : " << (currentNode->GetName() ? currentNode->GetName(): "(none)" ));
      scene->RemoveNode(currentNode);
      allOK = false;
      continue;
      }
    if (volumeNode->GetStorageNode() == NULL)
      {
      vtkWarningMacro("Volume storage node is null for volume node: "
          << currentNode->GetID());
      scene->RemoveNode(currentNode);
      allOK = false;
      continue;
      }

    try
      {
      std::cout << "Writing volume: " << volumeNode->GetName() << ": "
          << volumeNode->GetStorageNode()->GetFileName() << "...";
      volumeNode->GetStorageNode()->SetUseCompression(0);
      volumeNode->GetStorageNode()->WriteData(volumeNode);
      std::cout << "DONE" << std::endl;
      } catch (...)
      {
      vtkErrorMacro("Problem writing volume: " << volumeNode->GetID());
      allOK = false;
      }
    }

  //
  // write the MRML scene file
  try
    {
    scene->Commit();
    } catch (...)
    {
    vtkErrorMacro("Problem writing scene.");
    allOK = false;
    }

  return allOK;
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::SubParcelateSegmentation(vtkImageData* segmentation, vtkIdType nodeID)
{
  unsigned int numChildren = this->MRMLManager->GetTreeNodeNumberOfChildren(
      nodeID);
  for (unsigned int i = 0; i < numChildren; ++i)
    {
    vtkIdType childID = this->MRMLManager->GetTreeNodeChildNodeID(nodeID, i);
    if (this->MRMLManager->GetTreeNodeIsLeaf(childID))
      {
      vtkMRMLVolumeNode* parcellationNode =
          this->MRMLManager->GetAlignedSubParcellationFromTreeNodeID(childID);
      if (!parcellationNode || !parcellationNode->GetImageData())
        {
        continue;
        }
      int childLabel = this->MRMLManager->GetTreeNodeIntensityLabel(childID);
      cout << "==> Subparcellate " << childLabel << endl;
      vtkImageData* input = vtkImageData::New();
      input->DeepCopy(segmentation);

      vtkImageThreshold* roiMap = vtkImageThreshold::New();
      roiMap->SetInput(input);
      roiMap->ThresholdBetween(childLabel, childLabel);
      roiMap->ReplaceOutOn();
      roiMap->SetInValue(1);
      roiMap->SetOutValue(0);
      roiMap->Update();

      vtkImageCast* castParcellation = vtkImageCast::New();
      castParcellation->SetInput(parcellationNode->GetImageData());
      castParcellation->SetOutputScalarType(
          roiMap->GetOutput()->GetScalarType());
      castParcellation->Update();

      vtkImageMathematics* roiParcellation = vtkImageMathematics::New();
      roiParcellation->SetInput1(roiMap->GetOutput());
      roiParcellation->SetInput2(castParcellation->GetOutput());
      roiParcellation->SetOperationToMultiply();
      roiParcellation->Update();

      vtkImageThreshold* changedSeg = vtkImageThreshold::New();
      changedSeg->SetInput(input);
      changedSeg->ThresholdBetween(childLabel, childLabel);
      changedSeg->ReplaceOutOff();
      changedSeg->SetInValue(0);
      changedSeg->Update();

      vtkImageMathematics* parcellatedSeg = vtkImageMathematics::New();
      parcellatedSeg->SetInput1(changedSeg->GetOutput());
      parcellatedSeg->SetInput2(roiParcellation->GetOutput());
      parcellatedSeg->SetOperationToAdd();
      parcellatedSeg->Update();

      segmentation->DeepCopy(parcellatedSeg->GetOutput());
      parcellatedSeg->Delete();
      changedSeg->Delete();
      roiParcellation->Delete();
      castParcellation->Delete();
      roiMap->Delete();
      input->Delete();
      }
    else
      {
      this->SubParcelateSegmentation(segmentation, childID);
      }

    }
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::AddDefaultTasksToList(const char* FilePath, std::vector<
    std::string> & DefaultTasksName, std::vector<std::string> & DefaultTasksFile, std::vector<
    std::string> & DefinePreprocessingTasksName, std::vector<std::string> & DefinePreprocessingTasksFile)
{
  vtkDirectory *dir = vtkDirectory::New();
  // Do not give out an error message here bc it otherwise comes up when loading slicer 
  // the path might simply not be created !

  if (!dir->Open(FilePath))
    {
    dir->Delete();
    return;
    }

  int numberOfFiles = dir->GetNumberOfFiles();

  for (int i = 0; i < numberOfFiles; i++)
    {
    vtksys_stl::string filename = dir->GetFile(i);

    // do nothing if file is ".", ".." 
    if (strcmp(filename.c_str(), ".") && strcmp(filename.c_str(), ".."))
      {
      //  {
      //  continue;
      //  }
      vtksys_stl::string tmpFullFileName = vtksys_stl::string(FilePath)
          + vtksys_stl::string("/") + filename.c_str();
      vtksys_stl::string fullFileName =
          vtksys::SystemTools::ConvertToOutputPath(tmpFullFileName.c_str());

      // if it has a .mrml extension but is a directory, do nothing
      if (!vtksys::SystemTools::FileIsDirectory(fullFileName.c_str()))
        {

        if (!strcmp(
            vtksys::SystemTools::GetFilenameExtension(filename.c_str()).c_str(),
            ".mrml") && (filename.compare(0, 1, "_")))
          {
          // Generate Name of Task from File name
          vtksys_stl::string taskName =
              this->MRMLManager->TurnDefaultMRMLFileIntoTaskName(
                  filename.c_str());
          // make sure that file is not already in the list
          // we loop through the list and set existFlag to 1 if it exists already
          int existFlag = 0;
          // we need a new index for this inner loop *grrrrr took me long to find this one
          for (int j = 0; j < int(DefaultTasksName.size()); j++)
            {
            if (!DefaultTasksName[j].compare(taskName))
              {
              existFlag = 1;
              }
            }
          if (!existFlag)
            {
            // Add to List if it does not exist
            DefaultTasksFile.push_back(fullFileName);
            DefaultTasksName.push_back(taskName);
            }
          }
        else if ((!strcmp(vtksys::SystemTools::GetFilenameExtension(
            filename.c_str()).c_str(), ".tcl"))
            && (filename.compare(0, 1, "_")) && strcmp(filename.c_str(),
            vtkMRMLEMSGlobalParametersNode::GetDefaultTaskTclFileName()))
          {
          // Generate Name of Task from File name
          vtksys_stl::string taskName =
              this->MRMLManager->TurnDefaultTclFileIntoPreprocessingName(
                  filename.c_str());
          // make sure that file is not already in the list
          // we loop through the list and set existFlag to 1 if it exists already
          int existFlag = 0;
          // we need a new index for this inner loop *grrrrr took me long to find this one
          for (int j = 0; j < int(DefinePreprocessingTasksName.size()); j++)
            {
            if (!DefinePreprocessingTasksName[j].compare(taskName))
              {
              existFlag = 1;
              }
            }
          if (!existFlag)
            {
            // Add to List if it does not exist
            DefinePreprocessingTasksFile.push_back(fullFileName);
            DefinePreprocessingTasksName.push_back(taskName);
            }
          }
        } // check if it is not a directory
      } // check if the file is .,.. or does not have a .mrml extension
    } // loop through all the files

  dir->Delete();
}

//----------------------------------------------------------------------------
void vtkEMSegmentLogic::CreateOutputVolumeNode()
{

  // Version 1 - It is a little bit slower bc it creates image data that we do not need
  // (vtkSlicerApplication* app) 
  // vtkSlicerVolumesGUI *vgui = vtkSlicerVolumesGUI::SafeDownCast (app->GetModuleGUIByName ( "Volumes"));
  // if (!vgui)  
  // {
  //   vtkErrorMacro("CreateOutputVolumeNode: could not find vtkSlicerVolumesGUI "); 
  //   return;
  // }
  // vtkSlicerVolumesLogic* volLogic  = vgui->GetLogic();
  // if (!volLogic)  
  // {
  //   vtkErrorMacro("CreateOutputVolumeNode: could not find vtkSlicerVolumesLogic "); 
  //   return;
  // }
  //
  // vtkMRMLNode* snode = this->GetMRMLScene()->GetNodeByID(this->MRMLManager->GetTargetSelectedVolumeNthMRMLID(0));
  // vtkMRMLVolumeNode* vNode = vtkMRMLVolumeNode::SafeDownCast(snode);
  //
  // if (vNode == NULL)
  // {
  //   vtkErrorMacro("Invalid volume MRMLID: " << this->MRMLManager->GetTargetSelectedVolumeNthMRMLID(0));
  //   return;
  // }
  //  vtkMRMLScalarVolumeNode* outputNode = volLogic->CreateLabelVolume (this->GetMRMLScene(), vNode, "EM_MAP");

  // My version
  vtkMRMLScalarVolumeNode* outputNode = vtkMRMLScalarVolumeNode::New();
  outputNode->SetLabelMap(1);
  std::string uname = this->GetMRMLScene()->GetUniqueNameByString("EM_Map");
  outputNode->SetName(uname.c_str());
  this->GetMRMLScene()->AddNode(outputNode);

  vtkMRMLLabelMapVolumeDisplayNode* displayNode =
      vtkMRMLLabelMapVolumeDisplayNode::New();
  displayNode->SetScene(this->GetMRMLScene());
  this->GetMRMLScene()->AddNode(displayNode);
  displayNode->SetAndObserveColorNodeID(this->MRMLManager->GetColorNodeID());
  outputNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  displayNode->Delete();

  this->MRMLManager->SetOutputVolumeMRMLID(outputNode->GetID());
  outputNode->Delete();
}

//----------------------------------------------------------------------------
int vtkEMSegmentLogic::StartSegmentationWithoutPreprocessingAndSaving()
{
  //
  // make sure we're ready to start
  //
  ErrorMsg.clear();

  if (!this->GetMRMLManager()->GetWorkingDataNode()->GetAlignedTargetNodeIsValid()
      || !this->GetMRMLManager()->GetWorkingDataNode()->GetAlignedAtlasNodeIsValid())
    {
    ErrorMsg = "Preprocessing pipeline not up to date!  Aborting Segmentation.";
    vtkErrorMacro( << ErrorMsg );
    return EXIT_FAILURE;
    }

  // find output volume
  if (!this->GetMRMLManager()->GetNode())
    {
    ErrorMsg = "Template node is null---aborting segmentation.";
    vtkErrorMacro( << ErrorMsg );
    return EXIT_FAILURE;
    }
  vtkMRMLScalarVolumeNode *outVolume =
      this->GetMRMLManager()->GetOutputVolumeNode();
  if (outVolume == NULL)
    {
    ErrorMsg = "No output volume found---aborting segmentation.";
    vtkErrorMacro( << ErrorMsg );
    return EXIT_FAILURE;
    }

  //
  // Copy RASToIJK matrix, and other attributes from input to
  // output. Use first target volume as source for this data.
  //

  // get attributes from first target input volume
  const char* inMRLMID =
      this->GetMRMLManager()->GetTargetInputNode()->GetNthVolumeNodeID(0);
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(inMRLMID));
  if (inVolume == NULL)
    {
    ErrorMsg = "Can't get first target image.";
    vtkErrorMacro( << ErrorMsg);
    return EXIT_FAILURE;
    }

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  // if AMF flag is activated make sure that the segmentations of the first level are written out 
  int AMFFlag =
      this->GetMRMLManager()->GetGlobalParametersNode()->GetAMFSmoothing();
  if (AMFFlag)
    {
    this->GetMRMLManager()->PrintWeightOnForEntireTree();

    vtkIdType rootID = this->GetMRMLManager()->GetTreeRootNodeID();
    int printFreq = this->GetMRMLManager()->GetTreeNodePrintFrequency(rootID);
    if (printFreq != 1 && printFreq != -1)
      {
      this->GetMRMLManager()->SetTreeNodePrintFrequency(rootID, -1);
      }
    }

  //
  // create segmenter class
  //
  vtkImageEMLocalSegmenter* segmenter = vtkImageEMLocalSegmenter::New();
  if (segmenter == NULL)
    {
    ErrorMsg = "Could not create vtkImageEMLocalSegmenter pointer";
    vtkErrorMacro( << ErrorMsg );
    return EXIT_FAILURE;
    }

  //
  // copy mrml data to segmenter class
  //
  vtkstd::cout << "EMSEG: Copying data to algorithm class...";
  this->CopyDataToSegmenter(segmenter);
  vtkstd::cout << "DONE" << vtkstd::endl;

  if (this->GetDebug())
    {
    vtkstd::cout << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    vtkstd::cout << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    vtkstd::cout << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    vtkIndent indent;
    segmenter->PrintSelf(vtkstd::cout, indent);
    vtkstd::cout << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    vtkstd::cout << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    vtkstd::cout << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    }

  //
  // start segmentation
  //
  try
    {
    vtkstd::cout << "[Start] Segmentation algorithm..." << vtkstd::endl;
    segmenter->Update();
    vtkstd::cout << "[Done]  Segmentation algorithm." << vtkstd::endl;
    } catch (std::exception& e)
    {
    ErrorMsg = "Exception thrown during segmentation: " + std::string(e.what())
        + "\n";
    vtkErrorMacro( << ErrorMsg );
    return EXIT_FAILURE;
    }

  if (this->GetDebug())
    {
    vtkstd::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    vtkstd::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    vtkstd::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    segmenter->PrintSelf(vtkstd::cout, static_cast<vtkIndent> (0));
    vtkstd::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    vtkstd::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    vtkstd::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    }

  // POST PROCESSING 
  vtkstd::cout << "[Start] Postprocessing ..." << vtkstd::endl;
  vtkImageData* postProcessing = vtkImageData::New();
  postProcessing->ShallowCopy(segmenter->GetOutput());

  // AMF Smoothing 
  if (AMFFlag)
    {
    if (this->ActiveMeanField(segmenter, postProcessing) == EXIT_FAILURE)
      {
      postProcessing->Delete();
      return EXIT_FAILURE;
      }
    }
  // Subparcellation
  if (this->GetMRMLManager()->GetEnableSubParcellation())
    {
    vtkstd::cout << "=== Sub-Parcellation === " << vtkstd::endl;
    this->SubParcelateSegmentation(postProcessing,
        this->GetMRMLManager()->GetTreeRootNodeID());
    }

  // Island Removal
  if (this->GetMRMLManager()->GetMinimumIslandSize() > 1)
    {
    vtkstd::cout << "=== Island removal === " << vtkstd::endl;
    vtkImageData* input = vtkImageData::New();
    input->DeepCopy(postProcessing);
    vtkImageIslandFilter* islandFilter = vtkImageIslandFilter::New();
    islandFilter->SetInput(input);
    islandFilter->SetIslandMinSize(
        this->GetMRMLManager()->GetMinimumIslandSize());
    if (this->GetMRMLManager()->GetIsland2DFlag())
      {
      islandFilter->SetNeighborhoodDim2D();
      vtkstd::cout << "2D Neighborhood Island activated" << vtkstd::endl;
      }
    else
      {
      islandFilter->SetNeighborhoodDim3D();
      }
    islandFilter->SetPrintInformation(1);
    islandFilter->Update();
    postProcessing->DeepCopy(islandFilter->GetOutput());
    islandFilter->Delete();
    input->Delete();
    }
  vtkstd::cout << "[Done] Postprocessing" << vtkstd::endl;
  //
  // copy result to output volume
  //

  // set output of the filter to VolumeNode's ImageData

  outVolume->SetAndObserveImageData(postProcessing);
  postProcessing->Delete();
  // make sure the output volume is a labelmap

  if (!outVolume->GetLabelMap())
    {
    vtkWarningMacro("Changing output image to labelmap");
    outVolume->LabelMapOn();
    }

  // vtkstd::cout << "=== Define Display Node  === " << vtkstd::endl;

  vtkMRMLVolumeDisplayNode *outDisplayNode =
      vtkMRMLVolumeDisplayNode::SafeDownCast(outVolume->GetDisplayNode());
  if (!outDisplayNode)
    {
    vtkWarningMacro("Did not define lookup table bc display node is not defined ");
    }
  else
    {
    const char* colorID = this->GetMRMLManager()->GetColorNodeID();
    if (colorID && strcmp(outDisplayNode->GetColorNodeID(), colorID) != 0)
      {
      outDisplayNode->SetAndObserveColorNodeID(colorID);
      }
    }

  // vtkstd::cout << "=== Cleanup  === " << vtkstd::endl;
  outVolume->SetModifiedSinceRead(1);
  //
  // clean up
  //
  segmenter->Delete();
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
bool vtkEMSegmentLogic::SaveIntermediateResults(vtkSlicerApplicationLogic *appLogic)
{
  //
  // get output directory
  std::string
      outputDirectory(this->GetMRMLManager()->GetSaveWorkingDirectory());

  if (!vtksys::SystemTools::FileExists(outputDirectory.c_str()))
    {
    // try to create directory
    bool createdOK = true;
    createdOK = vtksys::SystemTools::MakeDirectory(outputDirectory.c_str());
    if (!createdOK)
      {
      std::string msg = "SaveIntermediateResults: could not create "
          + outputDirectory + "!";
      ErrorMsg += msg + "\n";
      vtkErrorMacro(<< msg);
      return false;
      }
    }

  // check again whether or not directory exists
  if (!vtksys::SystemTools::FileExists(outputDirectory.c_str()))
    {
    std::string msg = "SaveIntermediateResults: Directory " + outputDirectory
        + " does not exist !";
    ErrorMsg += msg + "\n";
    vtkErrorMacro(<< msg);
    return false;
    }

  //
  // package EMSeg-related parameters together and write them to disk
  bool writeSuccessful = this->PackageAndWriteData(appLogic,
      outputDirectory.c_str());

  return writeSuccessful;
}

//----------------------------------------------------------------------------
bool vtkEMSegmentLogic::PackageAndWriteData(vtkSlicerApplicationLogic* appLogic, const char* packageDirectory)
{
  //
  // create a scene and copy the EMSeg related nodes to it
  //
  if (!this->GetMRMLManager())
    {
    return false;
    }

  std::string outputDirectory(packageDirectory);
  std::string mrmlURL(outputDirectory + "/_EMSegmenterScene.mrml");

  vtkMRMLScene* newScene = vtkMRMLScene::New();
  newScene->SetRootDirectory(packageDirectory);
  newScene->SetURL(mrmlURL.c_str());

  vtkDataIOManagerLogic* dataIOManagerLogic = vtkDataIOManagerLogic::New();
  cout << " DEBUG" << endl;

  this->GetSlicerCommonInterface()->AddDataIOToScene(newScene, appLogic,
      dataIOManagerLogic);

  this->GetMRMLManager()->CopyEMRelatedNodesToMRMLScene(newScene);

  // update filenames to match standardized package structure
  this->CreatePackageFilenames(newScene, packageDirectory);

  //
  // create directory structure on disk
  bool errorFlag = !this->CreatePackageDirectories(packageDirectory);

  if (errorFlag)
    {
    vtkErrorMacro("PackageAndWriteData: failed to create directories");
    }
  else
    {
    //
    // write the scene out to disk
    errorFlag = !this->WritePackagedScene(newScene);
    if (errorFlag)
      {
      vtkErrorMacro("PackageAndWrite: failed to write scene");
      }
    }

  this->GetSlicerCommonInterface()->RemoveDataIOFromScene(newScene,
      dataIOManagerLogic);

  dataIOManagerLogic->Delete();
  dataIOManagerLogic = NULL;
  newScene->Delete();

  return !errorFlag;
}

//----------------------------------------------------------------------------
// This function is used for the UpdateButton in vtkEMSegmentParametersSetStep
vtkstd::string vtkEMSegmentLogic::GetTemporaryTaskDirectory()
{
  // FIXME, what happens if user has no write permission to this directory
  std::string taskDir("");

  const char* tmpDir =
      this->GetSlicerCommonInterface()->GetTemporaryDirectory();

  const char* svn_revision =
      this->GetSlicerCommonInterface()->GetRepositoryRevision();

  if (tmpDir)
    {
    std::string tmpTaskDir(std::string(tmpDir) + "/"
        + std::string(svn_revision) + std::string("/EMSegmentTask"));
    taskDir = vtksys::SystemTools::ConvertToOutputPath(tmpTaskDir.c_str());
    }
  else
    {
    // FIXME, make sure there is always a valid temporary directory
    vtkErrorMacro("GetTemporaryTaskDirectory:: Temporary Directory was not defined");
    }
  return taskDir;
}

//----------------------------------------------------------------------------
// Updates the .tcl Tasks from an external website and replaces the content
// in $tmpDir/EMSegmentTask (e.g. /home/Slicer3USER/EMSegmentTask)
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
int vtkEMSegmentLogic::UpdateTasks()
{

    { // we want our own scope in this one :D

    //
    // ** THE URL **
    //
    // the url to the EMSegment task repository
    //std::string taskRepository = "http://people.csail.mit.edu/pohl/EMSegmentUpdates/";
    std::string taskRepository = "http://slicer.org/EMSegmentUpdates/3.6.3/";

    //
    // ** PATH MANAGEMENT **
    //
    // the Slicer temporary directory
    const char* tmpDir =
        this->GetSlicerCommonInterface()->GetTemporaryDirectory();

    if (!tmpDir)
      {
      vtkErrorMacro("UpdateTasksCallback: Temporary directory is not defined!");
      return 0;
      }

    const char* svn_revision =
        this->GetSlicerCommonInterface()->GetRepositoryRevision();

    // also add the manifest filename
    std::string tmpManifestFilename(std::string(tmpDir) + "/" + std::string(
        svn_revision) + std::string("/EMSegmentTasksManifest.html"));
    std::string manifestFilename = vtksys::SystemTools::ConvertToOutputPath(
        tmpManifestFilename.c_str());

    // and add the EMSegmentTask directory
    std::string taskDir = this->GetTemporaryTaskDirectory();
    //
    // ** HTTP ACCESS **
    //
    // our HTTP handler
    //vtkHTTPHandler* httpHandler = vtkHTTPHandler::New();
    vtkHTTPHandler* httpHandler =
        this->GetSlicerCommonInterface()->GetHTTPHandler(this->GetMRMLScene());

    // prevent funny behavior on windows with the side-effect of more network resources are used
    // (o_o) who cares about traffic or the tcp/ip ports? *g
    httpHandler->SetForbidReuse(1);

    // safe-check if the handler can really handle the hardcoded uri protocol
    if (!httpHandler->CanHandleURI(taskRepository.c_str()))
      {
      vtkErrorMacro("UpdateTasksCallback: Invalid URI specified and you can't do anything about it bcuz it is *hardcoded*!")
      return 0;
      }

    //
    // ** THE ACTION STARTS **
    //
    // make sure we can access the task repository
    // TODO: this function will be provided by Wendy sooner or later :D
    //       for now, we just assume that we are on-line!


    // get the directory listing of the EMSegment task repository and save it as $tmpDir/EMSegmentTasksManifest.html.
    // the manifest always gets overwritten, but nobody should care
    httpHandler->StageFileRead(taskRepository.c_str(), manifestFilename.c_str());

    // sanity checks: if manifestFilename does not exist or size<1, exit here before it is too late!
    if (!vtksys::SystemTools::FileExists(manifestFilename.c_str())
        || vtksys::SystemTools::FileLength(manifestFilename.c_str()) < 1)
      {
      vtkErrorMacro("UpdateTasksCallback: Could not get the manifest! Try again later..")
      return 0;
      }

    // what happens now? answer: a three-step-processing chain!!!
    // (1) now we got the manifest and we can parse it for filenames of EMSegment tasks.
    // (2) then, download these files and copy them to our $tmpDir.
    //     after we are sure we got the files (we can not be really sure but we can check if some files where downloaded),
    // (3) we delete all old files in $taskDir and then copy our newly downloaded EMSegment tasks from $tmpDir to $taskDir.
    // sounds good!

    // 1) open the manifest and get the filenames
    std::ifstream fileStream(manifestFilename.c_str());
    std::string htmlManifestAsString;
    if (!fileStream.fail())
      {
      fileStream.seekg(0, std::ios::end);
      size_t length = fileStream.tellg();
      fileStream.seekg(0, std::ios::beg);
      char* htmlManifest = new char[length + 1];
      fileStream.read(htmlManifest, length);
      htmlManifest[length] = '\n';
      htmlManifestAsString = std::string(htmlManifest);
      delete[] htmlManifest;
      }

    fileStream.close();

    // when C++0x is released, we could easily do something like this to filter out the .tcl and .mrml filenames:
    //  cmatch regexResult;
    //  regex tclExpression("(\w*-*)+.tcl(?!\")");
    //  regex_search(htmlManifest, regexResult, tclExpression);
    //  regex mrmlExpression("(\w*-*)+.mrml(?!\")");
    //  regex_search(htmlManifest, regexResult, mrmlExpression);
    // but right now, we have to manually parse the string.
    // at least we can use std::string methods :D
    //
    // Fix for recent webservers does not include a space after HTML tags
    std::string beginTaskFilenameTag(".tcl\">");
    std::string endTaskFilenameTag(".tcl</a>");
    std::string beginMrmlFilenameTag(".mrml\">");
    std::string endMrmlFilenameTag(".mrml</a>");

    bool tclFilesExist = false;
    bool mrmlFilesExist = false;

    std::vector < std::string > taskFilenames;
    std::vector < std::string > mrmlFilenames;

    std::string::size_type beginTaskFilenameIndex = htmlManifestAsString.find(
        beginTaskFilenameTag, 0);

    // the loop for .tcl files
    while (beginTaskFilenameIndex != std::string::npos)
      {
      // as long as we find the beginning of a filename, do the following..

      // find the corresponding end
      std::string::size_type endTaskFilenameIndex = htmlManifestAsString.find(
          endTaskFilenameTag, beginTaskFilenameIndex);

      if (endTaskFilenameIndex == std::string::npos)
        {
        vtkErrorMacro("UpdateTasksCallback: Error during parsing! There was no end *AAAAAAAAAAAAAAAAAAAAHHHH*")
        return 0;
        }

      // now get the string between begin and end, then add it to the vector
      taskFilenames.push_back(htmlManifestAsString.substr(
          beginTaskFilenameIndex + beginTaskFilenameTag.size(),
          endTaskFilenameIndex - (beginTaskFilenameIndex
              + beginTaskFilenameTag.size())));

      // and try to find the next beginTag
      beginTaskFilenameIndex = htmlManifestAsString.find(beginTaskFilenameTag,
          endTaskFilenameIndex);
      }

    // enable copying of .tcl files if they exist
    if (taskFilenames.size() != 0)
      {
      tclFilesExist = true;
      }

    std::string::size_type beginMrmlFilenameIndex = htmlManifestAsString.find(
        beginMrmlFilenameTag, 0);

    // the loop for .mrml files
    while (beginMrmlFilenameIndex != std::string::npos)
      {
      // as long as we find the beginning of a filename, do the following..

      // find the corresponding end
      std::string::size_type endMrmlFilenameIndex = htmlManifestAsString.find(
          endMrmlFilenameTag, beginMrmlFilenameIndex);

      if (endMrmlFilenameIndex == std::string::npos)
        {
        vtkErrorMacro("UpdateTasksCallback: Error during parsing! There was no end *AAAAAAAAAAAAAAAAAAAAHHHH*")
        return 0;
        }

      // now get the string between begin and end, then add it to the vector
      mrmlFilenames.push_back(htmlManifestAsString.substr(
          beginMrmlFilenameIndex + beginMrmlFilenameTag.size(),
          endMrmlFilenameIndex - (beginMrmlFilenameIndex
              + beginMrmlFilenameTag.size())));

      // and try to find the next beginTag
      beginMrmlFilenameIndex = htmlManifestAsString.find(beginMrmlFilenameTag,
          endMrmlFilenameIndex);
      }

    // enable copying of .mrml files if they exist
    if (mrmlFilenames.size() != 0)
      {
      mrmlFilesExist = true;
      }

    // 2) loop through the vector and download the task files and the mrml files to the $tmpDir
    std::string currentTaskUrl;
    std::string currentTaskName;
    std::string currentTaskFilepath;

    std::string currentMrmlUrl;
    std::string currentMrmlName;
    std::string currentMrmlFilepath;

    if (tclFilesExist)
      {
      // loop for .tcl
      for (std::vector<std::string>::iterator i = taskFilenames.begin(); i
          != taskFilenames.end(); ++i)
        {

        currentTaskName = *i;

        // sanity checks: if the filename is "", exit here before it is too late!
        if (!strcmp(currentTaskName.c_str(), ""))
          {
          vtkErrorMacro("UpdateTasksCallback: At least one filename was empty, get outta here NOW! *AAAAAAAAAAAAAAAAAHHH*")
          return 0;
          }

        // generate the url of this task
        currentTaskUrl = std::string(taskRepository + currentTaskName
            + std::string(".tcl"));

        // generate the destination filename of this task in $tmpDir
        currentTaskFilepath = std::string(tmpDir + std::string("/")
            + currentTaskName + std::string(".tcl"));

        // and get the content and save it to $tmpDir
        httpHandler->StageFileRead(currentTaskUrl.c_str(),
            currentTaskFilepath.c_str());

        // sanity checks: if the downloaded file does not exist or size<1, exit here before it is too late!
        if (!vtksys::SystemTools::FileExists(currentTaskFilepath.c_str())
            || vtksys::SystemTools::FileLength(currentTaskFilepath.c_str()) < 1)
          {
          vtkErrorMacro("UpdateTasksCallback: At least one file was not downloaded correctly! Aborting.. *beepbeepbeep*")
          return 0;
          }

        }
      }

    if (mrmlFilesExist)
      {
      // loop for .mrml
      for (std::vector<std::string>::iterator i = mrmlFilenames.begin(); i
          != mrmlFilenames.end(); ++i)
        {

        currentMrmlName = *i;

        // sanity checks: if the filename is "", exit here before it is too late!
        if (!strcmp(currentMrmlName.c_str(), ""))
          {
          vtkErrorMacro("UpdateTasksCallback: At least one filename was empty, get outta here NOW! *AAAAAAAAAAAAAAAAAHHH*")
          return 0;
          }

        // generate the url of this mrml file
        currentMrmlUrl = std::string(taskRepository + currentMrmlName
            + std::string(".mrml"));

        // generate the destination filename of this task in $tmpDir
        currentMrmlFilepath = std::string(tmpDir + std::string("/")
            + currentMrmlName + std::string(".mrml"));

        // and get the content and save it to $tmpDir
        httpHandler->StageFileRead(currentMrmlUrl.c_str(),
            currentMrmlFilepath.c_str());

        // sanity checks: if the downloaded file does not exist or size<1, exit here before it is too late!
        if (!vtksys::SystemTools::FileExists(currentMrmlFilepath.c_str())
            || vtksys::SystemTools::FileLength(currentMrmlFilepath.c_str()) < 1)
          {
          vtkErrorMacro("UpdateTasksCallback: At least one file was not downloaded correctly! Aborting.. *beepbeepbeep*")
          return 0;
          }

        }
      }

    // we got the .tcl files and the .mrml files now at a safe location and they have at least some content :P
    // this makes it safe to delete all old EMSegment tasks and activate the new one :D

    // OMG did you realize that this is a kind of backdoor to take over your home directory?? the
    // downloaded .tcl files get sourced later and can do whatever they want to do!! but pssst let's keep it a secret
    // option for a Slicer backdoor :) on the other hand, the EMSegment tasks repository will be monitored closely and is not
    // public, but what happens if someone changes the URL to the repository *evilgrin*

    // 3) copy the $taskDir to a backup folder, delete the $taskDir. and create it again. then, move our downloaded files to it

    // purge, NOW!! but only if the $taskDir exists..
    if (vtksys::SystemTools::FileExists(taskDir.c_str()))
      {
      // create a backup of the old taskDir
      std::string backupTaskDir(taskDir + std::string("_old"));
      if (!vtksys::SystemTools::CopyADirectory(taskDir.c_str(),
          backupTaskDir.c_str()))
        {
        vtkErrorMacro("UpdateTasksCallback: Could not create backup " << backupTaskDir.c_str() << "! This is very bad, we abort the update..")
        return 0;
        }

      if (!vtksys::SystemTools::RemoveADirectory(taskDir.c_str()))
        {
        vtkErrorMacro("UpdateTasksCallback: Could not delete " << taskDir.c_str() << "! This is very bad, we abort the update..")
        return 0;
        }
      }

    // check if the taskDir is gone now!
    if (!vtksys::SystemTools::FileExists(taskDir.c_str()))
      {
      // the $taskDir does not exist, so create it
      bool couldCreateTaskDir = vtksys::SystemTools::MakeDirectory(
          taskDir.c_str());

      // sanity checks: if the directory could not be created, something is wrong!
      if (!couldCreateTaskDir)
        {
        vtkErrorMacro("UpdateTasksCallback: Could not (re-)create the EMSegmentTask directory: " << taskDir.c_str())
        return 0;
        }
      }

    std::string currentTaskDestinationFilepath;
    std::string currentMrmlDestinationFilepath;

    if (tclFilesExist)
      {
      // now move the downloaded .tcl files to the $taskDir
      for (std::vector<std::string>::iterator i = taskFilenames.begin(); i
          != taskFilenames.end(); ++i)
        {

        currentTaskName = *i;

        // generate the destination filename of this task in $tmpDir
        currentTaskFilepath = std::string(tmpDir + std::string("/")
            + currentTaskName + std::string(".tcl"));

        // generate the destination filename of this task in $taskDir
        currentTaskDestinationFilepath = std::string(taskDir + std::string("/")
            + currentTaskName + std::string(".tcl"));

        if (!vtksys::SystemTools::CopyFileAlways(currentTaskFilepath.c_str(),
            currentTaskDestinationFilepath.c_str()))
          {
          vtkErrorMacro("UpdateTasksCallback: Could not copy at least one downloaded task file. Everything is lost now! Sorry :( Just kidding: there was a backup in " << taskDir << "!")
          return 0;
          }
        }
      }

    if (mrmlFilesExist)
      {
      // now move the downloaded .mrml files to the $taskDir
      for (std::vector<std::string>::iterator i = mrmlFilenames.begin(); i
          != mrmlFilenames.end(); ++i)
        {

        currentMrmlName = *i;

        // generate the destination filename of this task in $tmpDir
        currentMrmlFilepath = std::string(tmpDir + std::string("/")
            + currentMrmlName + std::string(".mrml"));

        // generate the destination filename of this task in $taskDir
        currentMrmlDestinationFilepath = std::string(taskDir + std::string("/")
            + currentMrmlName + std::string(".mrml"));

        if (!vtksys::SystemTools::CopyFileAlways(currentMrmlFilepath.c_str(),
            currentMrmlDestinationFilepath.c_str()))
          {
          vtkErrorMacro("UpdateTasksCallback: Could not copy at least one downloaded mrml file. Everything is lost now! Sorry :( Just kidding: there was a backup in " << taskDir << "!")
          return 0;
          }
        }
      }

    //
    // ** ALL DONE, NOW CLEANUP **
    //

    } // now go for destruction, donkey!!

  return 1;
}

//----------------------------------------------------------------------------
int vtkEMSegmentLogic::StartSegmentationWithoutPreprocessing(vtkSlicerApplicationLogic *appLogic)
{
  int flag = this->StartSegmentationWithoutPreprocessingAndSaving();
  ErrorMsg = this->GetErrorMessage();
  if (flag == EXIT_FAILURE)
    {
    return EXIT_FAILURE;
    }

  //
  // save intermediate results
  if (this->GetMRMLManager()->GetSaveIntermediateResults())
    {
    vtkstd::cout << "[Start] Saving intermediate results..." << vtkstd::endl;
    bool savedResults = this->SaveIntermediateResults(appLogic);
    vtkstd::cout << "[Done]  Saving intermediate results." << vtkstd::endl;
    if (!savedResults)
      {
      std::string msg = "Error writing intermediate results";
      ErrorMsg += msg + "\n";
      vtkErrorMacro( << msg);
      return EXIT_FAILURE;
      }
    }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
// New Task Specific Pipeline
//----------------------------------------------------------------------------

int vtkEMSegmentLogic::SourceTclFile(const char *tclFile)
{
  return this->GetSlicerCommonInterface()->SourceTclFile(tclFile);
}

//----------------------------------------------------------------------------
const char* vtkEMSegmentLogic::GetTemporaryDirectory()
{
  return this->GetSlicerCommonInterface()->GetTemporaryDirectory();
}

//----------------------------------------------------------------------------
const char* vtkEMSegmentLogic::GetPluginsDirectory()
{
  return this->GetSlicerCommonInterface()->GetPluginsDirectory();
}

//----------------------------------------------------------------------------
vtkstd::string vtkEMSegmentLogic::GetTclTaskDirectory()
{
  //workaround for the mrml library, we need to have write access to this folder
  const char* tmp_dir =
      this->GetSlicerCommonInterface()->GetTemporaryDirectory();

  if (tmp_dir)
    {
    vtkstd::string copied_task_dir;
    copied_task_dir += tmp_dir;
    copied_task_dir += "/EMSegmentTaskCopy";
    /**
     * Copy content directory to another directory with all files and
     * sub-directories.  If the "always" argument is true all files are
     * always copied.  If it is false, only files that have changed or
     * are new are copied.
     */
    // copy not always, only new files
    // Later do automatically
    vtkstd::string orig_task_dir = vtkstd::string(
        this->GetModuleShareDirectory()) + vtkstd::string("/Tasks");

    if (!vtksys::SystemTools::CopyADirectory(orig_task_dir.c_str(),
        copied_task_dir.c_str(), false))
      {
      cout << "GetTclTaskDirectory:: Couldn't copy task directory "
          << orig_task_dir.c_str() << " to " << copied_task_dir.c_str() << endl;
      vtkErrorMacro("GetTclTaskDirectory:: Couldn't copy task directory " << orig_task_dir.c_str() << " to " << copied_task_dir.c_str());
      return vtksys::SystemTools::ConvertToOutputPath("");
      }
    return copied_task_dir;
    }
  else
    {
    // FIXME, make sure there is always a valid temporary directory
    vtkErrorMacro("GetTclTaskDirectory:: Tcl Task Directory was not found, set temporary directory first");
    }

  // return empty string if not found
  return vtksys::SystemTools::ConvertToOutputPath("");
}

//----------------------------------------------------------------------------
int vtkEMSegmentLogic::SourceTaskFiles()
{
  vtkstd::string generalFile = this->DefineTclTaskFullPathName(
      vtkMRMLEMSGlobalParametersNode::GetDefaultTaskTclFileName());
  vtkstd::string specificFile = std::string(this->DefineTclTaskFileFromMRML());
  cout << "Sourcing general Task file : " << generalFile.c_str() << endl;
  // Have to first source the default file to set up the basic structure"
  if (this->SourceTclFile(generalFile.c_str()))
    {
    return 1;
    }
  // Now we overwrite anything from the default
  if (specificFile.compare(generalFile))
    {
    cout << "Sourcing task specific file: " << specificFile << endl;
    return this->SourceTclFile(specificFile.c_str());
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkEMSegmentLogic::SourcePreprocessingTclFiles()
{
  if (this->SourceTaskFiles())
    {
    return 1;
    }
  // Source all files here as we otherwise sometimes do not find the function as Tcl did not finish sourcing but our cxx file is already trying to call the function
  vtkstd::string tclFile = this->GetModuleShareDirectory();

  // on Slicer3 _WIN32 is defined, on Slicer4 WIN32 is defined
#if defined(_WIN32) || defined(WIN32)
  tclFile.append("\\Tcl\\EMSegmentAutoSample.tcl");
#else
  tclFile.append("/Tcl/EMSegmentAutoSample.tcl");
#endif
  return this->SourceTclFile(tclFile.c_str());
}

//----------------------------------------------------------------------------
const char* vtkEMSegmentLogic::DefineTclTaskFileFromMRML()
{
  std::string tclFile("");
  this->StringHolder = this->DefineTclTaskFullPathName(
                                                       this->GetMRMLManager()->GetTclTaskFilename());

  if (!vtksys::SystemTools::FileExists(this->StringHolder.c_str())
      || vtksys::SystemTools::FileIsDirectory(this->StringHolder.c_str()))
    {

      cout << "vtkEMSegmentTclConnector::DefineTclTaskFileFromMRML: "
           << this->StringHolder.c_str() << " does not exist - using default file" << endl;

      this->StringHolder = this->DefineTclTaskFullPathName(
           vtkMRMLEMSGlobalParametersNode::GetDefaultTaskTclFileName());
    }
  return this->StringHolder.c_str();
}

//----------------------------------------------------------------------------
// Make sure you source EMSegmentAutoSample.tcl
int vtkEMSegmentLogic::ComputeIntensityDistributionsFromSpatialPrior()
{
  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType> NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;

  this->GetMRMLManager()->GetListOfTreeNodeIDs(
      this->GetMRMLManager()->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
    if (this->GetMRMLManager()->GetTreeNodeIsLeaf(*i))
      {
      this->UpdateIntensityDistributionAuto(*i);
      }
    }
  return 0;
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::UpdateIntensityDistributionAuto(vtkIdType nodeID)
{

  if (!this->GetMRMLManager()->GetTreeNodeSpatialPriorVolumeID(nodeID))
    {
    vtkWarningMacro("Nothing to update for " << nodeID << " as atlas is not defined");
    return;
    }

  vtkMRMLVolumeNode* atlasNode =
      this->GetMRMLManager()->GetAlignedSpatialPriorFromTreeNodeID(nodeID);
  if (!atlasNode)
    {
    vtkErrorMacro("Atlas not yet aligned for " << nodeID << " ! ");
    return;
    }

  // get working node
  vtkMRMLEMSVolumeCollectionNode* workingTarget = NULL;
  if (this->GetMRMLManager()->GetWorkingDataNode()->GetAlignedTargetNode()
      && this->GetMRMLManager()->GetWorkingDataNode()->GetAlignedTargetNodeIsValid())
    {
    workingTarget
        = this->GetMRMLManager()->GetWorkingDataNode()->GetAlignedTargetNode();
    }
  else
    {
    vtkErrorMacro("Cannot update intensity distribution bc Aligned Target is not correctly defined for node " << nodeID);
    return;
    }

  int numTargetImages = workingTarget->GetNumberOfVolumes();

  // Sample
    {
    vtkstd::stringstream CMD;

    CMD << "::EMSegmenterAutoSampleTcl::EMSegmentGaussCurveCalculationFromID "
        << this->GetSlicerCommonInterface()->GetTclNameFromPointer(this)
        << " 0.95 1 { ";

    for (int i = 0; i < numTargetImages; i++)
      {
      CMD << workingTarget->GetNthVolumeNodeID(i) << " ";
      }
    CMD << " } ";
    CMD << atlasNode->GetID() << " {"
        << this->GetMRMLManager()->GetTreeNodeName(nodeID) << "}";
    // cout << CMD.str().c_str() << endl;


    if (atoi(this->GetSlicerCommonInterface()->EvaluateTcl(CMD.str().c_str())))
      {
      return;
      }

    }

  //
  // propagate data to mrml node
  //

  vtkMRMLEMSTreeParametersLeafNode* leafNode =
      this->GetMRMLManager()->GetTreeParametersLeafNode(nodeID);
  for (int r = 0; r < numTargetImages; ++r)
    {
      { // own scope starts

      std::ostringstream os;
      os << "expr $::EMSegment(GaussCurveCalc,Mean,";
      os << r;
      os << ")";

      double value = atof(this->GetSlicerCommonInterface()->EvaluateTcl(
          os.str().c_str()));

      cout << "::::::LOGMEAN:" << value << endl;

      leafNode->SetLogMean(r, value);
      } // own scope ends


    for (int c = 0; c < numTargetImages; ++c)
      {

      std::ostringstream os;
      os << "expr $::EMSegment(GaussCurveCalc,Covariance,";
      os << r;
      os << ",";
      os << c;
      os << ")";

      double value = atof(this->GetSlicerCommonInterface()->EvaluateTcl(
          os.str().c_str()));

      leafNode->SetLogCovariance(r, c, value);

      } // for


    } // for
}

//----------------------------------------------------------------------------
std::string vtkEMSegmentLogic::DefineTclTaskFullPathName(const char* TclFileName)
{
  //  std::string task_dir = this->GetTclTaskDirectory(app);
  //  cout << "TEST 1" << task_dir << " " << vtksys::SystemTools::FileExists(task_dir.c_str()) << endl;
  vtkstd::string tmp_full_file_path = this->GetTclTaskDirectory()
      + vtkstd::string("/") + vtkstd::string(TclFileName);
  //  vtkstd::string full_file_path = vtksys::SystemTools::ConvertToOutputPath(tmp_full_file_path.c_str());
  if (vtksys::SystemTools::FileExists(tmp_full_file_path.c_str()))
    {
    return tmp_full_file_path;
    }

  tmp_full_file_path = this->GetTemporaryTaskDirectory() + vtkstd::string("/")
      + vtkstd::string(TclFileName);
  //  full_file_path = vtksys::SystemTools::ConvertToOutputPath(tmp_full_file_path.c_str());
  if (vtksys::SystemTools::FileExists(tmp_full_file_path.c_str()))
    {
    return tmp_full_file_path;
    }

  vtkErrorMacro("DefineTclTaskFullPathName : could not find tcl file with name  " << TclFileName );
  tmp_full_file_path = vtkstd::string("");
  return tmp_full_file_path;
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::CreateDefaultTasksList(std::vector<std::string> & DefaultTasksName, std::vector<
    std::string> & DefaultTasksFile, std::vector<std::string> & DefinePreprocessingTasksName, std::vector<
    std::string> & DefinePreprocessingTasksFile)
{
  DefaultTasksName.clear();
  DefaultTasksFile.clear();
  DefinePreprocessingTasksName.clear();
  DefinePreprocessingTasksFile.clear();

  this->AddDefaultTasksToList(this->GetTclTaskDirectory().c_str(),
      DefaultTasksName, DefaultTasksFile, DefinePreprocessingTasksName,
      DefinePreprocessingTasksFile);
  this->AddDefaultTasksToList(this->GetTemporaryTaskDirectory().c_str(),
      DefaultTasksName, DefaultTasksFile, DefinePreprocessingTasksName,
      DefinePreprocessingTasksFile);
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::RunAtlasCreator(vtkMRMLNode *mNode)
{
#ifdef Slicer3_USE_KWWIDGETS

  vtkMRMLAtlasCreatorNode *node = vtkMRMLAtlasCreatorNode::SafeDownCast(mNode);

  if (!node) {
    std::cout << "RunAtlasCreator: Error - not a valid MRML node!" << std::endl;
    return;
  }

  std::string pythonCommand = "";
  vtksys_stl::string module_path = std::string(
      this->GetSlicerCommonInterface()->GetBinDirectory());
  module_path += std::string("/../lib/Slicer3/Modules/AtlasCreator/");

  pythonCommand += "from Slicer import slicer\n";
  pythonCommand += "import sys\n";
  pythonCommand += "sys.path.append('";
  pythonCommand += vtksys::SystemTools::CollapseFullPath(module_path.c_str());
  pythonCommand += "')\n";

  pythonCommand += "from AtlasCreatorLogic import *\n";
  pythonCommand += "logic = AtlasCreatorLogic(0)\n";
  pythonCommand += "node = slicer.vtkMRMLAtlasCreatorNode()\n";

  pythonCommand += "node.SetOriginalImagesFilePathList('" + std::string(
      node->GetOriginalImagesFilePathList()) + "')\n";
  pythonCommand += "node.SetSegmentationsFilePathList('" + std::string(
      node->GetSegmentationsFilePathList()) + "')\n";
  pythonCommand += "node.SetOutputDirectory('" + std::string(
      node->GetOutputDirectory()) + "')\n";

  pythonCommand += "node.SetToolkit('" + std::string(node->GetToolkit())
      + "')\n";

  pythonCommand += "node.SetTemplateType('" + std::string(
      node->GetTemplateType()) + "')\n";

  std::stringstream convert;
  convert << node->GetDynamicTemplateIterations();
  pythonCommand += "node.SetDynamicTemplateIterations(" + convert.str() + ")\n";
  pythonCommand += "node.SetFixedTemplateDefaultCaseFilePath('" + std::string(
      node->GetFixedTemplateDefaultCaseFilePath()) + "')\n";

  convert.str("");
  convert << node->GetIgnoreTemplateSegmentation();
  pythonCommand += "node.SetIgnoreTemplateSegmentation(" + convert.str()
      + ")\n";

  pythonCommand += "node.SetLabelsList('" + std::string(node->GetLabelsList())
      + "')\n";

  pythonCommand += "node.SetRegistrationType('" + std::string(
      node->GetRegistrationType()) + "')\n";

  convert.str("");
  convert << node->GetSaveTransforms();
  pythonCommand += "node.SetSaveTransforms(" + convert.str() + ")\n";

  convert.str("");
  convert << node->GetDeleteAlignedImages();
  pythonCommand += "node.SetDeleteAlignedImages(" + convert.str() + ")\n";

  convert.str("");
  convert << node->GetDeleteAlignedSegmentations();
  pythonCommand += "node.SetDeleteAlignedSegmentations(" + convert.str()
      + ")\n";

  convert.str("");
  convert << node->GetNormalizeAtlases();
  pythonCommand += "node.SetNormalizeAtlases(" + convert.str() + ")\n";

  convert.str("");
  convert << node->GetNormalizeTo();
  pythonCommand += "node.SetNormalizeTo(" + convert.str() + ")\n";

  pythonCommand += "node.SetOutputCast('" + std::string(node->GetOutputCast())
      + "')\n";

  convert.str("");
  convert << node->GetPCAAnalysis();
  pythonCommand += "node.SetPCAAnalysis(" + convert.str() + ")\n";

  convert.str("");
  convert << node->GetPCAMaxEigenVectors();
  pythonCommand += "node.SetPCAMaxEigenVectors(" + convert.str() + ")\n";

  convert.str("");
  convert << node->GetPCACombine();
  pythonCommand += "node.SetPCACombine(" + convert.str() + ")\n";

  convert.str("");
  convert << node->GetUseCluster();
  pythonCommand += "node.SetUseCluster(" + convert.str() + ")\n";
  pythonCommand += "node.SetSchedulerCommand('" + std::string(
      node->GetSchedulerCommand()) + "')\n";

  convert.str("");
  convert << node->GetNumberOfThreads();
  pythonCommand += "node.SetNumberOfThreads(" + convert.str() + ")\n";

  convert.str("");
  convert << node->GetSkipRegistration();
  pythonCommand += "node.SetSkipRegistration(" + convert.str() + ")\n";
  pythonCommand += "node.SetExistingTemplate('" + std::string(
      node->GetExistingTemplate()) + "')\n";
  pythonCommand += "node.SetTransformsDirectory('" + std::string(
      node->GetTransformsDirectory()) + "')\n";

  convert.str("");
  convert << node->GetDebugMode();
  pythonCommand += "node.SetDebugMode(" + convert.str() + ")\n";

  convert.str("");
  convert << node->GetDryrunMode();
  pythonCommand += "node.SetDryrunMode(" + convert.str() + ")\n";

  convert.str("");
  convert << node->GetTestMode();
  pythonCommand += "node.SetTestMode(" + convert.str() + ")\n";

  convert.str("");
  convert << node->GetUseDRAMMS();
  pythonCommand += "node.SetUseDRAMMS(" + convert.str() + ")\n";

  pythonCommand += "logic.Start(node)\n";
  pythonCommand += "node = None\n";
  pythonCommand += "logic = None\n";

  this->GetSlicerCommonInterface()->EvaluatePython(pythonCommand.c_str());
#else

  // not supported in Slicer4
  std::cout << "The AtlasCreator is not supported in Slicer4." << std::endl;

#endif

}


//-----------------------------------------------------------------------------
int vtkEMSegmentLogic::ActiveMeanField(vtkImageEMLocalSegmenter* segmenter, vtkImageData* result)
{
  vtkIdType rootID = this->GetMRMLManager()->GetTreeRootNodeID();

  unsigned int numChildren = this->MRMLManager->GetTreeNodeNumberOfChildren(
      rootID);
  unsigned int probDim = numChildren + 1;
  int numCurves = int(probDim) - 1;

  vtkImageEMLocalSuperClass* rootNode = segmenter->GetHeadClass();

  //
  // Turn Probabilities into LogOdds
  //

  vtkImageLogOdds* logOdds = vtkImageLogOdds::New();
  logOdds->SetMode_Prob2Log();
  logOdds->SetDimProbSpace(probDim);
  logOdds->SetLogOddsInsidePositive();

  // Background  - dummy map - necessary so that real background pushes against foreground
  vtkImageEllipsoidSource* bgProbability = vtkImageEllipsoidSource::New();
  bgProbability->SetCenter(0, 0, 0);
  bgProbability->SetRadius(1, 1, 1);
  bgProbability->SetOutValue(0);
  bgProbability->SetInValue(0);
  bgProbability->SetOutputScalarTypeToFloat();

  labelListType labelList(probDim);
  for (unsigned int i = 0; i < numChildren; i++)
    {
    vtkImageEMLocalGenericClass* classNode =
        (vtkImageEMLocalGenericClass*) rootNode->GetClassListEntry(i);
    if (!classNode || !classNode->GetPosteriorImageData())
      {
      std::stringstream convert;
      convert << i;
      ErrorMsg
          = convert.str()
              + "th class node is NULL or no posterior defined -> could not proceed with postprocessing";
      vtkErrorMacro( <<ErrorMsg );
      return EXIT_FAILURE;
      }
    logOdds->SetProbabilities(i, classNode->GetPosteriorImageData());

    vtkIdType ID = this->MRMLManager->GetTreeNodeChildNodeID(rootID, i);
    if (this->MRMLManager->GetTreeNodeIsLeaf(ID))
      {
      labelList[i] = this->GetMRMLManager()->GetTreeNodeIntensityLabel(ID);
      }
    else
      {
      labelList[i] = 0;
      }
    if (!i)
      {
      bgProbability->SetWholeExtent(
          classNode->GetPosteriorImageData()->GetExtent());
      bgProbability->Update();
      logOdds->SetProbabilities(numChildren, bgProbability->GetOutput());
      labelList[numChildren] = 0;
      }
    }

  logOdds->Update();

  //
  // Set up  AMF
  //

  int* extent = logOdds->GetLogOdds(0) ->GetExtent();
  int numSlices = extent[5] - extent[4] + 1;

  vtkImageMultiLevelSets* aMF = vtkImageMultiLevelSets::New();
  aMF->SetMultiLevelVersion(0);
  aMF->SetNumberOfCurves(numCurves);
  aMF->SetprobCondWeightMin(0.05);
  aMF->SetLogCondIntensityInsideBright();

  typedef std::vector<vtkImageTranslateExtent *> logOddsShiftExtentType;
  logOddsShiftExtentType logOddsInShiftExtent(numCurves);

  typedef std::vector<vtkImageClip*> logOddsSliceType;
  logOddsSliceType logOddsInSlice(numCurves);

  typedef std::vector<vtkImageLevelSets*> levelsetCurvesInputType;
  levelsetCurvesInputType levelSetInCurves(numCurves);

  typedef std::vector<std::vector<vtkImageData*> > levelsetCurvesOutputType;
  levelsetCurvesOutputType levelSetOutCurves;
  levelSetOutCurves.resize(numCurves);

  typedef std::vector<vtkImageAppend*> logOddsVolumeType;
  logOddsVolumeType logOddsOutVolume(numCurves);

  // Just do 2D for Cardiology
  for (int i = 0; i < numCurves; i++)
    {

    logOddsInSlice[i] = vtkImageClip::New();
    logOddsInSlice[i]->SetInput(logOdds->GetLogOdds(i));
    logOddsInSlice[i]->ClipDataOn();

    logOddsInShiftExtent[i] = vtkImageTranslateExtent::New();
    logOddsInShiftExtent[i]->SetInput(logOddsInSlice[i]->GetOutput());

    levelSetInCurves[i] = vtkImageLevelSets::New();

    levelSetOutCurves[i].resize(numSlices);
    for (int j = 0; j < numSlices; j++)
      {
      levelSetOutCurves[i][j] = vtkImageData::New();
      }
    logOddsOutVolume[i] = vtkImageAppend::New();
    logOddsOutVolume[i]->SetAppendAxis(2);
    }

  vtkIdType matrixIndex = 0;
  for (vtkIdType sli = extent[4]; sli <= extent[5]; sli++)
    {
    //   for (int i = 6 ;  i < 6 ; i++ )

    for (int i = 0; i < numCurves; i++)
      {
      logOddsInSlice[i]->SetOutputWholeExtent(extent[0], extent[1], extent[2],
          extent[3], sli, sli);
      logOddsInSlice[i]->Update();
      logOddsInShiftExtent[i]->SetTranslation(0, 0, -sli);
      logOddsInShiftExtent[i]->Update();

      this->InitializeLevelSet(levelSetInCurves[i],
          logOddsInShiftExtent[i]->GetOutput());
      aMF->SetCurve(i, levelSetInCurves[i],
          logOddsInShiftExtent[i]->GetOutput(),
          logOddsInShiftExtent[i]->GetOutput(), 0.001, 0.001,
          levelSetOutCurves[i][matrixIndex]);
      }
    // Did not change spacing
    //        Volume(curve,$ID,resLevel,vol) SetSpacing 1 1 1
    aMF->InitParam();
    aMF->InitEvolution();

    //
    // Start AMF
    //
    cout << "\n=== Evolve Curves ===\n";
    cout << "Completed: ";
    for (int i = 0; i < 301; i++)
      {
      aMF->Iterate();
      if (!(i % 30) && i)
        {
        printf("%3d", i / 3);
        cout << "%";
        cout.flush();
        }
      }

    cout << "\n=== Completed Curve Evolution" << endl;

    // Will create leaks but was not called before either in the LevelSetSegmenterFct.tcl
    // aMF->EndEvolution();

    for (int i = 0; i < numCurves; i++)
      {
      logOddsOutVolume[i]->AddInput(levelSetOutCurves[i][matrixIndex]);
      }
    matrixIndex++;
    }

  //
  // Copy resulting Segmentation to result
  //

  vtkImageLogOdds* outcomeProb = vtkImageLogOdds::New();
  outcomeProb->SetMode_Log2Map();
  outcomeProb->SetLabelList(labelList);

  outcomeProb->SetMapMinProb(0.01);
  outcomeProb->SetLogOddsInsideNegative();
  outcomeProb->SetDimProbSpace(probDim);
  for (int i = 0; i < numCurves; i++)
    {
    logOddsOutVolume[i]->Update();
    outcomeProb->SetLogOdds(i, logOddsOutVolume[i]->GetOutput());
    }
  outcomeProb->Update();
  result->DeepCopy(outcomeProb->GetMap());

  //         vtkstd::stringstream filename;
  //         filename << "/tmp/log_sli_" << i <<  "_" << sli ;
  //         this->WriteImage(logOddsInSlice[i]->GetOutput(),filename.str().c_str());

  //
  // Clean up
  //
  // Delete  AMF
  outcomeProb->Delete();

  for (int i = 0; i < numCurves; i++)
    {

    logOddsOutVolume[i] ->Delete();

    for (int j = 0; j < numSlices; j++)
      {
      levelSetOutCurves[i][j]->Delete();
      }
    levelSetOutCurves[i].clear();

    levelSetInCurves[i]->Delete();

    logOddsInSlice[i]->Delete();
    logOddsInShiftExtent[i]->Delete();
    }
  logOddsOutVolume.clear();
  levelSetOutCurves.clear();
  levelSetInCurves.clear();
  logOddsInSlice.clear();
  logOddsInShiftExtent.clear();

  aMF->Delete();
  bgProbability->Delete();
  logOdds->Delete();

  // Set ImageData to null
  for (unsigned int i = 0; i < numChildren; i++)
    {
    vtkImageEMLocalGenericClass* classNode =
        (vtkImageEMLocalGenericClass*) rootNode->GetClassListEntry(i);
    if (!classNode)
      {
      std::stringstream convert;
      convert << i;
      ErrorMsg = convert.str()
          + "th class noide is NULL -> could not proceed with postprocessing";
      vtkErrorMacro(<< ErrorMsg );
      return EXIT_FAILURE;
      }
    classNode->SetPosteriorImageData(NULL);
    }
  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::InitializeLevelSet(vtkImageLevelSets* levelset, vtkImageData* initVolume)
{

  levelset->Setsavedistmap(0);
  levelset->SetNumIters(300);
  levelset->SetAdvectionCoeff(0);
  levelset->Setcoeff_curvature(0.04);
  levelset->Setballoon_coeff(1);
  levelset->Setballoon_value(0.025);
  levelset->SetDoMean(1);
  levelset->SetStepDt(0.8);

  vtkMultiThreader *thread = vtkMultiThreader::New();
  levelset->SetEvolveThreads(thread->GetGlobalDefaultNumberOfThreads());
  thread->Delete();

  levelset->SetBand(200);
  levelset->SetTube(199);
  levelset->SetReinitFreq(6);
  levelset->SetDimension(2 + (initVolume->GetDimensions()[2] > 1));
  levelset->SetHistoGradThreshold(0.2);
  levelset->Setadvection_scheme(2);
  levelset->SetDMmethod(4);
  levelset->SetNumGaussians(1);
  levelset->SetGaussian(0, 100, 15);
  levelset->SetProbabilityThreshold(0.3);
  levelset->SetProbabilityHighThreshold(0);
  levelset->SetinitImage(initVolume);
  levelset->SetInitThreshold(0);
  levelset->SetInitIntensityBright();
  levelset->SetlogCondIntensityCoefficient(0.001);
  levelset->SetlogCondIntensityImage(initVolume);
  levelset->SetLogCondIntensityInsideBright();
  levelset->SetprobCondWeightMin(0.05);
  levelset->Setverbose(0);
}

//-----------------------------------------------------------------------------
void vtkEMSegmentLogic::WriteImage(vtkImageData* Volume, const char* FileName)
{
  std::string name = std::string(FileName) + std::string(".nhdr");
  std::cout << "Write to file " << name.c_str() << endl;
  vtkITKImageWriter* export_iwriter = vtkITKImageWriter::New();
  export_iwriter->SetInput(Volume);
  export_iwriter->SetFileName(name.c_str());
  vtkMatrix4x4* mat = vtkMatrix4x4::New();
  export_iwriter->SetRasToIJKMatrix(mat);
  export_iwriter->SetUseCompression(1);
  export_iwriter->Write();
  mat->Delete();
  export_iwriter->Delete();
}

//-----------------------------------------------------------------------------
std::string vtkEMSegmentLogic::GetTasks()
{

  std::vector < std::string > pssDefaultTasksName;
  std::vector < std::string > pssDefaultTasksFile;
  std::vector < std::string > DefinePreprocessingTasksName;
  std::vector < std::string > DefinePreprocessingTasksFile;

  this->CreateDefaultTasksList(pssDefaultTasksName, pssDefaultTasksFile,
      DefinePreprocessingTasksName, DefinePreprocessingTasksFile);

  std::string tasksList = "";

  for (unsigned int i = 0; i < pssDefaultTasksName.size(); ++i)
    {
    tasksList += pssDefaultTasksName[i];
    tasksList += ":";
    tasksList += pssDefaultTasksFile[i];
    if (i != pssDefaultTasksName.size() - 1)
      tasksList += ",";
    }

  return tasksList;

}

//-----------------------------------------------------------------------------
std::string vtkEMSegmentLogic::GetPreprocessingTasks()
{

  std::vector < std::string > pssDefaultTasksName;
  std::vector < std::string > pssDefaultTasksFile;
  std::vector < std::string > DefinePreprocessingTasksName;
  std::vector < std::string > DefinePreprocessingTasksFile;

  this->CreateDefaultTasksList(pssDefaultTasksName, pssDefaultTasksFile,
      DefinePreprocessingTasksName, DefinePreprocessingTasksFile);

  std::string tasksList = "";

  for (unsigned int i = 0; i < DefinePreprocessingTasksName.size(); ++i)
    {
    tasksList += DefinePreprocessingTasksName[i];
    tasksList += ":";
    tasksList += DefinePreprocessingTasksFile[i];
    if (i != DefinePreprocessingTasksName.size() - 1)
      tasksList += ",";
    }

  return tasksList;

}
