
#include "vtkIGTOpenTrackerStream.h"
#include "vtkObjectFactory.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWApplication.h"
#include "vtkCommand.h"



#include <vtksys/SystemTools.hxx>
#include "vtkCallbackCommand.h"


vtkStandardNewMacro(vtkIGTOpenTrackerStream);
vtkCxxRevisionMacro(vtkIGTOpenTrackerStream, "$Revision: 1.0 $");

vtkIGTOpenTrackerStream::vtkIGTOpenTrackerStream()
{
    this->context = NULL;
}


vtkIGTOpenTrackerStream::~vtkIGTOpenTrackerStream()
{
    if (this->context)
    {
        delete this->context;
    }

}

void vtkIGTOpenTrackerStream::Init(const char *configFile)
{
    fprintf(stderr,"config file: %s\n",configFile);
    this->context = new Context(1); 
    // get callback module from the context
    CallbackModule * callbackMod = (CallbackModule *)context->getModule("CallbackConfig");

    // parse the configuration file
    context->parseConfiguration(configFile);  

    // sets the callback function
    // if we use NaviTrack (not opentracker), use this function
    // callbackMod->setCallback( "cb1", (OTCallbackFunction*)&callbackF ,this);    
    callbackMod->setCallback( "cb1", (OTCallbackFunction*)&callbackF ,this);    

    context->start();
}



void vtkIGTOpenTrackerStream::callbackF(const Node&, const Event &event, void *data)
{
    float position[3];
    float orientation[4];
    float norm[3];
    float transnorm[3];
    int j;

    vtkIGTOpenTrackerStream *VOT=(vtkIGTOpenTrackerStream *)data;


    // the original values are in the unit of meters
    position[0]=(float)(event.getPosition())[0] * VOT->MultiFactor; 
    position[1]=(float)(event.getPosition())[1] * VOT->MultiFactor;
    position[2]=(float)(event.getPosition())[2] * VOT->MultiFactor;

    orientation[0]=(float)(event.getOrientation())[0];
    orientation[1]=(float)(event.getOrientation())[1];
    orientation[2]=(float)(event.getOrientation())[2];
    orientation[3]=(float)(event.getOrientation())[3];

    VOT->QuaternionToXYZ(orientation, norm, transnorm);


    // Apply the transform matrix 
    // to the postion, norm and transnorm
    if (VOT->RegMatrix)
        VOT->ApplyTransform(position, norm, transnorm);

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



void vtkIGTOpenTrackerStream::StopPulling()
{
    context->close();
}



void vtkIGTOpenTrackerStream::PullRealTime()
{
    context->pushEvents();       // push event and
    context->pullEvents();       // pull event 
    context->stop();
}



void vtkIGTOpenTrackerStream::PrintSelf(ostream& os, vtkIndent indent)
{


}

