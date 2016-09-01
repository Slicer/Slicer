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
#include <vtkNRRDReader.h>
#include <vtkNRRDWriter.h>

// VTK includes
#include <vtkImageChangeInformation.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkVersion.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLNRRDStorageNode);

//----------------------------------------------------------------------------
vtkMRMLNRRDStorageNode::vtkMRMLNRRDStorageNode()
{
  this->CenterImage = 0;
  this->DefaultWriteFileExtension = "nhdr";
}

//----------------------------------------------------------------------------
vtkMRMLNRRDStorageNode::~vtkMRMLNRRDStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLNRRDStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  std::stringstream ss;
  ss << this->CenterImage;
  of << indent << " centerImage=\"" << ss.str() << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLNRRDStorageNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
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
  vtkMRMLVolumeNode *volNode = NULL;

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

  vtkNew<vtkNRRDReader> reader;

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
    volNode->SetAndObserveImageData (NULL);
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
    if (value == NULL)
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
    if (mat2 == NULL)
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
    if (mat2 == NULL)
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
  vtkMRMLVolumeNode *volNode = NULL;
  //Store volume nodes attributes.
  vtkNew<vtkMatrix4x4> mf;
  vtkDoubleArray *grads = NULL;
  vtkDoubleArray *bValues = NULL;
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

  if (volNode->GetImageData() == NULL)
    {
    vtkErrorMacro("WriteData: Cannot write NULL ImageData");
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("WriteData: File name not specified");
    return 0;
    }
  // Use here the NRRD Writer
  vtkNew<vtkNRRDWriter> writer;
  writer->SetFileName(fullName.c_str());
  writer->SetInputConnection(volNode->GetImageDataConnection());
  writer->SetUseCompression(this->GetUseCompression());

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
    vtkErrorMacro("ERROR writing NRRD file " << (writer->GetFileName() == NULL ? "null" : writer->GetFileName()));
    writeFlag = 0;
    }

  this->StageWriteData(refNode);

  return writeFlag;
}

//----------------------------------------------------------------------------
int vtkMRMLNRRDStorageNode::ParseDiffusionInformation(vtkNRRDReader *reader,vtkDoubleArray *grad,vtkDoubleArray *bvalues)
{
  std::string keys(reader->GetHeaderKeys());
  std::string key,value,num;
  std::string tag,tagnex;
  const char *tmp;
  vtkNew<vtkDoubleArray> factor;
  grad->SetNumberOfComponents(3);
  double g[3];
  int rep;

  // search for modality tag
  key = "modality";
  tmp = reader->GetHeaderValue(key.c_str());
  if (tmp == NULL)
    {
    return 0;
    }
  if (strcmp(tmp,"DWMRI") != 0)
    {
    return 0;
    }
  // search for tag DWMRI_gradient_
  tag = "DWMRI_gradient_";
  tagnex = "DWMRI_NEX_";
  unsigned int pos = 0;
  int gbeginpos =0;
  int gendpos = 0;
  pos = (unsigned int)keys.find(tag,pos);
  while ( pos < keys.size() )
    {
    num = keys.substr(pos+tag.size(),4);
    // Insert gradient
    key = tag+num;
    tmp = reader->GetHeaderValue(key.c_str());
    if (tmp == NULL)
      {
      continue;
      }
    else
      {
      value = tmp;
      }
    gbeginpos = -1;
    gendpos = 0;
    for (int i=0 ;i<3; i++)
      {
      do
        {
        gbeginpos++;
        gendpos=(int)value.find(" ",gbeginpos);
        }
      while(gendpos==gbeginpos);
      g[i] = atof(value.substr(gbeginpos,gendpos).c_str());
      gbeginpos = gendpos;
      }
    grad->InsertNextTuple3(g[0],g[1],g[2]);
    factor->InsertNextValue(sqrt(g[0]*g[0]+g[1]*g[1]+g[2]*g[2]));
    // find repetitions of this gradient
    key = tagnex+num;
    tmp = reader->GetHeaderValue(key.c_str());
    if (tmp == NULL)
      {
      value = "";
      }
    else
      {
      value = tmp;
      }
    if (value.size()>0) {
      rep = atoi(value.c_str());
      for (int i=0;i<rep-1;i++) {
        grad->InsertNextTuple3(g[0],g[1],g[2]);
        factor->InsertNextValue(sqrt( g[0]*g[0]+g[1]*g[1]+g[2]*g[2] ));
      }
    }
   pos = (unsigned int)keys.find(tag,pos+1);
  }

  grad->Modified();
  factor->Modified();
  double range[2];
  // search for tag DWMRI_b-value
  key = "DWMRI_b-value";
  tmp = reader->GetHeaderValue(key.c_str());
  if (tmp == NULL)
    {
    return 0;
    }
  double bval = atof(tmp);
  factor->GetRange(range);
  bvalues->SetNumberOfTuples(grad->GetNumberOfTuples());
  for (int i=0; i<grad->GetNumberOfTuples();i++)
    {
    // note: this is norm^2, per the NA-MIC NRRD DWI convention
    // http://wiki.na-mic.org/Wiki/index.php/NAMIC_Wiki:DTI:Nrrd_format
    bvalues->SetValue(i, bval * (pow(factor->GetValue(i)/range[1], 2)));
    }
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
void vtkMRMLNRRDStorageNode::ConfigureForDataExchange()
{
  this->UseCompressionOff();
}
