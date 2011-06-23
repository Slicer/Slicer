/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"
#include "vtkIGTLToMRMLLinearTransform.h"

#include "vtkMRMLLinearTransformNode.h"
#include "igtlTransformMessage.h"

vtkStandardNewMacro(vtkIGTLToMRMLLinearTransform);
vtkCxxRevisionMacro(vtkIGTLToMRMLLinearTransform, "$Revision$");


//---------------------------------------------------------------------------
vtkIGTLToMRMLLinearTransform::vtkIGTLToMRMLLinearTransform()
{
}


//---------------------------------------------------------------------------
vtkIGTLToMRMLLinearTransform::~vtkIGTLToMRMLLinearTransform()
{
}


//---------------------------------------------------------------------------
void vtkIGTLToMRMLLinearTransform::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
vtkMRMLNode* vtkIGTLToMRMLLinearTransform::CreateNewNode(vtkMRMLScene* scene, const char* name)
{
  vtkMRMLLinearTransformNode* transformNode;

  transformNode = vtkMRMLLinearTransformNode::New();
  transformNode->SetName(name);
  transformNode->SetDescription("Received by OpenIGTLink");

  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  transform->Identity();
  //transformNode->SetAndObserveImageData(transform);
  transformNode->ApplyTransform(transform);
  transform->Delete();

  vtkMRMLNode* n = scene->AddNode(transformNode);
  transformNode->Delete();

  return n;
}


//---------------------------------------------------------------------------
vtkIntArray* vtkIGTLToMRMLLinearTransform::GetNodeEvents()
{
  vtkIntArray* events;

  events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);

  return events;
}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLLinearTransform::IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node)
{
  // Create a message buffer to receive transform data
  igtl::TransformMessage::Pointer transMsg;
  transMsg = igtl::TransformMessage::New();
  transMsg->Copy(buffer);  // !! TODO: copy makes performance issue.

  // Deserialize the transform data
  // If CheckCRC==0, CRC check is skipped.
  int c = transMsg->Unpack(this->CheckCRC);

  if (!(c & igtl::MessageHeader::UNPACK_BODY)) // if CRC check fails
    {
    // TODO: error handling
    return 0;
    }

  if (node == NULL)
    {
    return 0;
    }

  vtkMRMLLinearTransformNode* transformNode = 
    vtkMRMLLinearTransformNode::SafeDownCast(node);

  igtl::Matrix4x4 matrix;
  transMsg->GetMatrix(matrix);

  float tx = matrix[0][0];
  float ty = matrix[1][0];
  float tz = matrix[2][0];
  float sx = matrix[0][1];
  float sy = matrix[1][1];
  float sz = matrix[2][1];
  float nx = matrix[0][2];
  float ny = matrix[1][2];
  float nz = matrix[2][2];
  float px = matrix[0][3];
  float py = matrix[1][3];
  float pz = matrix[2][3];

  //std::cerr << "\n\nmatrix = " << std::endl;
  //std::cerr << tx << ", " << ty << ", " << tz << std::endl;
  //std::cerr << sx << ", " << sy << ", " << sz << std::endl;
  //std::cerr << nx << ", " << ny << ", " << nz << std::endl;
  //std::cerr << px << ", " << py << ", " << pz << std::endl;
  
  // set volume orientation
  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  vtkMatrix4x4* transformToParent = transformNode->GetMatrixTransformToParent();

  transform->Identity();
  transform->Element[0][0] = tx;
  transform->Element[1][0] = ty;
  transform->Element[2][0] = tz;
  transform->Element[0][1] = sx;
  transform->Element[1][1] = sy;
  transform->Element[2][1] = sz;
  transform->Element[0][2] = nx;
  transform->Element[1][2] = ny;
  transform->Element[2][2] = nz;
  transform->Element[0][3] = px;
  transform->Element[1][3] = py;
  transform->Element[2][3] = pz;

  transformToParent->DeepCopy(transform);


  //std::cerr << "IGTL matrix = " << std::endl;
  //transform->Print(cerr);
  //std::cerr << "MRML matrix = " << std::endl;
  //transformToParent->Print(cerr);

  transform->Delete();

  return 1;

}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLLinearTransform::MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg)
{

  if (mrmlNode && event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    vtkMRMLLinearTransformNode* transformNode =
      vtkMRMLLinearTransformNode::SafeDownCast(mrmlNode);
    vtkMatrix4x4* matrix = transformNode->GetMatrixTransformToParent();
    
    //igtl::TransformMessage::Pointer OutTransformMsg;
    if (this->OutTransformMsg.IsNull())
      {
      this->OutTransformMsg = igtl::TransformMessage::New();
      }
    
    this->OutTransformMsg->SetDeviceName(mrmlNode->GetName());

    igtl::Matrix4x4 igtlmatrix;
    
    igtlmatrix[0][0]  = matrix->Element[0][0];
    igtlmatrix[1][0]  = matrix->Element[1][0];
    igtlmatrix[2][0]  = matrix->Element[2][0];
    igtlmatrix[3][0]  = matrix->Element[3][0];
    igtlmatrix[0][1]  = matrix->Element[0][1];
    igtlmatrix[1][1]  = matrix->Element[1][1];
    igtlmatrix[2][1]  = matrix->Element[2][1];
    igtlmatrix[3][1]  = matrix->Element[3][1];
    igtlmatrix[0][2]  = matrix->Element[0][2];
    igtlmatrix[1][2]  = matrix->Element[1][2];
    igtlmatrix[2][2]  = matrix->Element[2][2];
    igtlmatrix[3][2]  = matrix->Element[3][2];
    igtlmatrix[0][3]  = matrix->Element[0][3];
    igtlmatrix[1][3]  = matrix->Element[1][3];
    igtlmatrix[2][3]  = matrix->Element[2][3];
    igtlmatrix[3][3]  = matrix->Element[3][3];
    
    this->OutTransformMsg->SetMatrix(igtlmatrix);
    this->OutTransformMsg->Pack();

    *size = this->OutTransformMsg->GetPackSize();
    *igtlMsg = (void*)this->OutTransformMsg->GetPackPointer();

    return 1;
    }

  return 0;

}

