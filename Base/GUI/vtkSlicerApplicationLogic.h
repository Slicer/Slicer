#ifndef __vtkSlicerApplicationLogic_h
#define __vtkSlicerApplicationLogic_h

#include "vtkObject.h"

// This is just a dummy
class vtkSlicerApplicationLogic : public vtkObject {

 public:
    static vtkSlicerApplicationLogic* New ( );
    vtkTypeRevisionMacro (vtkSlicerApplicationLogic, vtkObject );
    virtual void SetMyState ( double s );
    virtual double GetMyState ( );

 protected:
    double state;
    vtkSlicerApplicationLogic ( );
    ~vtkSlicerApplicationLogic ( );
 private:

};

#endif
