#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerMeasurementFrameWidget.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"

#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWMatrixWidget.h"
#include "vtkKWCheckButton.h"
#include "vtkMatrix4x4.h"
#include "vtkKWFrameWithLabel.h"

#include "vtkTransform.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerMeasurementFrameWidget);
vtkCxxRevisionMacro (vtkSlicerMeasurementFrameWidget, "$Revision: 1.0 $");
//---------------------------------------------------------------------------
vtkSlicerMeasurementFrameWidget::vtkSlicerMeasurementFrameWidget(void)
  {
  this->ActiveVolumeNode = NULL;
  this->MeasurementFrame = NULL;
  this->MatrixWidget = NULL;
  this->Matrix = NULL;
  this->RotateButton = NULL;
  this->NegativeButton = NULL;
  this->SwapButton = NULL;
  this->AngleCombobox = NULL;
  this->AngleLabel = NULL;
  }

//---------------------------------------------------------------------------
vtkSlicerMeasurementFrameWidget::~vtkSlicerMeasurementFrameWidget(void)
  {
  this->RemoveWidgetObservers();
  if (this->ActiveVolumeNode)
    {
    this->ActiveVolumeNode->Delete();
    this->ActiveVolumeNode = NULL;
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
  if (this->NegativeButton)
    {
    this->NegativeButton->SetParent (NULL);
    this->NegativeButton->Delete();
    this->NegativeButton = NULL;
    }
  if (this->RotateButton)
    {
    this->RotateButton->SetParent (NULL);
    this->RotateButton->Delete();
    this->RotateButton = NULL;
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
  this->NegativeButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SwapButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  /*this->Matrix->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);*/
  for(int i=0; i<3;i ++)
    {
    this->Checkbuttons[i]->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  }
//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::RemoveWidgetObservers( )
  {
  this->RotateButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NegativeButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SwapButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);    
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
void vtkSlicerMeasurementFrameWidget::ProcessMRMLEvents (vtkObject *caller,unsigned long event, void *callData)
  {
  //if (this->Matrix == vtkMatrix4x4::SafeDownCast(caller) && event == vtkCommand::ModifiedEvent)
  //  {
  //  //vtkErrorMacro(" modified" << this->GetMRMLScene()->GetClassName());
  //  //this->GetMRMLScene()->Undo();
  //  }
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::UpdateMatrix()
  {
  if(this->Matrix == NULL)
    {
    this->Matrix = vtkMatrix4x4::New();
    }

  // update gui values
  for(int i=0;i<3;i++)
    {
    for(int j=0;j<3;j++)
      {
      this->MatrixWidget->SetElementValueAsDouble(j,i, this->Matrix->GetElement(j,i));
      }
    }

  // write internal matrix back to node
  if(this->ActiveVolumeNode != NULL)
    {
    this->ActiveVolumeNode->SetMeasurementFrameMatrix(this->Matrix);
    this->ActiveVolumeNode->SetModifiedSinceRead(1);
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *dwiNode)
  {
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, dwiNode);
  this->ActiveVolumeNode->GetMeasurementFrameMatrix(this->Matrix);
  this->UpdateMatrix();
  }

//---------------------------------------------------------------------------
void vtkSlicerMeasurementFrameWidget::ProcessWidgetEvents (vtkObject *caller, unsigned long event, void *callData)
  {
  //import the current matrix values, when enabled, as the user could have changed it
  for(int i=0;i<3;i++)
    {
    for(int j=0;j<3;j++)
      {
      this->Matrix->SetElement(j,i,this->MatrixWidget->GetElementValueAsDouble(j,i));               
      }
    }
  this->UpdateMatrix();

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
      if(this->Checkbuttons[i]->GetSelectedState())
        {
        numberSelected++;
        }
      }
    //enable/disable buttons
    if (numberSelected >= 1)
      {
      this->NegativeButton->SetEnabled(1);
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
      this->NegativeButton->SetEnabled(0);
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
    this->UpdateMatrix();
    transform->Delete();
    }

  //swap columns
  else if (this->SwapButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    int firstSelectedCheckbox  = -1;        
    int secondSelectedCheckbox = -1;
    //seek for selected checkboxes
    for(int i=0; i<3;i++)
      {
      if(this->Checkbuttons[i]->GetSelectedState())
        {
        firstSelectedCheckbox = i;
        break;
        }
      }
    for(int i=3-1; i>=0;i--)
      {
      if(this->Checkbuttons[i]->GetSelectedState())
        {
        secondSelectedCheckbox = i;
        break;
        }
      }
    //swap values
    for(int j=0; j<3; j++)
      {
      double value = this->Matrix->GetElement(j, firstSelectedCheckbox);
      this->Matrix->SetElement(j, firstSelectedCheckbox, this->Matrix->GetElement(j, secondSelectedCheckbox));
      this->Matrix->SetElement(j, secondSelectedCheckbox, value);
      }
    this->UpdateMatrix();
    }

  //negative columns
  else if (this->NegativeButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
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
            currentValue = currentValue-(2*currentValue);
            this->Matrix->SetElement(i,j,currentValue);
            }
          }//end for2
        }//end if selected
      }//end for1
    this->UpdateMatrix();
    }

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
  this->Script("pack %s -side top -anchor nw -fill x -padx 1 -pady 2", 
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

  //set default matrix
  this->UpdateMatrix();

  //create checkbuttons, one under each column of the matrix
  for(int i=0; i< 3; i++)
    {
    vtkKWCheckButton* checkButton = vtkKWCheckButton::New();
    this->Checkbuttons[i] = checkButton;
    this->Checkbuttons[i]->SetParent(this->MeasurementFrame->GetFrame());
    this->Checkbuttons[i]->Create();
    this->Script("grid %s -row 4 -column %d -sticky n", 
      checkButton->GetWidgetName(), i);
    }

  //create neagative button
  this->NegativeButton = vtkKWPushButton::New();
  this->NegativeButton->SetParent(this->MeasurementFrame->GetFrame());
  this->NegativeButton->Create();
  this->NegativeButton->SetText("Negative Selected");
  this->NegativeButton->SetWidth(14);
  this->NegativeButton->SetEnabled(0);

  //create swap button
  this->SwapButton = vtkKWPushButton::New();
  this->SwapButton->SetParent(this->MeasurementFrame->GetFrame());
  this->SwapButton->Create();
  this->SwapButton->SetText("Swap Selected");
  this->SwapButton->SetWidth(14);
  this->SwapButton->SetEnabled(0);

  //create rotate button
  this->RotateButton = vtkKWPushButton::New();
  this->RotateButton->SetParent(this->MeasurementFrame->GetFrame());
  this->RotateButton->Create();
  this->RotateButton->SetText("Rotate Selected");
  this->RotateButton->SetWidth(14);
  this->RotateButton->SetEnabled(0);

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

  //fill default angles in combobox
  const char *angleValues [] = {"+90", "-90", "+180", "-180", "+30", "-30"};
  for (int i=0; i<sizeof(angleValues)/sizeof(angleValues[0]); i++)
    {
    this->AngleCombobox->AddValue(angleValues[i]);
    }

  //pack all elements
  //this->Script("grid %s -row 0 -column 0 -columnspan 3 -sticky n", 
  //  this->EnableMatrixButton->GetWidgetName());
  this->Script("grid %s -row 1 -column 0 -columnspan 3 -rowspan 3", 
    this->MatrixWidget->GetWidgetName());
  this->Script("grid %s -row 3 -column 3 -sticky ne", 
    this->NegativeButton->GetWidgetName());
  this->Script("grid %s -row 2 -column 3 -sticky ne", 
    this->SwapButton->GetWidgetName());
  this->Script("grid %s -row 1 -column 3 -sticky ne", 
    this->RotateButton->GetWidgetName());
  this->Script("grid %s -row 1 -column 4 -sticky ne", 
    this->AngleLabel->GetWidgetName());
  this->Script("grid %s -row 1 -column 5 -sticky ne", 
    this->AngleCombobox->GetWidgetName());
  } 

