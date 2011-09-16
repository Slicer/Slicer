#ifndef __vtkEMSegmentIntensityDistributionsStep_h
#define __vtkEMSegmentIntensityDistributionsStep_h

#include "vtkEMSegmentStep.h"

class vtkKWLabel;
class vtkKWFrameWithLabel;
class vtkKWMatrixWidgetWithLabel;
class vtkKWNotebook;
class vtkKWMultiColumnListWithScrollbarsWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;
class vtkKWWidget;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentIntensityDistributionsStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentIntensityDistributionsStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentIntensityDistributionsStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();
  virtual void HideUserInterface();

  // Description:
  // Callbacks.
  virtual void IntensityDistributionSpecificationCallback(vtkIdType, int type);
  virtual void DisplaySelectedNodeIntensityDistributionsCallback();
  virtual void IntensityDistributionMeanChangedCallback(
    vtkIdType, int row, int col, const char *text);
  virtual void IntensityDistributionCovarianceChangedCallback(
    vtkIdType, int row, int col, const char *text);
  virtual void PopupManualIntensitySampleContextMenuCallback(
    int row, int col, int x, int y);
  virtual void DeleteManualIntensitySampleCallback(
    vtkIdType, int sample_index);
  virtual void DeleteAllManualIntensitySampleCallback(vtkIdType);

  // Description:
  // Observers
  virtual void AddManualIntensitySamplingGUIObservers();
  virtual void RemoveManualIntensitySamplingGUIObservers();
  virtual void ProcessManualIntensitySamplingGUIEvents(
    vtkObject *caller, unsigned long event, void *callData);
  
  virtual void PlotDistributionCallback(); 
  virtual void ResetDistributionCallback(); 

protected:
  vtkEMSegmentIntensityDistributionsStep();
  ~vtkEMSegmentIntensityDistributionsStep();

  vtkKWNotebook              *IntensityDistributionNotebook;
  vtkKWMenuButtonWithLabel   *IntensityDistributionSpecificationMenuButton;
  vtkKWMatrixWidgetWithLabel *IntensityDistributionMeanMatrix;
  vtkKWMatrixWidgetWithLabel *IntensityDistributionCovarianceMatrix;
  vtkKWMultiColumnListWithScrollbarsWithLabel 
                             *IntensityDistributionManualSamplingList;
  vtkKWMenu                  *ContextMenu;
  vtkKWLabel                 *NodeParametersLabel;
  vtkKWLabel                 *NodeParametersLabel2;

  virtual void AddIntensityDistributionSamplePoint(vtkIdType sel_vol_id, double ras[3]);

  vtkKWPushButton           *ShowGraphButton;
  vtkKWPushButton           *ResetIntensityButton;

  void RemovePlot();
  void RemoveResults() { this->RemovePlot(); }

private:
  vtkEMSegmentIntensityDistributionsStep(const vtkEMSegmentIntensityDistributionsStep&);
  void operator=(const vtkEMSegmentIntensityDistributionsStep&);
};

#endif
