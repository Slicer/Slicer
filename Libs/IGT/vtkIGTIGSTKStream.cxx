
#include "vtkIGTIGSTKSteeam.h"
#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWApplication.h"
#include "vtkCommand.h"


#include <vtksys/SystemTools.hxx>
#include "vtkCallbackCommand.h"


vtkStandardNewMacro(vtkIGTIGSTKSteeam);
vtkCxxRevisionMacro(vtkIGTIGSTKSteeam, "$Revision: 1.0 $");



vtkIGTIGSTKSteeam::vtkIGTIGSTKSteeam()
{
    igstk::RealTimeClock::Initialize();

    typedef itk::Logger        LoggerType;
    typedef itk::StdStreamLogOutput        LogOutputType;

    LoggerType::Pointer          m_Logger;
    m_Logger   = LoggerType::New();

    /** Logger */
    LogOutputType::Pointer              m_LogFileOutput;  // log output to file
    std::ofstream                       m_LogFile;        // file stream

    m_LogFileOutput = LogOutputType::New();
    std::string logFileName = "logIGSTK.txt";
    m_LogFile.open( logFileName.c_str() );
    if( !m_LogFile.fail() )
    {
        m_LogFileOutput->SetStream( m_LogFile );
        m_Logger->AddLogOutput( m_LogFileOutput );
    }


#ifdef _WIN32 
    //running on a windows system
    serialCommunication = igstk::SerialCommunicationForWindows::New();
#else //running on a unix system
    serialCommunication = igstk::SerialCommunicationForPosix::New();
#endif

    //serialCommunication->SetLogger( m_Logger );
    //set the communication settings
    //This is the serial port of your device. 'PortNumber2' == COM3 under windows
    serialCommunication->SetPortNumber(igstk::SerialCommunication::PortNumber2);

    serialCommunication->SetParity(igstk::SerialCommunication::NoParity);
    serialCommunication->SetBaudRate(igstk::SerialCommunication::BaudRate115200);
    serialCommunication->SetDataBits(igstk::SerialCommunication::DataBits8);
    serialCommunication->SetStopBits(igstk::SerialCommunication::StopBits1);
    serialCommunication->SetHardwareHandshake(igstk::SerialCommunication::HandshakeOff);  
    serialCommunication->OpenCommunication();  

    //Instantiate the tracker here
    tracker = igstk::PolarisTracker::New();
    tracker->SetLogger( m_Logger );
    tracker->SetCommunication(serialCommunication);

    //attach SROM file 
    tracker->AttachSROMFileNameToPort(3, "8700340.rom");
    tracker->RequestOpen();          
    tracker->RequestInitialize();
    tracker->RequestStartTracking();  

    igstk::Transform transform;               
    igstk::Transform::VectorType translation;
    igstk::Transform::VersorType rotation;
    std::cout<<"Start data acquisition\n";
    for(int i=0; i<100; i++) 
    {
        //get the tracking data for all tools
        tracker->RequestUpdateStatus();

        tracker->GetToolTransform(3, 0, transform);
        //translation = transform.GetTranslation();
        //rotation = transform.GetRotation(); 
        igstkLogMacro2( m_Logger, DEBUG, transform << "\n" );
        //igstkLogMacro2( m_Logger, DEBUG, translation << "\n" );
        //igstkLogMacro2( m_Logger, DEBUG, rotation << "\n" );
    }
    std::cout<<"End data acquisition.\n";

    tracker->RequestStopTracking();
    tracker->RequestClose();
    serialCommunication->CloseCommunication();



    this->Speed = 0;
    this->StartTimer = 0;
    this->LocatorNormalTransform = vtkTransform::New();
    this->LocatorMatrix = vtkMatrix4x4::New(); // Identity
    this->RegMatrix = NULL;
}



vtkIGTIGSTKSteeam::~vtkIGTIGSTKSteeam()
{
    this->LocatorNormalTransform->Delete();
    this->LocatorMatrix->Delete();

}



void vtkIGTIGSTKSteeam::Init(char *configFile)
{


}



void vtkIGTIGSTKSteeam::StopPolling()
{


}



void vtkIGTIGSTKSteeam::PollRealtime()
{


}



void vtkIGTIGSTKSteeam::PrintSelf(ostream& os, vtkIndent indent)
{


}



void vtkIGTIGSTKSteeam::quaternion2xyz(float* orientation, float *normal, float *transnormal) 
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


///Event becomes IGSTK event to extract the info
////////////////Edit here Noby Tuesday Feb 13
/////////////// Edit here
void vtkIGTIGSTK::callbackF(double* position, double* orientation)
{
  float f_position[3];
  float f_orientation[4];
    float norm[3];
    float transnorm[3];
    int j;

    vtkIGTDataManager *VOT=(vtkIGTDataManager *)this;

    // the original values are in the unit of meters
    //this part has to be changed
    f_position[0]=(float)position[0] * VOT->Ratio; 
    f_position[1]=(float)position[1] * VOT->Ratio;
    f_position[2]=(float)position[2] * VOT->Ratio;

    f_orientation[0]=(float)orientation[0];
    f_orientation[1]=(float)orientation[1];
    f_orientation[2]=(float)orientation[2];
    f_orientation[3]=(float)orientation[3];




    VOT->quaternion2xyz(f_orientation, norm, transnorm);


    // Apply the transform matrix 
    // to the postion, norm and transnorm
    if (VOT->RegMatrix)
        VOT->ApplyTransform(f_position, norm, transnorm);

    for (j=0; j<3; j++) {
        VOT->LocatorMatrix->SetElement(j,0,position[j]);
    }


    for (j=0; j<3; j++) {
        VOT->LocatorMatrix->SetElement(j,1,norm[j]);
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



void vtkIGTIGSTKSteeam::SetLocatorTransforms()
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


}



void vtkIGTIGSTKSteeam::Normalize(float *a)
{
    float d;
    d = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);

    if (d == 0.0) return;

    a[0] = a[0] / d;
    a[1] = a[1] / d;
    a[2] = a[2] / d;
}



// a = b x c
void vtkIGTIGSTKSteeam::Cross(float *a, float *b, float *c)
{
    a[0] = b[1]*c[2] - c[1]*b[2];
    a[1] = c[0]*b[2] - b[0]*c[2];
    a[2] = b[0]*c[1] - c[0]*b[1];
}



void vtkIGTIGSTKSteeam::ApplyTransform(float *position, float *norm, float *transnorm)
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



void vtkIGTIGSTKSteeam::ProcessTimerEvents()
{
    if (this->StartTimer)
    {
        this->PollRealtime();
        this->InvokeEvent (vtkCommand::ModifiedEvent);
        vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                200, this, "ProcessTimerEvents");        
    }
    else
    {
        this->StopPolling();
    }
}

