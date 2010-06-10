#ifndef __vtkBrainlabModuleConnectionStep_h
#define __vtkBrainlabModuleConnectionStep_h

#include "vtkBrainlabModuleStep.h"
#include "igtlMath.h"
#include "igtlClientSocket.h"
#include "igtlServerSocket.h"

class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWFrame;
class vtkKWPushButton;
class vtkKWCheckButton;
class vtkKWLoadSaveButtonWithLabel;
class vtkMultiThreader;
class vtkMRMLIGTLConnectorNode;
class vtkKWLabel;


class VTK_BRAINLABMODULE_EXPORT vtkBrainlabModuleConnectionStep : public vtkBrainlabModuleStep
{
public:
  static vtkBrainlabModuleConnectionStep *New();
  vtkTypeRevisionMacro(vtkBrainlabModuleConnectionStep,vtkBrainlabModuleStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks. Internal, do not use.
  virtual void ConnectButtonCallback();
  virtual void DisconnectButtonCallback();
  virtual void SimulatorButtonCallback(int checked);
  virtual void BrainlabButtonCallback(int checked);
  virtual void OpenIGTLinkButtonCallback();

  vtkGetMacro(StreamingOn, bool);


protected:

  vtkBrainlabModuleConnectionStep();
  ~vtkBrainlabModuleConnectionStep();

  void ImportFromCircularBuffers();
  int ReadSimulatorDataFromFile(); 
  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

  vtkKWPushButton               *ConnectButton; 
  vtkKWPushButton               *DisconnectButton; 
  vtkKWPushButton               *OpenIGTLinkButton; 
  vtkKWCheckButton              *SimulatorButton; 
  vtkKWCheckButton              *BrainlabButton; 
  vtkKWCheckButton              *RandomDataButton; 

  vtkKWLoadSaveButtonWithLabel  *LoadFileButton;
  vtkKWEntryWithLabel           *PortEntry;
  vtkKWEntryWithLabel           *ServerEntry;
  vtkKWEntryWithLabel           *FrequencyEntry;
  vtkKWFrameWithLabel           *ModeFrame;
  vtkKWFrameWithLabel           *StatusFrame;
  vtkKWFrameWithLabel           *FrequencyFrame;
  vtkKWFrameWithLabel           *SimulatorFrame;
  vtkKWFrameWithLabel           *TrackingDataFrame;
  vtkKWFrameWithLabel           *BrainlabFrame;
  vtkKWFrameWithLabel           *OpenIGTLinkFrame;
  vtkKWFrame                    *ConnectFrame;
  vtkKWFrame                    *NetworkFrame;

  vtkKWLabel                    *ConnectionStatus;

  vtkMRMLIGTLConnectorNode      *IGTLConnector;

  // whether we should stream data to OpenIGTLink
  bool                          StreamingOn;
 
  double                        FramesPerSeconds;
  int                           Option; // 0 = generate tracking data from memory
                                        // 1 = read tracking data from a file

  // Thread control
  int Start();
  int Stop();
  static void* ThreadFunction(void* ptr);

  vtkMultiThreader *Thread;
  int               ThreadID;
  int               ServerPort;
  int               ServerStopFlag;
  //BTX
  // need a space between > and >; otherwise compiler will complains.
  std::vector< std::vector<double> *>  SimulatorTrackingData;   
  std::string       ServerHostname;
  igtl::ClientSocket::Pointer Socket;
  void GetRandomTestMatrix(igtl::Matrix4x4 &matrix);
  void GenerateTrackingMatrix(std::vector<double> *values, igtl::Matrix4x4 &matrix); 
  //ETX

private:
  vtkBrainlabModuleConnectionStep(const vtkBrainlabModuleConnectionStep&);
  void operator=(const vtkBrainlabModuleConnectionStep&);
};

#endif
