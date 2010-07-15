/*==========================================================================
 
 Portions (c) Copyright 2009 Atsushi Yamada (Fujimoto Lab, Nagoya Institute of Technology (NIT) All Rights Reserved.
 
 Acknowledgement: K. Chinzei (AIST), Y. Hayashi(Nagoya Univ.), T. Takeuchi (SFC Corp.), J. Tokuda(BWH), N. Hata(BWH), and H. Fujimoto(NIT) 
 CMakeLists.txt, FindOpenCV.cmake, and FindOpenIGTLink.cmake are contributions of K. Chinzei(AIST) and T. Takeuchi (SFC Corp.).
 
 See README.txt
 or http://www.slicer.org/copyright/copyright.txt for details.
 
 Program:   OpenCV
 Module:    $HeadURL: $
 Date:      $Date:07/17/2009 $
 Version:   $Revision: $
 
 ==========================================================================*/

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkOpenCVGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSlicesGUI.h"

#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWEvent.h"

#include "vtkKWPushButton.h"

#include "vtkCornerAnnotation.h"

#include "vtkMRMLLinearTransformNode.h"
#include "math.h"

extern CvCapture *capture;
extern vtkImageViewer *viewer ;
extern vtkImageImport *importer;

char *buffer;
int i_buffer;

float tx;
float ty;
float tz;
float t0;
float sx;
float sy;
float sz;
float s0;
float nx;
float ny;
float nz;
float n0;
float px;
float py;
float pz;
float p0;

float tx2;
float ty2;
float tz2;
float t02;
float sx2;
float sy2;
float sz2;
float s02;
float nx2;
float ny2;
float nz2;
float n02;
float px2;
float py2;
float pz2;
float p02;

extern vtkTextActor *textActor1;
extern vtkTextActor *textActor2;
extern vtkTextActor *textActor3;
extern vtkTextActor *textActor4;
extern vtkTextActor *textActor5;
extern vtkTextActor *textActor6;

extern vtkTextActor *suctionActor;
extern vtkTextActor *suctionActor1;
extern vtkTextActor *suctionActor2;
extern vtkTextActor *suctionActor3;
extern vtkTextActor *suctionActor4;
extern vtkTextActor *suctionActor5;
extern vtkTextActor *suctionActor6;
extern vtkTextActor *suctionActor7;

extern vtkTextActor *baloonActor;
extern vtkTextActor *baloonActor1;
extern vtkTextActor *baloonActor2;
extern vtkTextActor *baloonActor3;
extern vtkTextActor *baloonActor4;
extern vtkTextActor *baloonActor5;
extern vtkTextActor *baloonActor6;
extern vtkTextActor *baloonActor7;
extern vtkTextActor *baloonActor8;

extern pthread_mutex_t work_mutex;

int textActor1Flag = 0;
int textActor2Flag = 0;
int textActor3Flag = 0;
int textActor4Flag = 0;
int textActor5Flag = 0;
//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkOpenCVGUI);
vtkCxxRevisionMacro (vtkOpenCVGUI, "$Revision: 1.0 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
vtkOpenCVGUI::vtkOpenCVGUI ( )
{

  //----------------------------------------------------------------
  // Logic values
  this->Logic = NULL;
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkOpenCVGUI::DataCallback);
  
  //----------------------------------------------------------------
  // GUI widgets
  this->showCaptureData = NULL;
  this->closeCaptureData = NULL;
  this->TestButton21 = NULL;
  this->TestButton22 = NULL;
  
  //----------------------------------------------------------------
  // Locator  (MRML)
  this->TimerFlag = 0;

}

//---------------------------------------------------------------------------
vtkOpenCVGUI::~vtkOpenCVGUI ( )
{

  //----------------------------------------------------------------
  // Remove Callbacks

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

  //----------------------------------------------------------------
  // Remove Observers

  this->RemoveGUIObservers();

  //----------------------------------------------------------------
  // Remove GUI widgets

  if (this->showCaptureData)
    {
    this->showCaptureData->SetParent(NULL);
    this->showCaptureData->Delete();
    }

  if (this->closeCaptureData)
    {
    this->closeCaptureData->SetParent(NULL);
    this->closeCaptureData->Delete();
    }

  if (this->TestButton21)
    {
    this->TestButton21->SetParent(NULL);
    this->TestButton21->Delete();
    }

  if (this->TestButton22)
    {
    this->TestButton22->SetParent(NULL);
    this->TestButton22->Delete();
    }

  //----------------------------------------------------------------
  // Unregister Logic class

  this->SetModuleLogic ( NULL );

}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::Init()
{
}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::Enter()
{
  // Fill in
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  if (this->TimerFlag == 0)
    {
    this->TimerFlag = 1;
        this->TimerInterval = 100;//20;//100;  // 100 ms
        
    ProcessTimerEvents();
    }

}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::Exit ( )
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "OpenCVGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::RemoveGUIObservers ( )
{
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  if (this->showCaptureData)
    {
    this->showCaptureData
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->closeCaptureData)
    {
    this->closeCaptureData
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->TestButton21)
    {
    this->TestButton21
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->TestButton22)
    {
    this->TestButton22
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }


  this->RemoveLogicObservers();

}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::AddGUIObservers ( )
{
  this->RemoveGUIObservers();

  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  //----------------------------------------------------------------
  // MRML

  vtkIntArray* events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  events->Delete();

  //----------------------------------------------------------------
  // GUI Observers

  this->showCaptureData
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->closeCaptureData
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TestButton21
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TestButton22
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->AddLogicObservers();

}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::RemoveLogicObservers ( )
{
  if (this->GetLogic())
    {
    this->GetLogic()->RemoveObservers(vtkCommand::ModifiedEvent,
                                      (vtkCommand *)this->LogicCallbackCommand);
    }
}




//---------------------------------------------------------------------------
void vtkOpenCVGUI::AddLogicObservers ( )
{
  this->RemoveLogicObservers();  

  if (this->GetLogic())
    {
    this->GetLogic()->AddObserver(vtkOpenCVLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkOpenCVGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  const char *eventName = vtkCommand::GetStringFromEventId(event);

  if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
    vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
    HandleMouseEvent(style);
    return;
    }

  
  if (this->showCaptureData == vtkKWPushButton::SafeDownCast(caller) 
      && event == vtkKWPushButton::InvokedEvent)
    {
    std::cerr << "showCaptureData button is pressed." << std::endl;
        
        // initialize mutex
        int resGlobal;
        resGlobal = pthread_mutex_init(&work_mutex, NULL);
        
        // for display capture data
        this->GetLogic()->SetVisibilityOfCaptureData("capture", 1);
        
    }
  else if (this->closeCaptureData == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    std::cerr << "MRML Observer button is pressed." << std::endl;
        
        // read MRML
        vtkMRMLScene* scene = this->GetMRMLScene();
        if (scene)
        {
            // test network connection check (baloon sensor)
            vtkMRMLNode* node = scene->GetNodeByID("vtkMRMLLinearTransformNode4");
            vtkMRMLLinearTransformNode* tnode = vtkMRMLLinearTransformNode::SafeDownCast(node);

            // test baloon sensor connection check (suction tube)
            vtkMRMLNode* node2 = scene->GetNodeByID("vtkMRMLLinearTransformNode5");
            vtkMRMLLinearTransformNode* tnode2 = vtkMRMLLinearTransformNode::SafeDownCast(node2);
            
            vtkMRMLNode *nnode = NULL; // TODO: is this OK?
            vtkIntArray* nodeEvents = vtkIntArray::New();
            nodeEvents->InsertNextValue(vtkMRMLLinearTransformNode::TransformModifiedEvent);
            vtkSetAndObserveMRMLNodeEventsMacro(nnode,tnode,nodeEvents);

            vtkMRMLNode *nnode2 = NULL; // TODO: is this OK?
            vtkIntArray* nodeEvents2 = vtkIntArray::New();
            nodeEvents2->InsertNextValue(vtkMRMLLinearTransformNode::TransformModifiedEvent);
            vtkSetAndObserveMRMLNodeEventsMacro(nnode2,tnode2,nodeEvents2);
            
            nodeEvents->Delete();
            nodeEvents2->Delete();
            
        }
        
        
        
    }
  else if (this->TestButton21 == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    std::cerr << "TestButton21 is pressed." << std::endl;
    }
  else if (this->TestButton22 == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    std::cerr << "TestButton22 is pressed." << std::endl;
    }

} 


void vtkOpenCVGUI::DataCallback(vtkObject *caller, 
                                     unsigned long eid, void *clientData, void *callData)
{
  vtkOpenCVGUI *self = reinterpret_cast<vtkOpenCVGUI *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkOpenCV DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{

  if (this->GetLogic() == vtkOpenCVLogic::SafeDownCast(caller))
    {
    if (event == vtkOpenCVLogic::StatusUpdateEvent)
      {
      //this->UpdateDeviceStatus();
      }
    }
}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  // Fill in
  if (event == vtkMRMLLinearTransformNode::TransformModifiedEvent)
    {
        std::cerr << "TransformModifiedEvent is invoked." << std::endl;

        
        vtkMRMLLinearTransformNode* node = vtkMRMLLinearTransformNode::SafeDownCast(caller);
        vtkMatrix4x4* transformToParent = node->GetMatrixTransformToParent();

        vtkMRMLLinearTransformNode* node2 = vtkMRMLLinearTransformNode::SafeDownCast(caller);
        vtkMatrix4x4* transformToParent2 = node2->GetMatrixTransformToParent();
        
        
        tx = transformToParent->GetElement(0, 0);
        ty = transformToParent->GetElement(1, 0);
        tz = transformToParent->GetElement(2, 0);
        t0 = transformToParent->GetElement(3, 0);
        sx = transformToParent->GetElement(0, 1);
        sy = transformToParent->GetElement(1, 1);
        sz = transformToParent->GetElement(2, 1);
        s0 = transformToParent->GetElement(3, 1);
        nx = transformToParent->GetElement(0, 2);
        ny = transformToParent->GetElement(1, 2);
        nz = transformToParent->GetElement(2, 2);
        n0 = transformToParent->GetElement(3, 2);
        px = transformToParent->GetElement(0, 3);
        py = transformToParent->GetElement(1, 3);
        pz = transformToParent->GetElement(2, 3);
        p0 = transformToParent->GetElement(3, 3);

        tx2 = transformToParent2->GetElement(0, 0);
        ty2 = transformToParent2->GetElement(1, 0);
        tz2 = transformToParent2->GetElement(2, 0);
        t02 = transformToParent2->GetElement(3, 0);
        sx2 = transformToParent2->GetElement(0, 1);
        sy2 = transformToParent2->GetElement(1, 1);
        sz2 = transformToParent2->GetElement(2, 1);
        s02 = transformToParent2->GetElement(3, 1);
        nx2 = transformToParent2->GetElement(0, 2);
        ny2 = transformToParent2->GetElement(1, 2);
        nz2 = transformToParent2->GetElement(2, 2);
        n02 = transformToParent2->GetElement(3, 2);
        px2 = transformToParent2->GetElement(0, 3);
        py2 = transformToParent2->GetElement(1, 3);
        pz2 = transformToParent2->GetElement(2, 3);
        p02 = transformToParent2->GetElement(3, 3);
        
        std::cerr << "tx  = "  << tx << std::endl;
        std::cerr << "ty  = "  << ty << std::endl;
        std::cerr << "tz  = "  << tz << std::endl;
        std::cerr << "t0  = "  << t0 << std::endl;
        std::cerr << "sx  = "  << sx << std::endl;
        std::cerr << "sy  = "  << sy << std::endl;
        std::cerr << "sz  = "  << sz << std::endl;
        std::cerr << "s0  = "  << s0 << std::endl;
        std::cerr << "nx  = "  << nx << std::endl;
        std::cerr << "ny  = "  << ny << std::endl;
        std::cerr << "nz  = "  << nz << std::endl;
        std::cerr << "n0  = "  << n0 << std::endl;
        std::cerr << "px  = "  << px << std::endl;
        std::cerr << "py  = "  << py << std::endl;
        std::cerr << "pz  = "  << pz << std::endl;
        std::cerr << "p0  = "  << p0 << std::endl;

        std::cerr << "tx2  = "  << tx2 << std::endl;
        std::cerr << "ty2  = "  << ty2 << std::endl;
        std::cerr << "tz2  = "  << tz2 << std::endl;
        std::cerr << "t02  = "  << t02 << std::endl;
        std::cerr << "sx2  = "  << sx2 << std::endl;
        std::cerr << "sy2  = "  << sy2 << std::endl;
        std::cerr << "sz2  = "  << sz2 << std::endl;
        std::cerr << "s02  = "  << s02 << std::endl;
        std::cerr << "nx2  = "  << nx2 << std::endl;
        std::cerr << "ny2  = "  << ny2 << std::endl;
        std::cerr << "nz2  = "  << nz2 << std::endl;
        std::cerr << "n02  = "  << n02 << std::endl;
        std::cerr << "px2  = "  << px2 << std::endl;
        std::cerr << "py2  = "  << py2 << std::endl;
        std::cerr << "pz2  = "  << pz2 << std::endl;
        std::cerr << "p02  = "  << p02 << std::endl;
        
        
        // demo of display status
        pthread_mutex_lock(&work_mutex);

        textActor3->SetInput("Network Connection: ON");
        textActor3Flag = 1;

        textActor2->SetInput("Robot Connection: ON");
        textActor2Flag = 1;
        
        textActor1->SetInput("Baloon Sensor: ON");
        textActor1Flag = 1;

        textActor5->SetInput("Suction Tube: ON");
        textActor5Flag = 1;

        // suction tube
        char buf1[100],buf2[100],buf3[100],buf4[100],buf5[100],buf6[100],buf7[100];
        sprintf(buf1, "Regulator / Suction Pressure   %f / %f",tx, sx);
        suctionActor1->SetInput(buf1);

        sprintf(buf2, "Suction Tube Switch %f",nx);
        suctionActor2->SetInput(buf2);
        
        sprintf(buf3, "Scalpel cut     %f ",ty);
        suctionActor3->SetInput(buf3);
        
        sprintf(buf4, "Scalpel coag     %f ",sy);
        suctionActor4->SetInput(buf4);
        
        sprintf(buf5, "Scalpel On/Off     %f ",ny);
        suctionActor5->SetInput(buf5);
        
        sprintf(buf6, "Scalpel Time     %f ",py);
        suctionActor6->SetInput(buf6);

        sprintf(buf7, "System Status     %f ",tz);
        suctionActor7->SetInput(buf7);

        // baloon tube
        char bufA[100],bufB[100],bufC[100],bufD[100];//,bufE[100],bufF[100],bufG[100];
        sprintf(bufA, "Status   %f",tx2);
        baloonActor1->SetInput(bufA);
        
        sprintf(bufB, "Pressure     %f ",tz2);
        baloonActor2->SetInput(bufB);
        
        sprintf(bufC,"Volume     %f ",sz2);
        baloonActor3->SetInput(bufC);
        
        sprintf(bufD, "Diff_Pressure   %f ",nz2);
        baloonActor4->SetInput(bufD);
        
        pthread_mutex_unlock(&work_mutex);
    
    }
    

  if (event == vtkMRMLScene::SceneClosedEvent)
    {
    }
}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::ProcessTimerEvents()
{
  if (this->TimerFlag)
    {
        
    // update timer
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::BuildGUI ( )
{

  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "OpenCV", "OpenCV", NULL );

  BuildGUIForHelpFrame();
  BuildGUIForTestFrame1();
  BuildGUIForTestFrame2();

}


void vtkOpenCVGUI::BuildGUIForHelpFrame ()
{
  // Define your help text here.
  const char *help = 
    "See "
    "<a>http://www.slicer.org/slicerWiki/index.php/Modules:OpenCV</a> for details.";
  const char *about =
    "This work is supported by NCIGT, NA-MIC.";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "OpenCV" );
  this->BuildHelpAndAboutFrame (page, help, about);
}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::BuildGUIForTestFrame1()
{

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("OpenCV");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Test Frame 1");
  //conBrowsFrame->CollapseFrame();
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Test child frame

  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New();
  frame->SetParent(conBrowsFrame->GetFrame());
  frame->Create();
  frame->SetLabelText ("Test child frame");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 frame->GetWidgetName() );

  // -----------------------------------------
  // Test push button

  this->showCaptureData = vtkKWPushButton::New ( );
  this->showCaptureData->SetParent ( frame->GetFrame() );
  this->showCaptureData->Create ( );
  this->showCaptureData->SetText ("secondary window on");
  this->showCaptureData->SetWidth (24);

  this->closeCaptureData = vtkKWPushButton::New ( );
  this->closeCaptureData->SetParent ( frame->GetFrame() );
  this->closeCaptureData->Create ( );
//  this->closeCaptureData->SetText ("secondary window off");
  this->closeCaptureData->SetText ("MRML observer on");
  this->closeCaptureData->SetWidth (24);

  this->Script("pack %s %s -side left -padx 2 -pady 2", 
               this->showCaptureData->GetWidgetName(),
               this->closeCaptureData->GetWidgetName());

  conBrowsFrame->Delete();
  frame->Delete();

}


//---------------------------------------------------------------------------
void vtkOpenCVGUI::BuildGUIForTestFrame2 ()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("OpenCV");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Test Frame 2");
  //conBrowsFrame->CollapseFrame();
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Test child frame

  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New();
  frame->SetParent(conBrowsFrame->GetFrame());
  frame->Create();
  frame->SetLabelText ("Test child frame");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 frame->GetWidgetName() );
  
  // -----------------------------------------
  // Test push button

  this->TestButton21 = vtkKWPushButton::New ( );
  this->TestButton21->SetParent ( frame->GetFrame() );
  this->TestButton21->Create ( );
  this->TestButton21->SetText ("Test 21");
  this->TestButton21->SetWidth (12);

  this->TestButton22 = vtkKWPushButton::New ( );
  this->TestButton22->SetParent ( frame->GetFrame() );
  this->TestButton22->Create ( );
  this->TestButton22->SetText ("Tset 22");
  this->TestButton22->SetWidth (12);

  this->Script("pack %s %s -side left -padx 2 -pady 2", 
               this->TestButton21->GetWidgetName(),
               this->TestButton22->GetWidgetName());


  conBrowsFrame->Delete();
  frame->Delete();
}


//----------------------------------------------------------------------------
void vtkOpenCVGUI::UpdateAll()
{
}



