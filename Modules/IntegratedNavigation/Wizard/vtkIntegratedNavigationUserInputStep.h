#ifndef __vtkIntegratedNavigationUserInputStep_h
#define __vtkIntegratedNavigationUserInputStep_h

#include "vtkIntegratedNavigationStep.h"

#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkResampleImageFilter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWPushButton;
class vtkKWMenuButton;


class VTK_IGT_EXPORT vtkIntegratedNavigationUserInputStep : public vtkIntegratedNavigationStep
{
public:
  static vtkIntegratedNavigationUserInputStep *New();
  vtkTypeRevisionMacro(vtkIntegratedNavigationUserInputStep,vtkIntegratedNavigationStep);
  void PrintSelf(ostream& os, vtkIndent indent);



  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  virtual void CustomizeButtonCallback();
  virtual void GoToNavButtonCallback();

protected:
  vtkIntegratedNavigationUserInputStep();
  ~vtkIntegratedNavigationUserInputStep();

  vtkKWFrameWithLabel *ProbeFrame;
  vtkKWFrameWithLabel *SpacingFrame;
  vtkKWFrameWithLabel *MoreFrame;
  vtkKWFrameWithLabel *CustomizeFrame;

  // Probe info
  vtkKWEntryWithLabel    *ProbeAEntry;
  vtkKWEntryWithLabel    *ProbeBEntry;

  // Sample spacing 
  vtkKWEntryWithLabel    *SpacingXEntry;
  vtkKWEntryWithLabel    *SpacingYEntry;
  vtkKWEntryWithLabel    *SpacingZEntry;

  vtkKWEntryWithLabel    *AngularResolutionEntry;
  vtkKWEntryWithLabel    *MaxAngleEntry;
  vtkKWEntryWithLabel    *NumberOfAblationsEntry;
  vtkKWEntryWithLabel    *NumberOfTrajectoriesEntry;
  vtkKWEntryWithLabel    *NumberOfPuncturesEntry;
  vtkKWEntryWithLabel    *TumorMarginEntry;
  vtkKWEntryWithLabel    *NoPassMarginEntry;

  vtkKWPushButton              *CustomizeButton;
//  vtkKWPushButton              *GoToNavButton;
  vtkKWMenuButton              *LabelmapSourceMenu;

  bool CheckInputErrors();
  void UpdateLabelmapSourceMenu();

  //BTX
  typedef unsigned short LabelType;
  typedef itk::Image<LabelType,3> LabeledVolumeType;
  typedef itk::ImageFileReader<LabeledVolumeType> LabeledVolumeReaderType;
  typedef itk::ImageFileWriter<LabeledVolumeType> LabeledVolumeWriterType;

  LabeledVolumeType::Pointer AddAblationMarginAndResample(double ablationMargin,
                                                        double noPassMargin,
                                                        double gridSpacing[3],
                                                        LabeledVolumeType::Pointer inputVolume);
  LabeledVolumeType::Pointer dilateLabeledVolume(LabeledVolumeType::Pointer inputVolume,
                                               LabeledVolumeType::PixelType dilateValue,
                                               double structuringElementRadius);

  void Customize(LabeledVolumeType::Pointer labeledVolume);
  //ETX

private:
  vtkIntegratedNavigationUserInputStep(const vtkIntegratedNavigationUserInputStep&);
  void operator=(const vtkIntegratedNavigationUserInputStep&);
};

#endif
