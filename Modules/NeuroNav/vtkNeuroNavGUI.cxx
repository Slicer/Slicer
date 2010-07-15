#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkNeuroNavGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkKWRenderWidget.h"
#include "vtkKWWidget.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWMenu.h"
#include "vtkKWLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWEvent.h"


#include "vtkKWTkUtilities.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkCylinderSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkCornerAnnotation.h"

#include "vtkIGTDataStream.h"

#include "vtkCylinderSource.h"
#include "vtkMRMLLinearTransformNode.h"


//---------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkNeuroNavGUI, "$Revision: 1.0 $");


vtkNeuroNavGUI* vtkNeuroNavGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkNeuroNavGUI");
  if (ret)
    {
    return (vtkNeuroNavGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkNeuroNavGUI;
}



//---------------------------------------------------------------------------
vtkNeuroNavGUI::vtkNeuroNavGUI ( )
{
  this->Logic = NULL;

  this->LocatorCheckButton = NULL;
  this->TractographyCheckButton = NULL;
  this->HandleCheckButton = NULL;
  this->GuideCheckButton = NULL;

  this->LocatorModeCheckButton = NULL;
  this->UserModeCheckButton = NULL;
  this->FreezeCheckButton = NULL;
  this->ObliqueCheckButton = NULL;

  this->RedSliceMenu = NULL;
  this->YellowSliceMenu = NULL;
  this->GreenSliceMenu = NULL;

  this->TransformNodeNameEntry = NULL;
  this->FiducialListNodeNameEntry = NULL;

  this->PatCoordinatesEntry = NULL;
  this->SlicerCoordinatesEntry = NULL;
  this->GetPatCoordinatesPushButton = NULL;
  this->AddPointPairPushButton = NULL;

  this->PointPairMultiColumnList = NULL;

  // this->LoadPointPairPushButton = NULL;
  // this->SavePointPairPushButton = NULL;
  this->DeletePointPairPushButton = NULL;
  this->DeleteAllPointPairPushButton = NULL;
  this->RegisterPushButton = NULL;
  this->ResetPushButton = NULL;

  this->CloseScene = false;
  this->TimerFlag = 0;

  this->TranslationScale = NULL;
}


//---------------------------------------------------------------------------
vtkNeuroNavGUI::~vtkNeuroNavGUI ( )
{
  this->RemoveGUIObservers();

  if (this->LocatorCheckButton)
    {
    this->LocatorCheckButton->SetParent(NULL );
    this->LocatorCheckButton->Delete ( );
    }
  if (this->TractographyCheckButton)
    {
    this->TractographyCheckButton->SetParent(NULL );
    this->TractographyCheckButton->Delete ( );
    }
  if (this->HandleCheckButton)
    {
    this->HandleCheckButton->SetParent(NULL );
    this->HandleCheckButton->Delete ( );
    }
  if (this->GuideCheckButton)
    {
    this->GuideCheckButton->SetParent(NULL );
    this->GuideCheckButton->Delete ( );
    }

  if (this->LocatorModeCheckButton)
    {
    this->LocatorModeCheckButton->SetParent(NULL );
    this->LocatorModeCheckButton->Delete ( );
    }
  if (this->UserModeCheckButton)
    {
    this->UserModeCheckButton->SetParent(NULL );
    this->UserModeCheckButton->Delete ( );
    }
  if (this->FreezeCheckButton)
    {
    this->FreezeCheckButton->SetParent(NULL );
    this->FreezeCheckButton->Delete ( );
    }
  if (this->ObliqueCheckButton)
    {
    this->ObliqueCheckButton->SetParent(NULL );
    this->ObliqueCheckButton->Delete ( );
    }


  if (this->RedSliceMenu)
    {
    this->RedSliceMenu->SetParent(NULL );
    this->RedSliceMenu->Delete ( );
    }
  if (this->YellowSliceMenu)
    {
    this->YellowSliceMenu->SetParent(NULL );
    this->YellowSliceMenu->Delete ( );
    }
  if (this->GreenSliceMenu)
    {
    this->GreenSliceMenu->SetParent(NULL );
    this->GreenSliceMenu->Delete ( );
    }
  if (this->TransformNodeNameEntry)
    {
    this->TransformNodeNameEntry->SetParent(NULL);
    this->TransformNodeNameEntry->Delete();
    }
  if (this->FiducialListNodeNameEntry)
    {
    this->FiducialListNodeNameEntry->SetParent(NULL);
    this->FiducialListNodeNameEntry->Delete();
    }
  if (this->PatCoordinatesEntry)
    {
    this->PatCoordinatesEntry->SetParent(NULL );
    this->PatCoordinatesEntry->Delete ( );
    }
  if (this->SlicerCoordinatesEntry)
    {
    this->SlicerCoordinatesEntry->SetParent(NULL );
    this->SlicerCoordinatesEntry->Delete ( );
    }
  if (this->GetPatCoordinatesPushButton)
    {
    this->GetPatCoordinatesPushButton->SetParent(NULL );
    this->GetPatCoordinatesPushButton->Delete ( );
    }
  if (this->AddPointPairPushButton)
    {
    this->AddPointPairPushButton->SetParent(NULL );
    this->AddPointPairPushButton->Delete ( );
    }

  if (this->PointPairMultiColumnList)
    {
    this->PointPairMultiColumnList->SetParent(NULL );
    this->PointPairMultiColumnList->Delete ( );
    }

  /*
     if (this->LoadPointPairPushButton)
     {
     this->LoadPointPairPushButton->SetParent(NULL );
     this->LoadPointPairPushButton->Delete ( );
     }
     if (this->SavePointPairPushButton)
     {
     this->SavePointPairPushButton->SetParent(NULL );
     this->SavePointPairPushButton->Delete ( );
     }
     */
  if (this->DeletePointPairPushButton)
    {
    this->DeletePointPairPushButton->SetParent(NULL );
    this->DeletePointPairPushButton->Delete ( );
    }
  if (this->DeleteAllPointPairPushButton)
    {
    this->DeleteAllPointPairPushButton->SetParent(NULL );
    this->DeleteAllPointPairPushButton->Delete ( );
    }
  if (this->RegisterPushButton)
    {
    this->RegisterPushButton->SetParent(NULL );
    this->RegisterPushButton->Delete ( );
    }
  if (this->ResetPushButton)
    {
    this->ResetPushButton->SetParent(NULL );
    this->ResetPushButton->Delete ( );
    }

  if (this->TranslationScale)
    {
    this->TranslationScale->SetParent(NULL);
    this->TranslationScale->Delete();
    this->TranslationScale = NULL;
    }

  this->SetModuleLogic ( NULL );
}



//---------------------------------------------------------------------------
void vtkNeuroNavGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "NeuroNavGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
  // print widgets?
}


//---------------------------------------------------------------------------
void vtkNeuroNavGUI::RemoveGUIObservers ( )
{
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  if ( appGUI )
    {
    appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()
      ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()
      ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()
      ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

//  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
//  appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
//  appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
//  appGUI->GetMainSliceGUI("Blue")->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);


  if (this->GetPatCoordinatesPushButton)
    {
    this->GetPatCoordinatesPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->AddPointPairPushButton)
    {
    this->AddPointPairPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->DeletePointPairPushButton)
    {
    this->DeletePointPairPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->DeleteAllPointPairPushButton)
    {
    this->DeleteAllPointPairPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->RegisterPushButton)
    {
    this->RegisterPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ResetPushButton)
    {
    this->ResetPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->LocatorCheckButton)
    {
    this->LocatorCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->LocatorModeCheckButton)
    {
    this->LocatorModeCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->UserModeCheckButton)
    {
    this->UserModeCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->FreezeCheckButton)
    {
    this->FreezeCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ObliqueCheckButton)
    {
    this->ObliqueCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
}


//---------------------------------------------------------------------------
void vtkNeuroNavGUI::AddGUIObservers ( )
{
  this->RemoveGUIObservers();

  // make a user interactor style to process our events
  // look at the InteractorStyle to get our events

  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  //----------------------------------------------------------------
  // MRML

  vtkIntArray* events = vtkIntArray::New();
  //events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  //events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  events->Delete();

  
  //----------------------------------------------------------------
  // Main Slice GUI

  appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  

  // Fill in
  // observer load volume button
  this->GetPatCoordinatesPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddPointPairPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeletePointPairPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllPointPairPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RegisterPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ResetPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->LocatorCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LocatorModeCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->UserModeCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FreezeCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ObliqueCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}



void vtkNeuroNavGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  vtkSlicerInteractorStyle *istyle0 = 
    vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()
                                           ->GetRenderWindowInteractor()->GetInteractorStyle());

  vtkSlicerInteractorStyle *istyle1 = 
    vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()
                                           ->GetRenderWindowInteractor()->GetInteractorStyle());

  vtkSlicerInteractorStyle *istyle2 = 
    vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()
                                           ->GetRenderWindowInteractor()->GetInteractorStyle());


/*
    vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Red")->GetSliceViewer()
                                           ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
  vtkSlicerInteractorStyle *istyle1 = 
    vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()
                                           ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
  vtkSlicerInteractorStyle *istyle2 = 
    vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Blue")->GetSliceViewer()
                                           ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
*/

  vtkCornerAnnotation *anno = NULL;
  if (style == istyle0)
    {
    anno = appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
  else if (style == istyle1)
    {
    anno = appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
  else if (style == istyle2)
    {
    anno = appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }

  if (anno)
    {
    const char *rasText = anno->GetText(1);
    if ( rasText != NULL )
      {
      std::string ras = std::string(rasText);

      // remove "R:," "A:," and "S:" from the string
      size_t loc = ras.find("R:", 0);
      if ( loc != std::string::npos ) 
        {
        ras = ras.replace(loc, 2, "");
        }
      loc = ras.find("A:", 0);
      if ( loc != std::string::npos ) 
        {
        ras = ras.replace(loc, 2, "");
        }
      loc = ras.find("S:", 0);
      if ( loc != std::string::npos ) 
        {
        ras = ras.replace(loc, 2, "");
        }

      // remove "\n" from the string
      size_t found = ras.find("\n", 0);
      while ( found != std::string::npos )
        {
        ras = ras.replace(found, 1, " ");
        found = ras.find("\n", 0);
        }

      this->SlicerCoordinatesEntry->GetWidget()->SetValue(ras.c_str());
      }
    }
}


//---------------------------------------------------------------------------
void vtkNeuroNavGUI::ProcessGUIEvents ( vtkObject *caller,
                                        unsigned long event, void *vtkNotUsed(callData))
{
  const char *eventName = vtkCommand::GetStringFromEventId(event);

  if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
    vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
    HandleMouseEvent(style);
    }
  else
    {
    if (this->GetPatCoordinatesPushButton == vtkKWPushButton::SafeDownCast(caller) 
        && event == vtkKWPushButton::InvokedEvent)
      {
      double px, py, pz;
      char value[50];
      this->GetLogic()->GetCurrentPosition(&px, &py, &pz);
      sprintf(value, "%6.2f  %6.2f  %6.2f", px, py, pz);
      this->PatCoordinatesEntry->GetWidget()->SetValue(value);
      }
      else if (this->AddPointPairPushButton == vtkKWPushButton::SafeDownCast(caller) 
               && event == vtkKWPushButton::InvokedEvent)
        {
        int scSize = 0;
        int pcSize = 0;
        const char *pc = this->PatCoordinatesEntry->GetWidget()->GetValue();
        const char *sc = this->SlicerCoordinatesEntry->GetWidget()->GetValue();

        if (pc) 
          {
          const vtksys_stl::string pcCor(pc);
          pcSize = pcCor.size();
        }
      if (sc) 
        {
        const vtksys_stl::string scCor(sc);
        scSize = scCor.size();
        }

      if (pcSize < 5 || scSize < 5)
        {
        vtkSlicerApplication::GetInstance()->ErrorMessage("Patient or Slicer coordinates are invalid."); 
        }
      else 
        {
        int row = this->PointPairMultiColumnList->GetWidget()->GetNumberOfRows();
        this->PointPairMultiColumnList->GetWidget()->AddRow();
        this->PointPairMultiColumnList->GetWidget()->SetCellText(row, 0, pc);
        this->PointPairMultiColumnList->GetWidget()->SetCellText(row, 1, sc);
        }
      }
    else if (this->DeletePointPairPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
      {
      int numOfRows = this->PointPairMultiColumnList->GetWidget()->GetNumberOfSelectedRows();
      if (numOfRows == 1)
        {
        int index[2];
        this->PointPairMultiColumnList->GetWidget()->GetSelectedRows(index);
        this->PointPairMultiColumnList->GetWidget()->DeleteRow(index[0]);
        }
      }
    else if (this->DeleteAllPointPairPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
      {
      this->PointPairMultiColumnList->GetWidget()->DeleteAllRows();
      }
    else if (this->RegisterPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
      {
      int row = this->PointPairMultiColumnList->GetWidget()->GetNumberOfRows();
      if (row < 2)
        {
        vtkSlicerApplication::GetInstance()->ErrorMessage("At least 2 pairs of landmarks are needed for patient to image registration.");
        }
      else
        {
        this->GetLogic()->GetPat2ImgReg()->SetNumberOfPoints(row);
        float pc1 = 0.0, pc2 = 0.0, pc3 = 0.0, sc1 = 0.0, sc2 = 0.0, sc3 = 0.0;
        for (int r = 0; r < row; r++)
          {
          for (int c = 0; c < 2; c++)
            {
            const char *val = this->PointPairMultiColumnList->GetWidget()->GetCellText(r, c);
            if (c == 0)
              {
              sscanf(val, "%f %f %f", &pc1, &pc2, &pc3);
              }
            else
              {
              sscanf(val, "%f %f %f", &sc1, &sc2, &sc3);
              }
            }
          this->GetLogic()->GetPat2ImgReg()->AddPoint(r, sc1, sc2, sc3, pc1, pc2, pc3);
          }

        int error = this->GetLogic()->PerformPatientToImageRegistration();
        if (error)
          {
          vtkSlicerApplication::GetInstance()->ErrorMessage("Error registration between patient and image land marks.");
          return;
          }
        }
      }
    else if (this->ResetPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
      {
      this->GetLogic()->SetUseRegistration(0);
      }
    else if (this->LocatorCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
             && event == vtkKWCheckButton::SelectedStateChangedEvent )
      {
      int checked = this->LocatorCheckButton->GetSelectedState(); 
      if (!this->CloseScene)
        {
        if (checked)
          {
          this->GetLogic()->SetVisibilityOfLocatorModel("IGTLocator", 1);
          }
        else
          {
          this->GetLogic()->SetVisibilityOfLocatorModel("IGTLocator", 0);
          }
        }
      else
        {
        this->CloseScene = false;
        }
      }
    else if (this->LocatorModeCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
             && event == vtkKWCheckButton::SelectedStateChangedEvent )
      {
      int checked = this->LocatorModeCheckButton->GetSelectedState(); 
      std::string val("Locator");

      if (checked)
        {
        this->UserModeCheckButton->SelectedStateOff();
        }
      else
        {
        this->UserModeCheckButton->SelectedStateOn();

        val = "User";
        }
      this->RedSliceMenu->SetValue(val.c_str());
      this->YellowSliceMenu->SetValue(val.c_str());
      this->GreenSliceMenu->SetValue(val.c_str());
      }
    else if (this->UserModeCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
             && event == vtkKWCheckButton::SelectedStateChangedEvent )
      {
      int checked = this->UserModeCheckButton->GetSelectedState(); 
      std::string val("User");

      if (checked)
        {
        this->LocatorModeCheckButton->SelectedStateOff();
        }
      else
        {
        this->LocatorModeCheckButton->SelectedStateOn();
        val = "Locator";
        }
      this->RedSliceMenu->SetValue(val.c_str());
      this->YellowSliceMenu->SetValue(val.c_str());
      this->GreenSliceMenu->SetValue(val.c_str());
      }
    else if (this->FreezeCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
             && event == vtkKWCheckButton::SelectedStateChangedEvent )
      {
      // int checked = this->FreezeCheckButton->GetSelectedState(); 
      // cout << "FreezeCheckButton = " << checked << endl;
      }
    else if (this->ObliqueCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
             && event == vtkKWCheckButton::SelectedStateChangedEvent )
      {
      int checked = this->ObliqueCheckButton->GetSelectedState(); 
      this->GetLogic()->SetEnableOblique(checked);
      }
    }
} 



//---------------------------------------------------------------------------
void vtkNeuroNavGUI::ProcessLogicEvents ( vtkObject *vtkNotUsed(caller),
                                          unsigned long vtkNotUsed(event),
                                          void *vtkNotUsed(callData))
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkNeuroNavGUI::ProcessMRMLEvents ( vtkObject *vtkNotUsed(caller),
                                         unsigned long event, void *vtkNotUsed(callData))
{
  if (event == vtkMRMLScene::SceneClosedEvent)
    {
    if (this->LocatorCheckButton != NULL && this->LocatorCheckButton->GetSelectedState())
      {
      this->CloseScene = true;
      this->LocatorCheckButton->SelectedStateOff();
      }
    }


}



//---------------------------------------------------------------------------
void vtkNeuroNavGUI::Enter ( )
{
  // Fill in

  if (this->TimerFlag == 0)
    {
    this->TimerFlag = 1;
    this->TimerInterval = 100;  // 100 ms
    ProcessTimerEvents();
    }

}



//---------------------------------------------------------------------------
void vtkNeuroNavGUI::ProcessTimerEvents()
{
  if (this->TimerFlag)
    {
    // -----------------------------------------
    // Check incomming new data

    // this->GetLogic()->ImportFromCircularBuffers();

    const char *nodeName = this->TransformNodeNameEntry->GetWidget()->GetValue();
    this->GetLogic()->UpdateTransformNodeByName(nodeName);
    int checked = this->FreezeCheckButton->GetSelectedState(); 
    if (!checked)
      {
      int sn1 = 0;  // 0 = Locator; 1 = User
      int sn2 = 0;
      int sn3 = 0;
      if (strcmp(this->RedSliceMenu->GetValue(), "Locator"))
        {
        sn1 = 1;
        }
      if (strcmp(this->YellowSliceMenu->GetValue(), "Locator"))
        {
        sn2 = 1;
        }
      if (strcmp(this->GreenSliceMenu->GetValue(), "Locator"))
        {
        sn3 = 1;
        }

      this->GetLogic()->UpdateDisplay(sn1, sn2, sn3);
      }


    // Tractography seeding
    checked = this->TractographyCheckButton->GetSelectedState(); 
    if (checked)
    {
    const char *nodeName2 = this->FiducialListNodeNameEntry->GetWidget()->GetValue();
    double offset = this->TranslationScale->GetValue();
    this->GetLogic()->UpdateFiducialSeeding(nodeName2, offset);
    }
 
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}



//---------------------------------------------------------------------------
void vtkNeuroNavGUI::Exit ( )
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkNeuroNavGUI::BuildGUI ( )
{
  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "NeuroNav", "NeuroNav", NULL );

  BuildGUIForHelpFrame();
 
  // BuildGUIForDeviceFrame ();
  BuildGUIForRegistrationFrame();
  BuildGUIForTrackingFrame();
  // BuildGUIForHandPieceFrame ();
}



void vtkNeuroNavGUI::BuildGUIForHelpFrame()
{

  // ----------------------------------------------------------------
  // HELP FRAME         
  // ----------------------------------------------------------------

  // Define your help text here.

  const char *help = "NeuroNav is an intraoperative navigation system for neurosurgery. Please check this link for details: \n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:NeuroNav-Documentation-3.6</a>";
  const char *about = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. The NeuroNav module was contributed by Haiying Liu and Noby Hata at SPL, BWH (Ron Kikinis).";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "NeuroNav" );
  this->BuildHelpAndAboutFrame (page, help, about);

}



void vtkNeuroNavGUI::BuildGUIForRegistrationFrame ()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "NeuroNav" );

  // ----------------------------------------------------------------
  // REGISTRATION FRAME            
  // ----------------------------------------------------------------
  vtkSlicerModuleCollapsibleFrame *regFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  regFrame->SetParent ( page );
  regFrame->Create ( );
  regFrame->SetLabelText ("Registration");
  regFrame->CollapseFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                regFrame->GetWidgetName(), page->GetWidgetName());


  // add a point pair 
  vtkKWFrameWithLabel *addFrame = vtkKWFrameWithLabel::New();
  addFrame->SetParent ( regFrame->GetFrame() );
  addFrame->Create ( );
  addFrame->SetLabelText ("Add a point pair");
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                addFrame->GetWidgetName());

  vtkKWFrame *patFrame = vtkKWFrame::New();
  patFrame->SetParent ( addFrame->GetFrame() );
  patFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                patFrame->GetWidgetName());

  vtkKWFrame *okFrame = vtkKWFrame::New();
  okFrame->SetParent ( addFrame->GetFrame() );
  okFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                okFrame->GetWidgetName());

  this->PatCoordinatesEntry = vtkKWEntryWithLabel::New();
  this->PatCoordinatesEntry->SetParent(patFrame);
  this->PatCoordinatesEntry->Create();
  this->PatCoordinatesEntry->SetWidth(30);
  this->PatCoordinatesEntry->SetLabelWidth(16);
  this->PatCoordinatesEntry->SetLabelText("Patient Coordinates:");
  this->PatCoordinatesEntry->GetWidget()->SetValue ( "" );

  this->GetPatCoordinatesPushButton = vtkKWPushButton::New();
  this->GetPatCoordinatesPushButton->SetParent(patFrame);
  this->GetPatCoordinatesPushButton->Create();
  this->GetPatCoordinatesPushButton->SetText("Get");
  this->GetPatCoordinatesPushButton->SetWidth ( 6 );

  this->Script(
               "pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2", 
               this->PatCoordinatesEntry->GetWidgetName(),
               this->GetPatCoordinatesPushButton->GetWidgetName());

  this->SlicerCoordinatesEntry = vtkKWEntryWithLabel::New();
  this->SlicerCoordinatesEntry->SetParent(okFrame);
  this->SlicerCoordinatesEntry->Create();
  this->SlicerCoordinatesEntry->SetWidth(30);
  this->SlicerCoordinatesEntry->SetLabelWidth(16);
  this->SlicerCoordinatesEntry->SetLabelText("Image Coordinates:");
  this->SlicerCoordinatesEntry->GetWidget()->SetValue ( "" );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->SlicerCoordinatesEntry->GetWidgetName());

  this->AddPointPairPushButton = vtkKWPushButton::New();
  this->AddPointPairPushButton->SetParent(okFrame);
  this->AddPointPairPushButton->Create();
  this->AddPointPairPushButton->SetText( "OK" );
  this->AddPointPairPushButton->SetWidth ( 12 );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->AddPointPairPushButton->GetWidgetName());

  // list of defined point pairs 
  vtkKWFrameWithLabel *listFrame = vtkKWFrameWithLabel::New();
  listFrame->SetParent ( regFrame->GetFrame() );
  listFrame->Create ( );
  listFrame->SetLabelText ("Defined point pairs");
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                listFrame->GetWidgetName());


  // add the multicolumn list to show the points
  this->PointPairMultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->PointPairMultiColumnList->SetParent ( listFrame->GetFrame() );
  this->PointPairMultiColumnList->Create ( );
  this->PointPairMultiColumnList->SetHeight(1);
  this->PointPairMultiColumnList->GetWidget()->SetSelectionTypeToRow();
  this->PointPairMultiColumnList->GetWidget()->MovableRowsOff();
  this->PointPairMultiColumnList->GetWidget()->MovableColumnsOff();
  // set up the columns of data for each point
  // refer to the header file for order
  this->PointPairMultiColumnList->GetWidget()->AddColumn("Patient Coordinates");
  this->PointPairMultiColumnList->GetWidget()->AddColumn("Image Coordinates");

  // make the selected column editable by checkbox
  //    this->PointPairMultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(this->SelectedColumn);

  // now set the attributes that are equal across the columns
  for (int col = 0; col < 2; col++)
    {
    this->PointPairMultiColumnList->GetWidget()->SetColumnWidth(col, 22);

    this->PointPairMultiColumnList->GetWidget()->SetColumnAlignmentToLeft(col);
    this->PointPairMultiColumnList->GetWidget()->ColumnEditableOff(col);
    }
  /*
     if (col >= this->XColumn && col <= this->OrZColumn)
     {
     this->PointPairMultiColumnList->GetWidget()->SetColumnEditWindowToSpinBox(col);
     }
     */
  // set the name column width to be higher
  // this->PointPairMultiColumnList->GetWidget()->SetColumnWidth(this->NameColumn, 15);
  // set the selected column width a bit higher
  // this->PointPairMultiColumnList->GetWidget()->SetColumnWidth(this->SelectedColumn, 9);

  app->Script ( "pack %s -fill both -expand true",
                this->PointPairMultiColumnList->GetWidgetName());
  //                  listFrame->GetWidgetName());
  //    this->PointPairMultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "UpdateElement");

  // button frame
  vtkKWFrame *buttonFrame = vtkKWFrame::New();
  buttonFrame->SetParent ( listFrame->GetFrame() );
  buttonFrame->Create ( );
  app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
               buttonFrame->GetWidgetName(),
               listFrame->GetFrame()->GetWidgetName());
  /*    
  // add an load button
  this->LoadPointPairPushButton = vtkKWPushButton::New ( );
  this->LoadPointPairPushButton->SetParent ( buttonFrame );
  this->LoadPointPairPushButton->Create ( );
  this->LoadPointPairPushButton->SetText ("Load Points");
  this->LoadPointPairPushButton->SetWidth (12);
  this->LoadPointPairPushButton->SetBalloonHelpString("Load point pairs from a file.");

  // add a save button
  this->SavePointPairPushButton = vtkKWPushButton::New ( );
  this->SavePointPairPushButton->SetParent ( buttonFrame );
  this->SavePointPairPushButton->Create ( );
  this->SavePointPairPushButton->SetText ("Save Points");
  this->SavePointPairPushButton->SetWidth (12);
  this->SavePointPairPushButton->SetBalloonHelpString("Save all defined point pairs to a file.");
  */

  // add a delete button 
  this->DeletePointPairPushButton = vtkKWPushButton::New ( );
  this->DeletePointPairPushButton->SetParent ( buttonFrame );
  this->DeletePointPairPushButton->Create ( );
  this->DeletePointPairPushButton->SetText ("Delete Points");
  this->DeletePointPairPushButton->SetWidth (12);
  this->DeletePointPairPushButton->SetBalloonHelpString("Delete the selected point pair.");

  // add a delete button 
  this->DeleteAllPointPairPushButton = vtkKWPushButton::New ( );
  this->DeleteAllPointPairPushButton->SetParent ( buttonFrame );
  this->DeleteAllPointPairPushButton->Create ( );
  this->DeleteAllPointPairPushButton->SetText ("Delete All Points");
  this->DeleteAllPointPairPushButton->SetWidth (12);
  this->DeleteAllPointPairPushButton->SetBalloonHelpString("Delete all point pairs.");

  app->Script("pack %s %s -side left -anchor w -padx 2 -pady 2", 
              this->DeletePointPairPushButton->GetWidgetName(),
              this->DeleteAllPointPairPushButton->GetWidgetName());



  // do registration
  vtkKWFrame *actionFrame = vtkKWFrame::New();
  actionFrame->SetParent ( regFrame->GetFrame() );
  actionFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                actionFrame->GetWidgetName());

  // add a register button 
  this->RegisterPushButton = vtkKWPushButton::New ( );
  this->RegisterPushButton->SetParent ( actionFrame );
  this->RegisterPushButton->Create ( );
  this->RegisterPushButton->SetText ("Register");
  this->RegisterPushButton->SetWidth (12);
  this->RegisterPushButton->SetBalloonHelpString("Perform patient to image registration.");

  // add a reset button 
  this->ResetPushButton = vtkKWPushButton::New ( );
  this->ResetPushButton->SetParent ( actionFrame );
  this->ResetPushButton->Create ( );
  this->ResetPushButton->SetText ("Reset");
  this->ResetPushButton->SetWidth (12);
  this->ResetPushButton->SetBalloonHelpString("Ignore the current registration.");


  app->Script("pack %s %s -side left -anchor w -padx 2 -pady 2", 
              this->RegisterPushButton->GetWidgetName(),
              this->ResetPushButton->GetWidgetName());


  regFrame->Delete ();
  addFrame->Delete ();
  patFrame->Delete ();
  okFrame->Delete ();
  listFrame->Delete ();
  buttonFrame->Delete ();
  actionFrame->Delete ();

}


void vtkNeuroNavGUI::BuildGUIForTrackingFrame ()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "NeuroNav" );


  // ----------------------------------------------------------------
  // Navigation FRAME            
  // ----------------------------------------------------------------
  vtkSlicerModuleCollapsibleFrame *trackingFrame = vtkSlicerModuleCollapsibleFrame::New ( );    
  trackingFrame->SetParent ( page );
  trackingFrame->Create ( );
  trackingFrame->SetLabelText ("Navigation");
  //trackingFrame->ExpandFrame ( );
  trackingFrame->CollapseFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                trackingFrame->GetWidgetName(), page->GetWidgetName());



  // Display frame: Options to locator display 
  // -----------------------------------------
  vtkKWFrameWithLabel *displayFrame = vtkKWFrameWithLabel::New ( );
  displayFrame->SetParent ( trackingFrame->GetFrame() );
  displayFrame->Create ( );
  displayFrame->SetLabelText ("Locator Display");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 displayFrame->GetWidgetName() );


  this->TransformNodeNameEntry = vtkKWEntryWithLabel::New();
  this->TransformNodeNameEntry->SetParent(displayFrame->GetFrame());
  this->TransformNodeNameEntry->Create();
  this->TransformNodeNameEntry->SetWidth(40);
  this->TransformNodeNameEntry->SetLabelWidth(30);
  this->TransformNodeNameEntry->SetLabelText("Input (Transform) Node Name:");
  this->TransformNodeNameEntry->GetWidget()->SetValue ( "Tracker" );
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->TransformNodeNameEntry->GetWidgetName());


  this->LocatorCheckButton = vtkKWCheckButton::New();
  this->LocatorCheckButton->SetParent(displayFrame->GetFrame());
  this->LocatorCheckButton->Create();
  this->LocatorCheckButton->SelectedStateOff();
  this->LocatorCheckButton->SetText("Show Locator");

  /*
     this->HandleCheckButton = vtkKWCheckButton::New();
     this->HandleCheckButton->SetParent(displayFrame->GetFrame());
     this->HandleCheckButton->Create();
     this->HandleCheckButton->SelectedStateOff();
     this->HandleCheckButton->SetText("Show Handle");

     this->GuideCheckButton = vtkKWCheckButton::New();
     this->GuideCheckButton->SetParent(displayFrame->GetFrame());
     this->GuideCheckButton->Create();
     this->GuideCheckButton->SelectedStateOff();
     this->GuideCheckButton->SetText("Show Guide");


     this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
     this->LocatorCheckButton->GetWidgetName(),
     this->HandleCheckButton->GetWidgetName(),
     this->GuideCheckButton->GetWidgetName());
     */


  this->Script("pack %s -side left -anchor w -padx 2 -pady 2", 
               this->LocatorCheckButton->GetWidgetName());


  // Tractography frame: Options to tractography display 
  // -----------------------------------------
  vtkKWFrameWithLabel *tractographyFrame = vtkKWFrameWithLabel::New ( );
  tractographyFrame->SetParent ( trackingFrame->GetFrame() );
  tractographyFrame->Create ( );
  tractographyFrame->SetLabelText ("Tractography Seeding");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 tractographyFrame->GetWidgetName() );


  this->FiducialListNodeNameEntry = vtkKWEntryWithLabel::New();
  this->FiducialListNodeNameEntry->SetParent(tractographyFrame->GetFrame());
  this->FiducialListNodeNameEntry->Create();
  this->FiducialListNodeNameEntry->SetWidth(40);
  this->FiducialListNodeNameEntry->SetLabelWidth(30);
  this->FiducialListNodeNameEntry->SetLabelText("Fiducial List Node Name:");
  this->FiducialListNodeNameEntry->GetWidget()->SetValue ( "L" );
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->FiducialListNodeNameEntry->GetWidgetName());

  this->TranslationScale =  vtkKWScaleWithEntry::New() ;
  this->TranslationScale->SetParent( tractographyFrame->GetFrame() );
  this->TranslationScale->Create();
  this->TranslationScale->SetLabelText("Fiducial Translation: ");
  this->TranslationScale->SetWidth ( 40 );
  this->TranslationScale->SetLabelWidth(30);
  this->TranslationScale->SetRange(-80, 80);
  this->TranslationScale->SetStartCommand(this, "TransformChangingCallback");
  this->TranslationScale->SetCommand(this, "TransformChangingCallback");
  this->TranslationScale->SetEndCommand(this, "TransformChangedCallback");
  this->TranslationScale->SetEntryCommand(this, "TransformChangedCallback");
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 3", 
          this->TranslationScale->GetWidgetName());

  this->TractographyCheckButton = vtkKWCheckButton::New();
  this->TractographyCheckButton->SetParent(tractographyFrame->GetFrame());
  this->TractographyCheckButton->Create();
  this->TractographyCheckButton->SelectedStateOff();
  this->TractographyCheckButton->SetText("Fiducial Seeding");

  this->Script("pack %s -side left -anchor w -padx 2 -pady 2", 
               this->TractographyCheckButton->GetWidgetName());


  // Driver frame: Locator can drive slices 
  // -----------------------------------------
  vtkKWFrameWithLabel *driverFrame = vtkKWFrameWithLabel::New ( );
  driverFrame->SetParent ( trackingFrame->GetFrame() );
  driverFrame->Create ( );
  driverFrame->SetLabelText ("Driver");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 driverFrame->GetWidgetName() );

  // Mode frame
  vtkKWFrame *modeFrame = vtkKWFrame::New();
  modeFrame->SetParent ( driverFrame->GetFrame() );
  modeFrame->Create ( );
  app->Script ("pack %s -side top -anchor nw -fill x -pady 1 -in %s",
               modeFrame->GetWidgetName(),
               driverFrame->GetFrame()->GetWidgetName());


  this->LocatorModeCheckButton = vtkKWCheckButton::New();
  this->LocatorModeCheckButton->SetParent(modeFrame);
  this->LocatorModeCheckButton->Create();
  this->LocatorModeCheckButton->SelectedStateOff();
  this->LocatorModeCheckButton->SetText("Locator");

  this->UserModeCheckButton = vtkKWCheckButton::New();
  this->UserModeCheckButton->SetParent(modeFrame);
  this->UserModeCheckButton->Create();
  this->UserModeCheckButton->SelectedStateOn();
  this->UserModeCheckButton->SetText("User");

  this->FreezeCheckButton = vtkKWCheckButton::New();
  this->FreezeCheckButton->SetParent(modeFrame);
  this->FreezeCheckButton->Create();
  this->FreezeCheckButton->SelectedStateOff();
  this->FreezeCheckButton->SetText("Freeze");

  this->ObliqueCheckButton = vtkKWCheckButton::New();
  this->ObliqueCheckButton->SetParent(modeFrame);
  this->ObliqueCheckButton->Create();
  this->ObliqueCheckButton->SelectedStateOff();
  this->ObliqueCheckButton->SetText("Oblique");

  this->Script("pack %s %s %s %s -side left -anchor w -padx 2 -pady 2", 
               this->LocatorModeCheckButton->GetWidgetName(),
               this->UserModeCheckButton->GetWidgetName(),
               this->FreezeCheckButton->GetWidgetName(),
               this->ObliqueCheckButton->GetWidgetName());


  // slice frame
  vtkKWFrame *sliceFrame = vtkKWFrame::New();
  sliceFrame->SetParent ( driverFrame->GetFrame() );
  sliceFrame->Create ( );
  app->Script ("pack %s -side top -anchor nw -fill x -pady 1 -in %s",
               sliceFrame->GetWidgetName(),
               driverFrame->GetFrame()->GetWidgetName());


  // Contents in slice frame 
  vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );

  this->RedSliceMenu = vtkKWMenuButton::New();
  this->RedSliceMenu->SetParent(sliceFrame);
  this->RedSliceMenu->Create();
  this->RedSliceMenu->SetWidth(10);
  this->RedSliceMenu->SetBackgroundColor(color->SliceGUIRed);
  this->RedSliceMenu->SetActiveBackgroundColor(color->SliceGUIRed);
  this->RedSliceMenu->GetMenu()->AddRadioButton ("User");
  this->RedSliceMenu->GetMenu()->AddRadioButton ("Locator");
  this->RedSliceMenu->SetValue ("User");

  this->YellowSliceMenu = vtkKWMenuButton::New();
  this->YellowSliceMenu->SetParent(sliceFrame);
  this->YellowSliceMenu->Create();
  this->YellowSliceMenu->SetWidth(10);
  this->YellowSliceMenu->SetBackgroundColor(color->SliceGUIYellow);
  this->YellowSliceMenu->SetActiveBackgroundColor(color->SliceGUIYellow);
  this->YellowSliceMenu->GetMenu()->AddRadioButton ("User");
  this->YellowSliceMenu->GetMenu()->AddRadioButton ("Locator");
  this->YellowSliceMenu->SetValue ("User");

  this->GreenSliceMenu = vtkKWMenuButton::New();
  this->GreenSliceMenu->SetParent(sliceFrame);
  this->GreenSliceMenu->Create();
  this->GreenSliceMenu->SetWidth(10);
  this->GreenSliceMenu->SetBackgroundColor(color->SliceGUIGreen);
  this->GreenSliceMenu->SetActiveBackgroundColor(color->SliceGUIGreen);
  this->GreenSliceMenu->GetMenu()->AddRadioButton ("User");
  this->GreenSliceMenu->GetMenu()->AddRadioButton ("Locator");
  this->GreenSliceMenu->SetValue ("User");

  this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
               this->RedSliceMenu->GetWidgetName(),
               this->YellowSliceMenu->GetWidgetName(),
               this->GreenSliceMenu->GetWidgetName());

  trackingFrame->Delete();
  displayFrame->Delete();
  tractographyFrame->Delete();
  driverFrame->Delete();
  modeFrame->Delete();
  sliceFrame->Delete();
}


void vtkNeuroNavGUI::TransformChangedCallback(double)
{
//    cout << "Transform changed.\n";
}


void vtkNeuroNavGUI::TransformChangingCallback(double)
{
//    cout << "Transform changing.\n";
}

