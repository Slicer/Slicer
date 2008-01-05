/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLNRRDStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"

#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkNRRDReader.h"
#include "vtkNRRDWriter.h"
#include "vtkDoubleArray.h"

//------------------------------------------------------------------------------
vtkMRMLNRRDStorageNode* vtkMRMLNRRDStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLNRRDStorageNode");
  if(ret)
    {
    return (vtkMRMLNRRDStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLNRRDStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLNRRDStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLNRRDStorageNode");
  if(ret)
    {
    return (vtkMRMLNRRDStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLNRRDStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLNRRDStorageNode::vtkMRMLNRRDStorageNode()
{
  this->CenterImage = 1;
}

//----------------------------------------------------------------------------
vtkMRMLNRRDStorageNode::~vtkMRMLNRRDStorageNode()
{
}

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
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLNRRDStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLNRRDStorageNode *node = (vtkMRMLNRRDStorageNode *) anode;

  this->SetCenterImage(node->CenterImage);
}

//----------------------------------------------------------------------------
void vtkMRMLNRRDStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
  os << indent << "CenterImage:   " << this->CenterImage << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLNRRDStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------

int vtkMRMLNRRDStorageNode::ReadData(vtkMRMLNode *refNode)
{
  vtkDebugMacro("Reading NRRD data");
  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLScalarVolumeNode") || refNode->IsA("vtkMRMLVectorVolumeNode" ) || 
          refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") ||
          refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
     )
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLVolumeNode");
    return 0;         
    }
  if (this->GetFileName() == NULL) 
    {
      return 0;
    }
  vtkMRMLVolumeNode *volNode;

  vtkNRRDReader* reader;

  if ( refNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    volNode = dynamic_cast <vtkMRMLScalarVolumeNode *> (refNode);
    }
  else if ( refNode->IsA("vtkMRMLVectorVolumeNode") ) 
    {
    volNode = dynamic_cast <vtkMRMLVectorVolumeNode *> (refNode);
    }
  else if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") )
    {
    volNode = dynamic_cast <vtkMRMLDiffusionWeightedVolumeNode *> (refNode);
    }
  else if ( refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    volNode = dynamic_cast <vtkMRMLDiffusionTensorVolumeNode *> (refNode);
    }

  reader = vtkNRRDReader::New();

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

  std::string fullName;
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }

  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLVolumeNode: File name not specified");
    reader->Delete();
    return 0;
    }

  reader->SetFileName(fullName.c_str());

  // Check if this is a NRRD file that we can read
  if (!reader->CanReadFile(fullName.c_str()))
    {
    vtkDebugMacro("vtkMRMLNRRDStorageNode: This is not a nrrd file");
    reader->Delete();
    return 0;
    }

  // Read the header to see if the NRRD file corresponds to the
  // MRML Node
  reader->UpdateInformation();
  // Check type
  if ( refNode->IsA("vtkMRMLScalarVolumeNode") )
    {
    if (!(reader->GetPointDataType() == SCALARS && 
        (reader->GetNumberOfComponents() == 1 || reader->GetNumberOfComponents()==3) ))
      {
      vtkErrorMacro("MRMLVolumeNode does not match file kind");
      reader->Delete();
      return 0;
      }
    }
  else if ( refNode->IsA("vtkMRMLVectorVolumeNode") )
    {
    if (! (reader->GetPointDataType() == VECTORS || reader->GetPointDataType() == NORMALS))
      {
      vtkDebugMacro("MRMLVolumeNode does not match file kind");
      reader->Delete();
      return 0;
      }
    }
  else if ( refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    if ( ! (reader->GetPointDataType() == TENSORS))
      {
      vtkDebugMacro("MRMLVolumeNode does not match file kind");
      reader->Delete();
      return 0;
      }
    }
  else if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode"))
    {
    vtkDebugMacro("Checking we have right info in file");
    char *value = reader->GetHeaderValue("modality");
    if (value == NULL)
      {
      reader->Delete();
      return 0;
      }
    if ( ! (reader->GetPointDataType() == SCALARS &&
            !strcmp(value,"DWMRI") ) )
      {
      vtkErrorMacro("MRMLVolumeNode does not match file kind");
      reader->Delete();
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
      vtkWarningMacro("Measurement frame is not provided");
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
      vtkWarningMacro("Measurement frame is not provided");
      } 
    else 
      {
      //dynamic_cast <vtkMRMLTensorVolumeNode *> (volNode)->SetMeasurementFrameMatrix(mat2);
      (vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(volNode))->SetMeasurementFrameMatrix(mat2);
      }
    }

  // parse additional key-value pairs
  if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") )
    {
    vtkDoubleArray *grad = vtkDoubleArray::New();
    vtkDoubleArray *bvalue = vtkDoubleArray::New();
    if (!this->ParseDiffusionInformation(reader,grad,bvalue))
      {
      vtkErrorMacro("vtkMRMLDiffusionWeightedVolumeNode: Cannot parse Diffusion Information");
      grad->Delete();
      bvalue->Delete();
      reader->Delete();
      return 0;
      }
    dynamic_cast <vtkMRMLDiffusionWeightedVolumeNode *> (volNode)->SetDiffusionGradients(grad);
    dynamic_cast <vtkMRMLDiffusionWeightedVolumeNode *> (volNode)->SetBValues(bvalue);
    grad->Delete();
    bvalue->Delete();
    }

  volNode->SetStorageNodeID(this->GetID());
  //TODO update scene to send Modified event
 
  vtkImageChangeInformation *ici = vtkImageChangeInformation::New();
  ici->SetInput (reader->GetOutput());
  ici->SetOutputSpacing( 1, 1, 1 );
  ici->SetOutputOrigin( 0, 0, 0 );
  ici->Update();

  volNode->SetAndObserveImageData (ici->GetOutput());

  reader->Delete();
  ici->Delete();
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLNRRDStorageNode::WriteData(vtkMRMLNode *refNode)
{

  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLScalarVolumeNode") || refNode->IsA("vtkMRMLVectorVolumeNode" ) || 
          refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") ||
          refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
     )
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLVolumeNode");
    return 0;         
    }    
  
  vtkMRMLVolumeNode *volNode;
  //Store volume nodes attributes.
  vtkMatrix4x4 *mf = vtkMatrix4x4::New();
  vtkDoubleArray *grads = NULL;
  vtkDoubleArray *bValues = NULL;
  vtkMatrix4x4* ijkToRas = vtkMatrix4x4::New();
  
  if ( refNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    volNode = vtkMRMLScalarVolumeNode::SafeDownCast(refNode);    
    }
  else if ( refNode->IsA("vtkMRMLVectorVolumeNode") ) 
    {
    volNode = vtkMRMLVectorVolumeNode::SafeDownCast(refNode);
    if (volNode)
      {
      ((vtkMRMLVectorVolumeNode *) volNode)->GetMeasurementFrameMatrix(mf);
      }
    }
  else if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") )
    {
    
    volNode = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(refNode);
    if (volNode)
      {
      ((vtkMRMLDiffusionWeightedVolumeNode *) volNode)->GetMeasurementFrameMatrix(mf);
      grads = ((vtkMRMLDiffusionWeightedVolumeNode *) volNode)->GetDiffusionGradients();
      bValues = ((vtkMRMLDiffusionWeightedVolumeNode *) volNode)->GetBValues();
      }
    }
  else if ( refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    volNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(refNode);
    if (volNode)
      {
      ((vtkMRMLDiffusionTensorVolumeNode *) volNode)->GetMeasurementFrameMatrix(mf);
      }
    }  

  volNode->GetIJKToRASMatrix(ijkToRas);
  
  if (volNode->GetImageData() == NULL) 
    {
    vtkErrorMacro("cannot write ImageData, it's NULL");
    }
  
  std::string fullName;
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }
  
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLVolumeNode: File name not specified");
    return 0;
    }
  // Use here the NRRD Writer
  vtkNRRDWriter *writer = vtkNRRDWriter::New();
  writer->SetFileName(fullName.c_str());
  writer->SetInput(volNode->GetImageData() );
  writer->SetUseCompression(this->GetUseCompression());

  // set volume attributes
  writer->SetIJKToRASMatrix(ijkToRas);
  writer->SetMeasurementFrameMatrix(mf);
  if (grads)
    {
    writer->SetDiffusionGradients(grads);
    }
  if (bValues)
    {
    writer->SetBValues(bValues);
    }
  
  writer->Write();
  int writeFlag = 1;
  if (writer->GetWriteError())
    {
    vtkErrorMacro("ERROR writing NRRD file " << writer->GetFileName());    
    writeFlag = 0;
    }
  writer->Delete();
  
  ijkToRas->Delete();
  mf->Delete();

  return writeFlag;

}

//----------------------------------------------------------------------------
int vtkMRMLNRRDStorageNode::ParseDiffusionInformation(vtkNRRDReader *reader,vtkDoubleArray *grad,vtkDoubleArray *bvalues)
{
  std::string keys(reader->GetHeaderKeys());
  std::string key,value,num;
  std::string tag,tagnex;
  char *tmp;
  vtkDoubleArray *factor = vtkDoubleArray::New();
  grad->SetNumberOfComponents(3);
  double g[3];
  int rep;

  // search for modality tag
  key = "modality";
  tmp = reader->GetHeaderValue((char *) key.c_str());
  if (tmp == NULL)
    {
    factor->Delete();
    return 0;
    }
  if (strcmp(tmp,"DWMRI") != 0)
    {
    factor->Delete();
    return 0;
    }
  // search for tag DWMRI_gradient_
  tag = "DWMRI_gradient_";
  tagnex = "DWMRI_NEX_";
  unsigned int pos = 0;
  int gbeginpos =0;
  int gendpos = 0;
  pos = keys.find(tag,pos);
  while ( pos < keys.size() )
    {
    num = keys.substr(pos+tag.size(),4);
    // Insert gradient
    key = tag+num;
    tmp = reader->GetHeaderValue((char *) key.c_str());
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
        gendpos=value.find(" ",gbeginpos);
        }
      while(gendpos==gbeginpos);
      g[i] = atof(value.substr(gbeginpos,gendpos).c_str());
      gbeginpos = gendpos;
      }
    grad->InsertNextTuple3(g[0],g[1],g[2]);
    factor->InsertNextValue(sqrt(g[0]*g[0]+g[1]*g[1]+g[2]*g[2]));
    // find repetitions of this gradient
    key = tagnex+num;
    tmp = reader->GetHeaderValue((char *) key.c_str());
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
        factor->InsertNextValue(sqrt(g[0]*g[0]+g[1]*g[1]+g[2]*g[2]));
      }
    }
   pos = keys.find(tag,pos+1);
  }

  grad->Modified();
  factor->Modified();
  double range[2];
  // search for tag DWMRI_b-value
  key = "DWMRI_b-value";
  tmp = reader->GetHeaderValue((char *) key.c_str());
  if (tmp == NULL)
    {
    factor->Delete();
    return 0;
    }
  double bval = atof(tmp);
  factor->GetRange(range);
  bvalues->SetNumberOfTuples(grad->GetNumberOfTuples());
  for (int i=0; i<grad->GetNumberOfTuples();i++)
    {
    bvalues->SetValue(i,bval*factor->GetValue(i)/range[1]);
    }
  factor->Delete();
  return 1;
}
