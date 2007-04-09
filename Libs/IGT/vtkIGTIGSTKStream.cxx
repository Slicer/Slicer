#include "vtkIGTIGSTKStream.h"
#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWApplication.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"

#include "vnl/vnl_float_3.h"

#include "itksys/SystemTools.hxx"
#include "itkStdStreamLogOutput.h"
#ifdef _WIN32
#include "igstkSerialCommunicationForWindows.h"
#else
#include "igstkSerialCommunicationForPosix.h"
#endif


vtkStandardNewMacro(vtkIGTIGSTKStream);
vtkCxxRevisionMacro(vtkIGTIGSTKStream, "$Revision: 1.0 $");



vtkIGTIGSTKStream::vtkIGTIGSTKStream()
{

    this->Speed = 0;
    this->Tracking = 0;
    this->TrackerType = 1;
    this->LocatorNormalTransform = vtkTransform::New();
    this->LocatorMatrix = vtkMatrix4x4::New(); // Identity
    this->RegMatrix = NULL;

    this->AuroraTracker = NULL;
    this->PolarisTracker = NULL;
}



vtkIGTIGSTKStream::~vtkIGTIGSTKStream()
{
    if (this->LocatorNormalTransform)
    {
        this->LocatorNormalTransform->Delete();
    }
    if (this->LocatorMatrix)
    {
        this->LocatorMatrix->Delete();
    }
}



void vtkIGTIGSTKStream::CleanTracker()
{
    if (this->AuroraTracker) 
    {
        this->AuroraTracker->SetLogger(NULL);
        // this->AuroraTracker->SetCommunication(NULL);
        this->AuroraTracker->Delete();
        this->AuroraTracker = NULL;
    }
    if (this->PolarisTracker) 
    {
        this->PolarisTracker->SetLogger(NULL);
        // this->PolarisTracker->SetCommunication(NULL);
        this->PolarisTracker->Delete();
        this->PolarisTracker = NULL;
    }
    /*
    if (this->SerialCommunication)
    {
        this->SerialCommunication->CloseCommunication();
        this->SerialCommunication->Delete();
    }
    */
}



void vtkIGTIGSTKStream::PrintSelf(ostream& os, vtkIndent indent)
{

}



void vtkIGTIGSTKStream::QuaternionToXYZ(float* orientation, float *normal, float *transnormal) 
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


void vtkIGTIGSTKStream::UpdateLocatorMatrix(float *position, float *orientation)
{
    float pos[3];
    float norm[3];
    float transnorm[3];
    int j;


    // the original values are in the unit of meters
    //this part has to be changed
    pos[0]=(float) (position[0] * this->MultiFactor); 
    pos[1]=(float) (position[1] * this->MultiFactor);
    pos[2]=(float) (position[2] * this->MultiFactor);


    QuaternionToXYZ(orientation, norm, transnorm);


    // Apply the transform matrix 
    // to the postion, norm and transnorm
    if (this->RegMatrix)
        ApplyTransform(pos, norm, transnorm);

    for (j=0; j<3; j++) {
        this->LocatorMatrix->SetElement(j,0,pos[j]);
    }


    for (j=0; j<3; j++) {
        this->LocatorMatrix->SetElement(j,1,norm[j]);
    }

    for (j=0; j<3; j++) {
        this->LocatorMatrix->SetElement(j,2,transnorm[j]);
    }

    for (j=0; j<3; j++) {
        this->LocatorMatrix->SetElement(j,3,0);
    }

    for (j=0; j<3; j++) {
        this->LocatorMatrix->SetElement(3,j,0);
    }

    this->LocatorMatrix->SetElement(3,3,1);
}



void vtkIGTIGSTKStream::SetLocatorTransforms()
{
    // Get locator matrix
    vnl_float_3 p, n, t, c;
    
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
    c = vnl_cross_3d(n, t);
    t = vnl_cross_3d(c, n);

    // Ensure vectors are normalized
    n.normalize();
    t.normalize();
    c.normalize(); 

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


}



void vtkIGTIGSTKStream::ApplyTransform(float *position, float *norm, float *transnorm)
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

    this->RegMatrix->MultiplyPoint(p, p);    // transform a position
    this->RegMatrix->MultiplyPoint(n, n);    // transform an orientation
    this->RegMatrix->MultiplyPoint(tn, tn);  // transform an orientation

    for (int i = 0; i < 3; i++)
    {
        position[i] = p[i];
        norm[i] = n[i];
        transnorm[i] = tn[i];
    }
}




void vtkIGTIGSTKStream::ProcessTimerEvents()
{
    if (this->Tracking)
    {
        this->PullRealTime();
        vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                100, this, "ProcessTimerEvents");        
 
    }
    else
    {
        this->StopPulling();
    }
}



void vtkIGTIGSTKStream::Init()
{

    igstk::RealTimeClock::Initialize();

    typedef itk::Logger                   LoggerType; 
    typedef itk::StdStreamLogOutput       LogOutputType;

    igstk::PolarisTrackerTool::Pointer tool = igstk::PolarisTrackerTool::New();
#ifdef _WIN32 
    //running on a windows system
    igstk::SerialCommunicationForWindows::Pointer
        serialComm = igstk::SerialCommunicationForWindows::New();
#else //running on a unix system
    igstk::SerialCommunicationForPosix::Pointer
        serialComm = igstk::SerialCommunicationForPosix::New();
#endif

    // logger object created 
    std::string testName = "IGSTK";
    std::string outputDirectory = EXECUTABLE_OUTPUT_PATH; 
    std::string filename = outputDirectory +"/";
    filename = filename + testName;
    filename = filename + "LoggerOutput.txt";

    std::ofstream loggerFile;
    loggerFile.open( filename.c_str() );
    LoggerType::Pointer   logger = LoggerType::New();
    LogOutputType::Pointer logOutput = LogOutputType::New();  
    logOutput->SetStream( loggerFile );
    logger->AddLogOutput( logOutput );
    logger->SetPriorityLevel( itk::Logger::DEBUG);


    // serialComm->SetLogger( logger );
    serialComm->SetPortNumber(this->PortNumber);
    serialComm->SetParity(this->Parity);
    serialComm->SetBaudRate(this->BaudRate);
    serialComm->SetDataBits(this->DataBits);
    serialComm->SetStopBits(this->StopBits);
    serialComm->SetHardwareHandshake(this->HandShake);


    serialComm->OpenCommunication();

    if (this->TrackerType == 0) // Polaris
    {
        this->PolarisTracker = igstk::PolarisTracker::New();

        // this->PolarisTracker->SetLogger( logger );
        this->PolarisTracker->SetCommunication( serialComm );
        this->PolarisTracker->RequestOpen();
        this->PolarisTracker->RequestInitialize();
        std::cout << "\n\nInit Polaris ...... done.\n\n";
    }
    else
    {
        this->AuroraTracker = igstk::AuroraTracker::New();

        // this->AuroraTracker->SetLogger( logger );
        this->AuroraTracker->SetCommunication( serialComm );
        this->AuroraTracker->RequestOpen();
        this->AuroraTracker->RequestInitialize();
        std::cout << "\n\nInit Aurora ...... done.\n\n";
    }

    itksys::SystemTools::Delay(10000);
 
}



void vtkIGTIGSTKStream::StopPulling()
{
    if (this->TrackerType == 0) // Polaris
    {
        this->PolarisTracker->RequestReset();
        this->PolarisTracker->RequestClose();
    }
    else
    {
        this->AuroraTracker->RequestReset();
        this->AuroraTracker->RequestClose();
    }
 
   // serialComm->CloseCommunication();
    std::cout << "Pulling data ...... done." << std::endl;
    // CleanTracker();
}



void vtkIGTIGSTKStream::PullRealTime()
{

    // unsigned int ntools = this->PolarisTracker->GetNumberOfTools();

    typedef igstk::Transform            TransformType;
    typedef ::itk::Vector<double, 3>    VectorType;
    typedef ::itk::Versor<double>       VersorType;

    TransformType             transform;
    VectorType                position;
    VersorType                rotation;

    float pos[3];
    float rot[4];

    if (this->TrackerType == 0) // Polaris
    {
        this->PolarisTracker->RequestStartTracking();
        for (int y = 0; y < 10; y++)
        {
            this->PolarisTracker->RequestUpdateStatus();
            this->PolarisTracker->GetToolTransform( 0, 0, transform );
            position = transform.GetTranslation();
            rotation = transform.GetRotation();

 
            for (int ii = 0; ii < 3; ii++) pos[ii] = position[ii];
            rot[0] = rotation.GetX();
            rot[1] = rotation.GetY();
            rot[2] = rotation.GetZ();
            rot[3] = rotation.GetW();
            UpdateLocatorMatrix(pos, rot);

            // std::cout << " Position = (" << position[0] << "," << position[1] << "," << position[2] << ")" << std::endl;
            // std::cout << " Rotation = (" << rot[0] << "," << rot[1] << "," << rot[2] << "," << rot[3] << ")" << std::endl;

            this->InvokeEvent (vtkCommand::ModifiedEvent);
            itksys::SystemTools::Delay(this->Speed);
        }

        this->PolarisTracker->RequestStopTracking();
    }
    else  // Aurora
    {
        this->AuroraTracker->RequestStartTracking();
        for (int y = 0; y < 10; y++)
        {
            this->AuroraTracker->RequestUpdateStatus();
            this->AuroraTracker->GetToolTransform( 0, 0, transform );

            position = transform.GetTranslation();
            rotation = transform.GetRotation();

 
            for (int ii = 0; ii < 3; ii++) pos[ii] = position[ii];
            rot[0] = rotation.GetX();
            rot[1] = rotation.GetY();
            rot[2] = rotation.GetZ();
            rot[3] = rotation.GetW();
            UpdateLocatorMatrix(pos, rot);

            // std::cout << " Position = (" << position[0] << "," << position[1] << "," << position[2] << ")" << std::endl;
            // std::cout << " Rotation = (" << rot[0] << "," << rot[1] << "," << rot[2] << "," << rot[3] << ")" << std::endl;

            this->InvokeEvent (vtkCommand::ModifiedEvent);
            itksys::SystemTools::Delay(this->Speed);
        }

        this->AuroraTracker->RequestStopTracking();
    }

    // std::cout << "Pull realtime." << std::endl;
}

