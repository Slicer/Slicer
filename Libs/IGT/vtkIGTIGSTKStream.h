// .NAME vtkIGTIGSTKStream - Central registry to provide control and I/O for
//  trackers and imagers
// .SECTION Description
// vtkIGTIGSTKStream registers arbitary number of trackers and imagers, created MRML nodes in the MRML secene. Designed and Coded by Nobuhiko Hata and Haiying Liu, Jan 12, 2007 @ NA-MIC All Hands Meeting, Salt Lake City, UT

#ifndef IGTIGSTKSTREAM_H
#define IGTIGSTKSTREAM_H


#include "vtkIGTWin32Header.h" 
#include "vtkObject.h"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"

#include <string>

#include "igstkSerialCommunication.h"

#ifdef _WIN32
#include "igstkSerialCommunicationForWindows.h"
#else
#include "igstkSerialCommunicationForPosix.h"
#endif

#include "igstkAuroraTracker.h"
#include "igstkPolarisTracker.h"
#include "itkStdStreamLogOutput.h"


typedef itk::Logger               LoggerType;
typedef itk::StdStreamLogOutput   LogOutputType;


class VTK_IGT_EXPORT vtkIGTIGSTKStream : public vtkObject
{
public:
    static vtkIGTIGSTKStream *New();
    vtkTypeRevisionMacro(vtkIGTIGSTKStream,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    vtkSetMacro(Speed,int);
    vtkSetMacro(MultiFactor,float);
    vtkSetMacro(StartTimer,int);
    vtkSetMacro(TrackerType,short);


    vtkSetObjectMacro(RegMatrix,vtkMatrix4x4);
    vtkGetObjectMacro(RegMatrix,vtkMatrix4x4);

    vtkGetObjectMacro(LocatorMatrix,vtkMatrix4x4);
    vtkGetObjectMacro(LocatorNormalTransform,vtkTransform);

    /**
     * Constructor
     */
    vtkIGTIGSTKStream();


    //Description:
    //Destructor
    virtual ~vtkIGTIGSTKStream ( );


    void Init();
    void StopPolling();
    void PollRealtime();
    void SetLocatorTransforms();
    void ProcessTimerEvents();

    void callbackF(double*, double*);

private:

    //BTX
    igstk::AuroraTracker::Pointer        AuroraTracker;
    igstk::PolarisTracker::Pointer       PolarisTracker;

    igstk::SerialCommunication::Pointer  SerialCommunication;

    LoggerType::Pointer                  Logger;
    LogOutputType::Pointer               LogFileOutput;  // log output to file
    //ETX


    int Speed;
    int StartTimer;
    float MultiFactor;
    short TrackerType;  // 0 - Aurora; 1 - Polaris

    vtkMatrix4x4 *LocatorMatrix;
    vtkMatrix4x4 *RegMatrix;
    vtkTransform *LocatorNormalTransform;

    void quaternion2xyz(float* orientation, float *normal, float *transnormal); 
    void ApplyTransform(float *position, float *norm, float *transnorm);

};

#endif // IGTIGSTKSTREAM_H

