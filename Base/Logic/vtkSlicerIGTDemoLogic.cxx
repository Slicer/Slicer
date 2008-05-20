/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkSlicerIGTDemoLogic.h"

#include "vtkLandmarkTransform.h"
#include "vtkCylinderSource.h"

#include "vtkSlicerConfigure.h" /* Slicer3_USE_* */

vtkCxxRevisionMacro(vtkSlicerIGTDemoLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerIGTDemoLogic);

vtkSlicerIGTDemoLogic::vtkSlicerIGTDemoLogic()
{
    this->LocatorMatrix = vtkMatrix4x4::New(); // Identity
    this->LandmarkTransformMatrix = vtkMatrix4x4::New(); // Identity

    this->SourceLandmarks = NULL; 
    this->TargetLandmarks = NULL; 

    this->UseRegistration = 0;
    this->NumberOfPoints = 0;

    this->p[0] = 0.0;
    this->p[1] = 0.0;
    this->p[2] = 0.0;
    this->n[0] = 0.0;
    this->n[1] = 0.0;
    this->n[2] = 0.0;

    this->pOld[0] = 0.0;
    this->pOld[1] = 0.0;
    this->pOld[2] = 0.0;
    this->nOld[0] = 0.0;
    this->nOld[1] = 0.0;
    this->nOld[2] = 0.0;

    this->LocatorNormalTransform = vtkTransform::New();

}



vtkSlicerIGTDemoLogic::~vtkSlicerIGTDemoLogic()
{
    if (this->LocatorMatrix)
    {
        this->LocatorMatrix->Delete();
    }
    if (this->LandmarkTransformMatrix)
    {
        this->LandmarkTransformMatrix->Delete();
    }
    if (this->SourceLandmarks)
    {
        this->SourceLandmarks->Delete();
    }
    if (this->TargetLandmarks)
    {
        this->TargetLandmarks->Delete();
    }
    if (this->LocatorNormalTransform)
    {
        this->LocatorNormalTransform->Delete();
    }

}



void vtkSlicerIGTDemoLogic::SetNumberOfPoints(int no)
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



void vtkSlicerIGTDemoLogic::Init(char *configfile)
{

#ifdef Slicer3_USE_OPENTRACKER
    fprintf(stderr,"config file: %s\n",configfile);
    this->context = new Context(1); 
    // get callback module from the context
    CallbackModule * callbackMod = (CallbackModule *)context->getModule("CallbackConfig");

    context->parseConfiguration(configfile);  // parse the configuration file

    callbackMod->setCallback( "cb1", (OTCallbackFunction*)&callbackF ,this);    // sets the callback function


    context->start();
#else
    // TODO: open a file

#endif
}



void vtkSlicerIGTDemoLogic::CloseConnection()
{
#ifdef Slicer3_USE_OPENTRACKER
    context->close();
#endif

}



void vtkSlicerIGTDemoLogic::quaternion2xyz(float* orientation, float *normal, float *transnormal) 
{
    float q0, qx, qy, qz;

    q0 = orientation[3];
    qx = orientation[0];
    qy = orientation[1];
    qz = orientation[2]; 

    transnormal[0] = 1-2*qy*qy-2*qz*qz;
    transnormal[1] = 2*qx*qy+2*qz*q0;
    transnormal[2] = 2*qx*qz-2*qy*q0;

    normal[0] = 2*qx*qz+2*qy*q0;
    normal[1] = 2*qy*qz-2*qx*q0;
    normal[2] = 1-2*qx*qx-2*qy*qy;
}



void vtkSlicerIGTDemoLogic::PollRealtime()
{
#ifdef Slicer3_USE_OPENTRACKER
    context->pushEvents();       // push event and
    context->pullEvents();       // pull event 
    context->stop();
#endif
}


#ifdef Slicer3_USE_OPENTRACKER
void vtkSlicerIGTDemoLogic::callbackF(const Node&, const Event &event, void *data)
{
    float position[3];
    float orientation[4];
    float norm[3];
    float transnorm[3];
    int j;

    vtkSlicerIGTDemoLogic *VOT=(vtkSlicerIGTDemoLogic *)data;

    // the original values are in the unit of meters
    position[0]=(float)(event.getPosition())[0] * 1000.0; 
    position[1]=(float)(event.getPosition())[1] * 1000.0;
    position[2]=(float)(event.getPosition())[2] * 1000.0;

    orientation[0]=(float)(event.getOrientation())[0];
    orientation[1]=(float)(event.getOrientation())[1];
    orientation[2]=(float)(event.getOrientation())[2];
    orientation[3]=(float)(event.getOrientation())[3];

    VOT->quaternion2xyz(orientation, norm, transnorm);


    // Apply the transform matrix 
    // to the postion, norm and transnorm
    if (VOT->UseRegistration)
        VOT->ApplyTransform(position, norm, transnorm);

    for (j=0; j<3; j++) {
        VOT->LocatorMatrix->SetElement(j,0,position[j]);
        VOT->p[j] = position[j];
    }


    for (j=0; j<3; j++) {
        VOT->LocatorMatrix->SetElement(j,1,norm[j]);
        VOT->n[j] = norm[j];
    }

    for (j=0; j<3; j++) {
        VOT->LocatorMatrix->SetElement(j,2,transnorm[j]);
    }

    for (j=0; j<3; j++) {
        VOT->LocatorMatrix->SetElement(j,3,0);
    }

    for (j=0; j<3; j++) {
        VOT->LocatorMatrix->SetElement(3,j,0);
    }

    VOT->LocatorMatrix->SetElement(3,3,1);
}
#endif


void vtkSlicerIGTDemoLogic::ApplyTransform(float *position, float *norm, float *transnorm)
{
    // Transform position, norm and transnorm
    // ---------------------------------------------------------
    float p[4];
    float n[4];
    float tn[4];

    for (int i = 0; i < 3; i++)
    {
        p[i] = position[i];
        n[i] = norm[i];
        tn[i] = transnorm[i];
    }
    p[3] = 1;     // translation affects a poistion
    n[3] = 0;     // translation doesn't affect an orientation
    tn[3] = 0;    // translation doesn't affect an orientation

    this->LandmarkTransformMatrix->MultiplyPoint(p, p);    // transform a position
    this->LandmarkTransformMatrix->MultiplyPoint(n, n);    // transform an orientation
    this->LandmarkTransformMatrix->MultiplyPoint(tn, tn);  // transform an orientation

    for (int i = 0; i < 3; i++)
    {
        position[i] = p[i];
        norm[i] = n[i];
        transnorm[i] = tn[i];
    }
}



void vtkSlicerIGTDemoLogic::AddPoint(int id, float t1, float t2, float t3, float s1, float s2, float s3)
{
    this->TargetLandmarks->InsertPoint(id, t1, t2, t3);
    this->TargetLandmarks->Modified();

    this->SourceLandmarks->InsertPoint(id, s1, s2, s3);
    this->SourceLandmarks->Modified();
}



int vtkSlicerIGTDemoLogic::DoRegistration()
{

    if (this->TargetLandmarks == NULL || this->SourceLandmarks == NULL)
    {
        vtkErrorMacro(<< "vtkSlicerIGTDemoLogic::DoRegistration(): Got NULL pointer.");
        return 1;
    }

    int tnp = this->TargetLandmarks->GetNumberOfPoints();
    int snp = this->SourceLandmarks->GetNumberOfPoints();
    if (tnp < 2 || snp < 2)
    {
        vtkErrorMacro(<< "vtkSlicerIGTDemoLogic::DoRegistration(): Number of points is less than 2 in either TargetLandmarks or SourceLandmarks.");
        return 1;
    }

    if (tnp != snp)
    {
        vtkErrorMacro(<< "vtkSlicerIGTDemoLogic::DoRegistration(): TargetLandmarks != SourceLandmarks in terms of number of points.");
        return 1;
    }

    vtkLandmarkTransform *landmark = vtkLandmarkTransform::New();
    landmark->SetTargetLandmarks(this->TargetLandmarks);
    landmark->SetSourceLandmarks(this->SourceLandmarks);
    landmark->SetModeToRigidBody();
    landmark->Update();
    this->LandmarkTransformMatrix->DeepCopy(landmark->GetMatrix());

    landmark->Delete();

    this->UseRegistration = 1;

    return 0; 
}



void vtkSlicerIGTDemoLogic::ProcessMRMLEvents(vtkObject * caller, unsigned long event, void * callData)
{
    // TODO: implement if needed
}



void vtkSlicerIGTDemoLogic::SetLocatorTransforms()
{
    // Get locator matrix
    float p[3], n[3], t[3], c[3];
    p[0] = this->LocatorMatrix->GetElement(0, 0);
    p[1] = this->LocatorMatrix->GetElement(1, 0);
    p[2] = this->LocatorMatrix->GetElement(2, 0);
    n[0] = this->LocatorMatrix->GetElement(0, 1);
    n[1] = this->LocatorMatrix->GetElement(1, 1);
    n[2] = this->LocatorMatrix->GetElement(2, 1);
    t[0] = this->LocatorMatrix->GetElement(0, 2);
    t[1] = this->LocatorMatrix->GetElement(1, 2);
    t[2] = this->LocatorMatrix->GetElement(2, 2);


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

    this->LocatorNormalTransform->DeepCopy(locator_transform);

    locator_matrix->Delete();
    locator_transform->Delete();


    // save 
    for (int i = 0; i < 3; i++)
    {
        this->pOld[i] = this->p[i];  
        this->nOld[i] = this->n[i];  
    }
}



void vtkSlicerIGTDemoLogic::UpdateSliceImages()
{


}



void vtkSlicerIGTDemoLogic::Normalize(float *a)
{
    float d;
    d = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);

    if (d == 0.0) return;

    a[0] = a[0] / d;
    a[1] = a[1] / d;
    a[2] = a[2] / d;
}



// a = b x c
void vtkSlicerIGTDemoLogic::Cross(float *a, float *b, float *c)
{
    a[0] = b[1]*c[2] - c[1]*b[2];
    a[1] = c[0]*b[2] - b[0]*c[2];
    a[2] = b[0]*c[1] - c[0]*b[1];
}



void vtkSlicerIGTDemoLogic::PrintSelf(ostream& os, vtkIndent indent)
{
    this->vtkObject::PrintSelf(os, indent);

    os << indent << "vtkSlicerIGTDemoLogic:             " << this->GetClassName() << "\n";

}

