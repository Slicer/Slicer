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
#include "vtkIGTLToMRMLCoordinate.h"

#include "vtkMRMLLinearTransformNode.h"
#include "igtlCoordinateMessage.h"
#include "igtlWin32Header.h"
#include "igtlMath.h"

vtkStandardNewMacro(vtkIGTLToMRMLCoordinate);
vtkCxxRevisionMacro(vtkIGTLToMRMLCoordinate, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkIGTLToMRMLCoordinate::vtkIGTLToMRMLCoordinate()
{
}


//---------------------------------------------------------------------------
vtkIGTLToMRMLCoordinate::~vtkIGTLToMRMLCoordinate()
{
}


//---------------------------------------------------------------------------
void vtkIGTLToMRMLCoordinate::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
vtkMRMLNode* vtkIGTLToMRMLCoordinate::CreateNewNode(vtkMRMLScene* scene, const char* name)
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
vtkIntArray* vtkIGTLToMRMLCoordinate::GetNodeEvents()
{
  //vtkIntArray* events;
  //events = vtkIntArray::New();
  //events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);

  return NULL;
}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLCoordinate::IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node)
{
  // Create a message buffer to receive transform data
  igtl::CoordinateMessage::Pointer coordinateMsg;
  coordinateMsg = igtl::CoordinateMessage::New();
  coordinateMsg->Copy(buffer);  // !! TODO: copy makes performance issue.

  // Deserialize the transform data
  // If you want to skip CRC check, call Unpack() without argument.
  int c = coordinateMsg->Unpack(1);
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
  float offset[3];
  float insertion;
  igtl::Matrix4x4 matrix;

  coordinateMsg->GetPosition(position);
  coordinateMsg->GetQuaternion(quaternion);
  coordinateMsg->GetOffset(offset);
  insertion = coordinateMsg->GetInsertion();

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
  
  std::cerr << "matrix = " << std::endl;
  std::cerr << tx << ", " << ty << ", " << tz << std::endl;
  std::cerr << sx << ", " << sy << ", " << sz << std::endl;
  std::cerr << nx << ", " << ny << ", " << nz << std::endl;
  std::cerr << px << ", " << py << ", " << pz << std::endl;
  
  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  vtkMatrix4x4* transformToParent = transformNode->GetMatrixTransformToParent();

  // normalize
  float psi = sqrt(tx*tx + ty*ty + tz*tz);
  float psj = sqrt(sx*sx + sy*sy + sz*sz);
  float psk = sqrt(nx*nx + ny*ny + nz*nz);
  float ntx = tx / psi;
  float nty = ty / psi;
  float ntz = tz / psi;
  float nsx = sx / psj;
  float nsy = sy / psj;
  float nsz = sz / psj;
  float nnx = nx / psk;
  float nny = ny / psk;
  float nnz = nz / psk;

  float offset2 = offset[2]+insertion;

  float xoff = ntx*offset[0] + nsx*offset[0] + nnx*offset[0];
  float yoff = nty*offset[1] + nsy*offset[1] + nny*offset[1];
  float zoff = ntz*(offset2) + nsz*(offset2) + nnz*(offset2);

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

  transform->SetElement(0, 3, px + xoff);
  transform->SetElement(1, 3, py + yoff);
  transform->SetElement(2, 3, pz + zoff);

  transformToParent->DeepCopy(transform);

  transform->Delete();

  return 1;

}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLCoordinate::MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg)
{

  if (mrmlNode && event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    vtkMRMLLinearTransformNode* transformNode =
      vtkMRMLLinearTransformNode::SafeDownCast(mrmlNode);
    vtkMatrix4x4* matrix = transformNode->GetMatrixTransformToParent();
    
    //igtl::CoordinateMessage::Pointer OutPositionMsg;
    if (this->OutPositionMsg.IsNull())
      {
      this->OutPositionMsg = igtl::CoordinateMessage::New();
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
    this->OutPositionMsg->SetOffset(0.0, 0.0, 0.0);
    this->OutPositionMsg->SetInsertion(0.0);
    this->OutPositionMsg->Pack();

    *size = this->OutPositionMsg->GetPackSize();
    *igtlMsg = (void*)this->OutPositionMsg->GetPackPointer();

    return 1;
    }

  return 0;

}

