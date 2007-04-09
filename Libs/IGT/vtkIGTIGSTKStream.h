// .NAME vtkIGTIGSTKStream - Central registry to provide control and I/O for
//  trackers and imagers
// .SECTION Description
// vtkIGTIGSTKStream registers arbitary number of trackers and imagers, created MRML nodes in the MRML secene. Designed and Coded by Nobuhiko Hata and Haiying Liu, Jan 12, 2007 @ NA-MIC All Hands Meeting, Salt Lake City, UT

#ifndef IGTIGSTKSTREAM_H
#define IGTIGSTKSTREAM_H


#include "itkCommand.h"

#include "vtkIGTWin32Header.h" 
#include "vtkObject.h"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"

#include <string>

#include "igstkSerialCommunication.h"
#include "igstkAuroraTracker.h"
#include "igstkPolarisTracker.h"


typedef igstk::SerialCommunication::PortNumberType PortNumberT;
typedef igstk::SerialCommunication::BaudRateType   BaudRateT;
typedef igstk::SerialCommunication::DataBitsType   DataBitsT;
typedef igstk::SerialCommunication::ParityType     ParityT;
typedef igstk::SerialCommunication::StopBitsType   StopBitsT;
typedef igstk::SerialCommunication::HandshakeType  HandshakeT;


class VTK_IGT_EXPORT vtkIGTIGSTKStream : public vtkObject
{
public:
    static vtkIGTIGSTKStream *New();
    vtkTypeRevisionMacro(vtkIGTIGSTKStream,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);


    vtkSetMacro(PortNumber,PortNumberT);
    vtkSetMacro(BaudRate,BaudRateT);
    vtkSetMacro(DataBits,DataBitsT);
    vtkSetMacro(Parity,ParityT);
    vtkSetMacro(StopBits,StopBitsT);
    vtkSetMacro(HandShake,HandshakeT);


    vtkSetMacro(Speed,int);
    vtkSetMacro(Tracking,int);
    vtkSetMacro(MultiFactor,float);
    vtkSetMacro(TrackerType,short);


    vtkSetObjectMacro(RegMatrix,vtkMatrix4x4);
    vtkGetObjectMacro(RegMatrix,vtkMatrix4x4);

    vtkGetObjectMacro(LocatorMatrix,vtkMatrix4x4);
    vtkGetObjectMacro(LocatorNormalTransform,vtkTransform);

    /**
     * Constructor    vtkSetMacro(StopBits,StopBitsType);    vtkSetMacro(StopBits,StopBitsType);
     */
    vtkIGTIGSTKStream();


    //Description:
    //Destructor
    virtual ~vtkIGTIGSTKStream ( );

    void ProcessTimerEvents();

    void Init();
    void StopPulling();
    void PullRealTime();    
    void SetLocatorTransforms();
    void CleanTracker();

private:

    //BTX
    igstk::AuroraTracker::Pointer        AuroraTracker;
    igstk::PolarisTracker::Pointer       PolarisTracker;


    // Communication Parameters
    /**  Port Number */
    PortNumberT PortNumber;   
    /** Baud rate of communication */
    BaudRateT   BaudRate;  
    /** Number of bits/byte */
    DataBitsT   DataBits;
    /** Parity */
    ParityT     Parity;
    /** Stop bits */
    StopBitsT   StopBits;
    /** Hardware handshaking */
    HandshakeT  HandShake;

    //ETX

    int Speed;
    float MultiFactor;
    short TrackerType;  // 0 - Polaris; 1 - Aurora 
    int Tracking;

    vtkMatrix4x4 *LocatorMatrix;
    vtkMatrix4x4 *RegMatrix;
    vtkTransform *LocatorNormalTransform;

    void UpdateLocatorMatrix(float *positon, float *orientation);
    void QuaternionToXYZ(float *orientation, float *normal, float *transnormal); 
    void ApplyTransform(float *position, float *norm, float *transnorm);

};




#endif // IGTIGSTKSTREAM_H

