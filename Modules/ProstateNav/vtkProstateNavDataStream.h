#ifndef __vtkProstateNavDataStream_h
#define __vtkProstateNavDataStream_h

#include "vtkIGTWin32Header.h" 
#include "vtkIGTDataStream.h"

#include "vtkMatrix4x4.h"

#include "vtkIGTOpenTrackerStream.h"
#include "vtkIGTMessageAttributeSet.h"
#include "vtkImageData.h"
#include "vtkTimeStamp.h"


class VTK_IGT_EXPORT vtkProstateNavDataStream : public vtkIGTOpenTrackerStream
{
public:

  static vtkProstateNavDataStream *New();
  vtkTypeRevisionMacro(vtkProstateNavDataStream,vtkIGTOpenTrackerStream);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro(NeedleMatrix,vtkMatrix4x4);
  vtkGetObjectMacro(RealtimeImage, vtkImageData);
  vtkGetObjectMacro(ImageTimeStamp, vtkTimeStamp)

  //Description:
  //Constructor
  vtkProstateNavDataStream();

  //Description:
  //Destructor
  virtual ~vtkProstateNavDataStream();
  
  void Init(const char *configFile);
  
  void AddCallbacks();
  static void OnRecieveMessageFromRobot(vtkIGTMessageAttributeSet* data, void* arg);
  static void OnRecieveMessageFromScanner(vtkIGTMessageAttributeSet* data, void* arg);

  //BTX
  std::string GetRobotStatus();
  std::string GetScanStatus();
  
  void SetRobotPosition(std::vector<float> pos, std::vector<float> ori);
  void SetRobotCommand(std::string key, std::string value);

  void SetScanPosition(std::vector<float> pos, std::vector<float> ori);
  void SetScanCommand(std::string key, std::string value);
  //ETX

  //vtkTransform* GetNeedleTransform();
  void GetNeedleTransform(vtkTransform* dest);

  void Normalize(float *a);  
  void Cross(float *a, float *b, float *c);

private:

  //Context *context;
  
  //void CloseConnection();
  vtkIGTMessageAttributeSet* AttrSetRobot;    // <- Should it be a circular buffer?
  vtkIGTMessageAttributeSet* AttrSetScanner;  // <- Should it be a circular buffer?

  vtkMatrix4x4* NeedleMatrix;
  vtkImageData* RealtimeImage;
  vtkTimeStamp* ImageTimeStamp;


};

#endif // __vtkProstateNavDataStream_h




