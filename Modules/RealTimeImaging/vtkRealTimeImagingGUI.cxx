#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkRealTimeImagingGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

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
#include "vtkKWMultiColumnListWithScrollbars.h"

#include "vtkKWTkUtilities.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkCylinderSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkActor.h"
#include "vtkProperty.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkRealTimeImagingGUI );
vtkCxxRevisionMacro ( vtkRealTimeImagingGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkRealTimeImagingGUI::vtkRealTimeImagingGUI ( )
{
    this->Logic = NULL;

    this->ServerMenu = NULL;
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

/*
    this->RedColorScale = NULL;
    this->GreenColorScale = NULL;
    this->BlueColorScale = NULL;
    */

    this->ConnectCheckButton = NULL;
    this->PauseCheckButton = NULL;

    this->LocatorCheckButton = NULL;
    this->HandleCheckButton = NULL;
    this->GuideCheckButton = NULL;

    this->LocatorModeCheckButton = NULL;
    this->UserModeCheckButton = NULL;

    /*
    this->RedSliceMenu = NULL;
    this->YellowSliceMenu = NULL;
    this->GreenSliceMenu = NULL;
    */

    this->LoadConfigButton = NULL;

    this->ConfigFileEntry;
    this->UpdateRateEntry;

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

    this->StopTimer = 1;
    this->LocatorMatrix = NULL;

    this->LocatorModelNode = NULL;
    this->LocatorModelDisplayNode = NULL;
}


//---------------------------------------------------------------------------
vtkRealTimeImagingGUI::~vtkRealTimeImagingGUI ( )
{
    this->RemoveGUIObservers();

    if (this->ServerMenu)
    {
        this->ServerMenu->SetParent(NULL );
        this->ServerMenu->Delete ( );
    }
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
    if (this->PauseCheckButton)
    {
        this->PauseCheckButton->SetParent(NULL );
        this->PauseCheckButton->Delete ( );
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

    /*
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
    */

    if (this->LoadConfigButton)
    {
        this->LoadConfigButton->SetParent(NULL );
        this->LoadConfigButton->Delete ( );
    }

    if (this->ConfigFileEntry)
    {
        this->ConfigFileEntry->SetParent(NULL );
        this->ConfigFileEntry->Delete ( );
    }
    if (this->UpdateRateEntry)
    {
        this->UpdateRateEntry->SetParent(NULL );
        this->UpdateRateEntry->Delete ( );
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

    if (this->LocatorModelNode)
    {
        this->LocatorModelNode->Delete ( );
    }

    this->SetModuleLogic ( NULL );
}


//---------------------------------------------------------------------------
void vtkRealTimeImagingGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "RealTimeImagingGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkRealTimeImagingGUI::RemoveGUIObservers ( )
{
    // Fill in
    if (this->LoadConfigButton)
    {
        this->LoadConfigButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
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
void vtkRealTimeImagingGUI::AddGUIObservers ( )
{

    // Fill in
    // observer load volume button
    this->LoadConfigButton->GetWidget()->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );

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


}



//---------------------------------------------------------------------------
void vtkRealTimeImagingGUI::ProcessGUIEvents ( vtkObject *caller,
        unsigned long event, void *callData )
{
    if (this->LoadConfigButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) 
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
    else if (this->ConnectCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
             && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
        vtkRealTimeImagingLogic *igtLogic = this->Logic;
        int checked = this->ConnectCheckButton->GetSelectedState(); 
        if (checked)
        {
            // connected
            const char * filename = this->LoadConfigButton->GetWidget()->GetFileName();
            if (! filename)
            {
                // TODO: Generate an error ...
            }
            else
            {
                igtLogic->Init(filename);
                vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
                int rate = atoi(this->UpdateRateEntry->GetWidget()->GetValue ());

                vtkKWTkUtilities::CreateTimerHandler (app, rate, this, "ProcessTimerEvents");
                this->StopTimer = 0;
            }
        }
        else
        {
            this->StopTimer = 1;
            igtLogic->CloseConnection();
        }
    }
    else if (this->GetPatCoordinatesPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
    {
        float position[3];
        char value[50];
        if (this->LocatorMatrix)
        {
            for (int j = 0; j < 3; j++) 
            {
                position[j] = this->LocatorMatrix->GetElement(j,0);
                // position[j] = j / 1.0;
            }
            sprintf(value, "%6.2f  %6.2f  %6.2f", position[0], position[1], position[2]);
            this->PatCoordinatesEntry->GetWidget()->SetValue(value);

        }
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
            vtkRealTimeImagingLogic *igtLogic = this->Logic;
            this->Logic->SetNumberOfPoints(row);
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
                igtLogic->AddPoint(r, sc1, sc2, sc3, pc1, pc2, pc3);
            }

            int error = igtLogic->DoRegistration();
            // igtLogic->GetLandmarkTransformMatrix()->PrintSelf(cout, 5);


            if (error)
            {
                vtkSlicerApplication::GetInstance()->ErrorMessage("Error registration between patient and image land marks.");
            }
        }
    }
    else if (this->ResetPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
    {
        vtkRealTimeImagingLogic *igtLogic = this->Logic;
        igtLogic->SetUseRegistration(0);
    }
    else if (this->LocatorCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
             && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
        if (this->LocatorModelNode == NULL)
        {
            this->MakeLocator();
        }

        int checked = this->LocatorCheckButton->GetSelectedState(); 
        this->LocatorModelDisplayNode->SetVisibility(checked);
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
        this->LocatorModelDisplayNode->SetColor(color->SliceGUIGreen);
    }
    else if (this->LocatorModeCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
             && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
        int checked = this->LocatorModeCheckButton->GetSelectedState(); 
        if (checked)
        {
            this->UserModeCheckButton->SelectedStateOff();
        }
        else
        {
            this->UserModeCheckButton->SelectedStateOn();
        }

    }
    else if (this->UserModeCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
             && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
        int checked = this->UserModeCheckButton->GetSelectedState(); 
        if (checked)
        {
            this->LocatorModeCheckButton->SelectedStateOff();
        }
        else
        {
            this->LocatorModeCheckButton->SelectedStateOn();
        }

    }

} 


//---------------------------------------------------------------------------
void vtkRealTimeImagingGUI::ProcessLogicEvents ( vtkObject *caller,
        unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkRealTimeImagingGUI::ProcessMRMLEvents ( vtkObject *caller,
        unsigned long event, void *callData )
{
    // Fill in
}


void vtkRealTimeImagingGUI::ProcessTimerEvents ()
{
    if (! this->StopTimer)
    {
      
        int rate = atoi(this->UpdateRateEntry->GetWidget()->GetValue ());
        vtkKWTkUtilities::CreateTimerHandler (vtkKWApplication::GetMainInterp(), rate, this, "ProcessTimerEvents");
    }

    vtkRealTimeImagingLogic *igtLogic = this->Logic;
    igtLogic->PollRealtime();
    this->LocatorMatrix = igtLogic->GetLocatorMatrix();
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


        int checked = this->LocatorModeCheckButton->GetSelectedState(); 
        if (checked)
        {
            // update the display of locator
            igtLogic->SetLocatorTransforms();
            this->UpdateLocator();

            this->UpdateSliceDisplay(px, py, pz);
        }

        //simond
        static int toggle=0;
        vtkRealTimeImagingLogic *logic = this->GetLogic();
        vtkUnsignedShortArray   *PixelArray = logic->GetPixelArray();
        vtkMRMLScalarVolumeNode *scalarNode = logic->GetscalarNode();
        //short                   *OTInputImage = logic->GetOTInputImage();
        //for(int i=0; i<10*10; i++)
        //{
/*
          if(toggle)
          {
             //PixelArray->SetValue(i,i);
             memset(PixelArray->GetPointer(0), 255, 256*256*sizeof(short));
          }
          else
            {
              //PixelArray->SetValue(i,99-i);
              // Transfer image to Pixel Array.
              memcpy(PixelArray->GetPointer(0), OTInputImage, 256*256*sizeof(short));
              //memset(PixelArray->GetPointer(0), 0, 256*256*sizeof(short));
            }
        //}
        if(toggle==0) toggle=1;
        else toggle=0;
*/
        //PixelArray->Modified();
        memcpy(PixelArray->GetPointer(0), logic->OTInputImage, 256*256*sizeof(short));
        scalarNode->Modified();
    }
}


void vtkRealTimeImagingGUI::MakeLocator()
{
    this->LocatorModelNode = vtkMRMLModelNode::New();
    this->LocatorModelDisplayNode = vtkMRMLModelDisplayNode::New();

    this->GetMRMLScene()->SaveStateForUndo();
    this->GetMRMLScene()->AddNode(this->LocatorModelDisplayNode);
    this->GetMRMLScene()->AddNode(this->LocatorModelNode);  

    this->LocatorModelDisplayNode->SetScene(this->GetMRMLScene());

    this->LocatorModelNode->SetName("Locator");
    this->LocatorModelNode->SetScene(this->GetMRMLScene());
    this->LocatorModelNode->SetAndObserveDisplayNodeID(this->LocatorModelDisplayNode->GetID());  

    vtkCylinderSource *cylinder = vtkCylinderSource::New();
    cylinder->SetRadius(1.5);
    cylinder->SetHeight(100);
    this->LocatorModelNode->SetAndObservePolyData(cylinder->GetOutput());
    // THIS API CALL IS INVALID
    //this->LocatorModelDisplayNode->SetPolyData(this->LocatorModelNode->GetPolydata());
    this->Modified();  

    // modelNode->Delete();
    cylinder->Delete();
    // displayNode->Delete();
}



void vtkRealTimeImagingGUI::UpdateSliceDisplay(float px, float py, float pz)
{
    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

    vtkSlicerSliceLogic *logic0 = appGUI->GetMainSliceGUI0()->GetLogic();
    vtkSlicerSliceLogic *logic1 = appGUI->GetMainSliceGUI1()->GetLogic();
    vtkSlicerSliceLogic *logic2 = appGUI->GetMainSliceGUI2()->GetLogic();

    vtkSlicerSliceControllerWidget *control0 = appGUI->GetMainSliceGUI0()->GetSliceController();
    vtkSlicerSliceControllerWidget *control1 = appGUI->GetMainSliceGUI1()->GetSliceController();
    vtkSlicerSliceControllerWidget *control2 = appGUI->GetMainSliceGUI2()->GetSliceController();
    control0->GetOffsetScale()->SetValue(pz);
    control1->GetOffsetScale()->SetValue(px);
    control2->GetOffsetScale()->SetValue(py);

    logic0->SetSliceOffset(pz);
    logic1->SetSliceOffset(px);
    logic2->SetSliceOffset(py);

}



void vtkRealTimeImagingGUI::UpdateLocator()
{
    vtkRealTimeImagingLogic *igtLogic = this->Logic;
    vtkTransform *transform = igtLogic->GetLocatorNormalTransform(); 

    vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
    vtkSlicerViewerWidget *viewerWidget = appGUI->GetViewerWidget();

    const char *id = this->LocatorModelNode->GetID();    
    vtkProp3D *locatorActor = viewerWidget->GetActorByID(id);
    if (locatorActor)
    {
        //locatorActor->GetProperty()->SetColor(1, 0, 0);

        locatorActor->SetUserMatrix(transform->GetMatrix());
        locatorActor->Modified();
    }
}



//---------------------------------------------------------------------------
void vtkRealTimeImagingGUI::Enter ( )
{
    // Fill in

}

//---------------------------------------------------------------------------
void vtkRealTimeImagingGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkRealTimeImagingGUI::BuildGUI ( )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    // Define your help text here.
    const char *help = "The **RealTimeImaging Module** connects Slicer to a SPLOT server that feeds a real-time stream of images from a tracking device. The objective is to display images from the OR, e.g. for ultrasound or endoscopic video streams."; 

    // ---
    // MODULE GUI FRAME 
    // create a page
    this->UIPanel->AddPage ( "RealTimeImaging", "RealTimeImaging", NULL );

    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "RealTimeImaging" );


    // ----------------------------------------------------------------
    // HELP FRAME            
    // ----------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *RealTimeImagingHelpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    RealTimeImagingHelpFrame->SetParent ( page );
    RealTimeImagingHelpFrame->Create ( );
    RealTimeImagingHelpFrame->CollapseFrame ( );
    RealTimeImagingHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
            RealTimeImagingHelpFrame->GetWidgetName(), page->GetWidgetName());

    // configure the parent classes help text widget
    this->HelpText->SetParent ( RealTimeImagingHelpFrame->GetFrame() );
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

    RealTimeImagingHelpFrame->Delete();


    BuildGUIForServerFrame ();
    BuildGUIForRegistrationFrame ();
    BuildGUIForTrackingFrame ();
    BuildGUIForHandPieceFrame ();
}


void vtkRealTimeImagingGUI::BuildGUIForRegistrationFrame ()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "RealTimeImaging" );

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


void vtkRealTimeImagingGUI::BuildGUIForServerFrame ()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "RealTimeImaging" );

    // ----------------------------------------------------------------
    // SERVER FRAME            
    // ----------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *serverFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    serverFrame->SetParent ( page );
    serverFrame->Create ( );
    serverFrame->SetLabelText ("Server");
    serverFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
            serverFrame->GetWidgetName(), page->GetWidgetName());

    // Active server frame: Server options 
    // -----------------------------------------
    vtkKWFrame *activeServerFrame = vtkKWFrame::New();
    activeServerFrame->SetParent ( serverFrame->GetFrame() );
    activeServerFrame->Create ( );
    app->Script ("pack %s -side top -anchor nw -fill x -pady 1 -in %s",
                 activeServerFrame->GetWidgetName(),
                 serverFrame->GetFrame()->GetWidgetName());

    // active server 
    this->ServerMenu = vtkKWMenuButtonWithLabel::New();
    this->ServerMenu->SetParent(activeServerFrame);
    this->ServerMenu->Create();
    this->ServerMenu->SetWidth(25);
    this->ServerMenu->SetLabelWidth(12);
    this->ServerMenu->SetLabelText("Active Server:");
    // this->ServerMenu->GetWidget()->GetMenu()->AddRadioButton ( "None");
    this->ServerMenu->GetWidget()->GetMenu()->AddRadioButton ( "SPL Open Tracker");
    this->ServerMenu->GetWidget()->SetValue ( "SPL Open Tracker" );
    this->Script(
            "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
            this->ServerMenu->GetWidgetName());


    // Setup frame: Config file and update rate 
    // -----------------------------------------
    vtkKWFrameWithLabel *setupFrame = vtkKWFrameWithLabel::New ( );
    setupFrame->SetParent ( serverFrame->GetFrame() );
    setupFrame->Create ( );
    setupFrame->SetLabelText ("Setup");
    app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 1 -in %s",
                 setupFrame->GetWidgetName(),
                 serverFrame->GetFrame()->GetWidgetName());

    // add a file browser 
    vtkKWFrame *fileFrame = vtkKWFrame::New();
    fileFrame->SetParent ( setupFrame->GetFrame() );
    fileFrame->Create ( );
    this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                  fileFrame->GetWidgetName());

    this->ConfigFileEntry = vtkKWEntry::New();
    this->ConfigFileEntry->SetParent(fileFrame);
    this->ConfigFileEntry->Create();
    this->ConfigFileEntry->SetWidth(50);
    this->ConfigFileEntry->SetValue ( "" );

    this->LoadConfigButton = vtkKWLoadSaveButtonWithLabel::New ( );
    this->LoadConfigButton->SetParent ( fileFrame );
    this->LoadConfigButton->Create ( );
    this->LoadConfigButton->SetWidth(15);
    this->LoadConfigButton->GetWidget()->SetText ("Browse Config File");
    this->LoadConfigButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
                                                              "{ {RealTimeImaging} {*.xml} }");
    this->LoadConfigButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
      "OpenPath");

    this->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
                this->LoadConfigButton->GetWidgetName(),
                this->ConfigFileEntry->GetWidgetName());

    // update rate 
    vtkKWFrame *rateFrame = vtkKWFrame::New();
    rateFrame->SetParent ( setupFrame->GetFrame() );
    rateFrame->Create ( );
    this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                  rateFrame->GetWidgetName());

    this->UpdateRateEntry = vtkKWEntryWithLabel::New();
    this->UpdateRateEntry->SetParent(rateFrame);
    this->UpdateRateEntry->Create();
    this->UpdateRateEntry->SetWidth(25);
    this->UpdateRateEntry->SetLabelWidth(15);
    this->UpdateRateEntry->SetLabelText("Update Rate (ms):");
    this->UpdateRateEntry->GetWidget()->SetValue ( "200" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
      this->UpdateRateEntry->GetWidgetName());


    // Connect frame: Connects to server 
    // -----------------------------------------
    vtkKWFrameWithLabel *connectFrame = vtkKWFrameWithLabel::New ( );
    connectFrame->SetParent ( serverFrame->GetFrame() );
    connectFrame->Create ( );
    connectFrame->SetLabelText ("Connection to server");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   connectFrame->GetWidgetName() );

    this->ConnectCheckButton = vtkKWCheckButton::New();
    this->ConnectCheckButton->SetParent(connectFrame->GetFrame());
    this->ConnectCheckButton->Create();
    this->ConnectCheckButton->SelectedStateOff();
    this->ConnectCheckButton->SetText("Connect");

/*
    this->PauseCheckButton = vtkKWCheckButton::New();
    this->PauseCheckButton->SetParent(connectFrame->GetFrame());
    this->PauseCheckButton->Create();
    this->PauseCheckButton->SelectedStateOff();
    this->PauseCheckButton->SetText("Pause");

    this->Script("pack %s %s -side left -anchor w -padx 2 -pady 2", 
                this->ConnectCheckButton->GetWidgetName(),
                this->PauseCheckButton->GetWidgetName());
*/

    this->Script("pack %s -side left -anchor w -padx 2 -pady 2", 
                this->ConnectCheckButton->GetWidgetName());

    serverFrame->Delete ();
    activeServerFrame->Delete ();
    setupFrame->Delete ();
    fileFrame->Delete ();
    rateFrame->Delete ();
    connectFrame->Delete ();
}



void vtkRealTimeImagingGUI::BuildGUIForTrackingFrame ()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "RealTimeImaging" );


    // ----------------------------------------------------------------
    // TRACKING FRAME            
    // ----------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *trackingFrame = vtkSlicerModuleCollapsibleFrame::New ( );    
    trackingFrame->SetParent ( page );
    trackingFrame->Create ( );
    trackingFrame->SetLabelText ("Tracking");
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

    /*
    // Slice frame
    vtkKWFrame *sliceFrame = vtkKWFrame::New();
    sliceFrame->SetParent ( driverFrame->GetFrame() );
    sliceFrame->Create ( );
    app->Script ("pack %s -side top -anchor nw -fill x -pady 1 -in %s",
                 sliceFrame->GetWidgetName(),
                 driverFrame->GetFrame()->GetWidgetName());
   */

    /*
    // Contents in mode frame 
    vtkKWLabel *modeLabel = vtkKWLabel::New();
    modeLabel->SetParent(modeFrame);
    modeLabel->Create();
    modeLabel->SetWidth(5);
    modeLabel->SetText("Mode:");
    */

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

    /*
    this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
                modeLabel->GetWidgetName(),
                this->LocatorModeCheckButton->GetWidgetName(),
                this->UserModeCheckButton->GetWidgetName());
                */

    this->Script("pack %s %s -side left -anchor w -padx 2 -pady 2", 
                this->LocatorModeCheckButton->GetWidgetName(),
                this->UserModeCheckButton->GetWidgetName());

/*
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
*/

    trackingFrame->Delete ();
    displayFrame->Delete ();
    driverFrame->Delete ();
    modeFrame->Delete ();
//    sliceFrame->Delete ();

}



void vtkRealTimeImagingGUI::BuildGUIForHandPieceFrame ()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "RealTimeImaging" );

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

