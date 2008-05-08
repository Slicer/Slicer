#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerMeasurementFrameWidget.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkSlicerDiffusionEditorLogic.h"
#include "vtkDiffusionTensorMathematicsSimple.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
//widgets
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWMatrixWidget.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFrameWithLabel.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerMeasurementFrameWidget);
vtkCxxRevisionMacro (vtkSlicerMeasurementFrameWidget, "$Revision: 1.0 $");
//---------------------------------------------------------------------------

vtkSlicerMeasurementFrameWidget::vtkSlicerMeasurementFrameWidget(void)
  {
  this->ActiveVolumeNode = NULL;
  this->Matrix = vtkMatrix4x4::New();
  this->MeasurementFrame = NULL;
  this->MatrixWidget = NULL;
  this->RotateButton = NULL;
  this->InvertButton = NULL;
  this->IdentityButton = NULL;
  this->SwapButton = NULL;
  this->AngleCombobox = NULL;
  this->AngleLabel = NULL;
  this->Logic = NULL;
  }

//---------------------------------------------------------------------------
vtkSlicerMeasurementFrameWidget::~vtkSlicerMeasurementFrameWidget(void)
  {
  this->RemoveWidgetObservers();
  if (this->ActiveVolumeNode)
    {
    vtkSetMRMLNodeMacro(this->ActiveVolumeNode, NULL);
    }
  if (this->MeasurementFrame)
    {
    this->MeasurementFrame->SetParent (NULL);
    this->MeasurementFrame->Delete();
    this->MeasurementFrame = NULL;
    }
  if (this->AngleLabel)
    {
    this->AngleLabel->SetParent (NULL);
    this->AngleLabel->Delete();
    this->AngleLabel = NULL;
    }
  if (this->AngleCombobox)
    {
    this->AngleCombobox->SetParent (NULL);
    this->AngleCombobox->Delete();
    this->AngleCombobox = NULL;
    }
  if (this->SwapButton)
    {
    this->SwapButton->SetParent (NULL);
    this->SwapButton->Delete();
    this->SwapButton = NULL;
    }
  if (this->InvertButton)
    {
    this->InvertButton->SetParent (NULL);
    this->InvertButton->Delete();
    this->InvertButton = NULL;
    }
  if (this->RotateButton)
    {
    this->RotateButton->SetParent (NULL);
    this->RotateButton->Delete();
    this->RotateButton = NULL;
    }
  if (this->IdentityButton)
    {
    this->IdentityButton->SetParent (NULL);
    this->IdentityButton->Delete();
    this->IdentityButton = NULL;
    }
  if (this->MatrixWidget)
    {
    this->MatrixWidget->SetParent (NULL);
    this->MatrixWidget->Delete();
    this->MatrixWidget = NULL;
    }
  for (int i=0; i<3; i++){
    this->Checkbuttons[i]->SetParent (NULL);
    this->Checkbuttons[i]->Delete();
    this->Checkbuttons[i] = NULL;
    }
  if (this->Matrix)
    {
    this->Matrix->Delete();
    this->Matrix = NULL;
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::AddWidgetObservers ( )
  {
  this->RotateButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->InvertButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SwapButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);  
  this->IdentityButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);  
  this->MatrixWidget->AddObserver(vtkKWMatrixWidget::ElementChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  for(int i=0; i<3;i ++)
    {
    this->Checkbuttons[i]->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::RemoveWidgetObservers( )
  {
  this->RotateButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->InvertButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SwapButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);   
  this->IdentityButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);  
  this->MatrixWidget->RemoveObservers(vtkKWMatrixWidget::ElementChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  for(int i=0; i<3;i ++)
    {
    this->Checkbuttons[i]->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerMeasurementFrameWidget: " << this->GetClassName ( ) << "\n";
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::UpdateWidget(vtkMRMLVolumeNode *node)
  {
  if(this->ActiveVolumeNode != node)
    {
    vtkSetMRMLNodeMacro(this->ActiveVolumeNode, node); //set activeVolumeNode
    }
  this->ActiveVolumeNode->GetMeasurementFrameMatrix(this->Matrix); //set internal matrix
  this->UpdateMatrix(); //update gui
  this->CheckDeterminant();
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::UpdateMatrix()
  {
  if(this->Matrix != NULL && this->ActiveVolumeNode != NULL)
    {
    // update gui values
    for(int i=0;i<3;i++)
      {
      for(int j=0;j<3;j++)
        {
        this->MatrixWidget->SetElementValueAsDouble(j,i, this->Matrix->GetElement(j,i));
        }
      }
    }//end if
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::SaveMatrix()
  {
  if(this->CheckDeterminant())
    {
    this->InvokeEvent(this->ChangedEvent);
    //if its a DTI do not save changes to original node
    if(this->ActiveVolumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode")) return;
    this->Logic->SaveStateForUndoRedo();
    // write internal matrix back to node
    this->ActiveVolumeNode->SetMeasurementFrameMatrix(this->Matrix);
    // mark as modified in save menu
    this->ActiveVolumeNode->SetModifiedSinceRead(1);
    }
  }

//---------------------------------------------------------------------------
int vtkSlicerMeasurementFrameWidget::CheckDeterminant()
  {
  double det = this->Matrix->Determinant();
  const double delta = 0.001;

  if((det<=1+delta && det>=1-delta) || (det>=-1-delta && det<=-1+delta))
    {
    this->MeasurementFrame->SetLabelText("Measurement Frame");
    this->MeasurementFrame->GetLabel()->SetBackgroundColor(1,1,1);
    return 1;
    }
  else
    {
    this->MeasurementFrame->SetLabelText("Measurement Frame   ** Determinant not 1 **");
    this->MeasurementFrame->GetLabel()->SetBackgroundColor(1,0,0);
    return 0;
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::ProcessWidgetEvents (vtkObject *caller, unsigned long event, void *callData)
  {

  //import current matrix values 
  if(event == vtkKWMatrixWidget::ElementChangedEvent && this->MatrixWidget == vtkKWMatrixWidget::SafeDownCast(caller))
    {
    for(int i=0;i<3;i++)
      {
      for(int j=0;j<3;j++)
        {
        this->Matrix->SetElement(j,i,this->MatrixWidget->GetElementValueAsDouble(j,i));               
        }
      }
    // write internal matrix back to node
    this->SaveMatrix();
    }

  //enable/disable buttons depending on how many checkbuttons are selected 
  if(event == vtkKWCheckButton::SelectedStateChangedEvent 
    && (this->Checkbuttons[0] == vtkKWCheckButton::SafeDownCast(caller)
    ||  this->Checkbuttons[1] == vtkKWCheckButton::SafeDownCast(caller)
    ||  this->Checkbuttons[2] == vtkKWCheckButton::SafeDownCast(caller)))
    {
    //check how many Checkbuttons are selected
    int numberSelected = 0;
    for(int i=0; i<3;i++)
      {
      if(this->Checkbuttons[i]->GetSelectedState()) numberSelected++;
      }
    //enable/disable buttons
    if (numberSelected >= 1)
      {
      this->InvertButton->SetEnabled(1);
      if(numberSelected == 2)
        {
        this->SwapButton->SetEnabled(1);
        }
      else 
        {
        this->SwapButton->SetEnabled(0);
        }
      if(numberSelected == 1)
        {
        this->RotateButton->SetEnabled(1);
        this->AngleLabel->SetEnabled(1);
        this->AngleCombobox->SetEnabled(1);                
        }
      else 
        {
        this->RotateButton->SetEnabled(0);
        this->AngleCombobox->SetEnabled(0);
        this->AngleLabel->SetEnabled(0);
        }
      }
    else 
      {
      this->InvertButton->SetEnabled(0);
      this->RotateButton->SetEnabled(0);
      this->AngleLabel->SetEnabled(0);
      this->AngleCombobox->SetEnabled(0);
      }
    }

  //rotate matrix
  else if (this->RotateButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    vtkTransform* transform = vtkTransform::New();
    transform->SetMatrix(this->Matrix);

    //rotate
    if(this->Checkbuttons[0]->GetSelectedState())
      {
      transform->RotateX(this->AngleCombobox->GetValueAsDouble());
      }
    if(this->Checkbuttons[1]->GetSelectedState())
      {
      transform->RotateY(this->AngleCombobox->GetValueAsDouble());
      }
    if(this->Checkbuttons[2]->GetSelectedState())
      {
      transform->RotateZ(this->AngleCombobox->GetValueAsDouble());
      }
    //copy transformed matrix back
    this->Matrix->DeepCopy(transform->GetMatrix());
    transform->Delete();
    this->UpdateMatrix();
    this->SaveMatrix();
    }

  //swap columns
  else if (this->SwapButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    //seek for selected checkboxes
    int firstSelectedCheckbox  = (this->Checkbuttons[0]->GetSelectedState() == 1) ? 0 : 1;        
    int secondSelectedCheckbox = (this->Checkbuttons[2]->GetSelectedState() == 1) ? 2 : 1;

    //swap values
    for(int j=0; j<3; j++)
      {
      double value = this->Matrix->GetElement(j, firstSelectedCheckbox);
      this->Matrix->SetElement(j, firstSelectedCheckbox, this->Matrix->GetElement(j, secondSelectedCheckbox));
      this->Matrix->SetElement(j, secondSelectedCheckbox, value);
      }
    this->UpdateMatrix();
    this->SaveMatrix();
    }

  //invert columns
  else if (this->InvertButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    for(unsigned int j=0; j<3; j++)
      {
      //seek selected checkbuttons
      if(this->Checkbuttons[j]->GetSelectedState())
        {            
        for(unsigned int i=0; i<3; i++)
          {
          double currentValue = this->Matrix->GetElement(i,j);
          //change sign of values != 0
          if(currentValue != 0)
            {
            currentValue *= -1;
            this->Matrix->SetElement(i,j,currentValue);
            }
          }//end for2
        }//end if selected
      }//end for1
    this->UpdateMatrix();
    this->SaveMatrix();
    }

  //set to identity
  else if(event == vtkKWPushButton::InvokedEvent && this->IdentityButton == vtkKWPushButton::SafeDownCast(caller))
    {
    this->Matrix->Identity();
    this->UpdateMatrix();
    this->SaveMatrix();
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::SetLogic(vtkSlicerDiffusionEditorLogic *logic)
  {
  this->Logic = logic;
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::CreateWidget( )
  {
  //check if already created
  if (this->IsCreated()){
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  //call the superclass to create the whole widget
  this->Superclass::CreateWidget();

  //create the measurement frame
  this->MeasurementFrame = vtkKWFrameWithLabel::New();
  this->MeasurementFrame->SetParent(this->GetParent());
  this->MeasurementFrame->Create();
  this->MeasurementFrame->SetLabelText("Measurement Frame");
  this->Script("pack %s -side top -anchor nw -fill x -pady 2 -padx 2", 
    this->MeasurementFrame->GetWidgetName());

  //create MatrixWidget for measurement frame
  this->MatrixWidget = vtkKWMatrixWidget::New();
  this->MatrixWidget->SetParent(this->MeasurementFrame->GetFrame());
  this->MatrixWidget->Create();
  this->MatrixWidget->SetNumberOfColumns(3);
  this->MatrixWidget->SetNumberOfRows(3);
  this->MatrixWidget->SetPadX(3);
  this->MatrixWidget->SetRestrictElementValueToDouble();
  this->MatrixWidget->SetElementWidth(7);
  this->MatrixWidget->SetElementChangedCommandTrigger(2);

  //create checkbuttons, one under each column of the matrix
  for(int i=0; i< 3; i++)
    {
    vtkKWCheckButton* checkButton = vtkKWCheckButton::New();
    this->Checkbuttons[i] = checkButton;
    this->Checkbuttons[i]->SetParent(this->MeasurementFrame->GetFrame());
    this->Checkbuttons[i]->Create();
    this->Checkbuttons[i]->SetBalloonHelpString("Enable column for invert/rotate/swap option.");
    this->Script("grid %s -row 4 -column %d -sticky n", 
      checkButton->GetWidgetName(), i);
    }

  //create invert button
  this->InvertButton = vtkKWPushButton::New();
  this->InvertButton->SetParent(this->MeasurementFrame->GetFrame());
  this->InvertButton->Create();
  this->InvertButton->SetText("Invert Selected");
  this->InvertButton->SetWidth(14);
  this->InvertButton->SetEnabled(0);
  this->InvertButton->SetBalloonHelpString("Invert selected columns.");

  //create swap button
  this->SwapButton = vtkKWPushButton::New();
  this->SwapButton->SetParent(this->MeasurementFrame->GetFrame());
  this->SwapButton->Create();
  this->SwapButton->SetText("Swap Selected");
  this->SwapButton->SetWidth(14);
  this->SwapButton->SetEnabled(0);
  this->SwapButton->SetBalloonHelpString("Swap selected columns (two have to be selected).");

  //create rotate button
  this->RotateButton = vtkKWPushButton::New();
  this->RotateButton->SetParent(this->MeasurementFrame->GetFrame());
  this->RotateButton->Create();
  this->RotateButton->SetText("Rotate Selected");
  this->RotateButton->SetWidth(14);
  this->RotateButton->SetEnabled(0);
  this->RotateButton->SetBalloonHelpString("Rotate selected column by angle value (one has to be selected).");

  //create label for angle
  this->AngleLabel = vtkKWLabel::New();
  this->AngleLabel->SetParent(this->MeasurementFrame->GetFrame());
  this->AngleLabel->Create();
  this->AngleLabel->SetEnabled(0);
  this->AngleLabel->SetText("Angle:");

  //create combobox for angle
  this->AngleCombobox = vtkKWComboBox::New();
  this->AngleCombobox->SetParent(this->MeasurementFrame->GetFrame());
  this->AngleCombobox->Create();
  this->AngleCombobox->SetEnabled(0);
  this->AngleCombobox->SetWidth(4);
  this->AngleCombobox->SetValue("+90");
  this->AngleCombobox->SetBalloonHelpString("Select given value or type in your own.");

  //create identity button
  this->IdentityButton = vtkKWPushButton::New();
  this->IdentityButton->SetParent(this->MeasurementFrame->GetFrame());
  this->IdentityButton->Create();
  this->IdentityButton->SetText("Identity");
  this->IdentityButton->SetWidth(11);
  this->IdentityButton->SetBalloonHelpString("");
  this->IdentityButton->SetBalloonHelpString("Change to identity matrix.");

  //fill default angles in combobox
  const char *angleValues [] = {"+90", "-90", "+180", "-180", "+30", "-30"};
  for (int i=0; i<sizeof(angleValues)/sizeof(angleValues[0]); i++)
    {
    this->AngleCombobox->AddValue(angleValues[i]);
    }

  //pack all elements
  this->Script("grid %s -row 1 -column 0 -columnspan 3 -rowspan 3", 
    this->MatrixWidget->GetWidgetName());
  this->Script("grid %s -row 3 -column 3 -sticky ne", 
    this->InvertButton->GetWidgetName());
  this->Script("grid %s -row 2 -column 3 -sticky ne", 
    this->SwapButton->GetWidgetName());
  this->Script("grid %s -row 1 -column 3 -sticky ne", 
    this->RotateButton->GetWidgetName());
  this->Script("grid %s -row 1 -column 4 -sticky ne", 
    this->AngleLabel->GetWidgetName());
  this->Script("grid %s -row 1 -column 5 -sticky ne", 
    this->AngleCombobox->GetWidgetName());
  this->Script("grid %s -row 2 -column 4 -columnspan 2 -sticky ne", 
    this->IdentityButton->GetWidgetName());
  } 

