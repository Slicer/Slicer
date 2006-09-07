#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerVisibilityIcons.h"

#include "vtkKWMessage.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"

#include "vtkKWFrameWithLabel.h"

#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkSlicerFiducialsGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerFiducialsGUI );
vtkCxxRevisionMacro ( vtkSlicerFiducialsGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerFiducialsGUI::vtkSlicerFiducialsGUI ( )
{
    this->Logic = NULL;
    this->FiducialListSelectorWidget = NULL;
    this->FiducialListNode = NULL;
    this->FiducialListNodeID = "(none)";

    this->AddFiducialListButton = NULL;
    this->AddFiducialButton = NULL;

    this->VisibilityToggle = NULL;
    this->VisibilityIcons = NULL;

    this->ListColorButton = NULL;
    this->ListSymbolScale = NULL;
    this->ListTextScale = NULL;
    
    this->MultiColumnList = NULL;
    this->NumberOfColumns = 9;
    std::cout << "vtkSlicerFiducialsGUI: constructor done\n";
}


//---------------------------------------------------------------------------
vtkSlicerFiducialsGUI::~vtkSlicerFiducialsGUI ( )
{

    this->SetModuleLogic ( NULL );

    if (this->FiducialListSelectorWidget)
    {
        this->FiducialListSelectorWidget->SetParent(NULL);
        this->FiducialListSelectorWidget->Delete();
        this->FiducialListSelectorWidget = NULL;
    }

    if (this->AddFiducialListButton ) {
        this->AddFiducialListButton->SetParent (NULL );
        this->AddFiducialListButton->Delete ( );
        this->AddFiducialListButton = NULL;
    }
     
    if (this->AddFiducialButton ) {
        this->AddFiducialButton->SetParent (NULL );
        this->AddFiducialButton->Delete ( );
        this->AddFiducialButton = NULL;
    }

    if (this->VisibilityToggle) {
        this->VisibilityToggle->SetParent(NULL);
        this->VisibilityToggle->Delete();
        this->VisibilityToggle = NULL;
    }

    if ( this->VisibilityIcons ) {
        this->VisibilityIcons->Delete  ( );
        this->VisibilityIcons = NULL;
    }

    if (this->ListColorButton) {
        this->ListColorButton->SetParent(NULL);
        this->ListColorButton->Delete();
        this->ListColorButton = NULL;
    }

    if (this->ListSymbolScale) {
        this->ListSymbolScale->SetParent(NULL);
        this->ListSymbolScale->Delete();
        this->ListSymbolScale = NULL;
    }

    if (this->ListTextScale) {
        this->ListTextScale->SetParent(NULL);
        this->ListTextScale->Delete();
        this->ListTextScale = NULL;
    }

    if (this->MultiColumnList) {
      this->MultiColumnList->SetParent(NULL);
      this->MultiColumnList->Delete();
      this->MultiColumnList = NULL;
    }

    this->SetFiducialListNodeID("(none)");
}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerFiducialsGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "FiducialsNode: " << this->GetFiducialListNode ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::RemoveGUIObservers ( )
{
    std::cout << "vtkSlicerFiducialsGUI: RemoveGUIObservers\n";
    this->FiducialListSelectorWidget->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->AddFiducialListButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->AddFiducialButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VisibilityToggle->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListColorButton->RemoveObservers (vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListSymbolScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListTextScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::AddGUIObservers ( )
{
    std::cout << "vtkSlicerFiducialsGUI: AddGUIObservers\n";
    this->FiducialListSelectorWidget->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
    this->AddFiducialListButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->AddFiducialButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

    this->VisibilityToggle->AddObserver (vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->ListColorButton->AddObserver (vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListSymbolScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListTextScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
}



//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // process fiducial list node selector events
    vtkSlicerNodeSelectorWidget *fidListSelector = 
        vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
    if (fidListSelector == this->FiducialListSelectorWidget &&
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
    {
        vtkMRMLFiducialListNode *fidList =
            vtkMRMLFiducialListNode::SafeDownCast(this->FiducialListSelectorWidget->GetSelected());
        if (fidList != NULL)
        {
            std::cout << "vtkSlicerFiducialsGUI: ProcessGUIEvent Node Selector Event: " << event << ".\n";
            this->SetFiducialListNode(fidList);
//            this->SetFiducialListNodeID(fidList->GetID());
//            std::cout << "\tset this list node ID\n";
            this->ProcessMRMLEvents(fidList,vtkCommand::ModifiedEvent, NULL);
        }
        return;
    }
    vtkMRMLFiducialListNode *fidList =
        vtkMRMLFiducialListNode::SafeDownCast(caller);
    if (fidList == this->FiducialListNode &&
        event == vtkCommand::ModifiedEvent)
    {
//        std::cout << "vtkSlicerFiducialsGUI: ProcessGUIEvent list node modified event " << event << endl;
        return;
    }
  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
  if (button == this->AddFiducialListButton && event ==  vtkKWPushButton::InvokedEvent)
    {
       std::cout << "vtkSlicerFiducialsGUI: ProcessGUIEvent: Add Fiducial List Button event: " << event << ".\n";
       vtkSlicerFiducialsLogic* modelLogic = this->GetLogic();
        if (modelLogic == NULL)
        {
            // TODO; generate an error...
            std::cerr << " ERROR getting the Logic of the Fiducials Gui\n";
            return;
        }
        // add the new list and set it selected
        this->SetFiducialListNode(modelLogic->AddFiducials());

        // now update the fiducial list pane for the new list
        this->ProcessMRMLEvents(this->FiducialListSelectorWidget->GetSelected(), vtkCommand::ModifiedEvent, callData);
        
        std::cerr << "Done adding fiducials list\n";
        return;
    }
  // get the active list
  vtkSlicerFiducialsLogic* modelLogic = this->GetLogic();
  if (modelLogic == NULL)
  {
      // TODO; generate an error...
      std::cerr << " ERROR getting the Logic of the Fiducials Gui\n";
      return;
  }
  
  // is there an active list?
  if (modelLogic->GetActiveFiducialListNode() == NULL)
  {
      // 
      std::cerr << "ERROR: No Fiducial List, add one first!\n";
      return;
  }
  // save state for undo
  this->MRMLScene->SaveStateForUndo(modelLogic->GetActiveFiducialListNode());
        
  if (button == this->AddFiducialButton  && event ==  vtkKWPushButton::InvokedEvent)
    {
       std::cout << "vtkSlicerFiducialsGUI: ProcessGUIEvent: Add Fiducial Button event: " << event << ".\n";
     
        vtkMRMLFiducial *modelNode = modelLogic->AddFiducial( );
        if ( modelNode == NULL ) 
        {
            // TODO: generate an error...
            std::cerr << "ERROR adding a new fiducial point\n";
            return;
        }
        
        // std::cout << "Adding a row to the table...\n";
        this->MultiColumnList->GetWidget()->AddRow();
        int NumberOfRows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
        if (NumberOfRows == -1)
        {
            std::cerr << "ERROR: adding a new row\n";
            return;
        }

        // reduce the row number by one to use it as an index
        int row = NumberOfRows - 1;
        
        // now populate it
        float *xyz;
        float *wxyz;
        
        xyz = modelNode->GetXYZ();
        wxyz = modelNode->GetOrientationWXYZ();
        if (modelNode->GetLabelText() != NULL)
        {
            this->MultiColumnList->GetWidget()->SetCellText(row,0,modelNode->GetLabelText());
        }
        else
        {
            std::cerr << "WARNING: new fiducial doesn't have a name\n";
            this->MultiColumnList->GetWidget()->SetCellText(row,0,"(none)");
        }
        // make it editable
        this->MultiColumnList->GetWidget()->SetCellEditWindowToEntry(row,0);
        this->MultiColumnList->GetWidget()->CellEditableOn(row,0);
        if (xyz != NULL)
        {
            this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,1,xyz[0]);
            this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,2,xyz[1]);
            this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,3,xyz[2]);
        }
        if (wxyz != NULL)
        {
            this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,4,wxyz[0]);
            this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,5,wxyz[1]);
            this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,6,wxyz[2]);
            this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,7,wxyz[3]);
        }
        // selected
        this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,8,(modelNode->GetSelected() ? 1 : 0));
        this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row,8);
    }
  if (button == this->GetVisibilityToggle()  && event ==  vtkKWPushButton::InvokedEvent)
    {
       std::cout << "vtkSlicerFiducialsGUI: ProcessGUIEvent: Visibility  button event: " << event << ".\n";
       // change the visibility
        modelLogic->GetActiveFiducialListNode()->SetVisibility( ! modelLogic->GetActiveFiducialListNode()->GetVisibility());
        // update the icon via  process mrml event that should get pushed
        this->ProcessMRMLEvents(caller, event, callData); 

    }

  // list colour
  vtkKWChangeColorButton *colorButton = vtkKWChangeColorButton::SafeDownCast(caller);
  if (colorButton == this->ListColorButton && event == vtkKWChangeColorButton::ColorChangedEvent)
  {
      std::cout << "ProcessGUIEvents: list colour button change event\n";
       // change the colour
        modelLogic->GetActiveFiducialListNode()->SetColor(this->ListColorButton->GetColor());
        // this->ProcessMRMLEvents(caller, event, callData); 

  }
  // list symbol and text sizes
  vtkKWScaleWithEntry *scale = vtkKWScaleWithEntry::SafeDownCast(caller);
  if (scale == this->ListSymbolScale && event == vtkKWScale::ScaleValueChangedEvent)
  {
      modelLogic->GetActiveFiducialListNode()->SetSymbolScale(this->ListSymbolScale->GetValue());
      // this->ProcessMRMLEvents(caller, event, callData);                                                     
  }
  else if (scale == this->ListTextScale && event == vtkKWScale::ScaleValueChangedEvent)
  {
      modelLogic->GetActiveFiducialListNode()->SetTextScale(this->ListTextScale->GetValue());
      // this->ProcessMRMLEvents(caller, event, callData); 
  }
  return;
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ProcessLogicEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // std::cout << "vtkSlicerFiducialsGUI: Process Logic Events " << event << ".\n";
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ProcessMRMLEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
//    std::cout << "vtkSlicerFiducialsGUI: Process Mrml Events " << event << ".\n";
    // Fill in

    vtkMRMLFiducialListNode *node = vtkMRMLFiducialListNode::SafeDownCast(caller);
    if (node == this->FiducialListNode && event == vtkCommand::ModifiedEvent)
    {
        std::cout << "\tmodified event on the fiducial list node.\n";
        if (node == NULL)
        {
            std::cout << "\tBUT: the node is null\n";
//            return;
        }
        std::cout << "\t\tUPDATE THE GUI HERE!\n";
        // update the table
        return;
    }
    if (node == this->FiducialListSelectorWidget->GetSelected() && event == vtkCommand::ModifiedEvent)
    {
        std::cout << "\tmodified event on the fiducial list selected node.\n";
        //std::cout << "\t\t selected = " << this->FiducialListSelectorWidget->GetSelected() << "\n";
        if (this->GetLogic())
        {
            vtkDebugMacro(<< "\t\t active = " << this->GetLogic()->GetActiveFiducialListNode() << "\n");
            // when the list is created, it's set active, but when select it
            // from the drop down menu, need to set it active
            if ( this->GetLogic()->GetActiveFiducialListNode() !=  this->FiducialListSelectorWidget->GetSelected())
            {
                // select it first off
                this->GetLogic()->SetActiveFiducialListNode((vtkMRMLFiducialListNode *)(this->FiducialListSelectorWidget->GetSelected()));
                
            }

            // clear out the multi column list box and fill it in with the
            // new list
            this->MultiColumnList->GetWidget()->DeleteAllRows();
            // get the points in the active fid list
            vtkMRMLFiducialListNode *listNode  = this->GetLogic()->GetActiveFiducialListNode();
            // add rows for each point
            int numPoints = listNode->GetNumberOfFiducials();
            float *xyz;
            float *wxyz;
            for (int row = 0; row < numPoints; row++)
            {
                //std::cout << "Adding point " << row << " to the table" << endl;
                vtkMRMLFiducial * pointNode = listNode->GetNthFiducial(row);
                // add a row for this point
                this->MultiColumnList->GetWidget()->AddRow();
                
                // now populate it
                xyz = pointNode->GetXYZ();
                wxyz = pointNode->GetOrientationWXYZ();
                
                if (pointNode->GetLabelText() != NULL)
                {
                    this->MultiColumnList->GetWidget()->SetCellText(row,0,pointNode->GetLabelText());
                }
                else
                {
                    this->MultiColumnList->GetWidget()->SetCellText(row,0,"(none)");
                }
                if (xyz != NULL)
                {
                    this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,1,xyz[0]);
                    this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,2,xyz[1]);
                    this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,3,xyz[2]);
                }
                if (wxyz != NULL)
                {
                    this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,4,wxyz[0]);
                    this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,5,wxyz[1]);
                    this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,6,wxyz[2]);
                    this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,7,wxyz[3]);
                }
                // selected
                this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,8,(pointNode->GetSelected() ? 1 : 0));
                this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row,8);
            }
        }
    }

    vtkDebugMacro("Now going to update GUI from the logic's active list");
    // update the visibility, color, scale buttons to match the logic state
    vtkSlicerFiducialsLogic* modelLogic = this->GetLogic();
    if ((modelLogic != NULL))
        //&& (node == modelLogic->GetActiveFiducialListNode()) &&
        //(event == vtkKWPushButton::InvokedEvent))
    {
        if (modelLogic->GetActiveFiducialListNode() == NULL)
        {
            std::cerr << "vtkSlicerFiducialsGUI::ProcessMRMLEvents: ERROR: no active fiducial list node in the logic class!\n";
            // is there a gui fid?
            if (this->GetFiducialListNode() != NULL)
            {
                std::cout << "\t BUT there is a list node in the gui, set it active\n\n\n";
                modelLogic->SetActiveFiducialListNode(this->GetFiducialListNode());
            }
            else
            {
                return;
            }
        }
        vtkDebugMacro(<< "\tupdating the visibility button\n");
        if (this->GetVisibilityToggle() != NULL &&
            this->GetVisibilityIcons() != NULL)
        {
            if (modelLogic->GetActiveFiducialListNode()->GetVisibility() > 0)
            {
                this->GetVisibilityToggle()->SetImageToIcon(
                    this->GetVisibilityIcons()->GetVisibleIcon());
            }
            else
            {
                this->GetVisibilityToggle()->SetImageToIcon(
                    this->GetVisibilityIcons()->GetInvisibleIcon());
            }
        }
        else
        {
            std::cerr << "ERROR; trying up update null visibility toggle!\n";
        }

        // color
        vtkDebugMacro(<< "\tupdating the colour\n");
        double *nodeColor = modelLogic->GetActiveFiducialListNode()->GetColor();
        if (this->ListColorButton != NULL)
        {
            double *buttonColor = this->ListColorButton->GetColor();
            if (nodeColor != NULL && buttonColor != NULL && 
                (nodeColor[0] != buttonColor[0] ||
                 nodeColor[1] != buttonColor[1] ||
                 nodeColor[2] != buttonColor[2]))
            {
                std::cout << "Updating list color button\n";
                this->ListColorButton->SetColor(nodeColor);
            }
        }
        else
        {
            std::cerr << "No colour button!\n";
        }
        // symbol scale
        vtkDebugMacro(<< "\tupdating the symbol scale");
        double scale = modelLogic->GetActiveFiducialListNode()->GetSymbolScale();
        if (this->ListSymbolScale != NULL &&
            scale != this->ListSymbolScale->GetValue())
        {
            this->ListSymbolScale->SetValue(scale);
        }

        // text scale
        vtkDebugMacro(<< "\tupdating the text scale.");
        scale = modelLogic->GetActiveFiducialListNode()->GetTextScale();
        if (this->ListTextScale != NULL &&
            scale != this->ListTextScale->GetValue())
        {
            this->ListTextScale->SetValue(scale);
        }
    }
    vtkDebugMacro("Done processing mrml events...");
//    std::cerr << "vtkSlicerFiducialsGUI::ProcessMRMLEvents  mismatched caller and event (" << event << ")\n";
}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::Enter ( )
{
    std::cout << "vtkSlicerFiducialsGUI: Enter\n";
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::Exit ( )
{
    std::cout <<  "vtkSlicerFiducialsGUI: Exit\n";
    // Fill in
}




//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::BuildGUI ( )
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Fiducials", "Fiducials", NULL );

    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Fiducials" );
    
    // HELP FRAME
    vtkKWFrameWithLabel *modHelpFrame = vtkKWFrameWithLabel::New ( );
    modHelpFrame->SetParent (page );
    modHelpFrame->Create ( );
    modHelpFrame->CollapseFrame ( );
    modHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modHelpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Fiducials")->GetWidgetName());

    vtkKWMessage *helpMessage = vtkKWMessage::New();
    helpMessage->SetParent(modHelpFrame->GetFrame());
    helpMessage->Create();
    helpMessage->SetWidth(100);
    helpMessage->SetText("Create and manage Fiducial lists.");
    app->Script ( "pack %s", helpMessage->GetWidgetName());
                  
    // ---
    // DISPLAY FRAME            
    vtkKWFrameWithLabel *displayFrame = vtkKWFrameWithLabel::New ( );
    displayFrame->SetParent ( page );
    displayFrame->Create ( );
    displayFrame->SetLabelText ("Display");
    displayFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  displayFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("Fiducials")->GetWidgetName());


    // new list button
    this->AddFiducialListButton = vtkKWPushButton::New ( );
    this->AddFiducialListButton->SetParent ( displayFrame->GetFrame() );
    this->AddFiducialListButton->Create ( );
    this->AddFiducialListButton->SetText ("New Fiducial List");
    
    app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->AddFiducialListButton->GetWidgetName());
    
    // node selector
    this->FiducialListSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->FiducialListSelectorWidget->SetParent(displayFrame->GetFrame());
    this->FiducialListSelectorWidget->Create();
    this->FiducialListSelectorWidget->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL, NULL);
    this->FiducialListSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->FiducialListSelectorWidget->SetBorderWidth(2);
    this->FiducialListSelectorWidget->SetPadX(2);
    this->FiducialListSelectorWidget->SetPadY(2);
    //this->FiducialListSelectorWidget->GetWidget()->IndicatorVisibilityOff();
    this->FiducialListSelectorWidget->GetWidget()->SetWidth(24);
    this->FiducialListSelectorWidget->SetLabelText( "Fiducial List Select: ");
    this->FiducialListSelectorWidget->SetBalloonHelpString("select a fiducial list from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->FiducialListSelectorWidget->GetWidgetName());
    this->FiducialListSelectorWidget->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
 

    // ---
    // LIST FRAME
    
    vtkKWFrameWithLabel *listFrame = vtkKWFrameWithLabel::New();
    listFrame->SetParent( page );
    listFrame->Create();
    listFrame->SetLabelText("Fiducial List");
    listFrame->ExpandFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  listFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Fiducials")->GetWidgetName());

    // visibility
    this->VisibilityIcons = vtkSlicerVisibilityIcons::New ( );
    this->VisibilityToggle = vtkKWPushButton::New();
    this->VisibilityToggle->SetParent ( listFrame->GetFrame() );
    this->VisibilityToggle->Create ( );
    this->VisibilityToggle->SetReliefToFlat ( );
    this->VisibilityToggle->SetOverReliefToNone ( );
    this->VisibilityToggle->SetBorderWidth ( 0 );
    this->VisibilityToggle->SetImageToIcon ( this->VisibilityIcons->GetInvisibleIcon ( ) );        
    this->VisibilityToggle->SetBalloonHelpString ( "Toggles fiducial list visibility in the MainViewer." );
    this->VisibilityToggle->SetText ("Visibility");
    
    app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->VisibilityToggle->GetWidgetName());
 
    // text scale
    this->ListTextScale = vtkKWScaleWithEntry::New();
    this->ListTextScale->SetParent( listFrame->GetFrame() );
    this->ListTextScale->Create();
    this->ListTextScale->SetLabelText("Text Scale:");
    this->ListTextScale->SetBalloonHelpString ( "Set the scale of the fiducial list points text.");
    this->ListTextScale->GetWidget()->SetRange(0.0, 20.0);
    this->ListTextScale->GetWidget()->SetOrientationToHorizontal ();
    this->ListTextScale->GetWidget()->SetResolution(0.5);
    this->ListTextScale->SetEntryWidth(5);
    app->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
                this->ListTextScale->GetWidgetName());

    // symbol scale
    this->ListSymbolScale = vtkKWScaleWithEntry::New();
    this->ListSymbolScale->SetParent( listFrame->GetFrame() );
    this->ListSymbolScale->Create();
    this->ListSymbolScale->SetLabelText("Symbol Scale:");
    this->ListSymbolScale->SetBalloonHelpString ( "Set the scale of the fiducial list symbols.");
    this->ListSymbolScale->GetWidget()->SetRange(0.0, 80.0);
    this->ListSymbolScale->GetWidget()->SetOrientationToHorizontal();
    this->ListSymbolScale->GetWidget()->SetResolution(0.5);
    this->ListSymbolScale->SetEntryWidth(5);
    app->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
                this->ListSymbolScale->GetWidgetName());
    
    // color
    this->ListColorButton = vtkKWChangeColorButton::New();
    this->ListColorButton->SetParent( listFrame->GetFrame() );
    this->ListColorButton->Create();
    this->ListColorButton->SetBorderWidth(0);
    this->ListColorButton->SetBalloonHelpString("Change the colour of the fiducial list symbols and text in the MainViewer");
    
    this->ListColorButton->SetDialogTitle("List symbol and text color");
    app->Script("pack %s -side top -anchor w -padx 2 -pady 2",
                this->ListColorButton->GetWidgetName());
    
    // add the multicolumn list to show the points
    this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
    this->MultiColumnList->SetParent ( listFrame->GetFrame() );
    this->MultiColumnList->Create ( );
    this->MultiColumnList->SetHeight(4);
    this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
    this->MultiColumnList->GetWidget()->MovableRowsOff();
    this->MultiColumnList->GetWidget()->MovableColumnsOff();
    // set up the columns of data for each point
    // name, x, y, z, orientation w, x, y, z, selected
    this->MultiColumnList->GetWidget()->AddColumn("Name");
    this->MultiColumnList->GetWidget()->AddColumn("X");
    this->MultiColumnList->GetWidget()->AddColumn("Y");
    this->MultiColumnList->GetWidget()->AddColumn("Z");
    this->MultiColumnList->GetWidget()->AddColumn("OrW");
    this->MultiColumnList->GetWidget()->AddColumn("OrX");
    this->MultiColumnList->GetWidget()->AddColumn("OrY");
    this->MultiColumnList->GetWidget()->AddColumn("OrZ");
    this->MultiColumnList->GetWidget()->AddColumn("Selected");

    // make the selected column editable by checkbox
    this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(8);
    
    
    // now set the attributes that are equal across the columns
    int col;
    for (col = 0; col < this->NumberOfColumns; col++)
    {        
        this->MultiColumnList->GetWidget()->SetColumnWidth(col, 12);
        this->MultiColumnList->GetWidget()->SetColumnAlignmentToLeft(col);
        this->MultiColumnList->GetWidget()->ColumnEditableOn(col);
    }
    app->Script ( "pack %s -fill both -expand true",
                  this->MultiColumnList->GetWidgetName());
//                  listFrame->GetWidgetName());
    this->MultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "UpdateElement");

     // add an add fiducial button
    this->AddFiducialButton = vtkKWPushButton::New ( );
    this->AddFiducialButton->SetParent ( listFrame->GetFrame() );
    this->AddFiducialButton->Create ( );
    this->AddFiducialButton->SetText ("Add Fiducial Point");
    
    app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->AddFiducialButton->GetWidgetName());

    if (this->MRMLScene != NULL)
    {
        this->MRMLScene->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
    }

     
    helpMessage->Delete ( );
    displayFrame->Delete ( );
    listFrame->Delete();
    modHelpFrame->Delete ( );
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::UpdateElement(int row, int col, char * str)
{
    std::cout << "vtkSlicerFiducialsGUI: UpdateElement " << row << " " << col << " " << str << "\n";
    cout << "row = " << row << ", col = " << col << ", str = " << str << endl;

    // make sure that the row and column exists in the table
    if ((row >= 0) && (row < this->MultiColumnList->GetWidget()->GetNumberOfRows()) &&
        (col >= 0) && (col < this->MultiColumnList->GetWidget()->GetNumberOfColumns()))
        {
            vtkSlicerFiducialsLogic* modelLogic = this->GetLogic();
            if (modelLogic == NULL)
            {
                // TODO; generate an error...
                std::cerr << "UpdateElement: ERROR getting the Logic of the Fiducials Gui\n";
                return;
            }
            // is there an active list?
            if (modelLogic->GetActiveFiducialListNode() == NULL)
            {
                // 
                std::cerr << "UpdateElement: ERROR: No Fiducial List, add one first!\n";
                return;
            }
        
            // get the fiducial point at that row in the table
            vtkMRMLFiducial * node = modelLogic->GetActiveFiducialListNode()->GetNthFiducial(row);
            if (node == NULL)
            {
                std::cerr << "UpdateElement: ERROR: null node at row " << row << endl;
                return;
            }
            std::cout << "\tgot the " << row << "th fiducial node from the active list\n";
            // now update the requested value
            if (col == 0)
            {
                std:: cout << "\tsetting the name?\n";
                node->SetLabelText(str);
            }
            else if (col > 0 && col < 4)
            {
                std::cout << "\tsetting position...\n";
                // get the current xyz
                float * xyz = node->GetXYZ();
                // now set the new one
                float newCoordinate = atof(str);
                if (col == 1) { node->SetXYZ(newCoordinate, xyz[1], xyz[2]); }
                if (col == 2) { node->SetXYZ(xyz[0], newCoordinate, xyz[2]); }
                if (col == 3) { node->SetXYZ(xyz[0], xyz[1], newCoordinate); }
            }
            else if (col > 3 && col < 8)
            {
                std::cout << "\tsetting orientation...\n";
                float * wxyz = node->GetOrientationWXYZ();
                float newCoordinate = atof(str);
                if (col == 4) { node->SetOrientationWXYZ(newCoordinate, wxyz[1], wxyz[2], wxyz[3]); }
                if (col == 5) { node->SetOrientationWXYZ(wxyz[0], newCoordinate, wxyz[2], wxyz[3]); }
                if (col == 6) { node->SetOrientationWXYZ(wxyz[0], wxyz[1], newCoordinate, wxyz[3]); }
                if (col == 7) { node->SetOrientationWXYZ(wxyz[0], wxyz[1], wxyz[2], newCoordinate); }
            }
            else if (col == 8)
            {
                std::cout << "\tsetting node selected to " << str << endl;
                // selected
                node->SetSelected(atoi(str));
            }
            else
            {
                std::cerr << "UpdateElement: ERROR: invalid column number " << col << ", valid values are 0-" << this->NumberOfColumns << endl;
                return;
            }
            //UpdateVTK();
        }
    else
    {
        std::cout << "Invalid row " << row << " or column " << col <<  ", valid columns are 0-" << this->NumberOfColumns << "\n";
    }
}
    
//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::UpdateVTK()
{
     std::cout << "vtkSlicerFiducialsGUI: UpdateVTK\n";
    // check for null list
    if (this->MultiColumnList == NULL)
    {
        return;
    }

    int row, col, lastrow = 0;
    // get the rows in the multicolumn list
    for (row = 0; row < lastrow; row++)
    {
        for (col = 0; col < this->NumberOfColumns; col++)
        {
            // update the fiducial qualities
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::SetFiducialListNode (vtkMRMLFiducialListNode *fiducialListNode)
{
    if (fiducialListNode == NULL)
    {
        std::cerr << "ERROR: SetFiducialListNode - list node is null.\n";
        return;
    }
    
    // select this fiducial list node
    
//    std::cout << "SetFiducialListNode : setting the selector widget to " << fiducialListNode->GetID() << endl;
    
    this->FiducialListSelectorWidget->SetSelected(fiducialListNode);

    // set the member variables and do a first process
    if (fiducialListNode != NULL)
    {
        //std::cout << "SetFiducialListNode: setting the fid list node id to " << fiducialListNode->GetID() << "\n";
        //this->SetFiducialListNodeID(fiducialListNode->GetID());
        // observe the node for future changes
        //vtkDebugMacro("Setting and observing the fidlist mrml node");
        //this->SetAndObserveMRMLNode(fiducialListNode);

        // now process it as modified
        vtkDebugMacro("Calling process mrml events with the modified fid list node");
        this->ProcessMRMLEvents(fiducialListNode, vtkCommand::ModifiedEvent, NULL);
    }
}

