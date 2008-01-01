#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

#include "vtkProstateNavDataStream.h"
#include "vtkIGTMessageGenericAttribute.h"
#include "vtkIGTMessageImageDataAttribute.h"
#include "vtkImageData.h"
#include "vtkMath.h"


vtkStandardNewMacro(vtkProstateNavDataStream);
vtkCxxRevisionMacro(vtkProstateNavDataStream, "$Revision: 1.0 $");

vtkProstateNavDataStream::vtkProstateNavDataStream()
{
  this->AttrSetRobot    = NULL;
  this->AttrSetScanner  = NULL;
  this->NeedleMatrix    = vtkMatrix4x4::New();
  this->ImageTimeStamp  = vtkTimeStamp::New();
  this->RealtimeImage   = vtkImageData::New();

  AddCallbacks();
  
}

vtkProstateNavDataStream::~vtkProstateNavDataStream()
{
}


void vtkProstateNavDataStream::Init(const char *configFile)
{
  Superclass::Init(configFile);
}


void vtkProstateNavDataStream::PrintSelf(ostream& os, vtkIndent indent)
{
}


void vtkProstateNavDataStream::AddCallbacks()
{
  // Callback for the Robot
  this->AttrSetRobot = vtkIGTMessageAttributeSet::New();
  if (this->AttrSetRobot)
    {
    this->AttrSetRobot->AddAttribute("position",    (std::vector<float>*)NULL);
    this->AttrSetRobot->AddAttribute("orientation", (std::vector<float>*)NULL);
    this->AttrSetRobot->AddAttribute("status",      (std::string*)NULL);
    this->AttrSetRobot->AddAttribute("message",     (std::string*)NULL);
    this->AttrSetRobot->AddAttribute("depth",       (std::vector<float>*)NULL);
    }

  // Callback for the Scanner
  this->AttrSetScanner = vtkIGTMessageAttributeSet::New();
  if (this->AttrSetScanner)
    {
    this->AttrSetScanner->AddAttribute("image",   (vtkImageData*)NULL);
    this->AttrSetScanner->AddAttribute("fov",     (float*)NULL);
    this->AttrSetScanner->AddAttribute("slthick", (float*)NULL);
    }

  this->AddCallback("cb_robot",
                    (vtkIGTMessageAttributeSet::MessageHandlingFunction*)OnRecieveMessageFromRobot,
                    this->AttrSetRobot, NULL);

  this->AddCallback("cb_scanner",
                    (vtkIGTMessageAttributeSet::MessageHandlingFunction*)OnRecieveMessageFromScanner,
                    this->AttrSetScanner, NULL);

} 


void vtkProstateNavDataStream::OnRecieveMessageFromRobot(vtkIGTMessageAttributeSet* attrSet, void* arg)
{
  std::vector<float> position(3, 0.0);
  std::vector<float> orientation(4, 0.0);

  attrSet->GetAttribute("position", &position);
  attrSet->GetAttribute("orientation", &orientation);

  std::cerr << "position = ("
            << position[0] << ", "
            << position[1] << ", "
            << position[2] << ")" << std::endl;

  std::cerr << "orientation = ("
            << orientation[0] << ", "
            << orientation[1] << ", "
            << orientation[2] << ", "
            << orientation[3] << ")" << std::endl;

  float ori[4];
  float norm[3];
  float transnorm[3];
  ori[0] = orientation[0];
  ori[1] = orientation[1];
  ori[2] = orientation[2];
  ori[3] = orientation[3];

  vtkProstateNavDataStream* ds = dynamic_cast<vtkProstateNavDataStream*>(attrSet->GetOpenTrackerStream());
  ds->QuaternionToXYZ(ori, norm, transnorm);

  int j;
  for (j=0; j<3; j++)
    {
    ds->NeedleMatrix->SetElement(j,0,position[j]);
    }
  for (j=0; j<3; j++)
    {
    ds->NeedleMatrix->SetElement(j,1,norm[j]);
    }
  for (j=0; j<3; j++)
    {
    ds->NeedleMatrix->SetElement(j,2,transnorm[j]);
    }
  for (j=0; j<3; j++)
    {
    ds->NeedleMatrix->SetElement(j,3,0);
    }
  for (j=0; j<3; j++)
    {
    ds->NeedleMatrix->SetElement(3,j,0);
    }

  ds->NeedleMatrix->SetElement(3,3,1);

  // get a 3x3 matrix from the quaterion
  float transform_matrix[3][3];
  vtkMath::QuaternionToMatrix3x3(ori, transform_matrix);
  
  // get the "needle depth" vector(3,1) and multiply it by the robot orientation,
  // this will give the offsets in Slicer coordinates

  std::vector<float> depth(3,0.0);
  attrSet->GetAttribute("depth", &depth);

  float needle_offset[3];
  for (j=0; j<3; j++) {
    needle_offset[j] = depth[j];
  }

  // multiply the vector in-place
  vtkMath::Multiply3x3(transform_matrix, needle_offset, needle_offset);
  
  // to make things simple, replace the robot position by the needle tip
  //  in the LocatorMatrix
  ds->NeedleMatrix->SetElement(0, 0, position[0] + needle_offset[0]);
  ds->NeedleMatrix->SetElement(1, 0, position[1] + needle_offset[1]);
  ds->NeedleMatrix->SetElement(2, 0, position[2] + needle_offset[2]);

}


void vtkProstateNavDataStream::OnRecieveMessageFromScanner(vtkIGTMessageAttributeSet* attrSet, void* arg)
{
  vtkProstateNavDataStream* ds = dynamic_cast<vtkProstateNavDataStream*>(attrSet->GetOpenTrackerStream());
  attrSet->GetAttribute("image", ds->RealtimeImage);
  ds->RealtimeImage->Update();
  ds->ImageTimeStamp->Modified();

}


std::string vtkProstateNavDataStream::GetRobotStatus()
{
  std::string status;
  this->AttrSetRobot->GetAttribute("status", &status);
  return status;
}


void vtkProstateNavDataStream::GetNeedleTransform(vtkTransform* dest)
{
  // this part should be integrated to vtkIGTDataStream

  // Get locator matrix
  float p[3], n[3], t[3], c[3];
  p[0] = this->NeedleMatrix->GetElement(0, 0);
  p[1] = this->NeedleMatrix->GetElement(1, 0);
  p[2] = this->NeedleMatrix->GetElement(2, 0);
  n[0] = this->NeedleMatrix->GetElement(0, 1);
  n[1] = this->NeedleMatrix->GetElement(1, 1);
  n[2] = this->NeedleMatrix->GetElement(2, 1);
  t[0] = this->NeedleMatrix->GetElement(0, 2);
  t[1] = this->NeedleMatrix->GetElement(1, 2);
  t[2] = this->NeedleMatrix->GetElement(2, 2);

  // Ensure N, T orthogonal:
  //    C = N x T
  //    T = C x N
  this->Cross(c, n, t);
  this->Cross(t, c, n);
  
  // Ensure vectors are normalized
  this->Normalize(n);
  this->Normalize(t);
  this->Normalize(c); 

  /*
  # Find transform, N, that brings the locator coordinate frame 
  # into the scanner frame.  Then invert N to M and set it to the locator's
  # userMatrix to position the locator within the world space.
  #
  # 1.) Concatenate a translation, T, TO the origin which is (-x,-y,-z)
  #     where the locator's position is (x,y,z).
  # 2.) Concatenate the R matrix.  If the locator's reference frame has
  #     axis Ux, Uy, Uz, then Ux is the TOP ROW of R, Uy is the second, etc.
  # 3.) Translate the cylinder so its tip is at the origin instead
  #     of the center of its tube.  Call this matrix C.
  # Then: N = C*R*T, M = Inv(N)
  #
  # (See page 419 and 429 of "Computer Graphics", Hearn & Baker, 1997,
  #  ISBN 0-13-530924-7)
  # 
  # The alternative approach used here is to find the transform, M, that
  # moves the scanner coordinate frame to the locator's.  
  # 
  # 1.) Translate the cylinder so its tip is at the origin instead
  #     of the center of its tube.  Call this matrix C.
  # 2.) Concatenate the R matrix.  If the locator's reference frame has
  #     axis Ux, Uy, Uz, then Ux is the LEFT COL of R, Uy is the second,etc.
  # 3.) Concatenate a translation, T, FROM the origin which is (x,y,z)
  #     where the locator's position is (x,y,z).
  # Then: M = T*R*C
  */
  vtkMatrix4x4 *locator_matrix = vtkMatrix4x4::New();
  vtkTransform *locator_transform = vtkTransform::New();

  // Locator's offset: p[0], p[1], p[2]
  float x0 = p[0];
  float y0 = p[1];
  float z0 = p[2];


  // Locator's coordinate axis:
  // Ux = T
  float Uxx = t[0];
  float Uxy = t[1];
  float Uxz = t[2];

  // Uy = -N
  float Uyx = -n[0];
  float Uyy = -n[1];
  float Uyz = -n[2];

  // Uz = Ux x Uy
  float Uzx = Uxy*Uyz - Uyy*Uxz;
  float Uzy = Uyx*Uxz - Uxx*Uyz;
  float Uzz = Uxx*Uyy - Uyx*Uxy;

  // Ux
  locator_matrix->SetElement(0, 0, Uxx);
  locator_matrix->SetElement(1, 0, Uxy);
  locator_matrix->SetElement(2, 0, Uxz);
  locator_matrix->SetElement(3, 0, 0);
  // Uy
  locator_matrix->SetElement(0, 1, Uyx);
  locator_matrix->SetElement(1, 1, Uyy);
  locator_matrix->SetElement(2, 1, Uyz);
  locator_matrix->SetElement(3, 1, 0);
  // Uz
  locator_matrix->SetElement(0, 2, Uzx);
  locator_matrix->SetElement(1, 2, Uzy);
  locator_matrix->SetElement(2, 2, Uzz);
  locator_matrix->SetElement(3, 2, 0);
  // Bottom row
  locator_matrix->SetElement(0, 3, 0);
  locator_matrix->SetElement(1, 3, 0);
  locator_matrix->SetElement(2, 3, 0);
  locator_matrix->SetElement(3, 3, 1);

  // Set the vtkTransform to PostMultiply so a concatenated matrix, C,
  // is multiplied by the existing matrix, M: C*M (not M*C)
  locator_transform->PostMultiply();
  // M = T*R*C
  
  // NORMAL PART
  locator_transform->Identity();
  // C:
  locator_transform->Translate(0, (100 / 2.0), 0);
  // R:
  locator_transform->Concatenate(locator_matrix);
  // T:
  locator_transform->Translate(x0, y0, z0);

  dest->DeepCopy(locator_transform);
  
  locator_matrix->Delete();
  locator_transform->Delete();

}


std::string vtkProstateNavDataStream::GetScanStatus()
{
  /*
  std::string status;
  this->AttrSetScanner->GetAttribute("status", &status);
  return status
  */
  return std::string("");
}


void vtkProstateNavDataStream::SetRobotPosition(std::vector<float> pos, std::vector<float> ori)
{
  vtkIGTMessageAttributeSet attr;
  attr.AddAttribute("position", &pos);
  attr.AddAttribute("orientation", &ori);
  SetAttributes("robot", &attr);
}


void vtkProstateNavDataStream::SetRobotCommand(std::string key, std::string value)
{
  vtkIGTMessageAttributeSet attr;
  attr.AddAttribute(key.c_str(), &value);
  SetAttributes("robot", &attr);
}


void vtkProstateNavDataStream::SetScanPosition(std::vector<float> pos, std::vector<float> ori)
{
}


void vtkProstateNavDataStream::SetScanCommand(std::string key, std::string value)
{
}


void vtkProstateNavDataStream::Normalize(float *a)
{
    float d;
    d = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);

    if (d == 0.0) return;

    a[0] = a[0] / d;
    a[1] = a[1] / d;
    a[2] = a[2] / d;
}


// a = b x c
void vtkProstateNavDataStream::Cross(float *a, float *b, float *c)
{
    a[0] = b[1]*c[2] - c[1]*b[2];
    a[1] = c[0]*b[2] - b[0]*c[2];
    a[2] = b[0]*c[1] - c[0]*b[1];
}
