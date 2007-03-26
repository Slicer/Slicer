#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"
#include "vtkSlicerVolumeFileHeaderWidget.h"


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
  
}


//---------------------------------------------------------------------------
vtkSlicerVolumeFileHeaderWidget::~vtkSlicerVolumeFileHeaderWidget ( )
{
  
  this->VolumeHeaderlessStorageNode->Delete();
  
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
} 



//---------------------------------------------------------------------------
void vtkSlicerVolumeFileHeaderWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeFileHeaderWidget::RemoveWidgetObservers ( ) 
{
}

//---------------------------------------------------------------------------
vtkMRMLVolumeHeaderlessStorageNode* vtkSlicerVolumeFileHeaderWidget::GetVolumeHeaderlessStorageNode ( )
{
  //this->VolumeHeaderlessStorageNode->SetFileLittleEndian();
  
  return this->VolumeHeaderlessStorageNode;
  
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeFileHeaderWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();
  
  // ---
  // DISPLAY FRAME            
  vtkKWFrame *volumeHeaderFrame = vtkKWFrame::New ( );
  volumeHeaderFrame->SetParent ( this->GetParent() );
  volumeHeaderFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 volumeHeaderFrame->GetWidgetName() );
  
  vtkKWFrame *dimensionFrame = vtkKWFrame::New ( );
  dimensionFrame->SetParent ( volumeHeaderFrame );
  dimensionFrame->Create ( );

  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 dimensionFrame->GetWidgetName() );

  vtkKWFrame *spacingFrame = vtkKWFrame::New ( );
  spacingFrame->SetParent ( volumeHeaderFrame );
  spacingFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 spacingFrame->GetWidgetName() );

  vtkKWFrame *frame = vtkKWFrame::New ( );
  frame->SetParent ( volumeHeaderFrame );
  frame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );


  this->DimensionEntry0 = vtkKWEntryWithLabel::New();
  this->DimensionEntry0->SetParent(dimensionFrame);
  this->DimensionEntry0->Create();
  this->DimensionEntry0->SetLabelText("Image Dimensions:");
  this->DimensionEntry0->SetLabelWidth(18);
  this->DimensionEntry0->GetWidget()->SetValueAsInt(0);
  this->DimensionEntry0->GetWidget()->SetWidth(8);

  this->DimensionEntry1 = vtkKWEntry::New();
  this->DimensionEntry1->SetParent(dimensionFrame);
  this->DimensionEntry1->Create();
  this->DimensionEntry1->SetValueAsInt(0);
  this->DimensionEntry1->SetWidth(8);

  this->Script ( "pack %s %s %s -side left -anchor nw -padx 2 -pady 2 -expand n",
                 this->DimensionEntry0->GetWidgetName ( ),
                 this->DimensionEntry1->GetWidgetName());

  this->SpacingEntry0 = vtkKWEntryWithLabel::New();
  this->SpacingEntry0->SetParent(spacingFrame);
  this->SpacingEntry0->Create();
  this->SpacingEntry0->SetLabelText("Pixel Size:");
  this->SpacingEntry0->GetWidget()->SetValueAsInt(0);
  this->SpacingEntry0->SetLabelWidth(18);
  this->SpacingEntry0->GetWidget()->SetWidth(8);
 
  this->SpacingEntry1 = vtkKWEntry::New();
  this->SpacingEntry1->SetParent(spacingFrame);
  this->SpacingEntry1->Create();
  this->SpacingEntry1->SetValueAsInt(0);
  this->SpacingEntry1->SetWidth(8);
  
  this->Script ( "pack %s %s %s -side left -anchor nw -padx 2 -pady 2 -expand n",
                 this->SpacingEntry0->GetWidgetName ( ),
                 this->SpacingEntry1->GetWidgetName());

  this->SliceThicknessEntry = vtkKWEntryWithLabel::New();
  this->SliceThicknessEntry->SetParent(frame);
  this->SliceThicknessEntry->Create();
  this->SliceThicknessEntry->SetLabelText("Slice Thickness:");
  this->SliceThicknessEntry->GetWidget()->SetValue("");
  this->SliceThicknessEntry->SetWidth(48);
  this->SliceThicknessEntry->SetLabelWidth(18);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->SliceThicknessEntry->GetWidgetName());

  this->SliceSpacingEntry = vtkKWEntryWithLabel::New();
  this->SliceSpacingEntry->SetParent(frame);
  this->SliceSpacingEntry->Create();
  this->SliceSpacingEntry->SetLabelText("Slice Spacing:");
  this->SliceSpacingEntry->GetWidget()->SetValue("");
  this->SliceSpacingEntry->SetWidth(48);
  this->SliceSpacingEntry->SetLabelWidth(18);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->SliceSpacingEntry->GetWidgetName());

  

  this->ScanOrderMenu = vtkKWMenuButtonWithLabel::New();
  this->ScanOrderMenu->SetParent(frame);
  this->ScanOrderMenu->Create();
  this->ScanOrderMenu->SetLabelText("Scan Order:");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Sagital LR");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Coronal PA");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Axial IS");
  this->ScanOrderMenu->GetWidget()->SetValue ( "Sagital LR" );
  this->ScanOrderMenu->SetLabelWidth(18);
  this->ScanOrderMenu->SetWidth(48);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->ScanOrderMenu->GetWidgetName());

  this->ScalarTypeMenu = vtkKWMenuButtonWithLabel::New();
  this->ScalarTypeMenu->SetParent(frame);
  this->ScalarTypeMenu->Create();
  this->ScalarTypeMenu->SetLabelText("Scalar Type:");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Char");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "UnsignedChar");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Short");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "UnsignedShort");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Int");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "UnsignedInt");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Long");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "UnsignedLong");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Float");
  this->ScanOrderMenu->GetWidget()->GetMenu()->AddRadioButton ( "Double");
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
  this->NumScalarsEntry->GetWidget()->SetValueAsInt(0);
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


  volumeHeaderFrame->Delete();
  dimensionFrame->Delete();
  spacingFrame->Delete();
  frame->Delete();
  
}
