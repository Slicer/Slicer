#ifndef __vtkTumorGrowthStep_h
#define __vtkTumorGrowthStep_h

#include "vtkTumorGrowth.h"
#include "vtkKWWizardStep.h"

class vtkKWFrameWithLabel; 
class vtkTumorGrowthGUI;
class vtkKWPushButton;
class vtkVolumeTextureMapper3D;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class vtkVolumeProperty;
class vtkVolume;
class vtkMatrix4x4;      
class vtkMRMLVolumeNode;
class vtkImageData;  
class vtkFixedPointVolumeRayCastMapper;

#define TUMORGROWTH_MENU_BUTTON_WIDTH_SMALL 10
#define TUMORGROWTH_MENU_BUTTON_WIDTH 15
#define TUMORGROWTH_WIDGETS_LABEL_WIDTH 25
#define TUMORGROWTH_WIDGETS_SLIDER_WIDTH 100
#define TUMORGROWTH_WIDGETS_SLIDER_HEIGHT 25

class VTK_TUMORGROWTH_EXPORT vtkTumorGrowthStep : public vtkKWWizardStep
{
public:
  static vtkTumorGrowthStep *New();
  vtkTypeRevisionMacro(vtkTumorGrowthStep,vtkKWWizardStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // Get/Set GUI - 
  vtkGetObjectMacro(GUI, vtkTumorGrowthGUI);
  virtual void SetGUI(vtkTumorGrowthGUI*); 

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void HideUserInterface();
  virtual void Validate();
  virtual int CanGoToSelf();

  virtual void ShowUserInterface();

  void AddGUIObservers() { } 

  virtual void TransitionCallback() { };

  // Note : This should be called directly from workflow->BackButton but it is not currently possible 
  // -> That is why we use step  
  virtual void RemoveResults() {};

  virtual void UpdateMRML() { }

  virtual void UpdateGUI() { } 
  virtual void RemoveGUIObservers() { } 

  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData) { }

  void SetNextStep(vtkTumorGrowthStep *init) { this->NextStep = init;}

  virtual void GridCallback();
  virtual void SliceCallback();


  vtkGetObjectMacro(Render_VolumeProperty, vtkVolumeProperty);
  vtkGetObjectMacro(Render_Mapper, vtkVolumeTextureMapper3D);

protected:
  vtkTumorGrowthStep();
  ~vtkTumorGrowthStep();

  vtkTumorGrowthGUI *GUI;
  vtkKWFrameWithLabel               *Frame;
  vtkCallbackCommand *WizardGUICallbackCommand;
  // Needed so we can clean up mess when going backwards 
  vtkTumorGrowthStep *NextStep;

  void GridRemove();
  int  GridDefine();
  void CreateGridButton();
  vtkKWPushButton          *GridButton;

  void CreateResetButton(); 
  vtkKWPushButton          *ResetButton;

  void CreateSliceButton(); 
  vtkKWPushButton          *SliceButton;
  
  vtkImageData             *Render_Image;
  vtkVolumeTextureMapper3D *Render_Mapper;
  vtkPiecewiseFunction     *Render_Filter;
  vtkColorTransferFunction *Render_ColorMapping;
  vtkVolumeProperty        *Render_VolumeProperty;
  vtkVolume                *Render_Volume;
  vtkMatrix4x4             *Render_OrientationMatrix; 
  vtkFixedPointVolumeRayCastMapper *Render_RayCast_Mapper;

  void SetRender_BandPassFilter(double min, double max, float colorMin[3], float colorMax[3]);
  void ChangeRender_BandPassFilter(double min, double max); 
  void SetRender_HighPassFilter(double min, float colorMin[3], float colorMax[3]);
  void SetRender_BandStopFilter(double min, double max, float colorMin[3], float colorMax[3]);
  void RenderRemove();
  void CreateRender(vtkMRMLVolumeNode *volumeNode, int RayCastFlag);

private:
  vtkTumorGrowthStep(const vtkTumorGrowthStep&);
  void operator=(const vtkTumorGrowthStep&);
};

#endif
