#ifndef __vtkSlicerVRMenuButtonColorMode_h
#define __vtkSlicerVRMenuButtonColorMode_h

#include "vtkVolumeRenderingModule.h"
#include "vtkKWMenuButtonWithLabel.h"
#include <vector>
#include  "vtkColorTransferFunction.h"

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerVRMenuButtonColorMode :public vtkKWMenuButtonWithLabel
{
public:
    static vtkSlicerVRMenuButtonColorMode *New();
    vtkTypeRevisionMacro(vtkSlicerVRMenuButtonColorMode,vtkKWMenuButtonWithLabel);
    //BTX
    struct ColorEntry
    {
        ColorEntry(const char* name, double red,double green,double blue) : Name(name), Red(red), Green(green),Blue(blue) {};
        const char* Name;
        double Red;
        double Green;
        double Blue;
    };
    enum
    {
        ColorModeChangedEvent=1000001
    };
    //ETX

    virtual void CreateWidget(void);

    vtkSetObjectMacro(ColorTransferFunction,vtkColorTransferFunction);
    vtkSetVector2Macro(Range,double);
    double Range[2];
    vtkColorTransferFunction *ColorTransferFunction;
    //BTX
    typedef std::vector<ColorEntry> ColorModeList;
    static const ColorModeList Values;
    static ColorModeList CreateValues();
    //ETX
    void ProcessColorModeEvents(int index);
protected:
    vtkSlicerVRMenuButtonColorMode(void);
    ~vtkSlicerVRMenuButtonColorMode(void);
private:
    vtkSlicerVRMenuButtonColorMode(const vtkSlicerVRMenuButtonColorMode&);//not implemented
    void operator=(const vtkSlicerVRMenuButtonColorMode&);//not implemented
};
#endif
