#ifndef __vtkGradientAnisotropicDiffusionFilterWidget_h
#define __vtkGradientAnisotropicDiffusionFilterWidget_h

#include "vtkKWCompositeWidget.h"
#include "vtkKWCallbacksExampleWin32Header.h"

class vtkKWScale;

class VTK_GRADIENTANIOTROPICDIFFUSIONFILTER_EXPORT vtkGradientAnisotropicDiffusionFilterWidget : public vtkKWCompositeWidget
{
public:
  static vtkGradientAnisotropicDiffusionFilterWidget* New();
  vtkTypeRevisionMacro(vtkGradientAnisotropicDiffusionFilterWidget,vtkKWCompositeWidget);

  // Description:
  // Create the widget.
  virtual void Create();

  // Description:
  // Callbacks
  virtual void ScaleChangeNotifiedByCommandCallback(double value);

protected:
  vtkGradientAnisotropicDiffusionFilterWidget();
  ~vtkGradientAnisotropicDiffusionFilterWidget();

  vtkKWScale *Scale;

  // Description:
  // Processes the events that are passed through CallbackCommand (or others).
  virtual void ProcessCallbackCommandEvents(
    vtkObject *caller, unsigned long event, void *calldata);

private:
  vtkGradientAnisotropicDiffusionFilterWidget(const vtkGradientAnisotropicDiffusionFilterWidget&);   // Not implemented.
  void operator=(const vtkGradientAnisotropicDiffusionFilterWidget&);  // Not implemented.
};

#endif
