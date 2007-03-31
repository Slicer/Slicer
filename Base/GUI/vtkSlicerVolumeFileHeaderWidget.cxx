#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"
#include "vtkSlicerVolumeFileHeaderWidget.h"

#include "vtkKWDialog.h"
#include "vtkKWLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWPushButton.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerVolumeFileHeaderWidget );
vtkCxxRevisionMacro ( vtkSlicerVolumeFileHeaderWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerVolumeFileHeaderWidget::vtkSlicerVolumeFileHeaderWidget ( )
{
  this->VolumeHeaderlessStorageNode = vtkMRMLVolumeHeaderlessStorageNode::New();
  
  this->HeaderDialog = NULL;
  this->InfoLabel = NULL;

  this->DimensionEntry0 = NULL;
  this->DimensionEntry1 = NULL;
  
  this->SpacingEntry0 = NULL;
  this->SpacingEntry1 = NULL;

  this->SliceThicknessEntry = NULL;
  this->SliceSpacingEntry = NULL;
  
  this->NumScalarsEntry = NULL;
  
  this->ScanOrderMenu = NULL;
  
  this->ScalarTypeMenu = NULL;
  
  this->LittleEndianCheckButton = NULL;
  this->OkButton = NULL;
  this->CancelButton = NULL;
  
}


//---------------------------------------------------------------------------
vtkSlicerVolumeFileHeaderWidget::~vtkSlicerVolumeFileHeaderWidget ( )
{
  this->RemoveWidgetObservers();

  this->VolumeHeaderlessStorageNode->Delete();
  
  if (this->HeaderDialog)
    {
    this->HeaderDialog->SetParent(NULL);
    this->HeaderDialog->Delete();
    }
  if (this->InfoLabel)
    {
    this->InfoLabel->SetParent(NULL);
    this->InfoLabel->Delete();
    }
  if (this->OkButton)
    {
    this->OkButton->SetParent(NULL);
    this->OkButton->Delete();
    }
  if (this->CancelButton)
    {
    this->CancelButton->SetParent(NULL);
    this->CancelButton->Delete();
    }
  if (this->DimensionEntry0)
    {
    this->DimensionEntry0->SetParent(NULL);
    this->DimensionEntry0->Delete();
    }
  if (this->DimensionEntry1)
    {
    this->DimensionEntry1->SetParent(NULL);
    this->DimensionEntry1->Delete();
    }
   
  if (this->SpacingEntry0)
    {
    this->SpacingEntry0->SetParent(NULL);
    this->SpacingEntry0->Delete();
    }
  if (this->SpacingEntry1)
    {
    this->SpacingEntry1->SetParent(NULL);
    this->SpacingEntry1->Delete();
    }
  
  if (this->ScanOrderMenu)
    {
    this->ScanOrderMenu->SetParent(NULL);
    this->ScanOrderMenu->Delete();
    }
  
  if (this->NumScalarsEntry)
    {
    this->NumScalarsEntry->SetParent(NULL);
    this->NumScalarsEntry->Delete();
    }
  
  
  if (this->ScalarTypeMenu)
    {
    this->ScalarTypeMenu->SetParent(NULL);
    this->ScalarTypeMenu->Delete();
    }
  
  if (this->SliceThicknessEntry)
    {
    this->SliceThicknessEntry->SetParent(NULL);
    this->SliceThicknessEntry->Delete();
    }

  if (this->SliceSpacingEntry)
    {
    this->SliceSpacingEntry->SetParent(NULL);
    this->SliceSpacingEntry->Delete();
    }

  if (this->LittleEndianCheckButton)
    {
    this->LittleEndianCheckButton->SetParent(NULL);
    this->LittleEndianCheckButton->Delete();
    }

  this->SetMRMLScene ( NULL );  
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeFileHeaderWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerVolumeFileHeaderWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeFileHeaderWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  if (this->OkButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    {
    this->HeaderDialog->OK();
    this->InvokeEvent(vtkSlicerVolumeFileHeaderWidget::FileHeaderOKEvent);
    }
  else if (this->CancelButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    { 
    this->HeaderDialog->Cancel();
    }
} 



//---------------------------------------------------------------------------
void vtkSlicerVolumeFileHeaderWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeFileHeaderWidget::RemoveWidgetObservers ( ) 
{
  this->OkButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->CancelButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
vtkMRMLVolumeHeaderlessStorageNode* vtkSlicerVolumeFileHeaderWidget::GetVolumeHeaderlessStorageNode ( )
{
  this->VolumeHeaderlessStorageNode->SetFileLittleEndian(this->LittleEndianCheckButton->GetSelectedState());
  this->VolumeHeaderlessStorageNode->SetFileDimensions(this->DimensionEntry0->GetWidget()->GetValueAsInt(), 
                                                       this->DimensionEntry1->GetValueAsInt(), 
                                                       0);

  this->VolumeHeaderlessStorageNode->SetFileNumberOfScalarComponents(this->NumScalarsEntry->GetWidget()->GetValueAsInt());
  this->VolumeHeaderlessStorageNode->SetFileScalarTypeAsString(this->ScalarTypeMenu->GetWidget()->GetValue());
  this->VolumeHeaderlessStorageNode->SetFileScanOrder(this->ScanOrderMenu->GetWidget()->GetValue());
  this->VolumeHeaderlessStorageNode->SetFileSpacing(this->SpacingEntry0->GetWidget()->GetValueAsDouble(),
                                                    this->SpacingEntry1->GetValueAsDouble(),
                                                    this->SliceThicknessEntry->GetWidget()->GetValueAsDouble() +
                                                    this->SliceSpacingEntry->GetWidget()->GetValueAsDouble());


  return this->VolumeHeaderlessStorageNode;
  
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeFileHeaderWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    return;
    }

  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();
  
  this->HeaderDialog = vtkKWDialog::New();
  this->HeaderDialog->SetParent ( this->GetParent());
  this->HeaderDialog->SetTitle("Volume File Header");
  this->HeaderDialog->Create ( );
  this->HeaderDialog->SetSize(350, 350);
  
  this->InfoLabel = vtkKWLabel::New();
  this->InfoLabel->SetParent(this->HeaderDialog);
  this->InfoLabel->Create();
  this->InfoLabel->SetWidth(100);
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->InfoLabel->GetWidgetName() );

  vtkKWFrame *dimensionFrame = vtkKWFrame::New ( );
  dimensionFrame->SetParent ( this->HeaderDialog );
  dimensionFrame->Create ( );

  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 dimensionFrame->GetWidgetName() );

  vtkKWFrame *spacingFrame = vtkKWFrame::New ( );
  spacingFrame->SetParent ( this->HeaderDialog );
  spacingFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 spacingFrame->GetWidgetName() );

  vtkKWFrame *frame = vtkKWFrame::New ( );
  frame->SetParent ( this->HeaderDialog );
  frame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );

  vtkKWFrame *saveFrame = vtkKWFrame::New ( );
  saveFrame->SetParent ( this->HeaderDialog );
  saveFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 saveFrame->GetWidgetName() );


  this->DimensionEntry0 = vtkKWEntryWithLabel::New();
  this->DimensionEntry0->SetParent(dimensionFrame);
  this->DimensionEntry0->Create();
  this->DimensionEntry0->SetLabelText("Image Dimensions:");
  this->DimensionEntry0->SetLabelWidth(18);
  this->DimensionEntry0->GetWidget()->SetValueAsInt(256);
  this->DimensionEntry0->GetWidget()->SetWidth(8);

  this->DimensionEntry1 = vtkKWEntry::New();
  this->DimensionEntry1->SetParent(dimensionFrame);
  this->DimensionEntry1->Create();
  this->DimensionEntry1->SetValueAsInt(256);
  this->DimensionEntry1->SetWidth(8);

  this->Script ( "pack %s %s -side left -anchor nw -padx 2 -pady 2 -expand n",
                 this->DimensionEntry0->GetWidgetName ( ),
                 this->DimensionEntry1->GetWidgetName());

  this->SpacingEntry0 = vtkKWEntryWithLabel::New();
  this->SpacingEntry0->SetParent(spacingFrame);
  this->SpacingEntry0->Create();
  this->SpacingEntry0->SetLabelText("Pixel Size:");
  this->SpacingEntry0->GetWidget()->SetValueAsDouble(0.9375);
  this->SpacingEntry0->SetLabelWidth(18);
  this->SpacingEntry0->GetWidget()->SetWidth(8);
 
  this->SpacingEntry1 = vtkKWEntry::New();
  this->SpacingEntry1->SetParent(spacingFrame);
  this->SpacingEntry1->Create();
  this->SpacingEntry1->SetValueAsDouble(0.9375);
  this->SpacingEntry1->SetWidth(8);
  
  this->Script ( "pack %s %s -side left -anchor nw -padx 2 -pady 2 -expand n",
                 this->SpacingEntry0->GetWidgetName ( ),
                 this->SpacingEntry1->GetWidgetName());

  this->SliceThicknessEntry = vtkKWEntryWithLabel::New();
  this->SliceThicknessEntry->SetParent(frame);
  this->SliceThicknessEntry->Create();
  this->SliceThicknessEntry->SetLabelText("Slice Thickness:");
  this->SliceThicknessEntry->GetWidget()->SetValueAsDouble(1.5);
  this->SliceThicknessEntry->SetWidth(48);
  this->SliceThicknessEntry->SetLabelWidth(18);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->SliceThicknessEntry->GetWidgetName());

  this->SliceSpacingEntry = vtkKWEntryWithLabel::New();
  this->SliceSpacingEntry->SetParent(frame);
  this->SliceSpacingEntry->Create();
  this->SliceSpacingEntry->SetLabelText("Slice Spacing:");
  this->SliceSpacingEntry->GetWidget()->SetValueAsDouble(0.0);
  this->SliceSpacingEntry->SetWidth(48);
  this->SliceSpacingEntry->SetLabelWidth(18);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->SliceSpacingEntry->GetWidgetName());

  

  this->ScanOrderMenu = vtkKWMenuButtonWithLabel::New();
  this->ScanOrderMenu->SetParent(frame);
  this->ScanOrderMenu->Create();
  this->ScanOrderMenu->SetLabelText("Scan Order:");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Sagittal LR");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Sagittal RL");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Coronal PA");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Coronal AP");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Axial IS");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Axial SI");
  this->ScanOrderMenu->GetWidget()->SetValue ( "Sagittal LR" );
  this->ScanOrderMenu->SetLabelWidth(18);
  this->ScanOrderMenu->SetWidth(48);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->ScanOrderMenu->GetWidgetName());

  this->ScalarTypeMenu = vtkKWMenuButtonWithLabel::New();
  this->ScalarTypeMenu->SetParent(frame);
  this->ScalarTypeMenu->Create();
  this->ScalarTypeMenu->SetLabelText("Scalar Type:");
  this->ScalarTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "Char");
  this->ScalarTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "UnsignedChar");
  this->ScalarTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "Short");
  this->ScalarTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "UnsignedShort");
  this->ScalarTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "Int");
  this->ScalarTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "UnsignedInt");
  this->ScalarTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "Long");
  this->ScalarTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "UnsignedLong");
  this->ScalarTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "Float");
  this->ScalarTypeMenu->GetWidget()->GetMenu()->AddRadioButton ( "Double");
  this->ScalarTypeMenu->GetWidget()->SetValue("Short");
  this->ScalarTypeMenu->SetLabelWidth(18);
  this->ScalarTypeMenu->SetWidth(48);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->ScalarTypeMenu->GetWidgetName());


  this->NumScalarsEntry = vtkKWEntryWithLabel::New();
  this->NumScalarsEntry->SetParent(frame);
  this->NumScalarsEntry->Create();
  this->NumScalarsEntry->SetLabelText("Number of Scalars:");
  this->NumScalarsEntry->GetWidget()->SetValueAsInt(1);
  this->NumScalarsEntry->SetLabelWidth(18);
  this->NumScalarsEntry->SetWidth(48);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->NumScalarsEntry->GetWidgetName());

  // little endian?
  this->LittleEndianCheckButton = vtkKWCheckButton::New();
  this->LittleEndianCheckButton->SetParent(frame);
  this->LittleEndianCheckButton->Create();
  this->LittleEndianCheckButton->SelectedStateOff();
  this->LittleEndianCheckButton->SetText("Little Endian(PC, SGI):");
  //this->LittleEndianCheckButton->SetWidth(48);  
  this->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->LittleEndianCheckButton->GetWidgetName());


  // add OK button
  this->OkButton = vtkKWPushButton::New ( );
  this->OkButton->SetParent ( saveFrame );
  this->OkButton->Create ( );
  this->OkButton->SetText ("Read");
  this->Script("pack %s -side left -anchor w -padx 2 -pady 4", 
              this->OkButton->GetWidgetName());

  // add Cancel button
  this->CancelButton = vtkKWPushButton::New ( );
  this->CancelButton->SetParent ( saveFrame );
  this->CancelButton->Create ( );
  this->CancelButton->SetText ("Cancel");
  this->Script("pack %s -side left -anchor w -padx 36 -pady 4", 
              this->CancelButton->GetWidgetName());


  dimensionFrame->Delete();
  spacingFrame->Delete();
  frame->Delete();
  saveFrame->Delete();

  this->OkButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->CancelButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

 
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeFileHeaderWidget::Invoke ( )
{
  if (this->HeaderDialog != NULL)
    {
    this->HeaderDialog->Invoke ( );
    }
}

void vtkSlicerVolumeFileHeaderWidget::SetInfo(char * info)
{
  if (this->InfoLabel != NULL)
    {
    this->InfoLabel->SetText(info);
    }
}
