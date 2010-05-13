#include "vtkBrainlabModuleUserInputStep.h"

#include "vtkBrainlabModuleGUI.h"
#include "vtkBrainlabModuleMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWMessageDialog.h" 
#include "vtkSlicerApplication.h" 

#include "vtkImageData.h"
#include "itkImage.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkBrainlabModuleUserInputStep);
vtkCxxRevisionMacro(vtkBrainlabModuleUserInputStep, "$Revision: 1.4 $");

//----------------------------------------------------------------------------
vtkBrainlabModuleUserInputStep::vtkBrainLabModuleUserInputStep()
{
  this->SetName("2/3. User input");
  this->SetDescription("Collect user information.");

  this->ProbeFrame = NULL;
  this->SpacingFrame = NULL;
  this->MoreFrame = NULL;
  this->CustomizeFrame = NULL;

  this->ProbeAEntry   = vtkKWEntryWithLabel::New();
  this->ProbeBEntry   = vtkKWEntryWithLabel::New();
  this->SpacingXEntry = vtkKWEntryWithLabel::New();
  this->SpacingYEntry = vtkKWEntryWithLabel::New();
  this->SpacingZEntry = vtkKWEntryWithLabel::New();

  this->AngularResolutionEntry = vtkKWEntryWithLabel::New();
  this->MaxAngleEntry = vtkKWEntryWithLabel::New();
  this->NumberOfAblationsEntry = vtkKWEntryWithLabel::New();
  this->NumberOfTrajectoriesEntry = vtkKWEntryWithLabel::New();
  this->NumberOfPuncturesEntry = vtkKWEntryWithLabel::New();
  this->TumorMarginEntry = vtkKWEntryWithLabel::New();
  this->NoPassMarginEntry = vtkKWEntryWithLabel::New();

}



//----------------------------------------------------------------------------
vtkBrainlabModuleUserInputStep::~vtkBrainLabModuleUserInputStep()
{
  if(this->ProbeFrame)
    {
    this->ProbeFrame->Delete();
    this->ProbeFrame = NULL;
    }
  if(this->SpacingFrame)
    {
    this->SpacingFrame->Delete();
    this->SpacingFrame = NULL;
    }
  if(this->MoreFrame)
    {
    this->MoreFrame->Delete();
    this->MoreFrame = NULL;
    }
  if(this->CustomizeFrame)
    {
    this->CustomizeFrame->Delete();
    this->CustomizeFrame = NULL;
    }

  if (this->ProbeAEntry) 
    {
    this->ProbeAEntry->SetParent(NULL);
    this->ProbeAEntry->Delete();
    this->ProbeAEntry = NULL;
    }
  if (this->ProbeBEntry) 
    {
    this->ProbeBEntry->SetParent(NULL);
    this->ProbeBEntry->Delete();
    this->ProbeBEntry = NULL;
    }

  if (this->SpacingXEntry) 
    {
    this->SpacingXEntry->SetParent(NULL);
    this->SpacingXEntry->Delete();
    this->SpacingXEntry = NULL;
    }
  if (this->SpacingYEntry) 
    {
    this->SpacingYEntry->SetParent(NULL);
    this->SpacingYEntry->Delete();
    this->SpacingYEntry = NULL;
    }
  if (this->SpacingZEntry) 
    {
    this->SpacingZEntry->SetParent(NULL);
    this->SpacingZEntry->Delete();
    this->SpacingZEntry = NULL;
    }

  if (this->AngularResolutionEntry)
    {
    this->AngularResolutionEntry->SetParent(NULL);
    this->AngularResolutionEntry->Delete();
    this->AngularResolutionEntry = NULL;
    }
  if (this->MaxAngleEntry)
    {
    this->MaxAngleEntry->SetParent(NULL);
    this->MaxAngleEntry->Delete();
    this->MaxAngleEntry = NULL;
    }
  if (this->NumberOfAblationsEntry)
    {
    this->NumberOfAblationsEntry->SetParent(NULL);
    this->NumberOfAblationsEntry->Delete();
    this->NumberOfAblationsEntry = NULL;
    }
   if (this->NumberOfTrajectoriesEntry)
    {
    this->NumberOfTrajectoriesEntry->SetParent(NULL);
    this->NumberOfTrajectoriesEntry->Delete();
    this->NumberOfTrajectoriesEntry = NULL;
    }
  if (this->NumberOfPuncturesEntry)
    {
    this->NumberOfPuncturesEntry->SetParent(NULL);
    this->NumberOfPuncturesEntry->Delete();
    this->NumberOfPuncturesEntry = NULL;
    }
  if (this->TumorMarginEntry)
    {
    this->TumorMarginEntry->SetParent(NULL);
    this->TumorMarginEntry->Delete();
    this->TumorMarginEntry = NULL;
    }
  if (this->NoPassMarginEntry)
    {
    this->NoPassMarginEntry->SetParent(NULL);
    this->NoPassMarginEntry->Delete();
    this->NoPassMarginEntry = NULL;
    }

  if (this->CustomizeButton)
    {
    this->CustomizeButton->SetParent(NULL);
    this->CustomizeButton->Delete();
    this->CustomizeButton = NULL;
    }
  /*
  if (this->GoToNavButton)
    {
    this->GoToNavButton->SetParent(NULL);
    this->GoToNavButton->Delete();
    this->GoToNavButton = NULL;
    }
    */
}



//----------------------------------------------------------------------------
void vtkBrainlabModuleUserInputStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  wizard_widget->GetCancelButton()->SetEnabled(0);

  // Probe information frame
  // ======================================================================
  if (!this->ProbeFrame)
    {
    this->ProbeFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->ProbeFrame->IsCreated())
    {
    this->ProbeFrame->SetParent(
      wizard_widget->GetClientArea());
    this->ProbeFrame->Create();
    this->ProbeFrame->SetLabelText("Probe diameter [mm]");
//    this->ProbeFrame->SetHeight(100);
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->ProbeFrame->GetWidgetName());


  this->ProbeAEntry->SetParent (this->ProbeFrame->GetFrame());
  this->ProbeAEntry->Create ( );
  this->ProbeAEntry->SetLabelText("a:");
  this->ProbeAEntry->GetWidget()->SetValueAsDouble(30.0);
// this->ProbeAEntry->SetBalloonHelpString("Maximum number of seeds");

  this->ProbeBEntry->SetParent (this->ProbeFrame->GetFrame());
  this->ProbeBEntry->Create ( );
  this->ProbeBEntry->SetLabelText("b:");
  this->ProbeBEntry->GetWidget()->SetValueAsDouble(30.0);

  this->Script ( "pack %s %s -side top -anchor w -expand y -fill x -padx 2 -pady 2",
                 this->ProbeAEntry->GetWidgetName(),
                 this->ProbeBEntry->GetWidgetName());

  // Sample spacing frame
  // ======================================================================
  if (!this->SpacingFrame)
    {
    this->SpacingFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->SpacingFrame->IsCreated())
    {
    this->SpacingFrame->SetParent(
      wizard_widget->GetClientArea());
    this->SpacingFrame->Create();
    this->SpacingFrame->SetLabelText("Sample spacing [mm]");
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->SpacingFrame->GetWidgetName());

  this->SpacingXEntry->SetParent (this->SpacingFrame->GetFrame());
  this->SpacingXEntry->Create ( );
  this->SpacingXEntry->SetLabelText("X:");
  this->SpacingXEntry->GetWidget()->SetValueAsDouble(5.0);
// this->SpacingXEntry->SetBalloonHelpString("Maximum number of seeds");

  this->SpacingYEntry->SetParent (this->SpacingFrame->GetFrame());
  this->SpacingYEntry->Create ( );
  this->SpacingYEntry->SetLabelText("Y:");
  this->SpacingYEntry->GetWidget()->SetValueAsDouble(5.0);

  this->SpacingZEntry->SetParent (this->SpacingFrame->GetFrame());
  this->SpacingZEntry->Create ( );
  this->SpacingZEntry->SetLabelText("Z:");
  this->SpacingZEntry->GetWidget()->SetValueAsDouble(5.0);

  this->Script ( "pack %s %s %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->SpacingXEntry->GetWidgetName(),
                 this->SpacingYEntry->GetWidgetName(),
                 this->SpacingZEntry->GetWidgetName());

  // More info frame
  // ======================================================================
  if (!this->MoreFrame)
    {
    this->MoreFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->MoreFrame->IsCreated())
    {
    this->MoreFrame->SetParent(
      wizard_widget->GetClientArea());
    this->MoreFrame->Create();
    this->MoreFrame->SetLabelText("More information");
//    this->MoreFrame->SetHeight(100);
    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->MoreFrame->GetWidgetName());

  this->AngularResolutionEntry->SetParent (this->MoreFrame->GetFrame());
  this->AngularResolutionEntry->Create ( );
  this->AngularResolutionEntry->SetLabelText("Angular resolution [radians]:");
  this->AngularResolutionEntry->GetWidget()->SetValueAsDouble(0.1);
// this->AngularResolutionEntry->SetBalloonHelpString("Maximum number of seeds");

  this->MaxAngleEntry->SetParent (this->MoreFrame->GetFrame());
  this->MaxAngleEntry->Create ( );
  this->MaxAngleEntry->SetLabelText("Max angle [radians]:");
  this->MaxAngleEntry->GetWidget()->SetValueAsDouble(0.732);

  this->NumberOfAblationsEntry->SetParent (this->MoreFrame->GetFrame());
  this->NumberOfAblationsEntry->Create ( );
  this->NumberOfAblationsEntry->SetLabelText("Maximal # of ablations:");
  this->NumberOfAblationsEntry->GetWidget()->SetValueAsInt(8);

  this->NumberOfTrajectoriesEntry->SetParent (this->MoreFrame->GetFrame());
  this->NumberOfTrajectoriesEntry->Create ( );
  this->NumberOfTrajectoriesEntry->SetLabelText("Maximal # of rays:");
  this->NumberOfTrajectoriesEntry->GetWidget()->SetValueAsInt(4);

  this->NumberOfPuncturesEntry->SetParent (this->MoreFrame->GetFrame());
  this->NumberOfPuncturesEntry->Create ( );
  this->NumberOfPuncturesEntry->SetLabelText("Maximal # of punctures:");
  this->NumberOfPuncturesEntry->GetWidget()->SetValueAsInt(8);

  this->TumorMarginEntry->SetParent (this->MoreFrame->GetFrame());
  this->TumorMarginEntry->Create ( );
  this->TumorMarginEntry->SetLabelText("Tumor margin [mm]:");
  this->TumorMarginEntry->GetWidget()->SetValueAsDouble(10.0);

  this->NoPassMarginEntry->SetParent (this->MoreFrame->GetFrame());
  this->NoPassMarginEntry->Create ( );
  this->NoPassMarginEntry->SetLabelText("No pass margin [mm]:");
  this->NoPassMarginEntry->GetWidget()->SetValueAsDouble(5.0);

  this->Script ( "pack %s %s %s %s %s %s %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->AngularResolutionEntry->GetWidgetName(),
                 this->MaxAngleEntry->GetWidgetName(),
                 this->NumberOfAblationsEntry->GetWidgetName(),
                 this->NumberOfTrajectoriesEntry->GetWidgetName(),
                 this->NumberOfPuncturesEntry->GetWidgetName(),
                 this->TumorMarginEntry->GetWidgetName(),
                 this->NoPassMarginEntry->GetWidgetName());


  // Export and run frame
  // ======================================================================
  if (!this->CustomizeFrame)
    {
    this->CustomizeFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->CustomizeFrame->IsCreated())
    {
    this->CustomizeFrame->SetParent(
      wizard_widget->GetClientArea());
    this->CustomizeFrame->Create();
    this->CustomizeFrame->SetLabelText("Customize parameters for optimization");
//    this->CustomizeFrame->SetHeight(50);
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->CustomizeFrame->GetWidgetName());


  vtkKWFrame *nodeFrame = vtkKWFrame::New();
  nodeFrame->SetParent(this->CustomizeFrame->GetFrame());
  nodeFrame->Create();
  this->Script ( "pack %s -fill both -expand true",  
                nodeFrame->GetWidgetName());

  vtkKWLabel *nodeLabel = vtkKWLabel::New();
  nodeLabel->SetParent(nodeFrame);
  nodeLabel->Create();
  //nodeLabel->SetWidth(20);
  nodeLabel->SetText("Labelmap source: ");

  this->LabelmapSourceMenu = vtkKWMenuButton::New();
  this->LabelmapSourceMenu->SetParent(nodeFrame);
  this->LabelmapSourceMenu->Create();
  this->LabelmapSourceMenu->SetWidth(20);
  this->UpdateLabelmapSourceMenu();

  this->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
              nodeLabel->GetWidgetName() , this->LabelmapSourceMenu->GetWidgetName());

  this->CustomizeButton = vtkKWPushButton::New();
  this->CustomizeButton->SetParent (this->CustomizeFrame->GetFrame());
  this->CustomizeButton->Create();
  this->CustomizeButton->SetText("Customize");
  this->CustomizeButton->SetCommand(this, "CustomizeButtonCallback");
  this->CustomizeButton->SetWidth(20);

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->CustomizeButton->GetWidgetName()
               );

  /*
  this->GoToNavButton = vtkKWPushButton::New();
  this->GoToNavButton->SetParent (this->CustomizeFrame->GetFrame());
  this->GoToNavButton->Create();
  this->GoToNavButton->SetText("Go to Navigation");
  this->GoToNavButton->SetCommand(this, "GoToNavButtonCallback");
  this->GoToNavButton->SetWidth(20);

  this->Script("pack %s %s %s %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->DirectoryButton->GetWidgetName(),
               this->FileNameEntry->GetWidgetName(),
               this->CustomizeButton->GetWidgetName(),
               this->GoToNavButton->GetWidgetName()
               );
 
               */
  //Add a help to the step
  vtkKWPushButton * helpButton =  wizard_widget->GetHelpButton();

  vtkKWMessageDialog *msg_dlg1 = vtkKWMessageDialog::New();
  msg_dlg1->SetParent(wizard_widget->GetClientArea());
  msg_dlg1->SetStyleToOkCancel();
  msg_dlg1->Create();
  msg_dlg1->SetTitle("Optimization step");
  msg_dlg1->SetText( "This is the first step in IGT applications. This step includes establishing "
                     "communication with Planning system, tracking device, endoscope, external tracking "
                     "software such as IGSTK/OpenIGT Lin and robot controller if available");

  helpButton->SetCommand(msg_dlg1, "Invoke");

  msg_dlg1->Delete();
}


void vtkBrainlabModuleUserInputStep::CustomizeButtonCallback()
{
  // check user input errors
  if (! CheckInputErrors())
    {
    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent (this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow());
    dialog->SetStyleToMessage();
    std::string msg = "Please make sure all data fields have positive values.";
    dialog->SetText(msg.c_str());
    dialog->Create ( );
    dialog->Invoke();
    dialog->Delete();
    return;
    }

  // check if the device name exists in the MRML tree
  vtkMRMLScalarVolumeNode *volNode = NULL;
  vtkBrainlabModuleMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkCollection* collection = mrmlManager->GetMRMLScene()->GetNodesByName("Working");
  int nItems = collection->GetNumberOfItems();
  if (nItems > 0)
    {
    volNode = vtkMRMLScalarVolumeNode::SafeDownCast(collection->GetItemAsObject(0));
    }
  if (! volNode)
    {
    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent (this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow());
    dialog->SetStyleToMessage();
    std::string msg = "The labelmap doesn't exist.";
    dialog->SetText(msg.c_str());
    dialog->Create ( );
    dialog->Invoke();
    dialog->Delete();
    return;
    }


  // Wrap scalars into an ITK image
  // - mostly rely on defaults for spacing, origin etc for this filter
  vtkImageData *input = vtkImageData::New();
  input->DeepCopy(volNode->GetImageData());
  LabelType *inPtr = (LabelType *)input->GetScalarPointer();
  int dims[3];
  input->GetDimensions(dims);
  double spacing[3];
  volNode->GetSpacing(spacing);

  cout << "spacing: " << spacing[0] << "  " << spacing[1] << "  " << spacing[2] << endl;

//  typedef itk::Image<short, 3> LabeledVolumeType;
  LabeledVolumeType::Pointer inImage = LabeledVolumeType::New();
  inImage->GetPixelContainer()->SetImportPointer(inPtr, dims[0]*dims[1]*dims[2], false);
  LabeledVolumeType::RegionType region;
  LabeledVolumeType::IndexType index;
  LabeledVolumeType::SizeType size;
  LabeledVolumeType::SpacingType sp;
  index[0] = index[1] = index[2] = 0;
  size[0] = dims[0];
  size[1] = dims[1];
  size[2] = dims[2];
  sp[0] = spacing[0];
  sp[1] = spacing[1];
  sp[2] = spacing[2];
  region.SetIndex(index);
  region.SetSize(size);
  inImage->SetLargestPossibleRegion(region);
  inImage->SetBufferedRegion(region);
  inImage->SetSpacing(sp);

  this->Customize(inImage);
  input->Delete();
}





void vtkBrainlabModuleUserInputStep::GoToNavButtonCallback()
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
  std::string name = "IGT Navigation";
  vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName(name.c_str());        
  if ( currentModule )
    {
    currentModule->Enter( );
    currentModule->GetUIPanel()->Raise();
    this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText (name.c_str());
    }
}



bool vtkBrainlabModuleUserInputStep::CheckInputErrors()
{
  bool error = false;

  // all data fields must be positive values.
  if ( ! (this->ProbeAEntry->GetWidget()->GetValueAsInt()           > 0 &&
      this->ProbeBEntry->GetWidget()->GetValueAsInt()               > 0 &&
      this->SpacingXEntry->GetWidget()->GetValueAsInt()             > 0 &&
      this->SpacingYEntry->GetWidget()->GetValueAsInt()             > 0 &&
      this->SpacingZEntry->GetWidget()->GetValueAsInt()             > 0 &&
      this->AngularResolutionEntry->GetWidget()->GetValueAsInt()    > 0 &&
      this->MaxAngleEntry->GetWidget()->GetValueAsInt()             > 0 &&
      this->NumberOfAblationsEntry->GetWidget()->GetValueAsInt()    > 0 &&
      this->NumberOfTrajectoriesEntry->GetWidget()->GetValueAsInt() > 0 &&
      this->NumberOfPuncturesEntry->GetWidget()->GetValueAsInt()    > 0 &&
      this->TumorMarginEntry->GetWidget()->GetValueAsInt()          > 0 &&
      this->NoPassMarginEntry->GetWidget()->GetValueAsInt()         > 0))
    {
    error = true;
    }

  return error;
}



//---------------------------------------------------------------------------
void vtkBrainlabModuleUserInputStep::UpdateLabelmapSourceMenu()
{
  if (!this->LabelmapSourceMenu)
    {
    return;
    }
  
  /*
  this->GetLogic()->GetDeviceNamesFromMrml(this->LabelmapSourceList, "LinearTransform");
  vtkOpenIGTLinkIFLogic::IGTLMrmlNodeListType::iterator iter;
  this->LabelmapSourceMenu->GetMenu()->DeleteAllItems();
  for (iter = this->LabelmapSourceList.begin();
       iter != this->LabelmapSourceList.end();
       iter ++)
    {
    char str[256];
    sprintf(str, "%s (%s)", iter->name.c_str(), iter->type.c_str());
    this->LabelmapSourceMenu->GetMenu()->AddRadioButton(str);
    }
    */

   this->LabelmapSourceMenu->GetMenu()->AddRadioButton("Working"); 
}

 

//----------------------------------------------------------------------------
void vtkBrainlabModuleUserInputStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}



vtkBrainlabModuleUserInputStep::LabeledVolumeType::Pointer vtkBrainLabModuleUserInputStep::AddAblationMarginAndResample(double ablationMargin,
                                                        double noPassMargin,
                                                        double gridSpacing[3],
                                                        LabeledVolumeType::Pointer labeledVolume)
{
  unsigned short TUMOR = 1;
  unsigned short BACKGROUND = 0;
  unsigned short TUMOR_MARGIN = 4;
  unsigned short NO_PASS_REGION = 3;
  //unsigned short ENTRY_REGION = 2;

  LabeledVolumeType::Pointer dilatedTumorVolume = dilateLabeledVolume(labeledVolume,
                                                                      TUMOR,
                                                                      this->TumorMarginEntry->GetWidget()->GetValueAsInt());
  LabeledVolumeType::Pointer dilatedNoPassVolume = dilateLabeledVolume(labeledVolume,
                                                                       NO_PASS_REGION,
                                                                       this->NoPassMarginEntry->GetWidget()->GetValueAsInt());

       //go over the dilated tumor and no pass volumes update the original data so that we don't 
       //overwrite other labels except background and keep a margin from the no pass zones
  typedef itk::ImageRegionConstIterator< LabeledVolumeType > ConstIteratorType;
  typedef itk::ImageRegionIterator< LabeledVolumeType > IteratorType;
  ConstIteratorType constIteratorTumor(dilatedTumorVolume, 
                                       dilatedTumorVolume->GetLargestPossibleRegion());
  ConstIteratorType constIteratorNoPass(dilatedNoPassVolume, 
                                        dilatedNoPassVolume->GetLargestPossibleRegion());
  IteratorType iterator(labeledVolume, 
                        labeledVolume->GetLargestPossibleRegion());  
  

  for (constIteratorNoPass.GoToBegin(), constIteratorTumor.GoToBegin(), iterator.GoToBegin(); 
       !constIteratorTumor.IsAtEnd(); 
        ++constIteratorNoPass, ++constIteratorTumor, ++iterator) 
  {
    if(constIteratorTumor.Get() == TUMOR && 
       constIteratorNoPass.Get()!=NO_PASS_REGION && 
       iterator.Get() == BACKGROUND)      
      iterator.Set(TUMOR_MARGIN);
  }
           //write the results of "corrected" dilation so we can view them
  LabeledVolumeWriterType::Pointer writer = LabeledVolumeWriterType::New();
  writer->SetFileName("afterDilation.mhd");
  writer->SetInput(labeledVolume);
  writer->Update();

            //resample according to the requested sampling. we use nearest neighbor because
            //we are interpolating a discrete set of values
  itk::ResampleImageFilter<LabeledVolumeType,LabeledVolumeType>::Pointer resampleFilter =
    itk::ResampleImageFilter<LabeledVolumeType,LabeledVolumeType>::New();

  itk::NearestNeighborInterpolateImageFunction<LabeledVolumeType, double >::Pointer nnInterpolator =
    itk::NearestNeighborInterpolateImageFunction<LabeledVolumeType, double >::New();

  LabeledVolumeType::SizeType resampledSize, originalSize;
  originalSize = labeledVolume->GetLargestPossibleRegion().GetSize();
  LabeledVolumeType::SpacingType originalSpacing = labeledVolume->GetSpacing();

  resampledSize[0] = static_cast<unsigned int> ((originalSpacing[0]/gridSpacing[0])*originalSize[0]);
  resampledSize[1] = static_cast<unsigned int> ((originalSpacing[1]/gridSpacing[1])*originalSize[1]);
  resampledSize[2] = static_cast<unsigned int> ((originalSpacing[2]/gridSpacing[2])*originalSize[2]);

  resampleFilter->SetOutputSpacing(gridSpacing);
  resampleFilter->SetSize(resampledSize);
  resampleFilter->SetOutputOrigin(labeledVolume->GetOrigin());
  resampleFilter->SetOutputDirection(labeledVolume->GetDirection());
  resampleFilter->SetInterpolator(nnInterpolator);
  resampleFilter->SetInput(labeledVolume);
  
  resampleFilter->Update();
  return resampleFilter->GetOutput();
}


vtkBrainlabModuleUserInputStep::LabeledVolumeType::Pointer vtkBrainLabModuleUserInputStep::dilateLabeledVolume(LabeledVolumeType::Pointer inputVolume,
                                               LabeledVolumeType::PixelType dilateValue,
                                               double structuringElementRadius)
{
  typedef itk::BinaryBallStructuringElement<LabelType, 3> KernelType;
  typedef itk::BinaryDilateImageFilter<LabeledVolumeType, LabeledVolumeType, KernelType> DilationFilterType;

  DilationFilterType::Pointer dilationFilter = DilationFilterType::New();

  KernelType structuringElement;
  KernelType::SizeType elementSize;
  LabeledVolumeType::SpacingType imageSpacing = inputVolume->GetSpacing();
  elementSize[0] = (unsigned int)(structuringElementRadius/(double)imageSpacing[0] + 0.5);
  elementSize[1] = (unsigned int)(structuringElementRadius/(double)imageSpacing[1] + 0.5);
  elementSize[2] = (unsigned int)(structuringElementRadius/(double)imageSpacing[2] + 0.5);
  structuringElement.SetRadius(elementSize);
  structuringElement.CreateStructuringElement();

  dilationFilter->SetInput(inputVolume);
  dilationFilter->SetKernel(structuringElement);
                  //indicate the forground value, all other values are background
                  //this enables the use of a binary dilation filter on a labeled image
  dilationFilter->SetDilateValue(dilateValue);
  dilationFilter->Update();
  return dilationFilter->GetOutput();
}


void vtkBrainlabModuleUserInputStep::Customize(LabeledVolumeType::Pointer labeledVolume)
{
  //initialize all variables using default values
  double probeDiameters[2];
  probeDiameters[0] = this->ProbeAEntry->GetWidget()->GetValueAsDouble();
  probeDiameters[1] = this->ProbeBEntry->GetWidget()->GetValueAsDouble();
  double gridSpacing[3];
  gridSpacing[0] = this->SpacingXEntry->GetWidget()->GetValueAsDouble();
  gridSpacing[1] = this->SpacingYEntry->GetWidget()->GetValueAsDouble();
  gridSpacing[2] = this->SpacingZEntry->GetWidget()->GetValueAsDouble();

  double angularResolution = this->AngularResolutionEntry->GetWidget()->GetValueAsDouble();
  double maxAngle = this->MaxAngleEntry->GetWidget()->GetValueAsDouble();
  unsigned int maxAblations = this->NumberOfAblationsEntry->GetWidget()->GetValueAsInt();
  unsigned int maxRays = this->NumberOfTrajectoriesEntry->GetWidget()->GetValueAsInt();
  unsigned int maxPunctures = this->NumberOfPuncturesEntry->GetWidget()->GetValueAsInt();
  double ablationMargin = this->TumorMarginEntry->GetWidget()->GetValueAsDouble();
  double noPassMargin = this->NoPassMarginEntry->GetWidget()->GetValueAsDouble();

  //std::string inputFileName = DEFAULT_INPUT_FILE_NAME;
  std::string tpsFileName = "TPS.txt"; 
  LabeledVolumeType::Pointer result = AddAblationMarginAndResample(ablationMargin,
                                                                   noPassMargin,
                                                                   gridSpacing,  
                                                                   labeledVolume);
           //write the input for the optimization
  std::ofstream file(tpsFileName.c_str());

  file << "! Part 1: Scalar information" << std::endl;
  file << probeDiameters[0] << ',' << probeDiameters[1] << "\t! Ablation diameters [mm]" << std::endl;
  file << gridSpacing[0] << ',' << gridSpacing[1] << ',' << gridSpacing[2] << "\t! Grid spacing [mm]" << std::endl;
  file << angularResolution << "\t! Angular resolution" << std::endl;
  file << maxAngle << "\t! Maximum angle between directions" << std::endl;
  file << maxAblations << "\t! Maximum no. of ablations" << std::endl;
  file << maxRays << "\t! Maximum no. of directions" << std::endl;
  file << maxPunctures << "\t! Maximum no. of punctures" << std::endl;
  file << "! Part 2: Voxel information" << std::endl;

  typedef itk::ImageRegionConstIterator< LabeledVolumeType > ConstIteratorType;
  ConstIteratorType constIterator(result, 
                                  result->GetLargestPossibleRegion());
  LabeledVolumeType::IndexType index;
  LabeledVolumeType::PointType point;

  //Point< TCoordRep, VImageDimension > &point
  unsigned short BACKGROUND = 0;
  for (constIterator.GoToBegin(); !constIterator.IsAtEnd(); ++constIterator) 
  {
    if(constIterator.Get() != BACKGROUND)      
    {
      index = constIterator.GetIndex();
      result->TransformIndexToPhysicalPoint(index, point);
      file << point[0] << ',' << point[1] << ',' << point[2] << ',' << constIterator.Get() << std::endl;
    }
  }
}

