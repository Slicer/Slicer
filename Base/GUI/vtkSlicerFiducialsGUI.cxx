#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"

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
    this->AddFiducialsButton = NULL;

//    this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ();
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
    
    if (this->AddFiducialsButton ) {
        this->AddFiducialsButton->Delete ( );
        this->AddFiducialsButton = NULL;
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
    this->AddFiducialsButton->RemoveObservers ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::AddGUIObservers ( )
{
    std::cout << "vtkSlicerFiducialsGUI: AddGUIObservers\n";
    this->FiducialListSelectorWidget->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
//    this->AddFiducialsButton->AddObserver ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->AddFiducialsButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
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
  if (button == this->AddFiducialsButton  && event ==  vtkKWPushButton::InvokedEvent)
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
            // make a new one
            modelLogic->AddFiducials();
            std::cerr << "Done adding fiducials list\n";
        }
        
        vtkMRMLFiducialNode *modelNode = modelLogic->AddFiducial( );
        if ( modelNode == NULL ) 
        {
            // TODO: generate an error...
            std::cerr << "ERROR adding a new fiducial point\n";
            return;
        }
        modelNode->SetName("newfid");
        
        std::cout << "Adding a row to the table...\n";
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
    }
    else
    {
        // std::cerr << "vtkSlicerFiducialsGUI ProcessMRMLEvent: UNKNOWN caller/event pair\n";
    }
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
    // text scale

    // text color

    // symbol scale

    // symbol color
    

    // ---
    // LIST FRAME
    vtkKWFrameWithLabel *listFrame = vtkKWFrameWithLabel::New();
    listFrame->SetParent( page );
    listFrame->Create();
    listFrame->SetLabelText("Fiducial List");
    listFrame->ExpandFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  listFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Fiducials")->GetWidgetName());
    
    // add the multicolumn list
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
    this->AddFiducialsButton = vtkKWPushButton::New ( );
    this->AddFiducialsButton->SetParent ( listFrame->GetFrame() );
    this->AddFiducialsButton->Create ( );
    this->AddFiducialsButton->SetText ("Add Fiducial Point");
    
    app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->AddFiducialsButton->GetWidgetName());

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
    
    std::cout << "SetFiducialListNode : setting the selector widget to " << fiducialListNode->GetID() << endl;
    
    this->FiducialListSelectorWidget->SetSelected(fiducialListNode);

    // set the member variables and do a first process
    if (fiducialListNode != NULL)
    {
        //std::cout << "SetFiducialListNode: setting the fid list node id to " << fiducialListNode->GetID() << "\n";
        //this->SetFiducialListNodeID(fiducialListNode->GetID());
        this->ProcessMRMLEvents(fiducialListNode, vtkCommand::ModifiedEvent, NULL);
    }
}
