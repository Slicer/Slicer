/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkLabelStatisticsGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkLabelStatisticsGUI.h"
#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWMultiColumnList.h"
#include "vtkMRMLLabelStatisticsNode.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWTopLevel.h"
//#include "vtkSlicerWindow.h" 

//------------------------------------------------------------------------------
vtkLabelStatisticsGUI* vtkLabelStatisticsGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkLabelStatisticsGUI");
  if(ret)
    {
      return (vtkLabelStatisticsGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkLabelStatisticsGUI;
}


//----------------------------------------------------------------------------
vtkLabelStatisticsGUI::vtkLabelStatisticsGUI()
{
  this->GrayscaleSelector = vtkSlicerNodeSelectorWidget::New();
  this->LabelmapSelector = vtkSlicerNodeSelectorWidget::New();
  this->ApplyButton = vtkKWPushButton::New();
  this->SaveToFile = vtkKWLoadSaveButton::New();
  //this->VolStatsResult = vtkKWText::New();
  this->ResultList = vtkKWMultiColumnList::New();
  // this->SaveToClipboardButton = vtkKWPushButton::New();
  this->Logic = NULL;
  this->LabelStatisticsNode = NULL;
  this->SetPrimarySelectionTclProcedures();
}

//----------------------------------------------------------------------------
vtkLabelStatisticsGUI::~vtkLabelStatisticsGUI()
{
  if ( this->GrayscaleSelector ) 
    {
    this->GrayscaleSelector->SetParent(NULL);
    this->GrayscaleSelector->Delete();
    this->GrayscaleSelector = NULL;
    }
  if ( this->LabelmapSelector ) 
    {
    this->LabelmapSelector->SetParent(NULL);
    this->LabelmapSelector->Delete();
    this->LabelmapSelector = NULL;
    }
  if ( this->ApplyButton ) 
    {
    this->ApplyButton->SetParent(NULL);
    this->ApplyButton->Delete();
    this->ApplyButton = NULL;
    }
 //  if ( this->SaveToClipboardButton ) 
//     {
//     this->SaveToClipboardButton->SetParent(NULL);
//     this->SaveToClipboardButton->Delete();
//     this->SaveToClipboardButton = NULL;
//     }
  if ( this->SaveToFile ) 
    {
      this->SaveToFile->SetParent(NULL);
      this->SaveToFile->Delete();
      this->SaveToFile = NULL;
    }
  //  if ( this->VolStatsResult ) 
//     {
//     this->VolStatsResult->SetParent(NULL);
//     this->VolStatsResult->Delete();
//     this->VolStatsResult = NULL;
//     }
  if ( this->ResultList ) 
    {
    this->ResultList->SetParent(NULL);
    this->ResultList->Delete();
    this->ResultList = NULL;
    }
  
  this->SetLogic (NULL);
  
  if ( this->LabelStatisticsNode ) 
    {
    this->LabelStatisticsNode->Delete();
    vtkSetMRMLNodeMacro(this->LabelStatisticsNode, NULL);
    }
}

//----------------------------------------------------------------------------
void vtkLabelStatisticsGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkLabelStatisticsGUI::AddGUIObservers ( ) 
{
  this->GrayscaleSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->LabelmapSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  // this->SaveToClipboardButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveToFile->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->Logic->AddObserver (vtkLabelStatisticsLogic::LabelStatsOuterLoop, (vtkCommand *)this->LogicCallbackCommand );
  this->Logic->AddObserver (vtkLabelStatisticsLogic::LabelStatsInnerLoop, (vtkCommand *)this->LogicCallbackCommand );
  this->Logic->AddObserver (vtkLabelStatisticsLogic::StartLabelStats, (vtkCommand *)this->LogicCallbackCommand );
  this->Logic->AddObserver (vtkLabelStatisticsLogic::EndLabelStats, (vtkCommand *)this->LogicCallbackCommand );
}



//---------------------------------------------------------------------------
void vtkLabelStatisticsGUI::RemoveGUIObservers ( )
{
  this->GrayscaleSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->LabelmapSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->ApplyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  //this->SaveToClipboardButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SaveToFile->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->Logic->RemoveObservers (vtkLabelStatisticsLogic::LabelStatsOuterLoop, (vtkCommand *)this->LogicCallbackCommand );
  this->Logic->RemoveObservers (vtkLabelStatisticsLogic::LabelStatsInnerLoop, (vtkCommand *)this->LogicCallbackCommand );
  this->Logic->RemoveObservers (vtkLabelStatisticsLogic::StartLabelStats, (vtkCommand *)this->LogicCallbackCommand );
  this->Logic->RemoveObservers (vtkLabelStatisticsLogic::EndLabelStats, (vtkCommand *)this->LogicCallbackCommand );
}


//---------------------------------------------------------------------------
void vtkLabelStatisticsGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  vtkKWLoadSaveDialog *loadSaveDialog = vtkKWLoadSaveDialog::SafeDownCast(caller);
  
  if (selector == this->GrayscaleSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->GrayscaleSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML();
    }
  else if (selector == this->LabelmapSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->LabelmapSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML();
    }
 if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->ResultList->DeleteAllRows();
    this->UpdateMRML();
    this->Logic->Apply();
    }
//  if (b == this->SaveToClipboardButton && event == vtkKWPushButton::InvokedEvent ) 
//     {
//       vtkMRMLLabelStatisticsNode* n = this->GetLabelStatisticsNode();
//       this->SetPrimarySelection(n->GetResultText());
//     }
 if (loadSaveDialog == this->SaveToFile->GetLoadSaveDialog() && event == vtkKWTopLevel::WithdrawEvent ) 
   {
     const char *fileName = this->SaveToFile->GetFileName();
     if ( fileName ) 
       {
         //std::cout << "This is the filename: "<<  this->SaveToFile->GetFileName() << "\n";
         vtkMRMLLabelStatisticsNode* n = this->GetLabelStatisticsNode();
         n->SaveResultToTextFile(fileName);
       }
   }
}

//---------------------------------------------------------------------------
void vtkLabelStatisticsGUI::UpdateMRML ()
{
  //std::cout <<"UpdateMRML gets called!" << "\n";
  vtkMRMLLabelStatisticsNode* n = this->GetLabelStatisticsNode();
  if (n == NULL)
    {
    //no parameter node selected yet, create new
    vtkMRMLLabelStatisticsNode* volumeMathNode = vtkMRMLLabelStatisticsNode::New();
    n = volumeMathNode;
    //set an observe new node in Logic
    this->Logic->SetAndObserveLabelStatisticsNode(volumeMathNode);
    vtkSetAndObserveMRMLNodeMacro(this->LabelStatisticsNode, volumeMathNode);
    }
  
  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);
  // set node parameters from GUI widgets
  if (this->GrayscaleSelector->GetSelected() != NULL)
    {
    n->SetInputGrayscaleRef(this->GrayscaleSelector->GetSelected()->GetID());
    }

  if (this->LabelmapSelector->GetSelected() != NULL)
    {
    n->SetInputLabelmapRef(this->LabelmapSelector->GetSelected()->GetID());
    }
}

//---------------------------------------------------------------------------
void vtkLabelStatisticsGUI::UpdateGUI ()
{ 
  //std::cout <<"UpdateGUI gets called!" << "\n";
  vtkMRMLLabelStatisticsNode* n = this->GetLabelStatisticsNode();
  if (n != NULL)
    {
      // this->VolStatsResult->SetText(n->GetResultText());
    if(!n->LabelStats.empty()) 
      { 
      typedef std::list<vtkMRMLLabelStatisticsNode::LabelStatsEntry>::const_iterator LI;
        int i = 0;
        for (LI li = n->LabelStats.begin(); li != n->LabelStats.end(); ++li)
          {
           const vtkMRMLLabelStatisticsNode::LabelStatsEntry& label = *li;  
           //  std::cout << "This is i: " << i <<std::endl;
           // std::cout << "Label: " << label.Label << "\tMin: " << label.Min ;
           // std::cout << "\tMax: " << label.Max << "\tMean: " << label.Mean << std::endl ;
           
           this->ResultList->InsertCellTextAsInt(i, 0, label.Label);
           this->ResultList->InsertCellTextAsInt(i, 1, label.Count);
           
           this->ResultList->InsertCellTextAsInt(i, 2, label.Min);
           this->ResultList->InsertCellTextAsInt(i, 3, label.Max);
           this->ResultList->InsertCellTextAsDouble(i, 4, label.Mean);
           this->ResultList->InsertCellTextAsDouble(i, 5, label.StdDev);
           i++;
          }
      }
    }
}

//---------------------------------------------------------------------------
void vtkLabelStatisticsGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  //std::cout <<"ProcessMRMLEvents gets called!" << "\n";
  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLLabelStatisticsNode* node = vtkMRMLLabelStatisticsNode::SafeDownCast(caller);
  if (node != NULL && this->GetLabelStatisticsNode() == node) 
   {
   this->UpdateGUI();
   }
}

//---------------------------------------------------------------------------
void vtkLabelStatisticsGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkMRMLLabelStatisticsNode* n = vtkMRMLLabelStatisticsNode::New();
  this->Logic->GetMRMLScene()->RegisterNodeClass(n);
  n->Delete();

  this->UIPanel->AddPage ( "LabelStatistics", "LabelStatistics", NULL );
  // ---
  // MODULE GUI FRAME 
  // ---
   // Define your help text and build the help frame here.
  const char *help = "The LabelStatistics module....";
  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "LabelStatistics" );
  this->BuildHelpAndAboutFrame ( page, help, about );
    
  vtkSlicerModuleCollapsibleFrame *moduleFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "LabelStatistics" ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText ("Label Statistics");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("LabelStatistics")->GetWidgetName());
  
  this->GrayscaleSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->GrayscaleSelector->SetParent( moduleFrame->GetFrame() );
  this->GrayscaleSelector->Create();
  this->GrayscaleSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->GrayscaleSelector->UpdateMenu();

  this->GrayscaleSelector->SetBorderWidth(2);
  this->GrayscaleSelector->SetLabelText( "Input Grayscale Volume: ");
  this->GrayscaleSelector->SetBalloonHelpString("Select an input grayscale volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->GrayscaleSelector->GetWidgetName());
  
  this->LabelmapSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->LabelmapSelector->SetParent( moduleFrame->GetFrame() );
  this->LabelmapSelector->Create();
  this->LabelmapSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->LabelmapSelector->UpdateMenu();

  this->LabelmapSelector->SetBorderWidth(2);
  this->LabelmapSelector->SetLabelText( "Input Labelmap: ");
  this->LabelmapSelector->SetBalloonHelpString("Select an input labelmap from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->LabelmapSelector->GetWidgetName());


  this->ApplyButton->SetParent( moduleFrame->GetFrame() );
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth ( 8 );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                this->ApplyButton->GetWidgetName());

//   this->VolStatsResult->SetParent( moduleFrame->GetFrame());
//   this->VolStatsResult->Create();
//   this->VolStatsResult->SetWidth(0);
//   this->VolStatsResult->SetHeight(7);
//   this->VolStatsResult->SetWrapToChar();
//   this->VolStatsResult->ReadOnlyOn();
//   app->Script(
//     "pack %s -side top -anchor e -expand n -fill x -padx 2 -pady 6", 
//     VolStatsResult->GetWidgetName());
 
  this->ResultList->SetParent( moduleFrame->GetFrame());
  this->ResultList->Create();
  
  // this->ResultList->MovableColumnsOn();
  this->ResultList->SetWidth(0);
  this->ResultList->SetHeight(7);
  
  int col_index;

  // Add the columns (make some of them editable)

  col_index = this->ResultList->AddColumn("Label");
  this->ResultList->ColumnEditableOn(col_index);

  col_index = this->ResultList->AddColumn("Count");
  this->ResultList->ColumnEditableOn(col_index);

  col_index = this->ResultList->AddColumn("Min");
  this->ResultList->ColumnEditableOn(col_index);
  
  col_index = this->ResultList->AddColumn("Max");
  this->ResultList->ColumnEditableOn(col_index);
  
  col_index = this->ResultList->AddColumn("Mean");
  this->ResultList->ColumnEditableOn(col_index);
  
  col_index = this->ResultList->AddColumn("StdDev");
  this->ResultList->ColumnEditableOn(col_index);

  app->Script(
    "pack %s -side top -anchor e  -padx 20 -pady 10", 
    this->ResultList->GetWidgetName());
 
  // Create the button to copy result to clipboard
  // this->SaveToClipboardButton->SetParent( moduleFrame->GetFrame() );
//   this->SaveToClipboardButton->Create();
//   this->SaveToClipboardButton->SetText("Copy result to clipboard");
//   this->SaveToClipboardButton->SetWidth ( 20 );

  this->SaveToFile->SetParent( moduleFrame->GetFrame() );
  this->SaveToFile->Create();
  this->SaveToFile->SetText("Save to file");
  this->SaveToFile->GetLoadSaveDialog()->SaveDialogOn(); // load mode
  
  this->SaveToFile->GetLoadSaveDialog()->SetFileTypes("{{Text Document} {.txt}}");
  this->SaveToFile->GetLoadSaveDialog()->SetInitialFileName("LabelStatistics.txt"); 
  this->SaveToFile->GetLoadSaveDialog()->SetDefaultExtension("txt");

  app->Script(
    "pack %s -side right -anchor w  -padx 20 -pady 10", 
    this->SaveToFile->GetWidgetName());
    // this->SaveToClipboardButton->GetWidgetName(),
    

  ///--------
  moduleFrame->Delete();
}

void vtkLabelStatisticsGUI::ProcessLogicEvents ( vtkObject *caller,
                                                  unsigned long event,
                                                  void *callData)
{
  vtkLabelStatisticsLogic* logic =  vtkLabelStatisticsLogic::SafeDownCast(caller);
  const char * callDataStr = (const char *)callData;
  
  std::string innerLoopMsg = "Computing Stats for ";

  vtkSlicerWindow* mainWindow = this->ApplicationGUI->GetMainSlicerWindow();
  vtkKWProgressGauge* progressGauge =  mainWindow->GetProgressGauge(); 
 
  if (event == vtkLabelStatisticsLogic::StartLabelStats)
    {
      //std::cout << "StartLabelStats\n"<< "\n";
      progressGauge->SetValue(0);
      progressGauge->SetNthValue(1, 0);

      mainWindow->SetStatusText("Start calculating ...");
    } 
  else if (event == vtkLabelStatisticsLogic::EndLabelStats)
    {
      //std::cout << "EndLabelStats\n"<< "\n";
      mainWindow->SetStatusText("Done");
    }
  else if (event == vtkLabelStatisticsLogic::LabelStatsOuterLoop) 
    {
      //std::cout << "LabelStatsOuterLoop\n"<< "\n";
      //std::cout << "This is the progress in GUI: "<< logic->GetProgress() << " .\n";
      progressGauge->SetValue(logic->GetProgress()*100);
      mainWindow->SetStatusText(innerLoopMsg.append( callDataStr ).c_str() );

    } 
  else if (event == vtkLabelStatisticsLogic::LabelStatsInnerLoop)  
    {
      //std::cout << "LabelStatsInnerLoop\n"<< "\n";
      std::stringstream ss ( callDataStr );
      double innerProg = 0;
      ss >> innerProg;
      progressGauge->SetNthValue(1,innerProg*100);
    }
  else 
    {
      std::cout << "Event: "<< event << " is not handled here.\n";
    }
  

}

void vtkLabelStatisticsGUI::SetPrimarySelectionTclProcedures( )
{
  std::string cmd;
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  Tcl_Interp *interp = app->GetMainInterp(); 

  //  # selectText "text" --
  //   #       Sets the value of the PRIMARY selection to "$text".
  //   #
  //   #       (Note: this doesn't really "set the value" of the selection.
  //   #       More precisely, it arranges to provide the value given
  //   #       when another client requests it.)
    
  cmd = "proc selectText {text} { \
         variable currentSelection; \
        set currentSelection $text; \
        selection handle -selection CLIPBOARD \".\"  primaryTransfer;           \
        selection own -selection CLIPBOARD -command lostSelection \".\"; }";
  //selection handle -selection PRIMARY \".\"  primaryTransfer;         \
  //     selection own -selection PRIMARY -command lostSelection \".\"; }";
  
  Tcl_Eval( interp, cmd.c_str() );
  cmd.clear();

  //  # The following will be called whenever a client requests the value
  //   # of the PRIMARY selection.  See selection(n) for a description
  //   # of 'offset' and 'maxChars'; we probably ought to do something
  //   # sensible with these parameters, but it's mostly safe to
  //   # just ignore them.
  //   #
  cmd = "proc primaryTransfer {offset maxChars} { \
           variable currentSelection;             \
           return $currentSelection;              \
          }";
  
  Tcl_Eval( interp, cmd.c_str() );
  cmd.clear();
  
  //  # This is called when we lose ownership of the selection:
  //   #
  cmd = "proc lostSelection {} { \
           variable currentSelection; \
           set currentSelection \"\"; \
         }";
  
  Tcl_Eval( interp, cmd.c_str() );
  cmd.clear();
}


void vtkLabelStatisticsGUI::SetPrimarySelection( std::string text )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  Tcl_Interp *interp = app->GetMainInterp(); 
  std::string cmd = "selectText \"";
  cmd.append(text);
  cmd.append("\"");
  Tcl_Eval( interp, cmd.c_str() );
  cmd.clear();
}

