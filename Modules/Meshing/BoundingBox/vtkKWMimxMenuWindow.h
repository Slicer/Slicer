#ifndef __vtkKWMimxMenuWindow_h
#define __vtkKWMimxMenuWindow_h

#include "vtkBoundingBox.h"

#include "vtkKWWindowBase.h"
#include "vtkKWMimxViewWindow.h"

class vtkKWFrameWithScrollbar;
class vtkKWMenu;
class vtkKWMimxMainNotebook;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxMenuWindow : public vtkKWWindowBase
{
public:
        static vtkKWMimxMenuWindow* New();
        vtkTypeRevisionMacro(vtkKWMimxMenuWindow,vtkKWWindowBase);
        vtkSetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
        vtkGetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
protected:
        vtkKWMimxMenuWindow();
        ~vtkKWMimxMenuWindow();
        virtual void CreateWidget();
        vtkKWMenu *ViewMenu;
        vtkKWFrameWithScrollbar *MainNoteBookFrameScrollbar;
        vtkKWMimxMainNotebook *MainNotebook;
        vtkKWMimxViewWindow *MimxViewWindow;
private:
        vtkKWMimxMenuWindow(const vtkKWMimxMenuWindow&);   // Not implemented.
        void operator=(const vtkKWMimxMenuWindow&);  // Not implemented.
};

#endif
