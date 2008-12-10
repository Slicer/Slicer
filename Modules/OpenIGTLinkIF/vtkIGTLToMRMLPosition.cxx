/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"
#include "vtkIGTLToMRMLPosition.h"

#include "vtkMRMLLinearTransformNode.h"
#include "igtlPositionMessage.h"
#include "igtlWin32Header.h"
#include "igtlMath.h"

vtkStandardNewMacro(vtkIGTLToMRMLPosition);
vtkCxxRevisionMacro(vtkIGTLToMRMLPosition, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkIGTLToMRMLPosition::vtkIGTLToMRMLPosition()
{
}


//---------------------------------------------------------------------------
vtkIGTLToMRMLPosition::~vtkIGTLToMRMLPosition()
{
}


//---------------------------------------------------------------------------
void vtkIGTLToMRMLPosition::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
vtkMRMLNode* vtkIGTLToMRMLPosition::CreateNewNode(vtkMRMLScene* scene, const char* name)
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

  scene->AddNode(transformNode);  

  return transformNode;
}


//---------------------------------------------------------------------------
vtkIntArray* vtkIGTLToMRMLPosition::GetNodeEvents()
{
  vtkIntArray* events;

  events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);

  return events;
}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLPosition::IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node)
{
  // Create a message buffer to receive transform data
  igtl::PositionMessage::Pointer positionMsg;
  positionMsg = igtl::PositionMessage::New();
  positionMsg->Copy(buffer);  // !! TODO: copy makes performance issue.

  // Deserialize the transform data
  // If you want to skip CRC check, call Unpack() without argument.
  int c = positionMsg->Unpack(1);
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

  float position[3];
  float quaternion[4];
  igtl::Matrix4x4 matrix;
  positionMsg->GetPosition(position);
  positionMsg->GetQuaternion(quaternion);

  igtl::QuaternionToMatrix(quaternion, matrix);
  //matrix[0][3] = position[0];
  //matrix[1][3] = position[1];
  //matrix[2][3] = position[2];
  
  float tx = matrix[0][0];
  float ty = matrix[1][0];
  float tz = matrix[2][0];
  float sx = matrix[0][1];
  float sy = matrix[1][1];
  float sz = matrix[2][1];
  float nx = matrix[0][2];
  float ny = matrix[1][2];
  float nz = matrix[2][2];
  //float px = matrix[0][3];
  //float py = matrix[1][3];
  //float pz = matrix[2][3];
  float px = position[0];
  float py = position[1];
  float pz = position[2];
  
  //std::cerr << "matrix = " << std::endl;
  //std::cerr << tx << ", " << ty << ", " << tz << std::endl;
  //std::cerr << sx << ", " << sy << ", " << sz << std::endl;
  //std::cerr << nx << ", " << ny << ", " << nz << std::endl;
  //std::cerr << px << ", " << py << ", " << pz << std::endl;
  
  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  vtkMatrix4x4* transformToParent = transformNode->GetMatrixTransformToParent();

  transform->Identity();
  transform->SetElement(0, 0, tx);
  transform->SetElement(1, 0, ty);
  transform->SetElement(2, 0, tz);

  transform->SetElement(0, 1, sx);
  transform->SetElement(1, 1, sy);
  transform->SetElement(2, 1, sz);

  transform->SetElement(0, 2, nx);
  transform->SetElement(1, 2, ny);
  transform->SetElement(2, 2, nz);

  transform->SetElement(0, 3, px);
  transform->SetElement(1, 3, py);
  transform->SetElement(2, 3, pz);

  transformToParent->DeepCopy(transform);

  transform->Delete();

  return 1;

}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLPosition::MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg)
{

  if (mrmlNode && event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    vtkMRMLLinearTransformNode* transformNode =
      vtkMRMLLinearTransformNode::SafeDownCast(mrmlNode);
    vtkMatrix4x4* matrix = transformNode->GetMatrixTransformToParent();
    
    //igtl::PositionMessage::Pointer OutPositionMsg;
    if (this->OutPositionMsg.IsNull())
      {
      this->OutPositionMsg = igtl::PositionMessage::New();
      }
    
    this->OutPositionMsg->SetDeviceName(mrmlNode->GetName());

    igtl::Matrix4x4 igtlmatrix;
    
    igtlmatrix[0][0]  = matrix->GetElement(0, 0);
    igtlmatrix[1][0]  = matrix->GetElement(1, 0);
    igtlmatrix[2][0]  = matrix->GetElement(2, 0);
    igtlmatrix[3][0]  = matrix->GetElement(3, 0);
    
    igtlmatrix[0][1]  = matrix->GetElement(0, 1);
    igtlmatrix[1][1]  = matrix->GetElement(1, 1);
    igtlmatrix[2][1]  = matrix->GetElement(2, 1);
    igtlmatrix[3][1]  = matrix->GetElement(3, 1);
    
    igtlmatrix[0][2]  = matrix->GetElement(0, 2);
    igtlmatrix[1][2]  = matrix->GetElement(1, 2);
    igtlmatrix[2][2]  = matrix->GetElement(2, 2);
    igtlmatrix[3][2]  = matrix->GetElement(3, 2);
    
    igtlmatrix[0][3]  = matrix->GetElement(0, 3);
    igtlmatrix[1][3]  = matrix->GetElement(1, 3);
    igtlmatrix[2][3]  = matrix->GetElement(2, 3);
    igtlmatrix[3][3]  = matrix->GetElement(3, 3);

    float position[3];
    float quaternion[4];
    
    position[0] = igtlmatrix[0][3];
    position[1] = igtlmatrix[1][3];
    position[2] = igtlmatrix[2][3];
    igtl::MatrixToQuaternion(igtlmatrix, quaternion);
    
    //this->OutPositionMsg->SetMatrix(igtlmatrix);
    this->OutPositionMsg->SetPosition(position);
    this->OutPositionMsg->SetQuaternion(quaternion);
    this->OutPositionMsg->Pack();

    *size = this->OutPositionMsg->GetPackSize();
    *igtlMsg = (void*)this->OutPositionMsg->GetPackPointer();

    return 1;
    }

  return 0;

}

