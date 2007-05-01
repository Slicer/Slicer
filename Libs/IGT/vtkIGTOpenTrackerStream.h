// .NAME vtkIGTOpenTrackerStream - Central registry to provide control and I/O for
//  trackers and imagers
// .SECTION Description
// vtkIGTOpenTrackerStream registers arbitary number of trackers and imagers, created MRML nodes in the MRML secene. Designed and Coded by Nobuhiko Hata and Haiying Liu, Jan 12, 2007 @ NA-MIC All Hands Meeting, Salt Lake City, UT

#ifndef IGTOPENTRACKERSTREAM_H
#define IGTOPENTRACKERSTREAM_H


#include <string>
#include <vector>

#include "vtkIGTWin32Header.h" 
#include "vtkObject.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"

#include "OpenTracker/OpenTracker.h"
#include "OpenTracker/common/CallbackModule.h"


using namespace ot;


class VTK_IGT_EXPORT vtkIGTOpenTrackerStream : public vtkObject
{
public:


    static vtkIGTOpenTrackerStream *New();
    vtkTypeRevisionMacro(vtkIGTOpenTrackerStream,vtkObject);
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
    vtkIGTOpenTrackerStream();


    //Description:
    //Destructor
    virtual ~vtkIGTOpenTrackerStream ( );


    void Init(const char *configFile);
    void StopPolling();
    void PollRealtime();
    void SetLocatorTransforms();
    void ProcessTimerEvents();

    static void callbackF(const Node&, const Event &event, void *data);


private:

    int Speed;
    int StartTimer;
    float MultiFactor;

    vtkMatrix4x4 *LocatorMatrix;
    vtkMatrix4x4 *RegMatrix;
    vtkTransform *LocatorNormalTransform;

    Context *context;

    void Normalize(float *a);
    void Cross(float *a, float *b, float *c);
    void ApplyTransform(float *position, float *norm, float *transnorm);
    void CloseConnection();

    void quaternion2xyz(float* orientation, float *normal, float *transnormal); 


};

#endif // IGTOPENTRACKERSTREAM_H

