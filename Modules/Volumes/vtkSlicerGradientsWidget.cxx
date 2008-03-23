#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerGradientsWidget.h"
#include "vtkSlicerGradientEditorLogic.h"

#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkDoubleArray.h"
#include "vtkTimerLog.h"
#include <sstream>
//widgets
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWCheckButton.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkKWText.h"
#include "vtkKWLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWPushButton.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerGradientsWidget);
vtkCxxRevisionMacro (vtkSlicerGradientsWidget, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerGradientsWidget::vtkSlicerGradientsWidget(void)
  {
  this->ActiveVolumeNode = NULL;
  this->GradientsFrame = NULL;
  this->GradientsTextbox = NULL;
  this->LoadGradientsButton = NULL;
  this->ButtonsFrame = NULL;
  this->EnableGradientsButton = NULL;
  this->StatusLabel = NULL;
  this->Gradients = NULL;
  this->BValues = NULL;
  this->MessageDialog = NULL;
  this->Logic = NULL;
  }

//---------------------------------------------------------------------------
vtkSlicerGradientsWidget::~vtkSlicerGradientsWidget(void)
  {
  this->RemoveWidgetObservers();
  if (this->ActiveVolumeNode)
    {
    this->ActiveVolumeNode->Delete();
    this->ActiveVolumeNode = NULL;
    }
  if (this->GradientsFrame)
    {
    this->GradientsFrame->SetParent (NULL);
    this->GradientsFrame->Delete();
    this->GradientsFrame = NULL;
    }
  if (this->GradientsTextbox)
    {
    this->GradientsTextbox->SetParent (NULL);
    this->GradientsTextbox->Delete();
    this->GradientsTextbox = NULL;
    }
  if (this->LoadGradientsButton)
    {
    this->LoadGradientsButton->SetParent (NULL);
    this->LoadGradientsButton->Delete();
    this->LoadGradientsButton = NULL;
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
  if (this->StatusLabel)
    {
    this->StatusLabel->Delete();
    this->StatusLabel = NULL;
    }
  if (this->MessageDialog)
    {
    this->MessageDialog = NULL;
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::AddWidgetObservers ( )
  {
  this->EnableGradientsButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog()->AddObserver(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::RemoveWidgetObservers( )
  {
  this->EnableGradientsButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerGradientsWidget: " << this->GetClassName ( ) << "\n";
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::ProcessWidgetEvents (vtkObject *caller, unsigned long event, void *callData)
  {
  //enable the gradients textbox
  if(this->EnableGradientsButton == vtkKWCheckButton::SafeDownCast(caller) && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    this->GradientsTextbox->SetEnabled(this->EnableGradientsButton->GetSelectedState());
    this->UpdateWidget(this->ActiveVolumeNode);
    }

  //load gradients from file
  if (this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent)
    {
    const char *filename = this->LoadGradientsButton->GetWidget()->GetFileName();
    if(filename)
      {
      //check if file contains valid values
      int status = this->Logic->AddGradients(filename, this->ActiveVolumeNode->GetNumberOfGradients(), this->BValues, this->Gradients);
      if(status)
        {
        this->UpdateGradients(); //add gradients to gui, if valid
        this->SaveGradients(); //save to volumeNode
        }

      //show short message, that file has not valid values for gradients
      else
        {
        this->LoadGradientsButton->GetWidget()->SetText(""); //clear GUI text
        this->DisplayMessageDialog("File contains invalid values!");
        }
      }
    }//end load gradients from file

  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::DisplayMessageDialog(const char* message)
  {
  this->MessageDialog = vtkKWMessageDialog::New();
  this->MessageDialog->SetParent(this->GetParent());
  this->MessageDialog->SetDisplayPositionToMasterWindowCenter();
  this->MessageDialog->Create();
  this->MessageDialog->SetText(message);
  this->MessageDialog->Invoke();
  this->MessageDialog->Delete();
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *dwiNode)
  {
  if(this->ActiveVolumeNode != dwiNode)
    {
    vtkSetMRMLNodeMacro(this->ActiveVolumeNode, dwiNode); //set activeVolumeNode
    }
  this->Gradients = dwiNode->GetDiffusionGradients(); //set internal gradients
  this->BValues = dwiNode->GetBValues(); //set internal bValues
  this->UpdateGradients(); //update GUI
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::UpdateGradients()
  {
  std::stringstream output;
  vtkDoubleArray *factor = vtkDoubleArray::New();
  double g[3];

  // compute norm of each gradient 
  for(int i=0; i<this->Gradients->GetNumberOfTuples();i++)
    {
    this->Gradients->GetTuple(i,g);
    factor->InsertNextValue(sqrt(g[0]*g[0]+g[1]*g[1]+g[2]*g[2]));
    }

  // get range of norm array
  double range[2];
  factor->GetRange(range);

  // compute bValue
  double bValue = -1;
  for(int i = 0; i< this->BValues->GetSize(); i++)
    {
    double numerator = this->BValues->GetValue(i)*range[1];
    double denominator = factor->GetValue(i);
    if(!numerator == 0 && !denominator == 0)
      {
      bValue = numerator/denominator;
      break;
      }
    }

  // read in new bValue
  output << "DWMRI_b-value:= " << bValue << endl; 

  // read in new gradients
  // (this->Gradients->GetSize() is not always correct.)
  for(int i=0; i < this->Gradients->GetNumberOfTuples()*3; i=i+3)
    {
    output << "DWMRI_gradient_" << setfill('0') << setw(4) << i/3 << ":=" << " ";
    for(int j=i; j<i+3; j++)
      {
      output << this->Gradients->GetValue(j) << " ";
      }
    output << "\n";        
    }

  // write it on GUI
  this->GradientsTextbox->GetWidget()->SetText(output.str().c_str());
  this->UpdateStatusLabel(1);
  factor->Delete();
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::TextFieldModifiedCallback()
  {
  //Save and tricker changed event for undo
  this->Logic->SaveStateForUndoRedo();
  this->InvokeEvent(this->ChangedEvent);

  const char *oldGradients = this->GradientsTextbox->GetWidget()->GetText();
  int numberOfGradients = this->ActiveVolumeNode->GetNumberOfGradients();

  //parse new gradients and update status label
  int status = this->Logic->ParseGradients(oldGradients, numberOfGradients, this->BValues, this->Gradients);
  this->UpdateStatusLabel(status);

  //only if parsing was successful save gradients in activeVolumneNode
  if(status)
    {
    this->SaveGradients();
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::UpdateStatusLabel(int status)
  {
  if(!status)
    {
    this->StatusLabel->SetBackgroundColor(1, 0, 0); //set red     
    this->StatusLabel->SetText("Gradients: INVALID");
    }
  else
    {
    this->StatusLabel->SetBackgroundColor(0, 1, 0); //set green   
    this->StatusLabel->SetText("Gradients: VALID");    
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::SaveGradients( )
  {
  //TODO: takes to much time!! delete timer
  if (this->ActiveVolumeNode != NULL && this->Gradients != NULL && this->BValues != NULL)
    {
    vtkTimerLog *timer = vtkTimerLog::New();
    timer->StartTimer();
    this->ActiveVolumeNode->DisableModifiedEventOn();
    this->ActiveVolumeNode->SetDiffusionGradients(this->Gradients);
    this->ActiveVolumeNode->SetBValues(this->BValues);
    this->ActiveVolumeNode->SetModifiedSinceRead(1);
    this->ActiveVolumeNode->DisableModifiedEventOff();
    this->ActiveVolumeNode->InvokePendingModifiedEvent();
    timer->StopTimer();
    vtkWarningMacro("time: "<<timer->GetElapsedTime());
    timer->Delete();
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::SetLogic(vtkSlicerGradientEditorLogic *logic)
  {
  this->Logic = logic;
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::CreateWidget( )
  {
  //check if already created
  if (this->IsCreated()){
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  //call the superclass to create the whole widget
  this->Superclass::CreateWidget();

  //create gradient frame 
  this->GradientsFrame = vtkKWFrameWithLabel::New();
  this->GradientsFrame->SetParent(this->GetParent());
  this->GradientsFrame->Create();
  this->GradientsFrame->SetLabelText("Gradients");
  this->Script("pack %s -side top -anchor n -fill both -expand true -padx 2 -pady 2", 
    this->GradientsFrame->GetWidgetName());

  //create button frame 
  this->ButtonsFrame = vtkKWFrame::New();
  this->ButtonsFrame->SetParent(this->GradientsFrame->GetFrame());
  this->ButtonsFrame->Create();
  this->Script("pack %s -side top -anchor n -fill x -padx 1 -pady 1", 
    this->ButtonsFrame->GetWidgetName());

  //create enable gradientsTextbox button
  this->EnableGradientsButton = vtkKWCheckButton::New();
  this->EnableGradientsButton->SetParent(this->ButtonsFrame);
  this->EnableGradientsButton->SetText("Enable Textbox");
  this->EnableGradientsButton->Create();
  this->EnableGradientsButton->SetBalloonHelpString("Enable textbox to put in or change gradients manually.");
  this->Script("pack %s -side left -anchor nw -padx 2 ", 
    this->EnableGradientsButton->GetWidgetName());

  //create load button
  this->LoadGradientsButton = vtkKWLoadSaveButtonWithLabel::New();
  this->LoadGradientsButton->SetParent(this->ButtonsFrame);
  this->LoadGradientsButton->Create();
  this->LoadGradientsButton->SetLabelText("Load Gradients (.txt/.nhdr)");
  this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Open .txt/.nhdr File");
  this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes("{ {NHDRfile} {.nhdr} }{ {Textfile} {.txt} }");
  this->LoadGradientsButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
  this->LoadGradientsButton->SetBalloonHelpString("Load gradients from a plain text file or nhdr header.");
  this->Script("pack %s -side right -anchor ne -padx 2 ", 
    this->LoadGradientsButton->GetWidgetName());

  //create textbox for gradients
  this->GradientsTextbox = vtkKWTextWithScrollbars::New();
  this->GradientsTextbox->SetParent(this->GradientsFrame->GetFrame());
  this->GradientsTextbox->Create();
  this->GradientsTextbox->GetWidget()->SetBinding("<KeyRelease>", this, "TextFieldModifiedCallback");
  this->GradientsTextbox->SetHeight(100);
  this->GradientsTextbox->SetEnabled(0);
  this->Script("pack %s -side top -anchor s -fill both -expand true -padx 2 -pady 2", 
    this->GradientsTextbox->GetWidgetName());

  //create status label
  this->StatusLabel = vtkKWLabel::New();
  this->StatusLabel->SetParent(this->GradientsFrame->GetFrame());
  this->StatusLabel->Create();
  this->StatusLabel->SetBalloonHelpString("Shows current status of the given gradients in the textbox");
  this->Script("pack %s -side top -anchor s -fill both -expand true -padx 2 -pady 2", 
    this->StatusLabel->GetWidgetName());
  }
