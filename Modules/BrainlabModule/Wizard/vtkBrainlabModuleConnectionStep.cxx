#include "vtkObjectFactory.h"

#include "vtkBrainlabModuleConnectionStep.h"

#include "vtkBrainlabModuleGUI.h"
#include "vtkBrainlabModuleMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWMenu.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLabel.h"

#include "vtkMultiThreader.h"
#include "vtkMutexLock.h"
#include "igtlServerSocket.h"
#include "igtlClientSocket.h"
#include "igtlOSUtil.h"
#include "igtlMessageBase.h"
#include "igtlMessageHeader.h"
#include "igtlTransformMessage.h"

// basic file operations
#include <iostream>
#include <fstream>

#include  "vtkMRMLIGTLConnectorNode.h"

#include "vtkKWInternationalization.h"
#include "vtkSlicerApplication.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkBrainlabModuleConnectionStep);
vtkCxxRevisionMacro(vtkBrainlabModuleConnectionStep, "$Revision: 1.8 $");

//----------------------------------------------------------------------------
vtkBrainlabModuleConnectionStep::vtkBrainlabModuleConnectionStep()
{
  this->SetName("2/3. Connect with Tracking Source");
  this->SetDescription("Connect with a simulator or real Brainlab system.");

  this->ConnectButton = NULL; 
  this->DisconnectButton = NULL; 
  this->OpenIGTLinkButton = NULL; 
  this->SimulatorButton = NULL;
  this->BrainlabButton = NULL;
  this->RandomDataButton = NULL;

  this->LoadFileButton = NULL;
  this->PortEntry = NULL;
  this->ServerEntry = NULL;
  this->FrequencyEntry = NULL;
  this->ModeFrame = NULL;
  this->StatusFrame = NULL;
  this->NetworkFrame = NULL;
  this->SimulatorFrame = NULL;
  this->BrainlabFrame = NULL;
  this->TrackingDataFrame = NULL;
  this->OpenIGTLinkFrame = NULL;
  this->ConnectFrame = NULL;

  this->ServerStopFlag = false;
  this->ThreadID = -1;
  this->ServerHostname = "localhost";
  this->ServerPort = 18944;
  this->Socket =  igtl::ClientSocket::New();
  this->Thread = NULL;
  this->StreamingOn = false;
  this->SimulatorTrackingData.clear();   

  this->IGTLConnector = NULL;
  this->ConnectionStatus = NULL;
  this->Option = -1;
  this->FramesPerSeconds = 0;
 
}

//----------------------------------------------------------------------------
vtkBrainlabModuleConnectionStep::~vtkBrainlabModuleConnectionStep()
{  
  if(this->SimulatorButton)
    {
    this->SimulatorButton->Delete();
    this->SimulatorButton = NULL;
    }
  if(this->BrainlabButton)
    {
    this->BrainlabButton->Delete();
    this->BrainlabButton = NULL;
    }
  if(this->ConnectButton)
    {
    this->ConnectButton->Delete();
    this->ConnectButton = NULL;
    }
  if(this->DisconnectButton)
    {
    this->DisconnectButton->Delete();
    this->DisconnectButton = NULL;
    }
  if(this->LoadFileButton)
    {
    this->LoadFileButton->Delete();
    this->LoadFileButton = NULL;
    }
  if(this->OpenIGTLinkButton)
    {
    this->OpenIGTLinkButton->Delete();
    this->OpenIGTLinkButton = NULL;
    }
  if(this->RandomDataButton)
    {
    this->RandomDataButton->Delete();
    this->RandomDataButton = NULL;
    }

  if(this->PortEntry)
    {
    this->PortEntry->Delete();
    this->PortEntry = NULL;
    }
  if(this->ServerEntry)
    {
    this->ServerEntry->Delete();
    this->ServerEntry = NULL;
    }
  if(this->FrequencyEntry)
    {
    this->FrequencyEntry->Delete();
    this->FrequencyEntry = NULL;
    }

  if(this->ModeFrame)
    {
    this->ModeFrame->Delete();
    this->ModeFrame = NULL;
    }
  if(this->StatusFrame)
    {
    this->StatusFrame->Delete();
    this->StatusFrame = NULL;
    }
  if(this->NetworkFrame)
    {
    this->NetworkFrame->Delete();
    this->NetworkFrame = NULL;
    }
  if(this->SimulatorFrame)
    {
    this->SimulatorFrame->Delete();
    this->SimulatorFrame = NULL;
    }
  if(this->BrainlabFrame)
    {
    this->BrainlabFrame->Delete();
    this->BrainlabFrame = NULL;
    }
  if(this->TrackingDataFrame)
    {
    this->TrackingDataFrame->Delete();
    this->TrackingDataFrame = NULL;
    }
  if(this->OpenIGTLinkFrame)
    {
    this->OpenIGTLinkFrame->Delete();
    this->OpenIGTLinkFrame = NULL;
    }
  if(this->ConnectFrame)
    {
    this->ConnectFrame->Delete();
    this->ConnectFrame = NULL;
    }

  if(this->IGTLConnector)
    {
    this->IGTLConnector->Delete();
    this->IGTLConnector = NULL;
    }

  if(this->ConnectionStatus)
    {
    this->ConnectionStatus->Delete();
    this->ConnectionStatus = NULL;
    }

    // thread
  this->Stop();
  if (this->Thread)
    {
    this->Thread->Delete();
    this->Thread = NULL;
    }

  for (unsigned int i = 0; i < this->SimulatorTrackingData.size(); i++)
    {
    delete this->SimulatorTrackingData[i];
    }
}



//----------------------------------------------------------------------------
void vtkBrainlabModuleConnectionStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  wizard_widget->GetCancelButton()->SetEnabled(0);

//  vtkKWWidget *parent = wizard_widget->GetClientArea();

  // Mode frame
  // ======================================================================
  if (! this->ModeFrame)
    {
    this->ModeFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->ModeFrame->IsCreated())
    { 
    this->ModeFrame->SetParent(wizard_widget->GetClientArea());
    this->ModeFrame->Create();
    this->ModeFrame->SetLabelText("Mode"); 
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 4", 
               this->ModeFrame->GetWidgetName());

  // The mode frame has Brainlab frame and the simulator frame
  if (! this->BrainlabFrame)
    {
    this->BrainlabFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->BrainlabFrame->IsCreated())
    { 
    this->BrainlabFrame->SetParent(ModeFrame->GetFrame());
    this->BrainlabFrame->Create();
    this->BrainlabFrame->SetLabelText("Brainlab"); 
    this->BrainlabFrame->CollapseFrame();
    }
  if (! this->SimulatorFrame)
    {
    this->SimulatorFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->SimulatorFrame->IsCreated())
    { 
    this->SimulatorFrame->SetParent(ModeFrame->GetFrame());
    this->SimulatorFrame->Create();
    this->SimulatorFrame->SetLabelText("Simulator"); 
    }
  this->Script ("pack %s %s -side top -anchor center -expand n -fill both -padx 0 -pady 0",
                 this->SimulatorFrame->GetWidgetName(),
                 this->BrainlabFrame->GetWidgetName());


  // The simulator frame has simulator button and tracking data frame 
  if (! this->SimulatorButton)
    {
    this->SimulatorButton = vtkKWCheckButton::New();
    this->SimulatorButton->SetParent (this->SimulatorFrame->GetFrame());
    this->SimulatorButton->Create ( );
    this->SimulatorButton->SetText("Simulator");
    this->SimulatorButton->SelectedStateOn();
    this->SimulatorButton->SetCommand(this, "SimulatorButtonCallback");
    }
  if (! this->TrackingDataFrame)
    {
    this->TrackingDataFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->TrackingDataFrame->IsCreated())
    { 
    this->TrackingDataFrame->SetParent(this->SimulatorFrame->GetFrame());
    this->TrackingDataFrame->Create();
    this->TrackingDataFrame->SetLabelText("Data Source"); 
    }
  this->Script ("pack %s %s -side top -anchor center -expand n -fill both -padx 2 -pady 2",
                 this->SimulatorButton->GetWidgetName(),
                 this->TrackingDataFrame->GetWidgetName());


  // Tracking data frame
  if (! this->RandomDataButton)
    {
    this->RandomDataButton = vtkKWCheckButton::New();
    this->RandomDataButton->SetParent (this->TrackingDataFrame->GetFrame());
    this->RandomDataButton->Create ( );
    this->RandomDataButton->SetText("Random generation in memory");
    this->RandomDataButton->SelectedStateOn();
//    this->RandomDataButton->SetCommand(this, "SimulatorButtonCallback");
    }
  if (! this->LoadFileButton)
    {
    // add a file browser 
    this->LoadFileButton = vtkKWLoadSaveButtonWithLabel::New( );
    this->LoadFileButton->SetParent (this->TrackingDataFrame->GetFrame());
    this->LoadFileButton->GetWidget()->GetLoadSaveDialog()->SetMasterWindow(this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow());
    this->LoadFileButton->Create ( );
    this->LoadFileButton->SetWidth(50);
    this->LoadFileButton->GetWidget()->SetText ("Select tracking data file");
    this->LoadFileButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Open Data File");
    this->LoadFileButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes("{ {Tracking Data} {*.txt} }");
    this->LoadFileButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
    }
  this->Script ("pack %s %s -side left -anchor center -expand n -padx 2 -pady 2",
                 this->RandomDataButton->GetWidgetName(),
                 this->LoadFileButton->GetWidgetName());

  if (! this->BrainlabButton)
    {
    this->BrainlabButton = vtkKWCheckButton::New();
    this->BrainlabButton->SetParent (this->BrainlabFrame->GetFrame());
    this->BrainlabButton->Create ( );
    this->BrainlabButton->SetText("BrainLab");
    this->BrainlabButton->SelectedStateOff();
    this->BrainlabButton->SetCommand(this, "BrainlabButtonCallback");
    }
  // Network frame
  if (! this->NetworkFrame)
    {
    this->NetworkFrame = vtkKWFrame::New();
    }
  if (!this->NetworkFrame->IsCreated())
    { 
    this->NetworkFrame->SetParent(this->BrainlabFrame->GetFrame());
    this->NetworkFrame->Create();
    }
  this->Script ("pack %s -side top -anchor center -expand n -fill both -padx 0 -pady 0",
                 this->BrainlabButton->GetWidgetName());
  this->Script("pack %s -side top -anchor center -expand n -fill both -padx 0 -pady 4", 
               this->NetworkFrame->GetWidgetName());


  if (! this->PortEntry)
    {
    this->PortEntry = vtkKWEntryWithLabel::New();
    this->PortEntry->SetParent (NetworkFrame);
    this->PortEntry->Create ( );
    this->PortEntry->SetLabelText("Port: ");
    this->PortEntry->SetWidth(80);
    this->PortEntry->SetLabelWidth(8);
    this->PortEntry->GetWidget()->SetValueAsInt(18944);
    }
  if (! this->ServerEntry)
    {
    this->ServerEntry = vtkKWEntryWithLabel::New();
    this->ServerEntry->SetParent (NetworkFrame);
    this->ServerEntry->Create ( );
    this->ServerEntry->SetLabelText("Server: ");
    this->ServerEntry->SetWidth(80);
    this->ServerEntry->SetLabelWidth(8);
    this->ServerEntry->GetWidget()->SetValue("localhost");
    }
  this->Script ( "pack %s %s -side top -anchor center -expand n -padx 2 -pady 4",
                 this->PortEntry->GetWidgetName(),
                 this->ServerEntry->GetWidgetName());

  if (! this->FrequencyEntry)
    {
    this->FrequencyEntry = vtkKWEntryWithLabel::New();
    this->FrequencyEntry->SetParent (NetworkFrame);
    this->FrequencyEntry->Create ( );
    this->FrequencyEntry->SetLabelText("Frequency: ");
    this->FrequencyEntry->SetWidth(80);
    this->FrequencyEntry->SetLabelWidth(8);
    this->FrequencyEntry->GetWidget()->SetValue("10");
    }
  this->Script ( "pack %s -side top -anchor center -expand n -padx 2 -pady 4",
                 this->FrequencyEntry->GetWidgetName());

  // Status frame
  // ======================================================================
  if (! this->StatusFrame)
    {
    this->StatusFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->StatusFrame->IsCreated())
    { 
    this->StatusFrame->SetParent(wizard_widget->GetClientArea());
    this->StatusFrame->Create();
    this->StatusFrame->SetLabelText("Status"); 
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 4", 
               this->StatusFrame->GetWidgetName());

  if (! this->ConnectionStatus)
    {
    this->ConnectionStatus = vtkKWLabel::New();
    this->ConnectionStatus->SetParent (this->StatusFrame->GetFrame());
    this->ConnectionStatus->Create ( );
    this->ConnectionStatus->SetText("Disconnected");
//    this->ConnectionStatus->SetForegroundColor(255, 99, 71);
    this->ConnectionStatus->SetWidth(80);
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 4", 
               this->ConnectionStatus->GetWidgetName());

  // Connect frame
  // ======================================================================
   if (! this->ConnectFrame)
    {
    this->ConnectFrame = vtkKWFrame::New();
    }
  if (!this->ConnectFrame->IsCreated())
    { 
    this->ConnectFrame->SetParent(wizard_widget->GetClientArea());
    this->ConnectFrame->Create();
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 4", 
               this->ConnectFrame->GetWidgetName());

  if (!this->ConnectButton)
    {
    this->ConnectButton = vtkKWPushButton::New();
    this->ConnectButton->SetParent (this->ConnectFrame);
    this->ConnectButton->Create();
    this->ConnectButton->SetText("Connect");
    this->ConnectButton->SetCommand(this, "ConnectButtonCallback");
    this->ConnectButton->SetWidth(15);
    }
  if (!this->DisconnectButton)
    {
    this->DisconnectButton = vtkKWPushButton::New();
    this->DisconnectButton->SetParent (this->ConnectFrame);
    this->DisconnectButton->Create();
    this->DisconnectButton->SetText("Disconnect");
    this->DisconnectButton->SetCommand(this, "DisconnectButtonCallback");
    this->DisconnectButton->SetWidth(15);
    }

  this->Script(
    "pack %s %s -side right -anchor e -padx 2 -pady 5", 
    this->DisconnectButton->GetWidgetName(),
    this->ConnectButton->GetWidgetName());

   //Add a help to the step
  vtkKWPushButton * helpButton =  wizard_widget->GetHelpButton();

  vtkKWMessageDialog *msg_dlg1 = vtkKWMessageDialog::New();
  msg_dlg1->SetParent(wizard_widget->GetClientArea());
  msg_dlg1->SetStyleToOkCancel();
  msg_dlg1->Create();
  msg_dlg1->SetTitle("Connect with Tracking Source");
  msg_dlg1->SetText( "This is the second step in BrainlabModule. In this step, the user connects Slicer with "
                     "a simulator or a real Brainlab system. For simulation, the tracking data points can be "
                     "either loaded from a file or generated in the memory. To connect with a real Brainlab system, "
                     "the user needs to specify the server name, port number and data streaming frequency (frames per second)."
                     "The Status section displays the connection status: Disconnected or Connected."
                     "Press Connect button to initiate a connection. Press Disconnect button to terminate the connection.");

  helpButton->SetCommand(msg_dlg1, "Invoke");

  msg_dlg1->Delete();
}
 


//----------------------------------------------------------------------------
void vtkBrainlabModuleConnectionStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkBrainlabModuleConnectionStep::ProcessGUIEvents(vtkObject *caller,
                                          unsigned long event, void *callData)
{
  // ConnectButton Pressed
  if (this->ConnectButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
//    this->GetGUI()->GetApplication()->Script("::LoadVolume::ShowDialog");
    }
}



void vtkBrainlabModuleConnectionStep::ConnectButtonCallback()
{
  // ConnectButton Pressed

  if (this->ConnectButton)
    {
    if(! this->IGTLConnector)
      {
      this->IGTLConnector = vtkMRMLIGTLConnectorNode::New();
      this->IGTLConnector->SetRestrictDeviceName(0);

      if (this->Option == 3)
        {
        this->IGTLConnector->SetServerHostname(this->ServerHostname);
        this->IGTLConnector->SetServerPort(this->ServerPort);
        }
      else
        {
        this->IGTLConnector->SetServerHostname("localhost");
        this->IGTLConnector->SetServerPort(18944);
        }

      this->IGTLConnector->SetType(vtkMRMLIGTLConnectorNode::TYPE_SERVER);
      this->IGTLConnector->Modified();
      this->IGTLConnector->Start();
      this->GetGUI()->GetMRMLManager()->GetMRMLScene()->AddNode(this->IGTLConnector);
      this->GetGUI()->GetMRMLManager()->GetMRMLScene()->Modified();

      vtkSlicerApplication::GetInstance( )->GetModuleGUIByName("OpenIGTLink IF")->Enter();
      }

    if (this->SimulatorButton->GetSelectedState())
      {
      if (!this->RandomDataButton->GetSelectedState())
        {
        int rt = this->ReadSimulatorDataFromFile(); 
        if (!rt) return;
        }
      }
    this->Start();
    }
}



void vtkBrainlabModuleConnectionStep::DisconnectButtonCallback()
{
  // DisconnectButton Pressed


  if (this->DisconnectButton)
    {
    this->Stop();
    if (this->IGTLConnector)
      {
      this->IGTLConnector->Stop();
      this->GetGUI()->GetMRMLManager()->GetMRMLScene()->RemoveNode(this->IGTLConnector);
      this->GetGUI()->GetMRMLManager()->GetMRMLScene()->Modified();
      this->IGTLConnector->Delete();
      this->IGTLConnector = NULL;
      }
    }
}



void vtkBrainlabModuleConnectionStep::OpenIGTLinkButtonCallback()
{
  // Go to the OpenIGTLink interface
  if (this->OpenIGTLinkButton)
    {
    this->RaiseModule("OpenIGTLink IF");
    }
}



void vtkBrainlabModuleConnectionStep::SimulatorButtonCallback(int checked)
{
  // SimulatorButton Pressed
  // Either Simulator or Brainlab button may be checked.
  if (this->SimulatorButton)
    {
    if (checked)
      {
      this->BrainlabButton->SelectedStateOff();
      } 
    else
      {
      this->BrainlabButton->SelectedStateOn();
      }
    }
}



void vtkBrainlabModuleConnectionStep::BrainlabButtonCallback(int checked)
{
  // BrainlabButton Pressed
  // Either Simulator or Brainlab button may be checked.
  if (this->BrainlabButton)
    {
    if (checked)
      {
      this->SimulatorButton->SelectedStateOff();
      } 
    else
      {
      this->SimulatorButton->SelectedStateOn();
      }
   }
}



//---------------------------------------------------------------------------
int vtkBrainlabModuleConnectionStep::Start()
{
  // Check if thread is detached
  if (this->ThreadID >= 0)
    {
    //vtkErrorMacro("Thread exists.");
    return 0;
    }

  this->FramesPerSeconds = this->FrequencyEntry->GetWidget()->GetValueAsDouble();
 
  // Options of tracking data source
  if (this->BrainlabButton->GetSelectedState())
    {
    // from a real Brainlab system
    this->Option = 3;
    }
  else
    {
    if (this->SimulatorButton->GetSelectedState())
      {
      if (this->RandomDataButton->GetSelectedState())
        {
        // from random generation in memory
        this->Option = 0; 
        }
      else
        {
        // from a file 
        this->Option = 1; 
        }
      }
    }

  // Establish Connection
  int r;
  if (this->Option == 3)
    {
    r = this->Socket->ConnectToServer(this->ServerHostname.c_str(), this->ServerPort);
    }
  else
    {
    r = this->Socket->ConnectToServer("localhost", 18944);
    }

  if (r != 0)
    {
    std::string msg = "Cannot connect to the server."; 
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ("Error", msg.c_str());
    this->ConnectionStatus->SetText("Disconnected.");
    }
  else
    {
    this->ConnectionStatus->SetText("Connected.");
    this->StreamingOn = true;
    this->ServerStopFlag = false;
    this->Thread = vtkMultiThreader::New();
    this->ThreadID = this->Thread->SpawnThread((vtkThreadFunctionType) &vtkBrainlabModuleConnectionStep::ThreadFunction, this);
    }
  // Following line is necessary in some Linux environment,
  // since it takes for a while for the thread to update
  // this->State to non STATE_OFF value. This causes error
  // after calling vtkBrainlabConnectionStep::Start() in ProcessGUIEvent()
  // in vtkOpenIGTLinkIFGUI class.
  // this->State = STATE_WAIT_CONNECTION;
  // this->InvokeEvent(vtkBrainlabConnectionStep::ActivatedEvent);

  return 1;
}



//---------------------------------------------------------------------------
int vtkBrainlabModuleConnectionStep::Stop()
{
  if (this->ConnectionStatus) this->ConnectionStatus->SetText("Disconnected.");
//  this->ConnectionStatus->SetForegroundColor(255, 99, 71);

  // Check if thread exists
  if (this->ThreadID >= 0)
    {
    // NOTE: Thread should be killed by activating ServerStopFlag.
    this->ServerStopFlag = true;
    this->StreamingOn = false;
    this->Thread->TerminateThread(this->ThreadID);
    this->Thread->Delete();
    this->Thread = NULL;
    this->ThreadID = -1;
    }

  if (this->Socket.IsNotNull())
    {
    this->Socket->CloseSocket();
    }

  return 1;

}



//---------------------------------------------------------------------------
void* vtkBrainlabModuleConnectionStep::ThreadFunction(void* ptr)
{
  vtkMultiThreader::ThreadInfo* vinfo = 
    static_cast<vtkMultiThreader::ThreadInfo*>(ptr);
  vtkBrainlabModuleConnectionStep* conStep = static_cast<vtkBrainlabModuleConnectionStep*>(vinfo->UserData);

  if (conStep)
    {
    double fps      = 10; 
    if (conStep->Option == 3)
      {
      fps = conStep->FramesPerSeconds;
      }

    int    interval = (int) (1000.0 / fps);

    //------------------------------------------------------------
    // Allocate Transform Message Class
    igtl::TransformMessage::Pointer transMsg;
    transMsg = igtl::TransformMessage::New();
    transMsg->SetDeviceName("brainlab_tracker");

    //------------------------------------------------------------
    // loop
    unsigned int index = 0;
    while (conStep->GetStreamingOn())
      {
      // streaming tracking points of simulation
      igtl::Matrix4x4 matrix;
      if (conStep->Option == 0)
        {
        conStep->GetRandomTestMatrix(matrix);
        }
      else if (conStep->Option == 1)
        {
        std::vector<double> *values = conStep->SimulatorTrackingData[index];
        conStep->GenerateTrackingMatrix(values, matrix);
        }

      transMsg->SetMatrix(matrix);
      transMsg->Pack();
      conStep->Socket->Send(transMsg->GetPackPointer(), transMsg->GetPackSize());
      igtl::Sleep(interval); // wait

      index++;
      if (index == conStep->SimulatorTrackingData.size()) index = 0;
      } 
    } 

  conStep->ThreadID = -1;
  return NULL;
}

//------------------------------------------------------------
// Function to read tracking data from file 
int vtkBrainlabModuleConnectionStep::ReadSimulatorDataFromFile() 
{
  const char *fileName = this->LoadFileButton->GetWidget()->GetFileName();
  if (fileName)
    {
    ifstream myfile (fileName);
    std::string line;
    if (myfile.is_open())
      {
      while (! myfile.eof() )
        {
        std::getline (myfile, line);
        std::vector<double> *values = new std::vector<double>; 
        values->reserve(7);
        char *pch = strtok ((char *)line.c_str(), " ");
        while (pch != NULL)
          {
          double v = atof(pch);
          values->push_back(v);
          pch = strtok (NULL, " ");
          }
        this->SimulatorTrackingData.push_back(values);   
        }
      myfile.close();

      return (this->SimulatorTrackingData.size() > 0 ? 1 : 0);
      }
      // report file not readable
    else
      {
      std::string msg = "Couldn't open the tracking data file: " + std::string(fileName);
      vtkErrorMacro(<< msg << std::endl);
      vtkSlicerApplication::GetInstance()->RequestDisplayMessage ("Error", msg.c_str());
      return 0;
      }
    }
  else
    {    
    std::string msg = "The tracking data file doesn't exist!"; 
    vtkErrorMacro(<< msg << std::endl);
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ("Error", msg.c_str());
    return 0;
    }
}



void vtkBrainlabModuleConnectionStep::GenerateTrackingMatrix(std::vector<double> *values, igtl::Matrix4x4 &matrix) 
{
  float orientation[4];

  orientation[0]= (*values)[3];
  orientation[1]= (*values)[4];
  orientation[2]= (*values)[5];
  orientation[3]= (*values)[6];

//  std::cerr << position[0] << "  " << position[1] << "  " << position[2] << "  ";
//  std::cerr << orientation[0] << "  " << orientation[1] << "  " << orientation[2] << "  " << orientation[3] << std::endl;

  //igtl::Matrix4x4 matrix;
  igtl::QuaternionToMatrix(orientation, matrix);

  matrix[0][3] = (*values)[0]; 
  matrix[1][3] = (*values)[1]; 
  matrix[2][3] = (*values)[2]; 
  
//  igtl::PrintMatrix(matrix);
}



//------------------------------------------------------------
// Function to generate random matrix.
void vtkBrainlabModuleConnectionStep::GetRandomTestMatrix(igtl::Matrix4x4& matrix)
{
  float position[3];
  float orientation[4];

  // random position
  static float phi = 0.0;
  position[0] = 15.0 * cos(phi);
  position[1] = 15.0 * sin(phi);
  position[2] = 15.0 * cos(phi);
  phi = phi + 0.2;

  // random orientation
  static float theta = 0.0;
  orientation[0]=0.0;
  orientation[1]=0.6666666666*cos(theta);
  orientation[2]=0.577350269189626;
  orientation[3]=0.6666666666*sin(theta);
  theta = theta + 0.1;

  /*
  std::cerr << position[0] << "  " << position[1] << "  " << position[2] << "  ";
  std::cerr << orientation[0] << "  " << orientation[1] << "  " << orientation[2] << "  " << orientation[3] << std::endl;
  */

  //igtl::Matrix4x4 matrix;
  igtl::QuaternionToMatrix(orientation, matrix);

  matrix[0][3] = position[0];
  matrix[1][3] = position[1];
  matrix[2][3] = position[2];
  
//  igtl::PrintMatrix(matrix);
}


//---------------------------------------------------------------------------
void vtkBrainlabModuleConnectionStep::ImportFromCircularBuffers()
{
  //ConnectorMapType::iterator cmiter;
  std::vector<vtkMRMLNode*> nodes;
  const char* className = this->GetGUI()->GetMRMLManager()->GetMRMLScene()->GetClassNameByTag("IGTLConnector");
  this->GetGUI()->GetMRMLManager()->GetMRMLScene()->GetNodesByClass(className, nodes);

  std::vector<vtkMRMLNode*>::iterator iter;
  
  //for (cmiter = this->ConnectorMap.begin(); cmiter != this->ConnectorMap.end(); cmiter ++)
  for (iter = nodes.begin(); iter != nodes.end(); iter ++)
    {
    vtkMRMLIGTLConnectorNode* connector = vtkMRMLIGTLConnectorNode::SafeDownCast(*iter);
    if (connector == NULL)
      {
      continue;
      }

    connector->ImportDataFromCircularBuffer();
    }
}


