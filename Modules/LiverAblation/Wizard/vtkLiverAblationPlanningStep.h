#ifndef __vtkLiverAblationPlanningStep_h
#define __vtkLiverAblationPlanningStep_h

#include "vtkLiverAblationStep.h"

#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkResampleImageFilter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"

class vtkKWFrameWithLabel;
class vtkKWFrame;
class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWPushButton;
class vtkKWMenuButton;
class vtkSlicerNodeSelectorWidget;


class VTK_LIVERABLATION_EXPORT vtkLiverAblationPlanningStep : public vtkLiverAblationStep
{
public:
  static vtkLiverAblationPlanningStep *New();
  vtkTypeRevisionMacro(vtkLiverAblationPlanningStep,vtkLiverAblationStep);
  void PrintSelf(ostream& os, vtkIndent indent);



  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  virtual void CustomizeButtonCallback();
  virtual void GoToNavButtonCallback();

protected:
  vtkLiverAblationPlanningStep();
  ~vtkLiverAblationPlanningStep();

  vtkKWFrameWithLabel *ProbeFrame;
  vtkKWFrameWithLabel *SpacingFrame;
  vtkKWFrameWithLabel *MoreFrame;
  vtkKWFrame         *CustomizeFrame;

  // Probe info
  vtkKWEntryWithLabel    *ProbeAEntry;
  vtkKWEntryWithLabel    *ProbeBEntry;
  vtkKWEntryWithLabel    *ProbeCEntry;

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
  vtkSlicerNodeSelectorWidget  *LabelmapSelector;

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
  vtkLiverAblationPlanningStep(const vtkLiverAblationPlanningStep&);
  void operator=(const vtkLiverAblationPlanningStep&);
};

#endif
