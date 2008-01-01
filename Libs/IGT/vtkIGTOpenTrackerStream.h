// .NAME vtkIGTOpenTrackerStream - Central registry to provide control and I/O for
//  trackers and imagers
// .SECTION Description
// vtkIGTOpenTrackerStream registers arbitary number of trackers and imagers, created MRML nodes in the MRML secene. Designed and Coded by Nobuhiko Hata and Haiying Liu, Jan 12, 2007 @ NA-MIC All Hands Meeting, Salt Lake City, UT

#ifndef IGTOPENTRACKERSTREAM_H
#define IGTOPENTRACKERSTREAM_H


#include "vtkIGTWin32Header.h" 
#include "vtkIGTDataStream.h"

#include "vtkIGTMessageAttributeSet.h"

#include "OpenTracker/OpenTracker.h"
#include "OpenTracker/common/CallbackModule.h"

#include <map>

using namespace ot;


class VTK_IGT_EXPORT vtkIGTOpenTrackerStream : public vtkIGTDataStream
{
public:

    static vtkIGTOpenTrackerStream *New();
    vtkTypeRevisionMacro(vtkIGTOpenTrackerStream,vtkIGTDataStream);
    void PrintSelf(ostream& os, vtkIndent indent);

    /**
     * Constructor
     */
    vtkIGTOpenTrackerStream();


    //Description:
    //Destructor
    virtual ~vtkIGTOpenTrackerStream ( );

    void Init(const char *configFile);
    static void callbackF(const Node&, const Event &event, void *data);
    static void GenericCallback(const Node &node, const Event &event, void *data);
 
    //BTX
    // Register callback functions to process data from NaviTrack data stream.
    // AddCallback() should be called before Init();
    void AddCallback(const char* cbname,
                     vtkIGTMessageAttributeSet::MessageHandlingFunction* func,
                     vtkIGTMessageAttributeSet* attrSet,
                     void* arg);

    // Send data to NaviTrack data stream.
    void SetAttributes(const char* srcName, vtkIGTMessageAttributeSet* attrSet);

    //ETX
    void StopPulling();
    void PullRealTime();    


private:

    Context *context;

    void CloseConnection();
    //BTX
    vtkIGTMessageAttributeSet::AttributeSetMap     AttributeSetMap;
    friend class vtkIGTMessageAttributeSet;
    //ETX
    
};

#endif // IGTOPENTRACKERSTREAM_H

