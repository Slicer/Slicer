#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkKWText.h"
#include "vtkKWCheckButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWMatrixWidget.h"
#include "vtkKWMultiColumnList.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerGradientEditorWidget.h"
#include "vtkSlicerGradientEditorLogic.h"

#include "vtkMatrix4x4.h"
#include "vtkDoubleArray.h"
#include "vtkTransform.h"
#include <sstream>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerGradientEditorWidget);
vtkCxxRevisionMacro (vtkSlicerGradientEditorWidget, "$Revision: 1.0 $");
//---------------------------------------------------------------------------
vtkSlicerGradientEditorWidget::vtkSlicerGradientEditorWidget(void)
  {
  this->MatrixGUI = NULL;
  this->Matrix = NULL;
  this->RotateButton = NULL;
  this->NegativeButton = NULL;
  this->SwapButton = NULL;
  this->AngleCombobox = NULL;
  this->AngleLabel = NULL;
  this->RunButton = NULL;
  this->Mask = NULL;
  this->GradientsTextfield = NULL;
  this->TestFrame = NULL;
  this->MeasurementFrame = NULL;
  this->GradientsFrame = NULL;
  this->CancelButton = NULL;
  this->LoadGradientsButton = NULL;
  this->ButtonsFrame = NULL;
  this->EnableGradientsButton = NULL;
  this->Gradients = NULL;
  this->BValues = NULL;
  this->ActiveVolumeNode = NULL;
  }

//---------------------------------------------------------------------------
vtkSlicerGradientEditorWidget::~vtkSlicerGradientEditorWidget(void)
  {
  this->RemoveWidgetObservers();
  if (this->GradientsFrame)
    {
    this->GradientsFrame->SetParent (NULL);
    this->GradientsFrame->Delete();
    this->GradientsFrame = NULL;
    }
  if (this->GradientsTextfield)
    {
    this->GradientsTextfield->SetParent (NULL);
    this->GradientsTextfield->Delete();
    this->GradientsTextfield = NULL;
    }
  if (this->Mask)
    {
    this->Mask->SetParent (NULL);
    this->Mask->Delete();
    this->Mask = NULL;
    }
  if (this->RunButton)
    {
    this->RunButton->SetParent (NULL);
    this->RunButton->Delete();
    this->RunButton = NULL;
    }
  if (this->CancelButton)
    {
    this->CancelButton->SetParent (NULL);
    this->CancelButton->Delete();
    this->CancelButton = NULL;
    }
  if (this->GradientsFrame)
    {
    this->GradientsFrame->SetParent (NULL);
    this->GradientsFrame->Delete();
    this->GradientsFrame = NULL;
    }
  if (this->MeasurementFrame)
    {
    this->MeasurementFrame->SetParent (NULL);
    this->MeasurementFrame->Delete();
    this->MeasurementFrame = NULL;
    }
  if (this->TestFrame)
    {
    this->TestFrame->SetParent (NULL);
    this->TestFrame->Delete();
    this->TestFrame = NULL;
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
  if (this->MatrixGUI)
    {
    this->MatrixGUI->SetParent (NULL);
    this->MatrixGUI->Delete();
    this->MatrixGUI = NULL;
    }
  if (this->LoadGradientsButton)
    {
    this->LoadGradientsButton->SetParent (NULL);
    this->LoadGradientsButton->Delete();
    this->LoadGradientsButton = NULL;
    }
  for (int i=0; i<3; i++){
    this->Checkbuttons[i]->SetParent (NULL);
    this->Checkbuttons[i]->Delete();
    this->Checkbuttons[i] = NULL;
    }
  if (this->ButtonsFrame)
    {
    this->ButtonsFrame->SetParent (NULL);
    this->ButtonsFrame->Delete();
    this->ButtonsFrame = NULL;
    }
  if (this->EnableGradientsButton)
    {
    this->EnableGradientsButton->SetParent (NULL);
    this->EnableGradientsButton->Delete();
    this->EnableGradientsButton = NULL;
    }
  if (this->ActiveVolumeNode)
    {
    this->ActiveVolumeNode->Delete();
    this->ActiveVolumeNode = NULL;
    }
  if (this->Gradients)
    {
    this->Gradients->Delete();
    this->Gradients = NULL;
    }
  if (this->BValues)
    {
    this->BValues->Delete();
    this->BValues = NULL;
    }
  if (this->Matrix)
    {
    this->Matrix->Delete();
    this->Matrix = NULL;
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::AddWidgetObservers ( )
  {    
  this->EnableGradientsButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RotateButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NegativeButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SwapButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RunButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog()->AddObserver(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
  /*this->Matrix->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);*/
  for(int i=0; i<3;i ++)
    {
    this->Checkbuttons[i]->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  }
//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::RemoveWidgetObservers( )
  {
  this->EnableGradientsButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->RotateButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NegativeButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SwapButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);    
  this->RunButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
  for(int i=0; i<3;i ++)
    {
    this->Checkbuttons[i]->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::PrintSelf ( ostream& os, vtkIndent indent )
  {
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerGradientEditorWidget: " << this->GetClassName ( ) << "\n";
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                       unsigned long event, void *callData )
  {
  //if (this->Matrix == vtkMatrix4x4::SafeDownCast(caller) && event == vtkCommand::ModifiedEvent)
  //  {
  //  //vtkErrorMacro(" modified" << this->GetMRMLScene()->GetClassName());
  //  //this->GetMRMLScene()->Undo();
  //  }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData )
  {
  //import the current matrix values, when enabled, as the user could have changed it
  for(int i=0;i<3;i++)
    {
    for(int j=0;j<3;j++)
      {
      this->Matrix->SetElement(j,i,this->MatrixGUI->GetElementValueAsDouble(j,i));               
      }
    }
  this->UpdateMatrix();

  if (this->EnableGradientsButton->GetSelectedState() && this->GradientsTextfield->GetWidget()->HasFocus())
    {
    vtkSlicerGradientEditorLogic *myLogic = vtkSlicerGradientEditorLogic::New();
    vtkDoubleArray *newGradients = vtkDoubleArray::New();
    myLogic->ParseGradients(this->GradientsTextfield->GetWidget()->GetText(), newGradients);
    this->ActiveVolumeNode->SetDiffusionGradients(newGradients);
    this->UpdateGradients();
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

  //enable the gradients textbox
  else if(this->EnableGradientsButton == vtkKWCheckButton::SafeDownCast(caller) && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    this->GradientsTextfield->SetEnabled(this->EnableGradientsButton->GetSelectedState());
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

  //load gradients from file
  if (this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent)
    {
    const char *filename = this->LoadGradientsButton->GetWidget()->GetFileName();
    if(filename)
      {
      vtkSlicerGradientEditorLogic *myLogic = vtkSlicerGradientEditorLogic::New();
      vtkMRMLDiffusionWeightedVolumeNode *dwiNode = vtkMRMLDiffusionWeightedVolumeNode::New();
      myLogic->AddGradients(filename, dwiNode);
      this->Gradients = dwiNode->GetDiffusionGradients();
      this->BValues = dwiNode->GetBValues();
      this->UpdateGradients();
      }
    }

  //run test
  if (this->RunButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent)
    {
    //TODO
    }

  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *dwiNode)
  {
  if (dwiNode != NULL)
    {
    vtkSetMRMLNodeMacro(this->ActiveVolumeNode, dwiNode);
    // update the measurement frame, gradients and bValues 
    // when the active node changes
    this->ActiveVolumeNode->GetMeasurementFrameMatrix(this->Matrix);
    this->UpdateMatrix();
    this->UpdateGradients();
    }

  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::UpdateMatrix()
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
      this->MatrixGUI->SetElementValueAsDouble(j,i, this->Matrix->GetElement(j,i));
      }
    }

  // write matrix back to node
  if(this->ActiveVolumeNode != NULL)
    {
    this->ActiveVolumeNode->SetMeasurementFrameMatrix(this->Matrix);
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::UpdateGradients()
  {
  std::stringstream value;
  vtkDoubleArray *bValues = this->ActiveVolumeNode->GetBValues();
  vtkDoubleArray *gradients = this->ActiveVolumeNode->GetDiffusionGradients();

  //read in new bValues
  if(bValues != NULL)
    {
    for(int i=0; i < bValues->GetSize(); i++)
      {
      value << "DWMRI_b-value:=" << bValues->GetValue(i) << endl;        
      }
    }

  // read in new gradients
  if(gradients != NULL)
    {
    int n = gradients->GetNumberOfComponents();
    for(int i=0; i < gradients->GetSize(); i=i+n)
      {
      value << "DWMRI_gradient_" << setfill('0') << setw(4) << i/n << ":=";
      for(int j=i; j<i+n; j++)
        {
        value << gradients->GetValue(j) << " ";
        }
      value << endl;        
      }
    this->GradientsTextfield->GetWidget()->SetText(value.str().c_str());
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorWidget::CreateWidget( )
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
  this->MeasurementFrame->SetLabelText ("Measurement Frame");
  this->Script("pack %s -side top -anchor nw -fill x -padx 1 -pady 2", 
    this->MeasurementFrame->GetWidgetName());

  //create MatrixGUI for measurement frame
  this->MatrixGUI = vtkKWMatrixWidget::New();
  this->MatrixGUI->SetParent(this->MeasurementFrame->GetFrame());
  this->MatrixGUI->Create();
  this->MatrixGUI->SetNumberOfColumns(3);
  this->MatrixGUI->SetNumberOfRows(3);
  this->MatrixGUI->SetPadX(3);
  this->MatrixGUI->SetRestrictElementValueToDouble();
  this->MatrixGUI->SetElementWidth(7);

  //initialize with default values
  this->UpdateMatrix();

  for(int i=0; i< 3; i++)
    {
    vtkKWCheckButton* checkButton = vtkKWCheckButton::New();
    this->Checkbuttons[i] = checkButton;
    this->Checkbuttons[i]->SetParent(this->MeasurementFrame->GetFrame());
    this->Checkbuttons[i]->Create();
    this->Script("grid %s -row 4 -column %d -sticky n", 
      checkButton->GetWidgetName(), i);
    }

  //buttons for options
  this->NegativeButton = vtkKWPushButton::New();
  this->NegativeButton->SetParent(this->MeasurementFrame->GetFrame());
  this->NegativeButton->Create();
  this->NegativeButton->SetText("Negative Selected");
  this->NegativeButton->SetWidth(14);
  this->NegativeButton->SetEnabled(0);

  this->SwapButton = vtkKWPushButton::New();
  this->SwapButton->SetParent(this->MeasurementFrame->GetFrame());
  this->SwapButton->Create();
  this->SwapButton->SetText("Swap Selected");
  this->SwapButton->SetWidth(14);
  this->SwapButton->SetEnabled(0);

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

  const char *angleValues [] = {"+90", "-90", "+180", "-180", "+30", "-30"};
  for (int i=0; i<sizeof(angleValues)/sizeof(angleValues[0]); i++)
    {
    this->AngleCombobox->AddValue(angleValues[i]);
    }

  //pack all elements
  //this->Script("grid %s -row 0 -column 0 -columnspan 3 -sticky n", 
  //  this->EnableMatrixButton->GetWidgetName());
  this->Script("grid %s -row 1 -column 0 -columnspan 3 -rowspan 3", 
    this->MatrixGUI->GetWidgetName());
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

  //create gradient frame 
  this->GradientsFrame = vtkKWFrameWithLabel::New();
  this->GradientsFrame->SetParent(this->GetParent());
  this->GradientsFrame->Create();
  this->GradientsFrame->SetLabelText ("Gradients");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
    this->GradientsFrame->GetWidgetName());

  //create gradient frame 
  this->ButtonsFrame = vtkKWFrame::New();
  this->ButtonsFrame->SetParent(this->GradientsFrame->GetFrame());
  this->ButtonsFrame->Create();
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 1", 
    this->ButtonsFrame->GetWidgetName());

  //create textfield for GradientsTextfield
  this->GradientsTextfield = vtkKWTextWithScrollbars::New();
  this->GradientsTextfield->SetParent(this->GradientsFrame->GetFrame());
  this->GradientsTextfield->Create();
  this->GradientsTextfield->SetEnabled(0);
  this->GradientsTextfield->SetHeight(30);
  this->Script("pack %s -side top -anchor s -fill x -padx 2 -pady 2", 
    this->GradientsTextfield->GetWidgetName());

  //create enable gradients textfield button
  this->EnableGradientsButton = vtkKWCheckButton::New();
  this->EnableGradientsButton->SetParent(this->ButtonsFrame);
  this->EnableGradientsButton->SetText("Enable Textbox");
  this->EnableGradientsButton->Create();
  this->Script("pack %s -side left -anchor nw -padx 2 -pady 1", 
    this->EnableGradientsButton->GetWidgetName());

  //create load button
  this->LoadGradientsButton = vtkKWLoadSaveButtonWithLabel::New();
  this->LoadGradientsButton->SetParent(this->ButtonsFrame);
  this->LoadGradientsButton->Create();
  this->LoadGradientsButton->SetLabelText("Load Gradients (.txt/.nhdr)");
  this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Open .txt/.nhdr File");
  this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes("{ {NHDRfile} {.nhdr} }{ {Textfile} {.txt} }");
  this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
  this->Script("pack %s -side right -anchor ne -padx 2 -pady 1", 
    this->LoadGradientsButton->GetWidgetName());

  //create test frame 
  this->TestFrame = vtkKWFrameWithLabel::New();
  this->TestFrame->SetParent(this->GetParent());
  this->TestFrame->Create();
  this->TestFrame->SetLabelText ("Test");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
    this->TestFrame->GetWidgetName());

  //create run button
  this->RunButton = vtkKWPushButton::New();
  this->RunButton->SetParent(this->TestFrame->GetFrame());
  this->RunButton->Create();
  this->RunButton->SetText("Run");
  this->RunButton->SetWidth(7);
  this->Script("pack %s -side right -anchor ne -padx 2 -pady 2", 
    this->RunButton->GetWidgetName());

  //create fiducial list
  this->Mask = vtkSlicerNodeSelectorWidget::New();
  this->Mask->SetParent(this->TestFrame->GetFrame());
  this->Mask->Create();
  this->Mask->SetLabelText("Fiducial List:");
  this->Script("pack %s -side right -anchor ne -padx 3 -pady 2", 
    this->Mask->GetWidgetName());

  //create save button
  this->CancelButton = vtkKWPushButton::New();
  this->CancelButton->SetParent(this->GetParent());
  this->CancelButton->Create();
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetWidth(10);
  this->Script("pack %s -side top -anchor ne -padx 2 -pady 2", 
    this->CancelButton->GetWidgetName());
  } 
