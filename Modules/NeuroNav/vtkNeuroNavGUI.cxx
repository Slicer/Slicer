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

#include "vtkSlicerConfigure.h" /* Slicer3_USE_* */

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkNeuroNavGUI );
vtkCxxRevisionMacro ( vtkNeuroNavGUI, "$Revision: 1.0 $");


/* go redsox */


//---------------------------------------------------------------------------
vtkNeuroNavGUI::vtkNeuroNavGUI ( )
{
    this->Logic = NULL;

    this->NormalOffsetEntry = NULL; 
    this->TransOffsetEntry = NULL;
    this->NXTOffsetEntry = NULL;

    this->NormalSizeEntry = NULL;
    this->TransSizeEntry = NULL;
    this->RadiusEntry = NULL;

    this->NREntry = NULL;
    this->NAEntry = NULL;
    this->NSEntry = NULL;
    this->TREntry = NULL;
    this->TAEntry = NULL;
    this->TSEntry = NULL;
    this->PREntry = NULL;
    this->PAEntry = NULL;
    this->PSEntry = NULL;

    this->ExtraFrame = NULL;

/*
    this->RedColorScale = NULL;
    this->GreenColorScale = NULL;
    this->BlueColorScale = NULL;
    */

    this->ConnectCheckButton = NULL;

    this->LocatorCheckButton = NULL;
    this->HandleCheckButton = NULL;
    this->GuideCheckButton = NULL;

    this->LocatorModeCheckButton = NULL;
    this->UserModeCheckButton = NULL;

    this->RedSliceMenu = NULL;
    this->YellowSliceMenu = NULL;
    this->GreenSliceMenu = NULL;

//#ifdef Slicer3_USE_NAVITRACK
//    this->LoadConfigButton = NULL;
//    this->ConfigFileEntry = NULL;
//#endif
#ifdef Slicer3_USE_IGSTK
    this->DeviceMenuButton = NULL;
    this->PortNumberMenuButton = NULL;
    this->BaudRateMenuButton = NULL;
    this->DataBitsMenuButton = NULL;
    this->ParityTypeMenuButton = NULL;
    this->StopBitsMenuButton = NULL;
    this->HandShakeMenuButton = NULL;
#endif

    this->UpdateRateEntry = NULL;
    this->MultiFactorEntry = NULL;

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

    this->LocatorMatrix = NULL;

    this->LocatorModelDisplayNode = NULL;

    this->DataManager = vtkIGTDataManager::New();
    this->Pat2ImgReg = vtkIGTPat2ImgRegistration::New();

    this->DataCallbackCommand = vtkCallbackCommand::New();
    this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
    this->DataCallbackCommand->SetCallback(vtkNeuroNavGUI::DataCallback);

    this->Logic0 = NULL; 
    this->Logic1 = NULL; 
    this->Logic2 = NULL; 
    this->SliceNode0 = NULL; 
    this->SliceNode1 = NULL; 
    this->SliceNode2 = NULL; 
    this->Control0 = NULL; 
    this->Control1 = NULL; 
    this->Control2 = NULL; 

    this->NeedOrientationUpdate0 = 0;
    this->NeedOrientationUpdate1 = 0;
    this->NeedOrientationUpdate2 = 0;


//#ifdef Slicer3_USE_NAVITRACK
//    this->OpenTrackerStream = vtkIGTOpenTrackerStream::New();
//#endif
#ifdef Slicer3_USE_IGSTK
    this->IGSTKStream = vtkIGTIGSTKStream::New();
#endif

}


//---------------------------------------------------------------------------
vtkNeuroNavGUI::~vtkNeuroNavGUI ( )
{
//#ifdef Slicer3_USE_NAVITRACK
//    if (this->OpenTrackerStream)
//    {
//        this->OpenTrackerStream->Delete();
//    }
//#endif
#ifdef Slicer3_USE_IGSTK
    if (this->IGSTKStream)
    {
        this->IGSTKStream->Delete();
    }
#endif


    if (this->DataManager)
    {
        // If we don't set the scence to NULL for DataManager,
        // Slicer will report a lot leak when it is closed.
        this->DataManager->SetMRMLScene(NULL);
        this->DataManager->Delete();
    }
    if (this->Pat2ImgReg)
    {
        this->Pat2ImgReg->Delete();
    }
    if (this->DataCallbackCommand)
    {
        this->DataCallbackCommand->Delete();
    }

    this->RemoveGUIObservers();

    if (this->NormalOffsetEntry)
    {
        this->NormalOffsetEntry->SetParent(NULL );
        this->NormalOffsetEntry->Delete ( );
    }
    if (this->TransOffsetEntry)
    {
        this->TransOffsetEntry->SetParent(NULL );
        this->TransOffsetEntry->Delete ( );
    }
    if (this->NXTOffsetEntry)
    {
        this->NXTOffsetEntry->SetParent(NULL );
        this->NXTOffsetEntry->Delete ( );
    }
    if (this->NormalSizeEntry)
    {
        this->NormalSizeEntry->SetParent(NULL );
        this->NormalSizeEntry->Delete ( );
    }
    if (this->TransSizeEntry)
    {
        this->TransSizeEntry->SetParent(NULL );
        this->TransSizeEntry->Delete ( );
    }
    if (this->RadiusEntry)
    {
        this->RadiusEntry->SetParent(NULL );
        this->RadiusEntry->Delete ( );
    }
    if (this->NREntry)
    {
        this->NREntry->SetParent(NULL );
        this->NREntry->Delete ( );
    }
    if (this->NAEntry)
    {
        this->NAEntry->SetParent(NULL );
        this->NAEntry->Delete ( );
    }
    if (this->NSEntry)
    {
        this->NSEntry->SetParent(NULL );
        this->NSEntry->Delete ( );
    }
    if (this->TREntry)
    {
        this->TREntry->SetParent(NULL );
        this->TREntry->Delete ( );
    }
    if (this->TAEntry)
    {
        this->TAEntry->SetParent(NULL );
        this->TAEntry->Delete ( );
    }
    if (this->TSEntry)
    {
        this->TSEntry->SetParent(NULL );
        this->TSEntry->Delete ( );
    }
    if (this->PREntry)
    {
        this->PREntry->SetParent(NULL );
        this->PREntry->Delete ( );
    }
    if (this->PAEntry)
    {
        this->PAEntry->SetParent(NULL );
        this->PAEntry->Delete ( );
    }
    if (this->PSEntry)
    {
        this->PSEntry->SetParent(NULL );
        this->PSEntry->Delete ( );
    }
    /*
    if (this->RedColorScale)
    {
        this->RedColorScale->SetParent(NULL );
        this->RedColorScale->Delete ( );
    }
    if (this->GreenColorScale)
    {
        this->GreenColorScale->SetParent(NULL );
        this->GreenColorScale->Delete ( );
    }
    if (this->BlueColorScale)
    {
        this->BlueColorScale->SetParent(NULL );
        this->BlueColorScale->Delete ( );
    }
    */


    if (this->ConnectCheckButton)
    {
        this->ConnectCheckButton->SetParent(NULL );
        this->ConnectCheckButton->Delete ( );
    }

    if (this->LocatorCheckButton)
    {
        this->LocatorCheckButton->SetParent(NULL );
        this->LocatorCheckButton->Delete ( );
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

//#ifdef Slicer3_USE_NAVITRACK
//    if (this->LoadConfigButton)
//    {
//       this->LoadConfigButton->SetParent(NULL );
//        this->LoadConfigButton->Delete ( );
//    }
//
//    if (this->ConfigFileEntry)
//    {
//        this->ConfigFileEntry->SetParent(NULL );
//        this->ConfigFileEntry->Delete ( );
//    }
//#endif
#ifdef Slicer3_USE_IGSTK
    if (this->DeviceMenuButton) 
    {
        this->DeviceMenuButton->SetParent(NULL);
        this->DeviceMenuButton->Delete();
    }
    if (this->PortNumberMenuButton) 
    {
        this->PortNumberMenuButton->SetParent(NULL);
        this->PortNumberMenuButton->Delete();
    }
    if (this->BaudRateMenuButton) 
    {
        this->BaudRateMenuButton->SetParent(NULL);
        this->BaudRateMenuButton->Delete();
    }
    if (this->DataBitsMenuButton) 
    {
        this->DataBitsMenuButton->SetParent(NULL);
        this->DataBitsMenuButton->Delete();
    }
    if (this->ParityTypeMenuButton) 
    {
        this->ParityTypeMenuButton->SetParent(NULL);
        this->ParityTypeMenuButton->Delete();
    }
    if (this->StopBitsMenuButton) 
    {
        this->StopBitsMenuButton->SetParent(NULL);
        this->StopBitsMenuButton->Delete();
    }
    if (this->HandShakeMenuButton) 
    {
        this->HandShakeMenuButton->SetParent(NULL);
        this->HandShakeMenuButton->Delete();
    }

#endif


    if (this->UpdateRateEntry)
    {
        this->UpdateRateEntry->SetParent(NULL );
        this->UpdateRateEntry->Delete ( );
    }
    if (this->MultiFactorEntry)
    {
        this->MultiFactorEntry->SetParent(NULL );
        this->MultiFactorEntry->Delete ( );
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

    this->SetModuleLogic ( NULL );


    if (this->ExtraFrame)
    {
        this->ExtraFrame->Delete ( );
    }
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

    appGUI->GetMainSliceGUI0()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    appGUI->GetMainSliceGUI1()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    appGUI->GetMainSliceGUI2()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);


//#ifdef Slicer3_USE_NAVITRACK
//    this->OpenTrackerStream->RemoveObservers( vtkCommand::ModifiedEvent, this->DataCallbackCommand );
//    this->LoadConfigButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
//#endif


#ifdef Slicer3_USE_IGSTK
    this->IGSTKStream->RemoveObservers( vtkCommand::ModifiedEvent, this->DataCallbackCommand );
    this->DeviceMenuButton->GetWidget()->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
#endif

    if (this->ConnectCheckButton)
    {
        this->ConnectCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
    if (this->GetPatCoordinatesPushButton)
    {
        this->GetPatCoordinatesPushButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
    if (this->AddPointPairPushButton)
    {
        this->AddPointPairPushButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
    if (this->DeletePointPairPushButton)
    {
        this->DeletePointPairPushButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
    if (this->DeleteAllPointPairPushButton)
    {
        this->DeleteAllPointPairPushButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
    if (this->RegisterPushButton)
    {
        this->RegisterPushButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
    if (this->ResetPushButton)
    {
        this->ResetPushButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
    if (this->LocatorCheckButton)
    {
        this->LocatorCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
    if (this->LocatorModeCheckButton)
    {
        this->LocatorModeCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
    if (this->UserModeCheckButton)
    {
        this->UserModeCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkNeuroNavGUI::AddGUIObservers ( )
{
    this->RemoveGUIObservers();

    // make a user interactor style to process our events
    // look at the InteractorStyle to get our events

    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

    appGUI->GetMainSliceGUI0()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
    appGUI->GetMainSliceGUI1()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
    appGUI->GetMainSliceGUI2()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);


    // Fill in
    // observer load volume button
    this->ConnectCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->GetPatCoordinatesPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->AddPointPairPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DeletePointPairPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DeleteAllPointPairPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RegisterPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ResetPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->LocatorCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->LocatorModeCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->UserModeCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );


//#ifdef Slicer3_USE_NAVITRACK
//    this->OpenTrackerStream->AddObserver( vtkCommand::ModifiedEvent, this->DataCallbackCommand );
//    this->LoadConfigButton->GetWidget()->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
//#endif



#ifdef Slicer3_USE_IGSTK
    this->IGSTKStream->AddObserver( vtkCommand::ModifiedEvent, this->DataCallbackCommand );
    this->DeviceMenuButton->GetWidget()->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
#endif

}



void vtkNeuroNavGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
    vtkSlicerInteractorStyle *istyle0 = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI0()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
    vtkSlicerInteractorStyle *istyle1 = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI1()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
    vtkSlicerInteractorStyle *istyle2 = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI2()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());

    
    //
    //
    // So easy, a caveman can do it!
    //
    //



    vtkCornerAnnotation *anno = NULL;
    if (style == istyle0)
    {
        anno = appGUI->GetMainSliceGUI0()->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
    else if (style == istyle1)
    {
        anno = appGUI->GetMainSliceGUI1()->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
    else if (style == istyle2)
    {
        anno = appGUI->GetMainSliceGUI2()->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }

    if (anno)
    {
        const char *rasText = anno->GetText(1);
        if ( rasText != NULL )
          {
          std::string ras = std::string(rasText);

          // remove "R:," "A:," and "S:" from the string
          int loc = ras.find("R:", 0);
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
          int found = ras.find("\n", 0);
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
        unsigned long event, void *callData )
{
    const char *eventName = vtkCommand::GetStringFromEventId(event);
    if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
        vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
        HandleMouseEvent(style);
    }
    else
    {
        if (this->ConnectCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
                && event == vtkKWCheckButton::SelectedStateChangedEvent )
        {
//#ifdef Slicer3_USE_NAVITRACK
//            SetOpenTrackerConnectionParameters();
//#endif
#ifdef Slicer3_USE_IGSTK
            SetIGSTKConnectionParameters();
#endif
        }

/*
#ifdef Slicer3_USE_NAVITRACK
        else if (this->LoadConfigButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) 
                && event == vtkKWTopLevel::WithdrawEvent)
        {
            const char * filename = this->LoadConfigButton->GetWidget()->GetFileName();
            if (filename)
            {
                const vtksys_stl::string fname(filename);
                this->ConfigFileEntry->SetValue(fname.c_str());
            }
            else
            {
                this->ConfigFileEntry->SetValue("");
            }
            this->LoadConfigButton->GetWidget()->SetText ("Browse Config File");
        }  
#endif
*/

        else if (this->GetPatCoordinatesPushButton == vtkKWPushButton::SafeDownCast(caller) 
                && event == vtkKWPushButton::InvokedEvent)
        {
            float position[3];
            position[0] = position[1] = position[2] = 0.0;
            char value[50];
            if (this->LocatorMatrix)
            {
                for (int j = 0; j < 3; j++) 
                {
                    position[j] = this->LocatorMatrix->GetElement(j,0);
                    // position[j] = j / 1.0;
                }
            }
            sprintf(value, "%6.2f  %6.2f  %6.2f", position[0], position[1], position[2]);
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
                this->Pat2ImgReg->SetNumberOfPoints(row);
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
                    this->Pat2ImgReg->AddPoint(r, sc1, sc2, sc3, pc1, pc2, pc3);
                }

                int error = this->Pat2ImgReg->DoRegistration();
                if (error)
                {
                    vtkSlicerApplication::GetInstance()->ErrorMessage("Error registration between patient and image land marks.");
                    return;
                }

//#ifdef Slicer3_USE_NAVITRACK
//                this->OpenTrackerStream->SetRegMatrix(this->Pat2ImgReg->GetLandmarkTransformMatrix());
//#endif
#ifdef Slicer3_USE_IGSTK
                this->IGSTKStream->SetRegMatrix(this->Pat2ImgReg->GetLandmarkTransformMatrix());
#endif
            }
        }
        else if (this->ResetPushButton == vtkKWPushButton::SafeDownCast(caller) 
                && event == vtkKWPushButton::InvokedEvent)
        {
//#ifdef Slicer3_USE_NAVITRACK
//            this->OpenTrackerStream->SetRegMatrix(NULL);
//#endif
#ifdef Slicer3_USE_IGSTK
            this->IGSTKStream->SetRegMatrix(NULL);
#endif
        }
        else if (this->LocatorCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
                && event == vtkKWCheckButton::SelectedStateChangedEvent )
        {
            int checked = this->LocatorCheckButton->GetSelectedState(); 

            // vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->LocatorModelID.c_str())); 
            vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLModelNode1")); 
            if (model != NULL)
            {
                vtkMRMLModelDisplayNode *disp = model->GetModelDisplayNode();

                vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
                vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
                disp->SetColor(color->SliceGUIGreen);
                disp->SetVisibility(checked);
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
                this->SliceNode0->SetOrientationToAxial();
                this->SliceNode1->SetOrientationToSagittal();
                this->SliceNode2->SetOrientationToCoronal();
                this->NeedOrientationUpdate0 = 0;
                this->NeedOrientationUpdate1 = 0;
                this->NeedOrientationUpdate2 = 0;

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
                this->SliceNode0->SetOrientationToAxial();
                this->SliceNode1->SetOrientationToSagittal();
                this->SliceNode2->SetOrientationToCoronal();

                this->NeedOrientationUpdate0 = 0;
                this->NeedOrientationUpdate1 = 0;
                this->NeedOrientationUpdate2 = 0;
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
    }
} 



void vtkNeuroNavGUI::Init()
{
    this->DataManager->SetMRMLScene(this->GetMRMLScene());
    this->LocatorModelID = std::string(this->DataManager->RegisterStream(0));
}



void vtkNeuroNavGUI::DataCallback(vtkObject *caller, 
            unsigned long eid, void *clientData, void *callData)
{
    vtkNeuroNavGUI *self = reinterpret_cast<vtkNeuroNavGUI *>(clientData);
    vtkDebugWithObjectMacro(self, "In vtkNeuroNavGUI DataCallback");

    self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkNeuroNavGUI::ProcessLogicEvents ( vtkObject *caller,
        unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkNeuroNavGUI::ProcessMRMLEvents ( vtkObject *caller,
        unsigned long event, void *callData )
{
    // Fill in
}



//---------------------------------------------------------------------------
void vtkNeuroNavGUI::Enter ( )
{
    // Fill in
    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

    this->Logic0 = appGUI->GetMainSliceGUI0()->GetLogic();
    this->Logic1 = appGUI->GetMainSliceGUI1()->GetLogic();
    this->Logic2 = appGUI->GetMainSliceGUI2()->GetLogic();
    this->SliceNode0 = appGUI->GetMainSliceGUI0()->GetLogic()->GetSliceNode();
    this->SliceNode1 = appGUI->GetMainSliceGUI1()->GetLogic()->GetSliceNode();
    this->SliceNode2 = appGUI->GetMainSliceGUI2()->GetLogic()->GetSliceNode();
    this->Control0 = appGUI->GetMainSliceGUI0()->GetSliceController();
    this->Control1 = appGUI->GetMainSliceGUI1()->GetSliceController();
    this->Control2 = appGUI->GetMainSliceGUI2()->GetSliceController();

}

//---------------------------------------------------------------------------
void vtkNeuroNavGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkNeuroNavGUI::BuildGUI ( )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    // Define your help text here.
    const char *help = "The **NeuroNav Module** connects Slicer to a IGSTK-supported device that feeds a real-time stream of coordinates from a tracking device. This is useful in the operating room to allow the surgeons to navigate the preloaded volume(s) by pointing to a location with a probe that is tracked."; 

    // ---
    // MODULE GUI FRAME 
    // create a page
    this->UIPanel->AddPage ( "NeuroNav", "NeuroNav", NULL );

    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "NeuroNav" );


    // ----------------------------------------------------------------
    // HELP FRAME            
    // ----------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *NeuroNavHelpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    NeuroNavHelpFrame->SetParent ( page );
    NeuroNavHelpFrame->Create ( );
    NeuroNavHelpFrame->CollapseFrame ( );
    NeuroNavHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
            NeuroNavHelpFrame->GetWidgetName(), page->GetWidgetName());

    // configure the parent classes help text widget
    this->HelpText->SetParent ( NeuroNavHelpFrame->GetFrame() );
    this->HelpText->Create ( );
    this->HelpText->SetHorizontalScrollbarVisibility ( 0 );
    this->HelpText->SetVerticalScrollbarVisibility ( 1 );
    this->HelpText->GetWidget()->SetText ( help );
    this->HelpText->GetWidget()->SetReliefToFlat ( );
    this->HelpText->GetWidget()->SetWrapToWord ( );
    this->HelpText->GetWidget()->ReadOnlyOn ( );
    this->HelpText->GetWidget()->QuickFormattingOn ( );
    this->HelpText->GetWidget()->SetBalloonHelpString ( "" );
    app->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
            this->HelpText->GetWidgetName ( ) );

    NeuroNavHelpFrame->Delete();

    BuildGUIForDeviceFrame ();
    BuildGUIForRegistrationFrame ();
    BuildGUIForTrackingFrame ();
    BuildGUIForHandPieceFrame ();
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


void vtkNeuroNavGUI::BuildGUIForDeviceFrame ()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "NeuroNav" );

    // ----------------------------------------------------------------
    // DEVICE FRAME            
    // ----------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *deviceFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    deviceFrame->SetParent ( page );
    deviceFrame->Create ( );
    deviceFrame->SetLabelText ("Tracking Device");
    deviceFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
            deviceFrame->GetWidgetName(), page->GetWidgetName());

    /////////////////////////////////////////////////////////////////////
    /// Interface frame 
    /////////////////////////////////////////////////////////////////////

    vtkKWFrame *interfaceFrame = vtkKWFrame::New();
    interfaceFrame->SetParent ( deviceFrame->GetFrame() );
    interfaceFrame->Create ( );
    this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                  interfaceFrame->GetWidgetName());

    // interface: opentracker or igstk 
    vtkKWLabel *nameLabel = vtkKWLabel::New();
    nameLabel->SetParent(interfaceFrame);
    nameLabel->Create();
    nameLabel->SetWidth(8);
    nameLabel->SetText("Interface: ");

    vtkKWLabel *valueLabel = vtkKWLabel::New();
    valueLabel->SetParent(interfaceFrame);
    valueLabel->Create();
    valueLabel->SetWidth(21);
    valueLabel->SetText("None        ");
//#ifdef Slicer3_USE_NAVITRACK
//    valueLabel->SetText("NaviTrack");
//#endif
#ifdef Slicer3_USE_IGSTK
    valueLabel->SetText("IGSTK       ");
#endif

    this->Script(
            "pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2", 
            nameLabel->GetWidgetName(),
            valueLabel->GetWidgetName());


    /////////////////////////////////////////////////////////////////////
    /// Update rate frame 
    /////////////////////////////////////////////////////////////////////

    vtkKWFrame *rateFrame = vtkKWFrame::New();
    rateFrame->SetParent ( deviceFrame->GetFrame() );
    rateFrame->Create ( );
    this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                  rateFrame->GetWidgetName());

    this->UpdateRateEntry = vtkKWEntryWithLabel::New();
    this->UpdateRateEntry->SetParent(rateFrame);
    this->UpdateRateEntry->Create();
    this->UpdateRateEntry->SetWidth(25);
    this->UpdateRateEntry->SetLabelWidth(15);
    this->UpdateRateEntry->SetLabelText("Pulling Rate (ms)");
    this->UpdateRateEntry->GetWidget()->SetValue ( "100" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
      this->UpdateRateEntry->GetWidgetName());


    /////////////////////////////////////////////////////////////////////
    /// Multi frame 
    /////////////////////////////////////////////////////////////////////

    vtkKWFrame *multiFrame = vtkKWFrame::New();
    multiFrame->SetParent ( deviceFrame->GetFrame() );
    multiFrame->Create ( );
    this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                  multiFrame->GetWidgetName());

    // Multi factor
    this->MultiFactorEntry = vtkKWEntryWithLabel::New();
    this->MultiFactorEntry->SetParent(multiFrame);
    this->MultiFactorEntry->Create();
    this->MultiFactorEntry->SetWidth(20);
    this->MultiFactorEntry->SetLabelWidth(15);
    this->MultiFactorEntry->SetLabelText("Conversion Rate:");
    this->MultiFactorEntry->GetWidget()->SetValue ( "1.0" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
      this->MultiFactorEntry->GetWidgetName());


    /////////////////////////////////////////////////////////////////////
    /// Config file frame
    /////////////////////////////////////////////////////////////////////
    // add a file browser 
    this->ExtraFrame = vtkKWFrame::New();
    this->ExtraFrame->SetParent ( deviceFrame->GetFrame() );
    this->ExtraFrame->Create ( );
    this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                  this->ExtraFrame->GetWidgetName());
/*

#ifdef Slicer3_USE_NAVITRACK
    this->ConfigFileEntry = vtkKWEntry::New();
    this->ConfigFileEntry->SetParent(this->ExtraFrame);
    this->ConfigFileEntry->Create();
    this->ConfigFileEntry->SetWidth(50);
    this->ConfigFileEntry->SetValue ( "" );

    this->LoadConfigButton = vtkKWLoadSaveButtonWithLabel::New ( );
    this->LoadConfigButton->SetParent ( this->ExtraFrame );
    this->LoadConfigButton->Create ( );
    this->LoadConfigButton->SetWidth(15);
    this->LoadConfigButton->GetWidget()->SetText ("Browse Config File");
    this->LoadConfigButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
                                                              "{ {NeuroNav} {*.xml} }");
    this->LoadConfigButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
      "OpenPath");

    this->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
                this->LoadConfigButton->GetWidgetName(),
                this->ConfigFileEntry->GetWidgetName());
#endif
*/

#ifdef Slicer3_USE_IGSTK
    this->DeviceMenuButton = vtkKWMenuButtonWithLabel::New();
    this->DeviceMenuButton->SetParent(this->ExtraFrame);
    this->DeviceMenuButton->Create();
    this->DeviceMenuButton->SetWidth(50);
    this->DeviceMenuButton->SetLabelWidth(12);
    this->DeviceMenuButton->SetLabelText("Device Type:");
    this->DeviceMenuButton->GetWidget()->GetMenu()->AddRadioButton("Aurora");
    this->DeviceMenuButton->GetWidget()->GetMenu()->AddRadioButton("Polaris");
    this->DeviceMenuButton->GetWidget()->SetValue ("Polaris");
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->DeviceMenuButton->GetWidgetName());


    // Port numbers
    this->PortNumberMenuButton = vtkKWMenuButtonWithLabel::New();
    this->PortNumberMenuButton->SetParent(this->ExtraFrame);
    this->PortNumberMenuButton->Create();
    this->PortNumberMenuButton->SetWidth(50);
    this->PortNumberMenuButton->SetLabelWidth(12);
    this->PortNumberMenuButton->SetLabelText("Port Number:");
    this->PortNumberMenuButton->GetWidget()->GetMenu()->AddRadioButton("0");
    this->PortNumberMenuButton->GetWidget()->GetMenu()->AddRadioButton("1");
    this->PortNumberMenuButton->GetWidget()->GetMenu()->AddRadioButton("2");
    this->PortNumberMenuButton->GetWidget()->GetMenu()->AddRadioButton("3");
    this->PortNumberMenuButton->GetWidget()->GetMenu()->AddRadioButton("4");
    this->PortNumberMenuButton->GetWidget()->GetMenu()->AddRadioButton("5");
    this->PortNumberMenuButton->GetWidget()->GetMenu()->AddRadioButton("6");
    this->PortNumberMenuButton->GetWidget()->GetMenu()->AddRadioButton("7");
 
    this->PortNumberMenuButton->GetWidget()->SetValue ("0");
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->PortNumberMenuButton->GetWidgetName());


    // Baud rates
    this->BaudRateMenuButton = vtkKWMenuButtonWithLabel::New();
    this->BaudRateMenuButton->SetParent(this->ExtraFrame);
    this->BaudRateMenuButton->Create();
    this->BaudRateMenuButton->SetWidth(50);
    this->BaudRateMenuButton->SetLabelWidth(12);
    this->BaudRateMenuButton->SetLabelText("Baud Rate:");
    this->BaudRateMenuButton->GetWidget()->GetMenu()->AddRadioButton("9600");
    this->BaudRateMenuButton->GetWidget()->GetMenu()->AddRadioButton("19200");
    this->BaudRateMenuButton->GetWidget()->GetMenu()->AddRadioButton("38400");
    this->BaudRateMenuButton->GetWidget()->GetMenu()->AddRadioButton("57600");
    this->BaudRateMenuButton->GetWidget()->GetMenu()->AddRadioButton("115200");
 
    this->BaudRateMenuButton->GetWidget()->SetValue ("9600");
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->BaudRateMenuButton->GetWidgetName());


    // Data bits 
    this->DataBitsMenuButton = vtkKWMenuButtonWithLabel::New();
    this->DataBitsMenuButton->SetParent(this->ExtraFrame);
    this->DataBitsMenuButton->Create();
    this->DataBitsMenuButton->SetWidth(50);
    this->DataBitsMenuButton->SetLabelWidth(12);
    this->DataBitsMenuButton->SetLabelText("Data Bits:");
    this->DataBitsMenuButton->GetWidget()->GetMenu()->AddRadioButton("7");
    this->DataBitsMenuButton->GetWidget()->GetMenu()->AddRadioButton("8");
 
    this->DataBitsMenuButton->GetWidget()->SetValue ("8");
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->DataBitsMenuButton->GetWidgetName());


    // Parity 
    this->ParityTypeMenuButton = vtkKWMenuButtonWithLabel::New();
    this->ParityTypeMenuButton->SetParent(this->ExtraFrame);
    this->ParityTypeMenuButton->Create();
    this->ParityTypeMenuButton->SetWidth(50);
    this->ParityTypeMenuButton->SetLabelWidth(12);
    this->ParityTypeMenuButton->SetLabelText("Parity Type:");
    this->ParityTypeMenuButton->GetWidget()->GetMenu()->AddRadioButton("No");
    this->ParityTypeMenuButton->GetWidget()->GetMenu()->AddRadioButton("Odd");
    this->ParityTypeMenuButton->GetWidget()->GetMenu()->AddRadioButton("Even");
 
    this->ParityTypeMenuButton->GetWidget()->SetValue ("No");
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->ParityTypeMenuButton->GetWidgetName());


    // Stop bits 
    this->StopBitsMenuButton = vtkKWMenuButtonWithLabel::New();
    this->StopBitsMenuButton->SetParent(this->ExtraFrame);
    this->StopBitsMenuButton->Create();
    this->StopBitsMenuButton->SetWidth(50);
    this->StopBitsMenuButton->SetLabelWidth(12);
    this->StopBitsMenuButton->SetLabelText("Stop Bits:");
    this->StopBitsMenuButton->GetWidget()->GetMenu()->AddRadioButton("1");
    this->StopBitsMenuButton->GetWidget()->GetMenu()->AddRadioButton("2");

 
    this->StopBitsMenuButton->GetWidget()->SetValue ("1");
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->StopBitsMenuButton->GetWidgetName());


    // Hand shake 
    this->HandShakeMenuButton = vtkKWMenuButtonWithLabel::New();
    this->HandShakeMenuButton->SetParent(this->ExtraFrame);
    this->HandShakeMenuButton->Create();
    this->HandShakeMenuButton->SetWidth(50);
    this->HandShakeMenuButton->SetLabelWidth(12);
    this->HandShakeMenuButton->SetLabelText("Hand Shake:");
    this->HandShakeMenuButton->GetWidget()->GetMenu()->AddRadioButton("Off");
    this->HandShakeMenuButton->GetWidget()->GetMenu()->AddRadioButton("On");

 
    this->HandShakeMenuButton->GetWidget()->SetValue ("Off");
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->HandShakeMenuButton->GetWidgetName());

#endif


    /////////////////////////////////////////////////////////////////////
    /// Connnect button 
    /////////////////////////////////////////////////////////////////////

    this->ConnectCheckButton = vtkKWCheckButton::New();
    this->ConnectCheckButton->SetParent(deviceFrame->GetFrame());
    this->ConnectCheckButton->Create();
    this->ConnectCheckButton->SelectedStateOff();
    this->ConnectCheckButton->SetText("Connect");

    this->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
                this->ConnectCheckButton->GetWidgetName());


    nameLabel->Delete();
    valueLabel->Delete();
    interfaceFrame->Delete();
    multiFrame->Delete();
    rateFrame->Delete ();
    deviceFrame->Delete ();
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


    this->Script("pack %s %s -side left -anchor w -padx 2 -pady 2", 
                this->LocatorModeCheckButton->GetWidgetName(),
                this->UserModeCheckButton->GetWidgetName());


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
    driverFrame->Delete();
    modeFrame->Delete();
    sliceFrame->Delete();
}



void vtkNeuroNavGUI::BuildGUIForHandPieceFrame ()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "NeuroNav" );

    // ----------------------------------------------------------------
    // HANDPIECE FRAME            
    // ----------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *handPieceFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    handPieceFrame->SetParent ( page );
    handPieceFrame->Create ( );
    handPieceFrame->SetLabelText ("Handpiece");
    handPieceFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
            handPieceFrame->GetWidgetName(), page->GetWidgetName());

    // Offset frame: Offsets from the tip 
    // -----------------------------------------
    vtkKWFrameWithLabel *offsetFrame = vtkKWFrameWithLabel::New ( );
    offsetFrame->SetParent ( handPieceFrame->GetFrame() );
    offsetFrame->Create ( );
    offsetFrame->SetLabelText ("Offset from tip");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   offsetFrame->GetWidgetName() );

    this->NormalOffsetEntry = vtkKWEntryWithLabel::New();
    this->NormalOffsetEntry->SetParent(offsetFrame->GetFrame());
    this->NormalOffsetEntry->Create();
    this->NormalOffsetEntry->SetWidth(20);
    this->NormalOffsetEntry->SetLabelWidth(12);
    this->NormalOffsetEntry->SetLabelText("Normal:");
    this->NormalOffsetEntry->GetWidget()->SetValue ( "0" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
      this->NormalOffsetEntry->GetWidgetName());


    this->TransOffsetEntry = vtkKWEntryWithLabel::New();
    this->TransOffsetEntry->SetParent(offsetFrame->GetFrame());
    this->TransOffsetEntry->Create();
    this->TransOffsetEntry->SetWidth(20);
    this->TransOffsetEntry->SetLabelWidth(12);
    this->TransOffsetEntry->SetLabelText("Trans:");
    this->TransOffsetEntry->GetWidget()->SetValue ( "0" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
      this->TransOffsetEntry->GetWidgetName());


    this->NXTOffsetEntry = vtkKWEntryWithLabel::New();
    this->NXTOffsetEntry->SetParent(offsetFrame->GetFrame());
    this->NXTOffsetEntry->Create();
    this->NXTOffsetEntry->SetWidth(20);
    this->NXTOffsetEntry->SetLabelWidth(12);
    this->NXTOffsetEntry->SetLabelText("N x T:");
    this->NXTOffsetEntry->GetWidget()->SetValue ( "0" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
      this->NXTOffsetEntry->GetWidgetName());


    // Size frame: Locator size 
    // -----------------------------------------
    vtkKWFrameWithLabel *sizeFrame = vtkKWFrameWithLabel::New ( );
    sizeFrame->SetParent ( handPieceFrame->GetFrame() );
    sizeFrame->Create ( );
    sizeFrame->SetLabelText ("Size (mm)");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   sizeFrame->GetWidgetName() );

    this->NormalSizeEntry = vtkKWEntryWithLabel::New();
    this->NormalSizeEntry->SetParent(sizeFrame->GetFrame());
    this->NormalSizeEntry->Create();
    this->NormalSizeEntry->SetWidth(20);
    this->NormalSizeEntry->SetLabelWidth(12);
    this->NormalSizeEntry->SetLabelText("Normal:");
    this->NormalSizeEntry->GetWidget()->SetValue ( "100" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
      this->NormalSizeEntry->GetWidgetName());


    this->TransSizeEntry = vtkKWEntryWithLabel::New();
    this->TransSizeEntry->SetParent(sizeFrame->GetFrame());
    this->TransSizeEntry->Create();
    this->TransSizeEntry->SetWidth(20);
    this->TransSizeEntry->SetLabelWidth(12);
    this->TransSizeEntry->SetLabelText("Trans:");
    this->TransSizeEntry->GetWidget()->SetValue ( "25" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
      this->TransSizeEntry->GetWidgetName());


    this->RadiusEntry = vtkKWEntryWithLabel::New();
    this->RadiusEntry->SetParent(sizeFrame->GetFrame());
    this->RadiusEntry->Create();
    this->RadiusEntry->SetWidth(20);
    this->RadiusEntry->SetLabelWidth(12);
    this->RadiusEntry->SetLabelText("Radius:");
    this->RadiusEntry->GetWidget()->SetValue ( "1.5" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
      this->RadiusEntry->GetWidgetName());

    // Position frame: Locator position and orientation 
    // -----------------------------------------
    vtkKWFrameWithLabel *positionFrame = vtkKWFrameWithLabel::New ( );
    positionFrame->SetParent ( handPieceFrame->GetFrame() );
    positionFrame->Create ( );
    positionFrame->SetLabelText ("Position and Orientation");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   positionFrame->GetWidgetName() );

    // Header frame
    vtkKWFrame *headerFrame = vtkKWFrame::New();
    headerFrame->SetParent ( positionFrame->GetFrame() );
    headerFrame->Create ( );
    app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
                 headerFrame->GetWidgetName(),
                 positionFrame->GetFrame()->GetWidgetName());
    // N frame
    vtkKWFrame *nFrame = vtkKWFrame::New();
    nFrame->SetParent ( positionFrame->GetFrame() );
    nFrame->Create ( );
    app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
                 nFrame->GetWidgetName(),
                 positionFrame->GetFrame()->GetWidgetName());
    // T frame
    vtkKWFrame *tFrame = vtkKWFrame::New();
    tFrame->SetParent ( positionFrame->GetFrame() );
    tFrame->Create ( );
    app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
                 tFrame->GetWidgetName(),
                 positionFrame->GetFrame()->GetWidgetName());
    // P frame
    vtkKWFrame *pFrame = vtkKWFrame::New();
    pFrame->SetParent ( positionFrame->GetFrame() );
    pFrame->Create ( );
    app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
                 pFrame->GetWidgetName(),
                 positionFrame->GetFrame()->GetWidgetName());


    // Contents in header frame
    vtkKWLabel *emptyLabel = vtkKWLabel::New();
    emptyLabel->SetParent(headerFrame);
    emptyLabel->Create();
    emptyLabel->SetWidth(1);
    emptyLabel->SetText("");

    vtkKWLabel *rLabel = vtkKWLabel::New();
    rLabel->SetParent(headerFrame);
    rLabel->Create();
    rLabel->SetWidth(13);
    rLabel->SetText("R");

    vtkKWLabel *aLabel = vtkKWLabel::New();
    aLabel->SetParent(headerFrame);
    aLabel->Create();
    aLabel->SetWidth(13);
    aLabel->SetText("A");

    vtkKWLabel *sLabel = vtkKWLabel::New();
    sLabel->SetParent(headerFrame);
    sLabel->Create();
    sLabel->SetWidth(13);
    sLabel->SetText("S");

    this->Script("pack %s %s %s %s -side left -anchor w -padx 2 -pady 2", 
                emptyLabel->GetWidgetName(),
                rLabel->GetWidgetName(),
                aLabel->GetWidgetName(),
                sLabel->GetWidgetName());

    // Contents in N frame 
    vtkKWLabel *nLabel = vtkKWLabel::New();
    nLabel->SetParent(nFrame);
    nLabel->Create();
    nLabel->SetWidth(1);
    nLabel->SetText("N:");

    this->NREntry = vtkKWEntry::New();
    NREntry->SetParent(nFrame);
    NREntry->Create();
    NREntry->SetWidth(13);
    NREntry->SetValue("0.00");

    this->NAEntry = vtkKWEntry::New();
    NAEntry->SetParent(nFrame);
    NAEntry->Create();
    NAEntry->SetWidth(13);
    NAEntry->SetValue("-1.00");

    this->NSEntry = vtkKWEntry::New();
    NSEntry->SetParent(nFrame);
    NSEntry->Create();
    NSEntry->SetWidth(13);
    NSEntry->SetValue("0.00");

    this->Script("pack %s %s %s %s -side left -anchor w -padx 2 -pady 2", 
                nLabel->GetWidgetName(),
                this->NREntry->GetWidgetName(),
                this->NAEntry->GetWidgetName(),
                this->NSEntry->GetWidgetName());

    // Contents in T frame
    vtkKWLabel *tLabel = vtkKWLabel::New();
    tLabel->SetParent(tFrame);
    tLabel->Create();
    tLabel->SetWidth(1);
    tLabel->SetText("T:");

    this->TREntry = vtkKWEntry::New();
    TREntry->SetParent(tFrame);
    TREntry->Create();
    TREntry->SetWidth(13);
    TREntry->SetValue("1.00");

    this->TAEntry = vtkKWEntry::New();
    TAEntry->SetParent(tFrame);
    TAEntry->Create();
    TAEntry->SetWidth(13);
    TAEntry->SetValue("0.00");

    this->TSEntry = vtkKWEntry::New();
    TSEntry->SetParent(tFrame);
    TSEntry->Create();
    TSEntry->SetWidth(13);
    TSEntry->SetValue("0.00");

    this->Script("pack %s %s %s %s -side left -anchor w -padx 2 -pady 2", 
                tLabel->GetWidgetName(),
                this->TREntry->GetWidgetName(),
                this->TAEntry->GetWidgetName(),
                this->TSEntry->GetWidgetName());

    // Contents in P frame
    vtkKWLabel *pLabel = vtkKWLabel::New();
    pLabel->SetParent(pFrame);
    pLabel->Create();
    pLabel->SetWidth(1);
    pLabel->SetText("P:");

    this->PREntry = vtkKWEntry::New();
    PREntry->SetParent(pFrame);
    PREntry->Create();
    PREntry->SetWidth(13);
    PREntry->SetValue("0.00");

    this->PAEntry = vtkKWEntry::New();
    PAEntry->SetParent(pFrame);
    PAEntry->Create();
    PAEntry->SetWidth(13);
    PAEntry->SetValue("0.00");

    this->PSEntry = vtkKWEntry::New();
    PSEntry->SetParent(pFrame);
    PSEntry->Create();
    PSEntry->SetWidth(13);
    PSEntry->SetValue("0.00");

    this->Script("pack %s %s %s %s -side left -anchor w -padx 2 -pady 2", 
                pLabel->GetWidgetName(),
                this->PREntry->GetWidgetName(),
                this->PAEntry->GetWidgetName(),
                this->PSEntry->GetWidgetName());

/*

    // Color frame: Locator color 
    // -----------------------------------------
    vtkKWFrameWithLabel *colorFrame = vtkKWFrameWithLabel::New ( );
    colorFrame->SetParent ( handPieceFrame->GetFrame() );
    colorFrame->Create ( );
    colorFrame->SetLabelText ("Color");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   colorFrame->GetWidgetName() );

    this->RedColorScale =  vtkKWScaleWithEntry::New() ;
    this->RedColorScale->SetParent( colorFrame->GetFrame() );
    this->RedColorScale->Create();
    this->RedColorScale->SetRange(0.0, 1.0);
    this->RedColorScale->SetResolution(0.1);
    this->RedColorScale->SetValue(0.0);
    this->RedColorScale->SetLabelText("Red:   ");
    this->RedColorScale->SetWidth ( 20 );
    this->RedColorScale->SetStartCommand(this, "TransformChangingCallback");
    this->RedColorScale->SetCommand(this, "TransformChangingCallback");
    this->RedColorScale->SetEndCommand(this, "TransformChangedCallback");
    this->RedColorScale->SetEntryCommand(this, "TransformChangedCallback");
    this->Script("pack %s -side top -anchor w -padx 0 -pady 2", 
                  this->RedColorScale->GetWidgetName());

    this->GreenColorScale =  vtkKWScaleWithEntry::New() ;
    this->GreenColorScale->SetParent( colorFrame->GetFrame() );
    this->GreenColorScale->Create();
    this->GreenColorScale->SetRange(0.0, 1.0);
    this->GreenColorScale->SetResolution(0.1);
    this->GreenColorScale->SetValue(0.9);
    this->GreenColorScale->SetLabelText("Green:");
    this->GreenColorScale->SetWidth ( 20 );
    this->GreenColorScale->SetStartCommand(this, "TransformChangingCallback");
    this->GreenColorScale->SetCommand(this, "TransformChangingCallback");
    this->GreenColorScale->SetEndCommand(this, "TransformChangedCallback");
    this->GreenColorScale->SetEntryCommand(this, "TransformChangedCallback");
    this->Script("pack %s -side top -anchor w -padx 0 -pady 2", 
                  this->GreenColorScale->GetWidgetName());

    this->BlueColorScale =  vtkKWScaleWithEntry::New() ;
    this->BlueColorScale->SetParent( colorFrame->GetFrame() );
    this->BlueColorScale->Create();
    this->BlueColorScale->SetRange(0.0, 1.0);
    this->BlueColorScale->SetResolution(0.1);
    this->BlueColorScale->SetValue(0.0);
    this->BlueColorScale->SetLabelText("Blue:   ");
    this->BlueColorScale->SetWidth ( 20 );
    this->BlueColorScale->SetStartCommand(this, "TransformChangingCallback");
    this->BlueColorScale->SetCommand(this, "TransformChangingCallback");
    this->BlueColorScale->SetEndCommand(this, "TransformChangedCallback");
    this->BlueColorScale->SetEntryCommand(this, "TransformChangedCallback");
    this->Script("pack %s -side top -anchor w -padx 0 -pady 2", 
                  this->BlueColorScale->GetWidgetName());
*/

    pLabel->Delete();
    nLabel->Delete();
    emptyLabel->Delete();
    rLabel->Delete();
    aLabel->Delete();
    sLabel->Delete();
    tLabel->Delete();

    handPieceFrame->Delete ();
    offsetFrame->Delete ();
    sizeFrame->Delete ();
    positionFrame->Delete ();
    headerFrame->Delete ();
    nFrame->Delete ();
    tFrame->Delete ();
    pFrame->Delete ();
    // colorFrame->Delete ();

}


void vtkNeuroNavGUI::UpdateAll()
{
    this->LocatorMatrix = NULL;
//#ifdef Slicer3_USE_NAVITRACK
//    this->LocatorMatrix = this->OpenTrackerStream->GetLocatorMatrix();
//#endif
#ifdef Slicer3_USE_IGSTK
    this->LocatorMatrix = this->IGSTKStream->GetLocatorMatrix();
#endif

    if (this->LocatorMatrix)
    {
        char Val[10];

        float px = this->LocatorMatrix->GetElement(0, 0);
        float py = this->LocatorMatrix->GetElement(1, 0);
        float pz = this->LocatorMatrix->GetElement(2, 0);
        float nx = this->LocatorMatrix->GetElement(0, 1);
        float ny = this->LocatorMatrix->GetElement(1, 1);
        float nz = this->LocatorMatrix->GetElement(2, 1);
        float tx = this->LocatorMatrix->GetElement(0, 2);
        float ty = this->LocatorMatrix->GetElement(1, 2);
        float tz = this->LocatorMatrix->GetElement(2, 2);

        sprintf(Val, "%6.2f", px);
        this->PREntry->SetValue(Val);
        sprintf(Val, "%6.2f", py);
        this->PAEntry->SetValue(Val);
        sprintf(Val, "%6.2f", pz);
        this->PSEntry->SetValue(Val);

        sprintf(Val, "%6.2f", nx);
        this->NREntry->SetValue(Val);
        sprintf(Val, "%6.2f", ny);
        this->NAEntry->SetValue(Val);
        sprintf(Val, "%6.2f", nz);
        this->NSEntry->SetValue(Val);

        sprintf(Val, "%6.2f", tx);
        this->TREntry->SetValue(Val);
        sprintf(Val, "%6.2f", ty);
        this->TAEntry->SetValue(Val);
        sprintf(Val, "%6.2f", tz);
        this->TSEntry->SetValue(Val);


        // update the display of locator
        if (this->LocatorModeCheckButton->GetSelectedState()) this->UpdateLocator();

        //this->UpdateSliceDisplay(px, py, pz);  // RSierra 3/9/07: This line is redundant. If you remove it the slice views are still updated.
        this->UpdateSliceDisplay(nx, ny, nz, tx, ty, tz, px, py, pz);
    }
}


void vtkNeuroNavGUI::UpdateLocator()
{
    vtkTransform *transform = NULL;
//#ifdef Slicer3_USE_NAVITRACK
//    this->OpenTrackerStream->SetLocatorTransforms();
//    transform = this->OpenTrackerStream->GetLocatorNormalTransform(); 
//#endif
#ifdef Slicer3_USE_IGSTK
    this->IGSTKStream->SetLocatorTransforms();
    transform = this->IGSTKStream->GetLocatorNormalTransform(); 
#endif

    //vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->LocatorModelID.c_str())); 
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLModelNode1")); 
    if (model != NULL)
    {
        if (transform)
        {
            vtkMRMLLinearTransformNode *lnode = (vtkMRMLLinearTransformNode *)model->GetParentTransformNode();
            lnode->SetAndObserveMatrixTransformToParent(transform->GetMatrix());
            this->GetMRMLScene()->Modified();
        }
    }
}



void vtkNeuroNavGUI::UpdateSliceDisplay(float nx, float ny, float nz, 
                                        float tx, float ty, float tz, 
                                        float px, float py, float pz)
{
    // Axial
    if (strcmp(this->RedSliceMenu->GetValue(), "Locator"))
    {
        if (this->NeedOrientationUpdate0) 
        {
            this->SliceNode0->SetOrientationToAxial();
            this->NeedOrientationUpdate0 = 0;
        }
    }
    else
    {
        this->SliceNode0->SetSliceToRASByNTP( nx, ny, nz, tx, ty, tz, px, py, pz, 0);
        this->Control0->GetOffsetScale()->SetValue(pz);
        this->Logic0->SetSliceOffset(pz);
        this->NeedOrientationUpdate0 = 1;
    }

    // Sagittal
    if (strcmp(this->YellowSliceMenu->GetValue(), "Locator"))
    {
        if (this->NeedOrientationUpdate1) 
        {
            this->SliceNode1->SetOrientationToSagittal();
            this->NeedOrientationUpdate1 = 0;
        }
    }
    else
    {
        this->SliceNode1->SetSliceToRASByNTP( nx, ny, nz, tx, ty, tz, px, py, pz, 1);
        this->Control1->GetOffsetScale()->SetValue(px);
        this->Logic1->SetSliceOffset(px);
        this->NeedOrientationUpdate1 = 1;
    }

    // Coronal
    if (strcmp(this->GreenSliceMenu->GetValue(), "Locator"))
    {
        if (this->NeedOrientationUpdate2) 
        {
            this->SliceNode2->SetOrientationToCoronal();
            this->NeedOrientationUpdate2 = 0;
        }
    }
    else
    {
        this->SliceNode2->SetSliceToRASByNTP( nx, ny, nz, tx, ty, tz, px, py, pz, 2);
        this->Control2->GetOffsetScale()->SetValue(py);
        this->Logic2->SetSliceOffset(py);
        this->NeedOrientationUpdate2 = 1;
    }
}



/*
#ifdef Slicer3_USE_NAVITRACK
void vtkNeuroNavGUI::SetOpenTrackerConnectionParameters()
{
    int checked = this->ConnectCheckButton->GetSelectedState(); 
    if (checked)
    {
        // connected
        const char *filename = this->LoadConfigButton->GetWidget()->GetFileName();
        if (! filename)
        {
            vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
            dialog->SetParent ( this->ExtraFrame );
            dialog->SetStyleToMessage();
            std::string msg = std::string("Please input a valid configuration file (.xml).");
            dialog->SetText(msg.c_str());
            dialog->Create();
            dialog->Invoke();
            dialog->Delete();
            this->ConnectCheckButton->SetSelectedState(0);
        }
        else
        {
            this->OpenTrackerStream->Init(filename);

            int sp = atoi(this->UpdateRateEntry->GetWidget()->GetValue());
            float multi = atof(this->MultiFactorEntry->GetWidget()->GetValue());

            this->OpenTrackerStream->SetSpeed(sp);
            this->OpenTrackerStream->SetMultiFactor(multi);
            this->OpenTrackerStream->SetTracking(1);
            this->OpenTrackerStream->ProcessTimerEvents();
        }
    }
    else
    {
        this->OpenTrackerStream->SetTracking(0);
    }
}
#endif

*/

#ifdef Slicer3_USE_IGSTK
void vtkNeuroNavGUI::SetIGSTKConnectionParameters()
{
    int checked = this->ConnectCheckButton->GetSelectedState(); 
    if (checked)
    {

        // Pulling rate for data
        int sp = atoi(this->UpdateRateEntry->GetWidget()->GetValue());
        this->IGSTKStream->SetSpeed(sp);

        // Conversion rate
        float multi = atof(this->MultiFactorEntry->GetWidget()->GetValue());
        this->IGSTKStream->SetMultiFactor(multi);


        // Device type 
        vtkKWMenuButton *mb = this->DeviceMenuButton->GetWidget();
        if (!strcmp (mb->GetValue(), "Polaris"))   
        {
            this->IGSTKStream->SetTrackerType(0);
        }
        else 
        {
            this->IGSTKStream->SetTrackerType(1);

        }


        // Port number
        int pn = atoi(this->PortNumberMenuButton->GetWidget()->GetValue());
        this->IGSTKStream->SetPortNumber((PortNumberT)pn);

        // Baud rate 
        int br = atoi(this->BaudRateMenuButton->GetWidget()->GetValue());
        this->IGSTKStream->SetBaudRate((BaudRateT)br);

        // Data bits 
        int db = atoi(this->DataBitsMenuButton->GetWidget()->GetValue());
        this->IGSTKStream->SetBaudRate((BaudRateT)db);

        // Parity 
        mb = this->ParityTypeMenuButton->GetWidget();
        if (!strcmp (mb->GetValue(), "No"))   

        {
            this->IGSTKStream->SetParity(igstk::SerialCommunication::NoParity);
        }
        else if  (!strcmp (mb->GetValue(), "Odd")) 
        {
            this->IGSTKStream->SetParity(igstk::SerialCommunication::OddParity);
        }
        else
        {

            this->IGSTKStream->SetParity(igstk::SerialCommunication::EvenParity);
        }

        // Stop bits 
        int sb = atoi(this->StopBitsMenuButton->GetWidget()->GetValue());
        this->IGSTKStream->SetStopBits((StopBitsT)sb);

        // Hand shake
        mb = this->HandShakeMenuButton->GetWidget();
        if (!strcmp (mb->GetValue(), "Off"))   
        {
            this->IGSTKStream->SetHandShake(igstk::SerialCommunication::HandshakeOff);
        }
        else
        {
            this->IGSTKStream->SetHandShake(igstk::SerialCommunication::HandshakeOn);
        }

        this->IGSTKStream->SetTracking(1);
        this->IGSTKStream->SetSpeed(sp);
        this->IGSTKStream->SetMultiFactor(multi);
        this->IGSTKStream->Init();
        this->IGSTKStream->ProcessTimerEvents();
    }
    else
    {
        this->IGSTKStream->SetTracking(0);
    }
}
#endif
