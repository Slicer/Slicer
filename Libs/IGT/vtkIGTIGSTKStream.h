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

#include <igstkSerialCommunication.h>

#ifdef _WIN32
#include <igstkSerialCommunicationForWindows.h>
#else
#include <igstkSerialCommunicationForPosix.h>
#endif

#include <igstkAuroraTracker.h>
#include <igstkPolarisTracker.h>
#include "itkStdStreamLogOutput.h"


class VTK_IGT_EXPORT vtkIGTIGSTKStream : public vtkObject
{
public:


    static vtkIGTIGSTKStream *New();
    vtkTypeRevisionMacro(vtkIGTIGSTKStream,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    vtkSetMacro(Speed,int);
    vtkSetMacro(MultiFactor,float);

    vtkSetMacro(StartTimer,int);



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


    void Init(char *configFile);
    void StopPolling();
    void PollRealtime();
    void SetLocatorTransforms();
    void ProcessTimerEvents();

    void callbackF(double*, double*);

private:

    //BTX
    igstk::PolarisTracker::Pointer       tracker;
    igstk::SerialCommunication::Pointer  serialCommunication;
    //ETX


    int Speed;
    int StartTimer;
    float MultiFactor;

    vtkMatrix4x4 *LocatorMatrix;
    vtkMatrix4x4 *RegMatrix;
    vtkTransform *LocatorNormalTransform;

    void Normalize(float *a);
    void Cross(float *a, float *b, float *c);
    void ApplyTransform(float *position, float *norm, float *transnorm);
    void CloseConnection();

    void quaternion2xyz(float* orientation, float *normal, float *transnormal); 


};

#endif // IGTIGSTKSTREAM_H

