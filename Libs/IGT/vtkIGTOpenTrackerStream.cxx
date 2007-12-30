
#include "vtkIGTOpenTrackerStream.h"
#include "vtkObjectFactory.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWApplication.h"
#include "vtkCommand.h"



#include <vtksys/SystemTools.hxx>
#include "vtkCallbackCommand.h"

#include "OpenTracker/OpenTracker.h"
#include "OpenTracker/common/CallbackModule.h"
#include "OpenTracker/input/SlicerModule.h"
//#include "OpenTracker/input/SlicerSource.h"
#include "OpenTracker/types/Image.h"

#include "vtkIGTMessageGenericAttribute.h"
#include "vtkIGTMessageImageDataAttribute.h"
#include "vtkImageData.h"


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

    // Register NaviTrack module
    addSPLModules();

    this->context = new Context(1); 
    // get callback module from the context
    CallbackModule * callbackMod = (CallbackModule *)context->getModule("CallbackConfig");

    // parse the configuration file
    context->parseConfiguration(configFile);  

    // sets the callback function
    // if we use NaviTrack (not opentracker), use this function
    // callbackMod->setCallback( "cb1", (OTCallbackFunction*)&callbackF ,this);    
    callbackMod->setCallback( "cb1", (OTCallbackFunction*)&callbackF ,this);

    // Add additional callbacks
    vtkIGTMessageAttributeSet::AttributeSetMap::iterator iter;
    for (iter = AttributeSetMap.begin(); iter != AttributeSetMap.end(); iter ++)
      {
      const char* cbname                 = iter->first.c_str();
      vtkIGTMessageAttributeSet* attrSet = iter->second;
      //std::cerr << "Adding Callback: " << cbname << std::endl;

      callbackMod->setCallback(cbname, (OTCallbackFunction*)&GenericCallback, (void*)attrSet);
      }

    

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


void vtkIGTOpenTrackerStream::GenericCallback(const Node &node, const Event &event, void *data)
{

  if (!data)
    {
    return;
    }

  vtkIGTMessageAttributeSet* attrSet = (vtkIGTMessageAttributeSet*)data;
  
  // GET attributes
  //vtkIGTMessageAttributeSet::AttributeMapType& attrMap = attrSet->GetAttributeMap();
  //vtkIGTMessageAttributeSet::AttributeMapType::iterator iter;
  vtkIGTMessageAttributeSet::iterator iter;

  for (iter = attrSet->begin(); iter != attrSet->end(); iter ++)
    {
    std::string key = iter->first;
    vtkIGTMessageAttributeBase* attr = iter->second;
    //std::cout << "Searching ATTRIBUTE:  NAME = " << key << std::endl;

    if (event.hasAttribute(key))
      {
        //std::cout << " ====== found ATTRIBUTE:  NAME = " << key << std::endl;

      //========== Macro for switch(attr->GetTypeID()) {} ==========
      #define CASE_GETATTRIB_TYPE(TYPE_ID, TYPE)       \
        case TYPE_ID:            \
          {                                                   \
          TYPE data = static_cast<TYPE >(event.getAttribute<TYPE >((TYPE*)NULL, key)); \
          attr->SetAttribute(&data);                          \
          }                                                   \
        break;
      //============================================================

      switch(attr->GetTypeID())
        {
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_BOOL,           bool               );//(bool*)          false);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_CHAR,           char               );//(char*)          0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_SIGNED_CHAR,    signed char        );//(signed char*)   0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_UNSIGNED_CHAR,  unsigned char      );//(unsigned char*) 0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_INT,            int                );//(int*)           0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_LONG,           long               );//(long*)          0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_SHORT,          short              );//(short*)         0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_UNSIGNED_INT,   unsigned int       );//(unsigned int*)  0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_UNSIGNED_LONG,  unsigned long      );//(unsigned long*) 0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_UNSIGNED_SHORT, unsigned short     );//(unsigned short*)0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_DOUBLE,         double             );//(double*)        0.0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_LONG_DOUBLE,    long double        );//(long double*)   0.0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_FLOAT,          float              );//(float*)         0.0);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_STRING,         std::string        );//(char*)NULL);
        CASE_GETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_VECTOR_FLOAT,   std::vector<float> );//fvec);
        case vtkIGTMessageAttributeSet::TYPE_VTK_IMAGE_DATA:
          {
          // Since NaviTrack Image class doesn't have size information,
          // we suppose following size parameters:
          float spacing[3] = {1.0, 1.0, 1.0};
          int res[3];
          res[0] = 256;
          res[1] = 256;
          res[2] = 1;

          Image image = (Image)event.getAttribute((Image*)NULL, key);

          vtkImageData* vid = vtkImageData::New();
          vid->SetDimensions(res[0], res[1], res[2]);
          vid->SetExtent(0, res[0]-1, 0, res[1]-1, 0, 0 );
          vid->SetNumberOfScalarComponents( 1 );
          vid->SetOrigin( 0, 0, 0 );
          vid->SetSpacing( spacing[0], spacing[1], spacing[2] );
          vid->SetScalarTypeToShort();
          vid->AllocateScalars();
          short* dest = (short*) vid->GetScalarPointer();
          if (dest)
            {
            memcpy(dest, image.image_ptr, image.size());
            vid->Update();
            }
          attr->SetAttribute(vid);
          }

          break;

        default:
          break;
        }
      }
    }
  
  attrSet->GetTimeStamp()->Modified();

  vtkIGTMessageAttributeSet::MessageHandlingFunction* handler = attrSet->GetHandlerFunction();
  void* argument = attrSet->GetHandlerArgument();
  if (handler)
    {
      handler(attrSet, argument);
    }

}


void vtkIGTOpenTrackerStream::AddCallback(const char* cbname,
                                          vtkIGTMessageAttributeSet::MessageHandlingFunction* func,
                                          vtkIGTMessageAttributeSet* attrSet,
                                          void* arg)
{
    attrSet->SetOpenTrackerStream(this);
    attrSet->SetHandlerFunction(func, arg);
    this->AttributeSetMap[cbname] = attrSet;
}


void vtkIGTOpenTrackerStream::SetAttributes(const char* srcName, vtkIGTMessageAttributeSet* attrSet)
{
  SlicerModule* module = (SlicerModule *)context->getModule("SlicerConfig");
  //SlicerSource* source = module->getSource(srcName);

  if (module != NULL)
    {
    ot::Event* event;
    event = new ot::Event();   // memo by Junichi : where is it deleted ?
    
    vtkIGTMessageAttributeSet::iterator iter;
    for (iter = attrSet->begin(); iter != attrSet->end(); iter ++)
      {
      std::string key = iter->first;
      vtkIGTMessageAttributeBase* attr = iter->second;
      
      //========== Macro for switch(attr->GetTypeID()) {} ==========
      #define CASE_SETATTRIB_TYPE(TYPE_ID, TYPE)       \
        case TYPE_ID:                      \
          {                                                                            \
          TYPE arg;                                                                    \
          dynamic_cast<vtkIGTMessageGenericAttribute<TYPE>*>(attr)->GetAttribute(&arg); \
          event->setAttribute(key, arg);                                  \
          break;\
          }
      //============================================================
        
      switch(attr->GetTypeID())
        {
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_BOOL,           bool               );//(bool*)          false);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_CHAR,           char               );//(char*)          0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_SIGNED_CHAR,    signed char        );//(signed char*)   0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_UNSIGNED_CHAR,  unsigned char      );//(unsigned char*) 0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_INT,            int                );//(int*)           0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_LONG,           long               );//(long*)          0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_SHORT,          short              );//(short*)         0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_UNSIGNED_INT,   unsigned int       );//(unsigned int*)  0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_UNSIGNED_LONG,  unsigned long      );//(unsigned long*) 0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_UNSIGNED_SHORT, unsigned short     );//(unsigned short*)0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_DOUBLE,         double             );//(double*)        0.0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_LONG_DOUBLE,    long double        );//(long double*)   0.0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_FLOAT,          float              );//(float*)         0.0);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_STRING,         std::string        );//(char*)NULL);
        CASE_SETATTRIB_TYPE(vtkIGTMessageAttributeSet::TYPE_VECTOR_FLOAT,   std::vector<float> );//fvec);
        case vtkIGTMessageAttributeSet::TYPE_VTK_IMAGE_DATA:
          {
          vtkImageData* arg;
          dynamic_cast<vtkIGTMessageImageDataAttribute*>(attr)->GetAttribute(&arg);
          event->setAttribute(key, arg);
          }
          break;
        default:
          break;
        }
      }
    event->getButton()=0;
    event->getConfidence()=1.0;
    module->setData(srcName, event);
    }
    
}



void vtkIGTOpenTrackerStream::StopPulling()
{
    context->stop();
    context->close();
}



void vtkIGTOpenTrackerStream::PullRealTime()
{
  if (context)
    {
    context->pushEvents();       // push event and
    context->pullEvents();       // pull event 
    //context->stop();
    }
}



void vtkIGTOpenTrackerStream::PrintSelf(ostream& os, vtkIndent indent)
{


}

