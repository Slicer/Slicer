#include "vtkKWTextWithScrollbars.h"

#include <string>

#include "vtkSlicerBaseGUI.h"


class VTK_SLICER_BASE_GUI_EXPORT vtkKWTextWithHyperlinksWithScrollbars : public vtkKWTextWithScrollbars
{
public:

        static vtkKWTextWithHyperlinksWithScrollbars *New();
    vtkTypeRevisionMacro(vtkKWTextWithHyperlinksWithScrollbars, vtkKWTextWithScrollbars);
    virtual void CreateWidget(void);
    virtual void SetText(const char *s);
    virtual void ClickLink(int x, int y);
        void OpenLink(const char *url);

protected:
    vtkKWTextWithHyperlinksWithScrollbars(void);
    ~vtkKWTextWithHyperlinksWithScrollbars(void);



private:
     // Description:
    // Caution: Not implemented
    vtkKWTextWithHyperlinksWithScrollbars(const vtkKWTextWithHyperlinksWithScrollbars&); // Not implemented
    void operator=(const vtkKWTextWithHyperlinksWithScrollbars&); // Not implemented
};
