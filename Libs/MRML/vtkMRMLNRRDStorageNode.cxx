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
  of << indent << "centerImage=\"" << ss.str() << "\" ";

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

  cout<<"Reading NRRD data"<<endl;
  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLScalarVolumeNode") || refNode->IsA("vtkMRMLVectorVolumeNode" ) || 
          refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") ||
          refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
     )
    {
    vtkErrorMacro("Reference node is not a vtkMRMLVolumeNode");
    cout<<"Ref node is not a good type: "<<refNode->GetClassName()<<endl;
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
    cout<<"We have a DWI node"<<endl;
    volNode = dynamic_cast <vtkMRMLDiffusionWeightedVolumeNode *> (refNode);
    }
  else if ( refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    volNode = dynamic_cast <vtkMRMLDiffusionTensorVolumeNode *> (refNode);
    }

  reader = vtkNRRDReader::New();

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

  cout<<"NRRD reader reading file: "<<fullName.c_str()<<endl;
  reader->SetFileName(fullName.c_str());

  // Check if this is a NRRD file that we can read
  if (!reader->CanReadFile(fullName.c_str()))
    {
    vtkErrorMacro("vtkMRMLNRRDStorageNode: This is not a nrrd file");
    reader->Delete();
    return 0;
    }

  // Read the header to see if the NRRD file corresponds to the
  // MRML Node
  cout<<"Before update info"<<endl;
  reader->UpdateInformation();
  cout<<"After update info"<<endl;
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
      vtkErrorMacro("MRMLVolumeNode does not match file kind");
      reader->Delete();
      return 0;
      }
    }
  else if ( refNode->IsA("vtkMRMLDiffusionTensorVolumeNode") )
    {
    if ( ! (reader->GetPointDataType() == TENSORS))
      {
      vtkErrorMacro("MRMLVolumeNode does not match file kind");
      reader->Delete();
      return 0;
      }
    }
  else if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode"))
    {
    cout<<"Checking we have right info in file"<<endl;
    if ( ! (reader->GetPointDataType() == SCALARS &&
            !strcmp(reader->GetHeaderValue("modality"),"DWMRI") ) )
      {
      vtkErrorMacro("MRMLVolumeNode does not match file kind");
      reader->Delete();
      return 0;
      }
    }

  // Set Reader member variables
  if (this->CenterImage) 
    {
    reader->SetUseNativeOriginOff();
    }
  else
    {
    reader->SetUseNativeOriginOn();
    }
  cout<<"Doing update"<<endl;
  reader->Update();
  cout<<"After update"<<endl;
  // set volume attributes
  vtkMatrix4x4* mat = reader->GetRasToIjkMatrix();
  volNode->SetRASToIJKMatrix(mat);

  if ( !refNode->IsA("vtkMRMLScalarVolumeNode") )
    {
    vtkMatrix4x4 *mat2;
    mat2 = reader->GetMeasurementFrameMatrix();
    cout<<"Setting MF"<<endl;
    if (mat2 == NULL) 
      {
      vtkWarningMacro("Measurement frame is not provided");
      } 
    else 
     {
      //dynamic_cast <vtkMRMLTensorVolumeNode *> (volNode)->SetMeasurementFrameMatrix(mat2);
      cout<<"Before downcast"<<endl;
      (vtkMRMLTensorVolumeNode::SafeDownCast(volNode))->SetMeasurementFrameMatrix(mat2);
      cout<<"Set done"<<endl;
      }
    }

  // parse additional key-value pairs
  if ( refNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") )
    {
    vtkDoubleArray *grad = vtkDoubleArray::New();
    vtkDoubleArray *bvalue = vtkDoubleArray::New();
    cout<<"Parsing DWI info"<<endl;
    if (this->ParseDiffusionInformation(reader,grad,bvalue))
      {
      vtkErrorMacro("vtkMRMLDiffusionWeightedVolumeNode: Cannot parse Diffusion Information");
      grad->Delete();
      bvalue->Delete();
      reader->Delete();
      return 0;
      }
    dynamic_cast <vtkMRMLDiffusionWeightedVolumeNode *> (volNode)->SetGradients(grad);
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

  vtkErrorMacro("We cannot write NRRD data");
  return 0;

  // test whether refNode is a valid node to hold a volume
  if (!refNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLVolumeNode");
    return 0;
    }
  
  vtkMRMLVolumeNode *volNode;
  
  if ( refNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    volNode = vtkMRMLScalarVolumeNode::SafeDownCast(refNode);
    }
  
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
  //vtkITKImageWriter *writer = vtkITKImageWriter::New();
  //writer->SetFileName(fullName.c_str());
  //writer->SetInput( volNode->GetImageData() );

  // set volume attributes
  vtkMatrix4x4* mat = vtkMatrix4x4::New();
  volNode->GetRASToIJKMatrix(mat);
  //writer->SetRasToIJKMatrix(mat);

  //writer->Write();
  //writer->Delete();

  return 1;

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
  tag = "modality";
  value = std::string(reader->GetHeaderValue((char *) tag.c_str()));
  if (value.size() == 0)
  {
  return 0;
  }
  if (strcmp(value.c_str(),"DWMRI"))
  {
  return 0;
  }
  // search for tag DWMRI_gradient_
  tag = "DWMRI_gradient_";
  tagnex = "DWMRI_NEX_";
  int pos = 0;
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
      }
    }
   pos = keys.find(tag,pos+1);
  }

  grad->Modified();
  factor->Modified();
  double range[2];
  // search for tag DWMRI_b-value
  key = "DWMRI_b-value";
  double bval = atof(reader->GetHeaderValue((char *) key.c_str()));
  factor->GetRange(range);
  bvalues->SetNumberOfTuples(grad->GetNumberOfTuples());
  for (int i=0; i<grad->GetNumberOfTuples();i++)
    {
    bvalues->SetValue(i,bval*factor->GetValue(i)/range[1]);
    }
  factor->Delete();
}

