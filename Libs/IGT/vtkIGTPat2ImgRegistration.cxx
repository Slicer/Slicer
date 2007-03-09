
#include "vtkIGTPat2ImgRegistration.h"
#include "vtkObjectFactory.h"

#include "vtkLandmarkTransform.h"
#include "vtkTransform.h"



vtkStandardNewMacro(vtkIGTPat2ImgRegistration);
vtkCxxRevisionMacro(vtkIGTPat2ImgRegistration, "$Revision: 1.0 $");

vtkIGTPat2ImgRegistration::vtkIGTPat2ImgRegistration()
{
    this->SourceLandmarks = NULL; 
    this->TargetLandmarks = NULL; 
    this->NumberOfPoints = 0;

    this->LandmarkTransformMatrix = vtkMatrix4x4::New(); // Identity

}


vtkIGTPat2ImgRegistration::~vtkIGTPat2ImgRegistration()
{
    this->LandmarkTransformMatrix->Delete();

    if (this->SourceLandmarks)
    {
        this->SourceLandmarks->Delete();
    }
    if (this->TargetLandmarks)
    {
        this->TargetLandmarks->Delete();
    }

}


void vtkIGTPat2ImgRegistration::SetNumberOfPoints(int no)
{
    if (this->TargetLandmarks)
    {
        this->TargetLandmarks->Delete();
    }
    this->TargetLandmarks = vtkPoints::New();
    this->TargetLandmarks->SetDataTypeToFloat();
    this->TargetLandmarks->SetNumberOfPoints(no);


    if (this->SourceLandmarks)
    {
        this->SourceLandmarks->Delete();
    }
    this->SourceLandmarks = vtkPoints::New();
    this->SourceLandmarks->SetDataTypeToFloat();
    this->SourceLandmarks->SetNumberOfPoints(no);

    this->NumberOfPoints = no;
}


void vtkIGTPat2ImgRegistration::AddPoint(int id, float t1, float t2, float t3, float s1, float s2, float s3)
{
    this->TargetLandmarks->InsertPoint(id, t1, t2, t3);
    this->TargetLandmarks->Modified();

    this->SourceLandmarks->InsertPoint(id, s1, s2, s3);
    this->SourceLandmarks->Modified();
}



int vtkIGTPat2ImgRegistration::DoRegistration()
{

    if (this->TargetLandmarks == NULL || this->SourceLandmarks == NULL)
    {
        vtkErrorMacro(<< "vtkIGTPat2ImgRegistration::DoRegistration(): Got NULL pointer.");
        return 1;
    }

    int tnp = this->TargetLandmarks->GetNumberOfPoints();
    int snp = this->SourceLandmarks->GetNumberOfPoints();
    if (tnp < 2 || snp < 2)
    {
        vtkErrorMacro(<< "vtkIGTPat2ImgRegistration::DoRegistration(): Number of points is less than 2 in either TargetLandmarks or SourceLandmarks.");
        return 1;
    }

    if (tnp != snp)
    {
        vtkErrorMacro(<< "vtkIGTPat2ImgRegistration::DoRegistration(): TargetLandmarks != SourceLandmarks in terms of number of points.");
        return 1;
    }

    vtkLandmarkTransform *landmark = vtkLandmarkTransform::New();
    landmark->SetTargetLandmarks(this->TargetLandmarks);
    landmark->SetSourceLandmarks(this->SourceLandmarks);
    landmark->SetModeToRigidBody();
    landmark->Update();
    this->LandmarkTransformMatrix->DeepCopy(landmark->GetMatrix());

    landmark->Delete();
    return 0; 
}

void vtkIGTPat2ImgRegistration::PrintSelf(ostream& os, vtkIndent indent)
{


}

