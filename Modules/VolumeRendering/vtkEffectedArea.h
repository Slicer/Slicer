#ifndef __vtkEffectedArea_h
#define __vtkEffectedArea_h

#include "vtkVolumeRenderingModule.h"
#include "vtkObject.h"
class vtkRenderer;


class VTK_VOLUMERENDERINGMODULE_EXPORT vtkEffectedArea: public vtkObject
{
public:
    static vtkEffectedArea *New();
    vtkTypeRevisionMacro(vtkEffectedArea,vtkObject);
    void Init(vtkRenderer *ren);
    void AddWorldPoint(double x, double y,double z);
    void GetViewArea(int viewarea[4]);
protected:
    vtkEffectedArea(void);
    ~vtkEffectedArea(void);

private:
    bool FirstPoint;
    vtkRenderer *Ren;
    double VerticesWorldCoordinates[8][3];
    vtkEffectedArea(const vtkEffectedArea&);//Not implemented
  void operator=(const vtkEffectedArea&);//Not implemented
    
};
#endif
