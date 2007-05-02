#include "vtkIGTIGSTKStream.h"
#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWApplication.h"
#include "vtkCommand.h"
// #include "vtkCallbackCommand.h"

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



void vtkIGTIGSTKStream::ProcessTimerEvents()
{
    if (this->Tracking)
    {
        this->PullRealTime();
        vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                this->Speed, this, "ProcessTimerEvents");        
 
    }
    else
    {
        this->StopPulling();
    }
}



