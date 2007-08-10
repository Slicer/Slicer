#ifndef __vtkKWMimxViewWindow_h
#define __vtkKWMimxViewWindow_h

#include "vtkBoundingBox.h"

#include "vtkKWWindowBase.h"

class vtkCallbackCommand;
class vtkRenderer;
class vtkPVAxesActor;
class vtkKWRenderWidget;

class vtkMimxUnstructuredGridWidget;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxViewWindow : public vtkKWWindowBase
{
public:
  static vtkKWMimxViewWindow* New();
  vtkTypeRevisionMacro(vtkKWMimxViewWindow,vtkKWWindowBase);
  vtkGetObjectMacro(RenderWidget, vtkKWRenderWidget);
  vtkKWRenderWidget *RenderWidget;
  vtkRenderer *AxesRenderer;
  vtkPVAxesActor *PVAxesActor;
  vtkCallbackCommand *CallbackCommand;
  vtkMimxUnstructuredGridWidget *UnstructuredGridWidget;
protected:
  vtkKWMimxViewWindow();
  ~vtkKWMimxViewWindow();
   virtual void CreateWidget();
   //void Keypress();
private:
  vtkKWMimxViewWindow(const vtkKWMimxViewWindow&);   // Not implemented.
  void operator=(const vtkKWMimxViewWindow&);  // Not implemented.
};
void updateAxis(vtkObject* caller, unsigned long , void* arg, void* );

#endif
