/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLNRRDStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeNode.h"

// vtkTeem includes
#include <vtkTeemNRRDReader.h>
#include <vtkTeemNRRDWriter.h>

// VTK includes
#include <vtkImageChangeInformation.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkVersion.h>

// vnl includes
#include <vnl/vnl_double_3.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLNRRDStorageNode);

//----------------------------------------------------------------------------
vtkMRMLNRRDStorageNode::vtkMRMLNRRDStorageNode()
{
  this->CenterImage = 0;
  this->DefaultWriteFileExtension = "nhdr";

  this->CompressionPresets.emplace_back(this->GetCompressionParameterFastest(), "Fastest");
  this->CompressionPresets.emplace_back(this->GetCompressionParameterNormal(), "Normal");
  this->CompressionPresets.emplace_back(this->GetCompressionParameterMinimumSize(), "Minimum size");

  this->CompressionParameter = this->GetCompressionParameterFastest();
}

//----------------------------------------------------------------------------
vtkMRMLNRRDStorageNode::~vtkMRMLNRRDStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLNRRDStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  std::stringstream ss;
  ss << this->CenterImage;
  of << " centerImage=\"" << ss.str() << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLNRRDStorageNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "centerImage"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CenterImage;
      }
    }

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLNRRDStorageNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLNRRDStorageNode *node = (vtkMRMLNRRDStorageNode *) anode;

  this->SetCenterImage(node->CenterImage);

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLNRRDStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
  os << indent << "CenterImage:   " << this->CenterImage << "\n";
}

//----------------------------------------------------------------------------
bool vtkMRMLNRRDStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLScalarVolumeNode") ||
         refNode->IsA("vtkMRMLVectorVolumeNode" ) ||
         refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") ||
         refNode->IsA("vtkMRMLDiffusionTensorVolumeNode");
}

//----------------------------------------------------------------------------
int vtkMRMLNRRDStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLVolumeNode *volNode = nullptr;

  if ( refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    volNode = dynamic_cast <vtkMRMLDiffusionTensorVolumeNode *> (refNode);
    }
  else if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") )
    {
    volNode = dynamic_cast <vtkMRMLDiffusionWeightedVolumeNode *> (refNode);
    }
  else if ( refNode->IsA("vtkMRMLVectorVolumeNode") )
    {
    volNode = dynamic_cast <vtkMRMLVectorVolumeNode *> (refNode);
    }
  else if ( refNode->IsA("vtkMRMLScalarVolumeNode") )
    {
    volNode = dynamic_cast <vtkMRMLScalarVolumeNode *> (refNode);
    }
  else if ( refNode->IsA("vtkMRMLVolumeNode") )
    {
    // Generic case used for any VolumeNode. Used when Extensions add
    // node types that are subclasses of VolumeNode.
    volNode = dynamic_cast<vtkMRMLVolumeNode *>(refNode);
    }
  else
    {
    vtkErrorMacro(<< "Do not recognize node type " << refNode->GetClassName());
    return 0;
    }

  vtkNew<vtkTeemNRRDReader> reader;

  // Set Reader member variables
  if (this->CenterImage)
    {
    reader->SetUseNativeOriginOff();
    }
  else
    {
    reader->SetUseNativeOriginOn();
    }

  if (volNode->GetImageData())
    {
    volNode->SetAndObserveImageData (nullptr);
    }

  std::string fullName = this->GetFullNameFromFileName();

  if (fullName.empty())
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  reader->SetFileName(fullName.c_str());

  // Check if this is a NRRD file that we can read
  if (!reader->CanReadFile(fullName.c_str()))
    {
    vtkErrorMacro("ReadData: This is not a nrrd file");
    return 0;
    }

  // Read the header to see if the NRRD file corresponds to the
  // MRML Node
  reader->UpdateInformation();

  // Check type
  if ( refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    if ( ! (reader->GetPointDataType() == vtkDataSetAttributes::TENSORS))
      {
      vtkErrorMacro("ReadData: MRMLVolumeNode does not match file kind");
      return 0;
      }
    }
  else if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode"))
    {
    vtkDebugMacro("ReadData: Checking we have right info in file");
    const char *value = reader->GetHeaderValue("modality");
    if (value == nullptr)
      {
      return 0;
      }
    if ( ! (reader->GetPointDataType() == vtkDataSetAttributes::SCALARS &&
            !strcmp(value,"DWMRI") ) )
      {
      vtkErrorMacro("ReadData: MRMLVolumeNode does not match file kind");
      return 0;
      }
    }
  else if ( refNode->IsA("vtkMRMLVectorVolumeNode") )
    {
    if (! (reader->GetPointDataType() == vtkDataSetAttributes::VECTORS
           || reader->GetPointDataType() == vtkDataSetAttributes::NORMALS))
      {
      vtkErrorMacro("ReadData: MRMLVolumeNode does not match file kind");
      return 0;
      }
    }
  else if ( refNode->IsA("vtkMRMLScalarVolumeNode") )
    {
    if (!(reader->GetPointDataType() == vtkDataSetAttributes::SCALARS &&
        (reader->GetNumberOfComponents() == 1 || reader->GetNumberOfComponents()==3) ))
      {
      vtkErrorMacro("ReadData: MRMLVolumeNode does not match file kind");
      return 0;
      }
    }

  reader->Update();
  // set volume attributes
  vtkMatrix4x4* mat = reader->GetRasToIjkMatrix();
  volNode->SetRASToIJKMatrix(mat);

  // set measurement frame
  vtkMatrix4x4 *mat2;
  if ( refNode->IsA("vtkMRMLTensorVolumeNode") )
    {
    mat2 = reader->GetMeasurementFrameMatrix();
    if (mat2 == nullptr)
      {
      vtkWarningMacro("ReadData: Measurement frame is not provided");
      }
    else
      {
      //dynamic_cast <vtkMRMLTensorVolumeNode *> (volNode)->SetMeasurementFrameMatrix(mat2);
      (vtkMRMLTensorVolumeNode::SafeDownCast(volNode))->SetMeasurementFrameMatrix(mat2);
      }
    }
  if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") )
    {
    mat2 = reader->GetMeasurementFrameMatrix();
    if (mat2 == nullptr)
      {
      vtkWarningMacro("ReadData: Measurement frame is not provided");
      }
    else
      {
      //dynamic_cast <vtkMRMLTensorVolumeNode *> (volNode)->SetMeasurementFrameMatrix(mat2);
      (vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(volNode))->SetMeasurementFrameMatrix(mat2);
      }
    }

  // parse additional diffusion key-value pairs and handle specially
  if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") )
    {
    vtkNew<vtkDoubleArray> grad;
    vtkNew<vtkDoubleArray> bvalue;
    if (!this->ParseDiffusionInformation(reader.GetPointer(), grad.GetPointer(), bvalue.GetPointer()))
      {
      vtkErrorMacro("vtkMRMLDiffusionWeightedVolumeNode: Cannot parse Diffusion Information");
      return 0;
      }
    dynamic_cast <vtkMRMLDiffusionWeightedVolumeNode *> (volNode)->SetDiffusionGradients(grad.GetPointer());
    dynamic_cast <vtkMRMLDiffusionWeightedVolumeNode *> (volNode)->SetBValues(bvalue.GetPointer());
    }

  // parse non-specific key-value pairs
  std::vector<std::string> keys = reader->GetHeaderKeysVector();
  for ( std::vector<std::string>::iterator kit = keys.begin();
        kit != keys.end(); ++kit)
    {
    volNode->SetAttribute((*kit).c_str(), reader->GetHeaderValue((*kit).c_str()));
    }


  vtkNew<vtkImageChangeInformation> ici;
  ici->SetInputConnection(reader->GetOutputPort());
  ici->SetOutputSpacing( 1, 1, 1 );
  ici->SetOutputOrigin( 0, 0, 0 );
  ici->Update();

  volNode->SetImageDataConnection(ici->GetOutputPort());
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLNRRDStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLVolumeNode *volNode = nullptr;
  //Store volume nodes attributes.
  vtkNew<vtkMatrix4x4> mf;
  vtkDoubleArray *grads = nullptr;
  vtkDoubleArray *bValues = nullptr;
  vtkNew<vtkMatrix4x4> ijkToRas;

  if ( refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    volNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(refNode);
    if (volNode)
      {
      ((vtkMRMLDiffusionTensorVolumeNode *) volNode)->GetMeasurementFrameMatrix(mf.GetPointer());
      }
    }
  else if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") )
    {

    volNode = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(refNode);
    if (volNode)
      {
      ((vtkMRMLDiffusionWeightedVolumeNode *) volNode)->GetMeasurementFrameMatrix(mf.GetPointer());
      grads = ((vtkMRMLDiffusionWeightedVolumeNode *) volNode)->GetDiffusionGradients();
      bValues = ((vtkMRMLDiffusionWeightedVolumeNode *) volNode)->GetBValues();
      }
    }
  else if ( refNode->IsA("vtkMRMLVectorVolumeNode") )
    {
    volNode = vtkMRMLVectorVolumeNode::SafeDownCast(refNode);
    if (volNode)
      {
      ((vtkMRMLVectorVolumeNode *) volNode)->GetMeasurementFrameMatrix(mf.GetPointer());
      }
    }
  else if ( refNode->IsA("vtkMRMLScalarVolumeNode") )
    {
    volNode = vtkMRMLScalarVolumeNode::SafeDownCast(refNode);
    }
  else if ( refNode->IsA("vtkMRMLVolumeNode") )
    {
    // Generic case used for any VolumeNode. Used when Extensions add
    // node types that are subclasses of VolumeNode.
    volNode = vtkMRMLVolumeNode::SafeDownCast(refNode);
    }
  else
    {
    vtkErrorMacro(<< "WriteData: Do not recognize node type " << refNode->GetClassName());
    return 0;
    }

  volNode->GetIJKToRASMatrix(ijkToRas.GetPointer());

  if (volNode->GetImageData() == nullptr)
    {
    vtkErrorMacro("WriteData: Cannot write nullptr ImageData");
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("WriteData: File name not specified");
    return 0;
    }
  // Use here the NRRD Writer
  vtkNew<vtkTeemNRRDWriter> writer;
  writer->SetFileName(fullName.c_str());
  writer->SetInputConnection(volNode->GetImageDataConnection());
  writer->SetUseCompression(this->GetUseCompression());
  writer->SetCompressionLevel(this->GetGzipCompressionLevelFromCompressionParameter(this->CompressionParameter));

  // set volume attributes
  writer->SetIJKToRASMatrix(ijkToRas.GetPointer());
  writer->SetMeasurementFrameMatrix(mf.GetPointer());
  if (grads)
    {
    writer->SetDiffusionGradients(grads);
    }
  if (bValues)
    {
    writer->SetBValues(bValues);
    }

  // pass down all MRML attributes to NRRD
  std::vector<std::string> attributeNames = volNode->GetAttributeNames();
  std::vector<std::string>::iterator ait = attributeNames.begin();
  for (; ait != attributeNames.end(); ++ait)
    {
    writer->SetAttribute((*ait), volNode->GetAttribute((*ait).c_str()));
    }

  writer->Write();
  int writeFlag = 1;
  if (writer->GetWriteError())
    {
    vtkErrorMacro("ERROR writing NRRD file " << (writer->GetFileName() == nullptr ? "null" : writer->GetFileName()));
    writeFlag = 0;
    }

  this->StageWriteData(refNode);

  return writeFlag;
}

//----------------------------------------------------------------------------
// internal

const std::string dwmri_grad_tag("DWMRI_gradient_");
const std::string dwmri_bvalue_tag("DWMRI_b-value");

bool parse_gradient_key(std::string key, size_t &grad_number, size_t &gradkey_pad_width, std::string err)
{
  std::stringstream err_stream;

  // note: Slicer no longer supports NRRDs with DWMRI_NEX_ keys. This was removed
  //       from Dicom2Nrrd in the following commit:
  //           Slicer3/4 SVN: r26101, git-svn: 63a18f7d6900a
  //       and never un-commented in Dicom2Nrrd (or DWIConvert).
  //       If such a key is found, we print an error and fail.
  if (key.find("DWMRI_NEX_") != std::string::npos)
    {
    err_stream << "DWMRI_NEX_ NRRD tag is no longer supported (since SVN r6101)."
               << " Please adjust header manually to encode repeat excitations"
               << " as unique DWMRI_gradient_###N keys, and re-load.";
    err = err_stream.str();
    return false;
    }

  if (key.find(dwmri_grad_tag) == std::string::npos)
    {
    return false;
    }
  // below here key is: DWMRI_gradient_####

  // padding is the extra zeros to give a specific digit count
  //   0001
  //   ^^^  <- zeros here are padding to 4 digits
  // we enforce the constraint that the padding of the grad keys must be consistent
  if (gradkey_pad_width == 0) {
    gradkey_pad_width = key.size() - dwmri_grad_tag.size();
  }
  else if (gradkey_pad_width != key.size() - dwmri_grad_tag.size())
    {
    err_stream << "DWMRI NRRD gradient key-numbers must have consistent padding (####N)"
        << " Found tag: '" << key << "' but previous key had padding: " << gradkey_pad_width;
    err = err_stream.str();
    return false;
    }

  // slices key string from `dwmri_grad_tag.size()` to `key.size()`.
  //   e.g.: "DWMRI_gradient_000001" -> "000001"
  std::string cur_grad_num_str = key.substr(dwmri_grad_tag.size(), key.size());
  size_t cur_grad_num = atol(cur_grad_num_str.c_str());

  // enforce monotonic order
  if (cur_grad_num != grad_number)
    {
    err_stream << "DWMRI NRRD gradient key-numbers must be consecutive."
               << " Found tag: '" << key << "' but previous key was: " << grad_number;
    err = err_stream.str();
    return false;
    }

  grad_number += 1;
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLNRRDStorageNode::ParseDiffusionInformation(
        vtkTeemNRRDReader* reader,
        vtkDoubleArray* gradients_array,
        vtkDoubleArray* bvalues_array)
{
  // Validate modality tag
  std::string modality(reader->GetHeaderValue("modality"));
  if (modality != "DWMRI")
    {
    vtkErrorMacro(<< "NRRD header missing 'modality: DWMRI' tag!");
    return 0;
    }

  std::map<std::string, std::string> nrrd_keys = reader->GetHeaderKeysMap();

  /*
      Step 1: get DWMRI_b-value
  */
  std::string ref_bvalue_str(reader->GetHeaderValue(dwmri_bvalue_tag.c_str()));
  if (ref_bvalue_str.empty())
    {
    vtkErrorMacro(<< "Missing 'DWMRI_b-value' tag!");
    return 0;
    }
  double ref_bvalue = atof(ref_bvalue_str.c_str());


  /*
    Step 2: loop over all keys
      - for all DWMRI_gradient_ keys, validate
        - consecutive
        - consistent padding
      - save each gradient to tmp_grads
      - record maximum gradient length
  */
  vtkNew<vtkDoubleArray> tmp_grads;
  tmp_grads->SetNumberOfComponents(3);
  size_t grad_idx = 0;
  size_t gradkey_pad_width = 0;
  double max_grad_norm = 0;
  std::string err;

  std::map<std::string, std::string>::iterator nrrd_keys_iter = nrrd_keys.begin();
  for (; nrrd_keys_iter != nrrd_keys.end(); nrrd_keys_iter++)
    {
    std::string key = nrrd_keys_iter->first;

    if (!parse_gradient_key(key, grad_idx, gradkey_pad_width, err))
      {
      if (err.empty())
        {
        continue;
        }
      else
        {
        vtkErrorMacro(<< err);
        return 0;
        }
      }
    // parse the gradient vector into double[3]
    vnl_double_3 cur_grad(0,0,0);
    std::stringstream grad_value_stream(nrrd_keys_iter->second);
    grad_value_stream >> cur_grad[0] >> cur_grad[1] >> cur_grad[2];

    max_grad_norm = std::max(cur_grad.two_norm(), max_grad_norm);
    tmp_grads->InsertNextTuple(cur_grad.data_block());
    }

  assert(grad_idx == (size_t) tmp_grads->GetNumberOfTuples());

  /*
    Step 3: loop over gradients
      - calculate each b-value based on NA-MIC DWI gradient length-encoding
      - then normalize each gradient to unit-length
  */
  bvalues_array->SetNumberOfTuples(tmp_grads->GetNumberOfTuples());
  // calculate the b-values
  for (int i=0; i < tmp_grads->GetNumberOfTuples(); i++)
    {
    vnl_double_3 cur_grad(0,0,0);
    cur_grad.copy_in(tmp_grads->GetTuple3(i));

    // note: this is norm^2, per the NA-MIC NRRD DWI convention
    // http://wiki.na-mic.org/Wiki/index.php/NAMIC_Wiki:DTI:Nrrd_format
    double cur_bval = ref_bvalue * pow(cur_grad.two_norm() / max_grad_norm, 2);
    bvalues_array->SetValue(i, cur_bval);

    // normalize gradient vector to unit-length
    //   must be done *after* bvalue extraction
    cur_grad.normalize();
    tmp_grads->InsertTuple(i, cur_grad.data_block());
    }

  // Step 4: copy tmp_grads to output
  gradients_array->DeepCopy(tmp_grads.GetPointer());
  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLNRRDStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("NRRD (.nrrd)");
  this->SupportedReadFileTypes->InsertNextValue("NRRD (.nhdr)");
}

//----------------------------------------------------------------------------
void vtkMRMLNRRDStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("NRRD (.nrrd)");
  this->SupportedWriteFileTypes->InsertNextValue("NRRD (.nhdr)");
}

//----------------------------------------------------------------------------
int vtkMRMLNRRDStorageNode::GetGzipCompressionLevelFromCompressionParameter(std::string compressionParameter)
{
  if (compressionParameter == this->GetCompressionParameterFastest())
    {
    return 1;
    }
  else if(compressionParameter == this->GetCompressionParameterNormal())
    {
    return 6;
    }
  else if (compressionParameter == this->GetCompressionParameterMinimumSize())
    {
    return 9;
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLNRRDStorageNode::ConfigureForDataExchange()
{
  this->UseCompressionOff();
}
