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
        this->FiducialListSelectorWidget->Delete();
        this->FiducialListSelectorWidget = NULL;
    }

    if (this->AddFiducialListButton ) {
        this->AddFiducialListButton->Delete ( );
        this->AddFiducialListButton = NULL;
    }
     
    if (this->AddFiducialButton ) {
        this->AddFiducialButton->Delete ( );
        this->AddFiducialButton = NULL;
    }

    if (this->VisibilityToggle) {
        this->VisibilityToggle->Delete();
        this->VisibilityToggle = NULL;
    }

    if ( this->VisibilityIcons ) {
        this->VisibilityIcons->Delete  ( );
        this->VisibilityIcons = NULL;
    }

    this->MultiColumnList->Delete();

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

}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::AddGUIObservers ( )
{
    std::cout << "vtkSlicerFiducialsGUI: AddGUIObservers\n";
    this->FiducialListSelectorWidget->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
    this->AddFiducialListButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->AddFiducialButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

    this->VisibilityToggle->AddObserver (vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
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
//        modelLogic->AddFiducials();

        this->SetFiducialListNode(modelLogic->AddFiducials());

        // now update the fiducial list pane for the new list
        this->ProcessMRMLEvents(this->FiducialListSelectorWidget->GetSelected(), vtkCommand::ModifiedEvent, callData);
        
        std::cerr << "Done adding fiducials list\n";
        return;
    }
  if (button == this->AddFiducialButton  && event ==  vtkKWPushButton::InvokedEvent)
    {
       std::cout << "vtkSlicerFiducialsGUI: ProcessGUIEvent: Add Fiducial Button event: " << event << ".\n";

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
        
        vtkMRMLFiducialNode *modelNode = modelLogic->AddFiducial( );
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
        if (modelNode->GetName() != NULL)
        {
            this->MultiColumnList->GetWidget()->SetCellText(row,0,modelNode->GetName());
        }
        else
        {
            std::cerr << "WARNING: new fiducial doesn't have a name\n";
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
        this->MultiColumnList->GetWidget()->SetCellEditWindowToCheckButton(row,8);
        this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,8,(modelNode->GetSelected() ? 1 : 0));
    }
  if (button == this->GetVisibilityToggle()  && event ==  vtkKWPushButton::InvokedEvent)
    {
       std::cout << "vtkSlicerFiducialsGUI: ProcessGUIEvent: Visibility  button event: " << event << ".\n";
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
       // change the visibility
        modelLogic->GetActiveFiducialListNode()->SetVisibility( ! modelLogic->GetActiveFiducialListNode()->GetVisibility());
        // update the icon via  process mrml event that should get pushed
        this->ProcessMRMLEvents(caller, event, callData); //modelLogic->GetActiveFiducialListNode(), event, callData);

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
        // update the table 
        return;
    }
    if (node == this->FiducialListSelectorWidget->GetSelected() && event == vtkCommand::ModifiedEvent)
    {
        std::cout << "\tmodified event on the fiducial list selected node.\n";
        //std::cout << "\t\t selected = " << this->FiducialListSelectorWidget->GetSelected() << "\n";
        if (this->GetLogic())
        {
            //std::cout << "\t\t active = " << this->GetLogic()->GetActiveFiducialListNode() << "\n";
            // when the list is created, it's set active, but when select it
            // from the drop down menu, need to set it active
            if ( this->GetLogic()->GetActiveFiducialListNode() !=  this->FiducialListSelectorWidget->GetSelected())
            {
                // select it first off
                this->GetLogic()->SetActiveFiducialListNode((vtkMRMLFiducialListNode *)(this->FiducialListSelectorWidget->GetSelected()));
                
            }

            // visib  will get handled below

            // clear out the multi column list box and fill it in with the
            // new list
            this->MultiColumnList->GetWidget()->DeleteAllRows();
            // get the points in the active fid list
            vtkMRMLFiducialListNode *listNode  = this->GetLogic()->GetActiveFiducialListNode();
            // add rows for each point
            int numPoints = listNode->GetNumberOfFiducialNodes();
            float *xyz;
            float *wxyz;
            for (int row = 0; row < numPoints; row++)
            {
                //std::cout << "Adding point " << row << " to the table" << endl;
                vtkMRMLFiducialNode * pointNode = listNode->GetNthFiducialNode(row);
                // add a row for this point
                this->MultiColumnList->GetWidget()->AddRow();
                
                // now populate it
                xyz = pointNode->GetXYZ();
                wxyz = pointNode->GetOrientationWXYZ();
                
                if (pointNode->GetName() != NULL)
                {
                    this->MultiColumnList->GetWidget()->SetCellText(row,0,pointNode->GetName());
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
            }
        }
    }

    // update the visibility button to match the logic state
    vtkSlicerFiducialsLogic* modelLogic = this->GetLogic();
    if ((modelLogic != NULL))
        //&& (node == modelLogic->GetActiveFiducialListNode()) &&
        //(event == vtkKWPushButton::InvokedEvent))
    {
        //std::cout << "\tupdating the visibility button\n";
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
    this->FiducialListSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->FiducialListSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
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

    // text color

    // symbol scale

    // symbol color
    
    // add the multicolumn list to show the points
    this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
    this->MultiColumnList->SetParent ( listFrame->GetFrame() );
    this->MultiColumnList->Create ( );
    this->MultiColumnList->SetHeight(4);
    this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
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

    // now set the attributes that are equal across the columns
    this->MultiColumnList->GetWidget()->SetColumnLabelBackgroundColor(0.5,1.0,0.5);
    this->MultiColumnList->GetWidget()->SetSelectionBackgroundColor(0.5,0.5,1.0);
    this->MultiColumnList->GetWidget()->SetSelectionForegroundColor(0.0,0.0,0.0);
    int col;
    for (col = 0; col < this->NumberOfColumns; col++)
    {        
        this->MultiColumnList->GetWidget()->SetColumnWidth(col, 8);
        this->MultiColumnList->GetWidget()->SetColumnAlignmentToLeft(col);
        this->MultiColumnList->GetWidget()->ColumnEditableOn(col);
        if (0 && col % 2 == 0)
        {
            // every other row is a different colour, and that over rides this
            this->MultiColumnList->GetWidget()->SetColumnBackgroundColor(col, 0.8, 0.8, 0.8);
        }
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

     
    displayFrame->Delete ( );
    listFrame->Delete();
    modHelpFrame->Delete ( );
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::UpdateElement(int row, int col, double val)
{
    std::cout << "vtkSlicerFiducialsGUI: UpdateElement " << row << " " << col << " " << val << "\n";
    cout << "row = " << row << ", col = " << col << ", val = " << val << endl;
    UpdateVTK();
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
        this->ProcessMRMLEvents(fiducialListNode, vtkCommand::ModifiedEvent, NULL);
    }
}
