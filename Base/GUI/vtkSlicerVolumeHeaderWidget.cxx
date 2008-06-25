#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerVolumeHeaderWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonWithLabel.h"

#include <itksys/SystemTools.hxx> 

#include <vtksys/stl/string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerVolumeHeaderWidget );
vtkCxxRevisionMacro ( vtkSlicerVolumeHeaderWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerVolumeHeaderWidget::vtkSlicerVolumeHeaderWidget ( )
{
  this->VolumeNode = NULL;
  this->VolumeSelectorWidget = NULL;
  
  this->DimensionEntry0 = NULL;
  this->DimensionEntry1 = NULL;
  this->DimensionEntry2 = NULL;
  
  this->OriginEntry0 = NULL;
  this->OriginEntry1 = NULL;
  this->OriginEntry2 = NULL;
  
  this->SpacingEntry0 = NULL;
  this->SpacingEntry1 = NULL;
  this->SpacingEntry2 = NULL;

  this->CenterButton = NULL;
  
  this->ScanOrderEntry = NULL;
  
  this->NumScalarsEntry = NULL;
  
  this->ScalarTypeEntry = NULL;
  
  this->FileNameEntry = NULL;

  this->AddNodeSelectorWidget = 0;

  this->UpdatingFromMRML = 0;
}


//---------------------------------------------------------------------------
vtkSlicerVolumeHeaderWidget::~vtkSlicerVolumeHeaderWidget ( )
{
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->SetParent(NULL);
    this->VolumeSelectorWidget->Delete();
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
  if (this->DimensionEntry2)
    {
    this->DimensionEntry2->SetParent(NULL);
    this->DimensionEntry2->Delete();
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
  if (this->SpacingEntry2)
    {
    this->SpacingEntry2->SetParent(NULL);
    this->SpacingEntry2->Delete();
    }

  if (this->CenterButton)
    {
    this->CenterButton->SetParent(NULL);
    this->CenterButton->Delete();
    }
   
  if (this->OriginEntry0)
    {
    this->OriginEntry0->SetParent(NULL);
    this->OriginEntry0->Delete();
    }
  if (this->OriginEntry1)
    {
    this->OriginEntry1->SetParent(NULL);
    this->OriginEntry1->Delete();
    }
  if (this->OriginEntry2)
    {
    this->OriginEntry2->SetParent(NULL);
    this->OriginEntry2->Delete();
    }
   
  if (this->ScanOrderEntry)
    {
    this->ScanOrderEntry->SetParent(NULL);
    this->ScanOrderEntry->Delete();
    }
  if (this->NumScalarsEntry)
    {
    this->NumScalarsEntry->SetParent(NULL);
    this->NumScalarsEntry->Delete();
    }
  if (this->ScalarTypeEntry)
    {
    this->ScalarTypeEntry->SetParent(NULL);
    this->ScalarTypeEntry->Delete();
    }
  if (this->FileNameEntry)
    {
    this->FileNameEntry->SetParent(NULL);
    this->FileNameEntry->Delete();
    }

  this->SetMRMLScene ( NULL );  
  if (this->VolumeNode)
    {
    vtkSetAndObserveMRMLNodeMacro(this->VolumeNode, NULL);
    }
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeHeaderWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerVolumeHeaderWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

void vtkSlicerVolumeHeaderWidget::SetVolumeNode ( vtkMRMLVolumeNode *volumeNode )
{ 
  // Select this volume node
  if (volumeNode == this->GetVolumeNode ()) 
    {
    return;
    }

  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->SetSelected(volumeNode); 
    }
  // observe node modified events are not being observed
  vtkIntArray  *events = vtkIntArray::New();
  events->InsertNextValue( vtkCommand::ModifiedEvent);
  events->InsertNextValue( vtkMRMLVolumeNode::DisplayModifiedEvent);
  events->InsertNextValue( vtkMRMLVolumeNode::ImageDataModifiedEvent);

  vtkSetAndObserveMRMLNodeEventsMacro ( this->VolumeNode, volumeNode, events );
  events->Delete();
    
  // 
  // Set the member variables and do a first process
  //
  if ( volumeNode != NULL)
    {
    this->ProcessMRMLEvents(volumeNode, vtkCommand::ModifiedEvent, NULL);
    }
}

//---------------------------------------------------------------------------
vtkMRMLVolumeNode * vtkSlicerVolumeHeaderWidget::GetVolumeNode ()
{
  vtkMRMLVolumeNode *volume;
  if (this->VolumeSelectorWidget)
    {
    volume = 
        vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
    }
  else
    {
    volume = this->VolumeNode;
    }
   return volume;
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode * vtkSlicerVolumeHeaderWidget::GetVolumeStorageNode ()
{ 
   vtkMRMLStorageNode *storage = NULL;
   vtkMRMLVolumeNode *volume = this->GetVolumeNode();
   if (volume != NULL)
      {
      storage = volume->GetStorageNode();
      }
   return storage;
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeHeaderWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{

  //
  // process volume selector events
  //
  if (this->VolumeSelectorWidget)
    {
    vtkSlicerNodeSelectorWidget *volSelector = 
      vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

    if (volSelector == this->VolumeSelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
      {
      vtkMRMLVolumeNode *volume = 
        vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());

      if (volume != NULL)
        {
        this->SetVolumeNode(volume);
        this->UpdateWidgetFromMRML();
        }
      }
    return;
    }

  // assume that we need to do this update for any entry event
  vtkKWEntry *entry = vtkKWEntry::SafeDownCast(caller);
  if (entry)
    {
    double origin[3], spacing[3];
    origin[0] = this->OriginEntry0->GetWidget()->GetValueAsDouble();
    origin[1] = this->OriginEntry1->GetValueAsDouble();
    origin[2] = this->OriginEntry2->GetValueAsDouble();
    spacing[0] = this->SpacingEntry0->GetWidget()->GetValueAsDouble();
    spacing[1] = this->SpacingEntry1->GetValueAsDouble();
    spacing[2] = this->SpacingEntry2->GetValueAsDouble();

    vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();
    if ( volumeNode && !this->UpdatingFromMRML )
      {
      volumeNode->SetDisableModifiedEvent(1);
      volumeNode->SetSpacing(spacing);
      volumeNode->SetOrigin(origin);
      volumeNode->SetDisableModifiedEvent(0);
      volumeNode->InvokePendingModifiedEvent();
      }
    }

  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
  if ( button == this->CenterButton->GetWidget() )
    {
    vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();
    if ( volumeNode )
      {
      vtkImageData *image = volumeNode->GetImageData();
      if (image) 
        {
        vtkMatrix4x4 *ijkToRAS = vtkMatrix4x4::New();
        volumeNode->GetIJKToRASMatrix(ijkToRAS);

        double dimsH[4];
        double rasCorner[4];
        int *dims = image->GetDimensions();
        dimsH[0] = dims[0] - 1;
        dimsH[1] = dims[1] - 1;
        dimsH[2] = dims[2] - 1;
        dimsH[3] = 0.;
        ijkToRAS->MultiplyPoint(dimsH, rasCorner);

        double origin[3];
        int i;
        for (i = 0; i < 3; i++)
          {
          origin[i] = -0.5 * rasCorner[i];
          }
        volumeNode->SetDisableModifiedEvent(1);
        volumeNode->SetOrigin(origin);
        volumeNode->SetDisableModifiedEvent(0);
        volumeNode->InvokePendingModifiedEvent();

        ijkToRAS->Delete();
        }
      }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerVolumeHeaderWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  this->UpdateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeHeaderWidget::UpdateWidgetFromMRML ()
{
  this->UpdatingFromMRML = 1;

  vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();
  if (volumeNode != NULL)
    {
    double *vals = volumeNode->GetSpacing();
    this->SpacingEntry0->GetWidget()->SetValueAsDouble(vals[0]);
    this->SpacingEntry1->SetValueAsDouble(vals[1]);
    this->SpacingEntry2->SetValueAsDouble(vals[2]);
    
    vals = volumeNode->GetOrigin();
    this->OriginEntry0->GetWidget()->SetValueAsDouble(vals[0]);
    this->OriginEntry1->SetValueAsDouble(vals[1]);
    this->OriginEntry2->SetValueAsDouble(vals[2]);

    vtkMatrix4x4 *mat = vtkMatrix4x4::New();
    volumeNode->GetIJKToRASMatrix(mat);
    this->ScanOrderEntry->GetWidget()->SetValue(vtkMRMLVolumeNode::ComputeScanOrderFromIJKToRAS(mat));
    mat->Delete();

    vtkImageData *image = volumeNode->GetImageData();
    if (image) 
      {
      int *dims = image->GetDimensions();
      this->DimensionEntry0->GetWidget()->SetValueAsInt(dims[0]);
      this->DimensionEntry1->SetValueAsInt(dims[1]);
      this->DimensionEntry2->SetValueAsInt(dims[2]);

      this->NumScalarsEntry->GetWidget()->SetValueAsInt(image->GetNumberOfScalarComponents());
     
      this->ScalarTypeEntry->GetWidget()->SetValue(image->GetScalarTypeAsString());

      }
    }

  vtkMRMLStorageNode *storageNode = this->GetVolumeStorageNode();
  if (storageNode != NULL && storageNode->GetFileName() != NULL) 
    {
    itksys_stl::string dir =  
          itksys::SystemTools::GetParentDirectory(storageNode->GetFileName());   
    if (dir.size() > 0 && dir[dir.size()-1] != '/')
      {
      dir = dir + vtksys_stl::string("/");
      }
    itksys_stl::string name = itksys::SystemTools::GetFilenameName(storageNode->GetFileName());
    dir = dir+name;
    this->FileNameEntry->GetWidget()->SetValue(dir.c_str());
    }
  else
    {
    this->FileNameEntry->GetWidget()->SetValue("");
    }

  this->UpdatingFromMRML = 0;
  return;
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeHeaderWidget::AddWidgetObservers ( ) 
{
  if (this->VolumeSelectorWidget)
    {
    if (this->MRMLScene != NULL)
      {
      this->VolumeSelectorWidget->SetMRMLScene(this->MRMLScene);
      }
    this->VolumeSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->SpacingEntry0)
    {
    this->SpacingEntry0->GetWidget()->AddObserver(
        vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SpacingEntry1->AddObserver(
        vtkKWEntry::EntryValueChangedEvent , (vtkCommand *)this->GUICallbackCommand );
    this->SpacingEntry2->AddObserver(
        vtkKWEntry::EntryValueChangedEvent , (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->OriginEntry0)
    {
    this->OriginEntry0->GetWidget()->AddObserver(
        vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->OriginEntry1->AddObserver(
        vtkKWEntry::EntryValueChangedEvent , (vtkCommand *)this->GUICallbackCommand );
    this->OriginEntry2->AddObserver(
        vtkKWEntry::EntryValueChangedEvent , (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->CenterButton)
    {
    this->CenterButton->GetWidget()->AddObserver(
        vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

}

//---------------------------------------------------------------------------
void vtkSlicerVolumeHeaderWidget::RemoveWidgetObservers ( ) 
{
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->SetMRMLScene(NULL);
    this->VolumeSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->SpacingEntry0)
    {
    this->SpacingEntry0->GetWidget()->RemoveObservers(
        vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SpacingEntry1->RemoveObservers(
        vtkKWEntry::EntryValueChangedEvent , (vtkCommand *)this->GUICallbackCommand );
    this->SpacingEntry2->RemoveObservers(
        vtkKWEntry::EntryValueChangedEvent , (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->OriginEntry0)
    {
    this->OriginEntry0->GetWidget()->RemoveObservers(
        vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->OriginEntry1->RemoveObservers(
        vtkKWEntry::EntryValueChangedEvent , (vtkCommand *)this->GUICallbackCommand );
    this->OriginEntry2->RemoveObservers(
        vtkKWEntry::EntryValueChangedEvent , (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->CenterButton)
    {
    this->CenterButton->GetWidget()->RemoveObservers(
        vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeHeaderWidget::CreateWidget ( )
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

  if (this->AddNodeSelectorWidget)
    {
    this->VolumeSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->VolumeSelectorWidget->SetParent ( volumeHeaderFrame );
    this->VolumeSelectorWidget->Create ( );
    this->VolumeSelectorWidget->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->VolumeSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->VolumeSelectorWidget->SetBorderWidth(2);
    // this->VolumeSelectorWidget->SetReliefToGroove();
    this->VolumeSelectorWidget->SetPadX(2);
    this->VolumeSelectorWidget->SetPadY(2);
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->VolumeSelectorWidget->SetLabelText( "Volume Select: ");
    this->VolumeSelectorWidget->SetBalloonHelpString("select a volume from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->VolumeSelectorWidget->GetWidgetName());
    }
  else
    {
    this->VolumeSelectorWidget = NULL;
    }

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

  vtkKWFrame *originFrame = vtkKWFrame::New ( );
  originFrame->SetParent ( volumeHeaderFrame );
  originFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 originFrame->GetWidgetName() );

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
  this->DimensionEntry0->GetWidget()->SetStateToDisabled();

  this->DimensionEntry1 = vtkKWEntry::New();
  this->DimensionEntry1->SetParent(dimensionFrame);
  this->DimensionEntry1->Create();
  this->DimensionEntry1->SetValueAsInt(0);
  this->DimensionEntry1->SetWidth(8);
  this->DimensionEntry1->SetStateToDisabled();
  
  this->DimensionEntry2 = vtkKWEntry::New();
  this->DimensionEntry2->SetParent(dimensionFrame);
  this->DimensionEntry2->Create();
  this->DimensionEntry2->SetValueAsInt(0);
  this->DimensionEntry2->SetWidth(8);
  this->DimensionEntry2->SetStateToDisabled();

  this->Script ( "pack %s %s %s -side left -anchor nw -padx 2 -pady 2 -expand n",
                 this->DimensionEntry0->GetWidgetName ( ),
                 this->DimensionEntry1->GetWidgetName(),
                 this->DimensionEntry2->GetWidgetName());

  this->SpacingEntry0 = vtkKWEntryWithLabel::New();
  this->SpacingEntry0->SetParent(spacingFrame);
  this->SpacingEntry0->Create();
  this->SpacingEntry0->SetLabelText("Image Spacing:");
  this->SpacingEntry0->GetWidget()->SetValueAsInt(0);
  this->SpacingEntry0->SetLabelWidth(18);
  this->SpacingEntry0->GetWidget()->SetWidth(8);
  this->SpacingEntry0->GetWidget()->SetRestrictValueToDouble();
 
  this->SpacingEntry1 = vtkKWEntry::New();
  this->SpacingEntry1->SetParent(spacingFrame);
  this->SpacingEntry1->Create();
  this->SpacingEntry1->SetValueAsInt(0);
  this->SpacingEntry1->SetWidth(8);
  this->SpacingEntry1->SetRestrictValueToDouble();
  
  this->SpacingEntry2 = vtkKWEntry::New();
  this->SpacingEntry2->SetParent(spacingFrame);
  this->SpacingEntry2->Create();
  this->SpacingEntry2->SetValueAsInt(0);
  this->SpacingEntry2->SetWidth(8);
  this->SpacingEntry2->SetRestrictValueToDouble();
  
  this->Script ( "pack %s %s %s -side left -anchor nw -padx 2 -pady 2 -expand n",
                 this->SpacingEntry0->GetWidgetName ( ),
                 this->SpacingEntry1->GetWidgetName(),
                 this->SpacingEntry2->GetWidgetName());
  
  this->OriginEntry0 = vtkKWEntryWithLabel::New();
  this->OriginEntry0->SetParent(originFrame);
  this->OriginEntry0->Create();
  this->OriginEntry0->SetLabelText("Image Origin:");
  this->OriginEntry0->GetWidget()->SetValueAsInt(0);
  this->OriginEntry0->SetLabelWidth(18);
  this->OriginEntry0->GetWidget()->SetWidth(8);
  this->OriginEntry0->GetWidget()->SetRestrictValueToDouble();
  
  this->OriginEntry1 = vtkKWEntry::New();
  this->OriginEntry1->SetParent(originFrame);
  this->OriginEntry1->Create();
  this->OriginEntry1->SetValueAsInt(0);
  this->OriginEntry1->SetWidth(8);
  this->OriginEntry1->SetRestrictValueToDouble();
  
  this->OriginEntry2 = vtkKWEntry::New();
  this->OriginEntry2->SetParent(originFrame);
  this->OriginEntry2->Create();
  this->OriginEntry2->SetValueAsInt(0);
  this->OriginEntry2->SetWidth(8);
  this->OriginEntry2->SetRestrictValueToDouble();

  this->Script ( "pack %s %s %s -side left -anchor nw -padx 2 -pady 2 -expand n",
                 this->OriginEntry0->GetWidgetName ( ),
                 this->OriginEntry1->GetWidgetName(),
                 this->OriginEntry2->GetWidgetName());

  this->CenterButton = vtkKWPushButtonWithLabel::New();
  this->CenterButton->SetParent(frame);
  this->CenterButton->Create();
  this->CenterButton->SetLabelText("");
  this->CenterButton->GetWidget()->SetText("Center Volume");
  this->CenterButton->SetLabelWidth(18);
  this->CenterButton->SetWidth(48);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->CenterButton->GetWidgetName());

  this->ScanOrderEntry = vtkKWEntryWithLabel::New();
  this->ScanOrderEntry->SetParent(frame);
  this->ScanOrderEntry->Create();
  this->ScanOrderEntry->SetLabelText("Scan Order:");
  this->ScanOrderEntry->GetWidget()->SetValue("");
  this->ScanOrderEntry->SetLabelWidth(18);
  this->ScanOrderEntry->SetWidth(48);
  this->ScanOrderEntry->GetWidget()->SetStateToDisabled();
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->ScanOrderEntry->GetWidgetName());


  this->NumScalarsEntry = vtkKWEntryWithLabel::New();
  this->NumScalarsEntry->SetParent(frame);
  this->NumScalarsEntry->Create();
  this->NumScalarsEntry->SetLabelText("Number of Scalars:");
  this->NumScalarsEntry->GetWidget()->SetValueAsInt(0);
  this->NumScalarsEntry->SetLabelWidth(18);
  this->NumScalarsEntry->SetWidth(48);
  this->NumScalarsEntry->GetWidget()->SetStateToDisabled();
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->NumScalarsEntry->GetWidgetName());


  this->ScalarTypeEntry = vtkKWEntryWithLabel::New();
  this->ScalarTypeEntry->SetParent(frame);
  this->ScalarTypeEntry->Create();
  this->ScalarTypeEntry->SetLabelText("Scalar Type:");
  this->ScalarTypeEntry->GetWidget()->SetValue("");
  this->ScalarTypeEntry->SetLabelWidth(18);
  this->ScalarTypeEntry->SetWidth(48);
  this->ScalarTypeEntry->GetWidget()->SetStateToDisabled();
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->ScalarTypeEntry->GetWidgetName());


  this->FileNameEntry = vtkKWEntryWithLabel::New();
  this->FileNameEntry->SetParent(frame);
  this->FileNameEntry->Create();
  this->FileNameEntry->SetLabelText("File Name:");
  this->FileNameEntry->GetWidget()->SetValue("");
  this->FileNameEntry->SetWidth(48);
  this->FileNameEntry->SetLabelWidth(18);
  this->FileNameEntry->GetWidget()->SetStateToDisabled();  
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->FileNameEntry->GetWidgetName());


  this->AddWidgetObservers();
  volumeHeaderFrame->Delete();
  dimensionFrame->Delete();
  spacingFrame->Delete();
  originFrame->Delete();
  frame->Delete();

}
