/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkSlicerTransformsReader.h"

// Logic includes
#include "vtkSlicerTransformLogic.h"
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLTransformNode.h>

// ITK includes
#include <itkMetaDataObject.h>
#include <itkNiftiImageIO.h>
#include <itkNrrdImageIO.h>

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

vtkStandardNewMacro(vtkSlicerTransformsReader);

//----------------------------------------------------------------------------
vtkSlicerTransformsReader::vtkSlicerTransformsReader()
{
  this->SetFileType("TransformFile");
  this->SetDescription("Transform");
  this->SetNameFilters(std::vector<std::string>{ "Transform (*.h5 *.tfm *.mat *.nrrd *.nhdr *.mha *.mhd *.nii *.nii.gz *.txt *.hdf5 *.he5)" });
}

//----------------------------------------------------------------------------
vtkSlicerTransformsReader::~vtkSlicerTransformsReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerTransformsReader::SetTransformLogic(vtkSlicerTransformLogic* logic)
{
  this->TransformLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerTransformLogic* vtkSlicerTransformsReader::GetTransformLogic()
{
  return this->TransformLogic;
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformsReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  if (!properties || !this->TransformLogic)
  {
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  this->GetUserMessages()->ClearMessages();
  vtkMRMLTransformNode* node = this->TransformLogic->AddTransform(fileName.c_str(), this->GetScene(), this->GetUserMessages());
  if (!node)
  {
    return false;
  }
  this->AddLoadedNodeID(node->GetID());
  return true;
}

//----------------------------------------------------------------------------
double vtkSlicerTransformsReader::CanLoadFileConfidence(const std::string& filePath)
{
  double confidence = this->Superclass::CanLoadFileConfidence(filePath);
  if (confidence <= 0)
  {
    return confidence;
  }
  if (EndsWithNoCase(filePath, ".txt"))
  {
    // Many kinds of text files have .txt extension. Use higher confidence than the default (which is
    // the same as for text file readers) if the file looks like an ITK transform file and lower
    // confidence if it does not.
    return vtkSlicerTransformsReader::IsITKTextTransformFile(filePath) ? 0.6 : 0.3;
  }
  // Set higher confidence for NIFTI or NRRD files containing displacement field.
  // In CanLoadFileConfidence we often just peek into the text header, but since NIFTI
  // does not use a text header, we must parse.
  itk::ImageIOBase::Pointer imageIO;
  if (EndsWithNoCase(filePath, ".NII") || EndsWithNoCase(filePath, ".NII.GZ"))
  {
    imageIO = itk::NiftiImageIO::New();
  }
  else if (EndsWithNoCase(filePath, ".NRRD") || EndsWithNoCase(filePath, ".NHDR"))
  {
    imageIO = itk::NrrdImageIO::New();
  }
  if (!imageIO)
  {
    return confidence;
  }
  // Use lower than default confidence value unless it turns out that this file contains a displacement field.
  confidence = 0.4;
  imageIO->SetFileName(filePath);
  try
  {
    imageIO->ReadImageInformation();
    const itk::MetaDataDictionary& metadata = imageIO->GetMetaDataDictionary();
    if (imageIO->GetNumberOfDimensions() == 3 && imageIO->GetNumberOfComponents() > 1) // vector voxels in 3D array
    {
      // NIFTI "intent_code" field is used in both NIFTI and NRRD files
      std::string niftiIntentCode;
      if (itk::ExposeMetaData<std::string>(metadata, "intent_code", niftiIntentCode))
      {
        // This is a NIFTI file. Verify that it contains a displacement vector image
        // by checking that the "intent code" metadata field equals 1006 (NIFTI_INTENT_DISPVECT).
        if (niftiIntentCode == "1006")
        {
          confidence = 0.6;
        }
      }
    }
  }
  catch (...)
  {
    // Something went wrong, we do not need to know the details, it is enough to know that
    // this does not look like a valid NIFTI file.
  }
  return confidence;
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformsReader::IsITKTextTransformFile(const std::string& filePath)
{
  // ITK text transform files start with a "#Insight Transform File" comment (which is optional)
  // and specify the transform type in a "Transform:" line after optional comment lines.
  std::istringstream header(ReadFileHeader(filePath, 4096));
  std::string line;
  while (std::getline(header, line))
  {
    size_t start = line.find_first_not_of(" \t\r");
    if (start == std::string::npos)
    {
      // empty line
      continue;
    }
    line = line.substr(start);
    if (line.compare(0, 23, "#Insight Transform File") == 0)
    {
      return true;
    }
    if (line[0] == '#')
    {
      // comment
      continue;
    }
    // The first line that is not a comment must specify the transform type
    return line.compare(0, 10, "Transform:") == 0;
  }
  return false;
}
