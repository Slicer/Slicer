#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerGradientsWidget.h"
#include "vtkSlicerGradientEditorLogic.h"

#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWCheckButton.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkKWText.h"
#include "vtkKWLabel.h"

#include "vtkDoubleArray.h"
#include "vtkTimerLog.h"
#include <sstream>

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
      this->ActiveVolumeNode->SetDiffusionGradients(dwiNode->GetDiffusionGradients());
      this->ActiveVolumeNode->SetBValues(dwiNode->GetBValues());
      this->UpdateGradients();
      dwiNode->Delete();
      myLogic->Delete();
      }
    }

  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *dwiNode)
  {
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, dwiNode);
  this->UpdateGradients();
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::UpdateGradients()
  {
  std::stringstream output;
  vtkDoubleArray *factor = vtkDoubleArray::New();
  double g[3];

  vtkDoubleArray *gradients = this->ActiveVolumeNode->GetDiffusionGradients();
  vtkDoubleArray *bValues = this->ActiveVolumeNode->GetBValues();

  // compute factor of each gradient 
  for(int i=0; i<gradients->GetNumberOfTuples();i++)
    {
    gradients->GetTuple(i,g);
    factor->InsertNextValue(sqrt(g[0]*g[0]+g[1]*g[1]+g[2]*g[2]));
    }

  // get range of factor array
  double range[2];
  factor->GetRange(range);

  // get bValue
  double bValue = -1;
  for(int i = 0; i< bValues->GetSize(); i++)
    {
    double numerator = bValues->GetValue(i)*range[1];
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
  for(int i=0; i < gradients->GetSize(); i=i+3)
    {
    output << "DWMRI_gradient_" << setfill('0') << setw(4) << i/3 << ":=" << " ";
    for(int j=i; j<i+3; j++)
      {
      output << gradients->GetValue(j) << " ";
      }
    output << "\n";        
    }

  // write it on GUI
  this->GradientsTextbox->GetWidget()->SetText(output.str().c_str());
  factor->Delete();
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientsWidget::TextFieldModifiedCallback()
  {
  vtkTimerLog *timer = vtkTimerLog::New();

  vtkSlicerGradientEditorLogic *myLogic = vtkSlicerGradientEditorLogic::New();
  vtkDoubleArray *gradients = vtkDoubleArray::New();
  vtkDoubleArray *bValues = vtkDoubleArray::New();
  const char *oldGradients = this->GradientsTextbox->GetWidget()->GetText();
  int numberOfGradients = this->ActiveVolumeNode->GetNumberOfGradients();

  timer->StartTimer();
  if(!myLogic->ParseGradients(oldGradients, numberOfGradients, bValues, gradients))
    {
    this->StatusLabel->SetBackgroundColor(1, 0, 0);      
    this->StatusLabel->SetText("Gradients: INVALID");
    }
  else
    {
    this->StatusLabel->SetBackgroundColor(0, 1, 0);    
    this->StatusLabel->SetText("Gradients: VALID");

    // write gradients/bValues back to node
    if (this->ActiveVolumeNode != NULL && gradients != NULL && bValues != NULL)
      {
      this->ActiveVolumeNode->DisableModifiedEventOn();
      this->ActiveVolumeNode->SetDiffusionGradients(gradients);
      this->ActiveVolumeNode->SetBValues(bValues);
      this->ActiveVolumeNode->SetModifiedSinceRead(1);
      this->ActiveVolumeNode->DisableModifiedEventOff();
      this->ActiveVolumeNode->InvokePendingModifiedEvent();
      }
    }

  timer->StopTimer();
  vtkWarningMacro("time: "<<timer->GetElapsedTime());
  myLogic->Delete();
  timer->Delete();
  gradients->Delete();
  bValues->Delete();
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
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
    this->GradientsFrame->GetWidgetName());

  //create gradient frame 
  this->ButtonsFrame = vtkKWFrame::New();
  this->ButtonsFrame->SetParent(this->GradientsFrame->GetFrame());
  this->ButtonsFrame->Create();
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 1", 
    this->ButtonsFrame->GetWidgetName());

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

  //create textfield for GradientsTextbox
  this->GradientsTextbox = vtkKWTextWithScrollbars::New();
  this->GradientsTextbox->SetParent(this->GradientsFrame->GetFrame());
  this->GradientsTextbox->Create();
  this->GradientsTextbox->GetWidget()->SetBinding("<KeyRelease>", this, "TextFieldModifiedCallback");
  this->GradientsTextbox->SetEnabled(0);
  this->GradientsTextbox->SetHeight(30);
  this->Script("pack %s -side top -anchor s -fill x -padx 2 -pady 2", 
    this->GradientsTextbox->GetWidgetName());

  //create status label
  this->StatusLabel = vtkKWLabel::New();
  this->StatusLabel->SetParent(this->GradientsFrame->GetFrame());
  this->StatusLabel->Create();
  this->Script("pack %s -side top -anchor s -fill x -padx 2 -pady 2", 
    this->StatusLabel->GetWidgetName());

  }
