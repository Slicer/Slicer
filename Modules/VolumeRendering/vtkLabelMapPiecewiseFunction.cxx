#include "vtkLabelMapPiecewiseFunction.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"

vtkLabelMapPiecewiseFunction::vtkLabelMapPiecewiseFunction(void)
{
}

vtkLabelMapPiecewiseFunction::~vtkLabelMapPiecewiseFunction(void)
{
}

vtkLabelMapPiecewiseFunction* vtkLabelMapPiecewiseFunction::New(void)
{
 // First try to create the object from the vtkObjectFactory

    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkLabelMapPiecewiseFunction");
        if(ret)
        {
                return (vtkLabelMapPiecewiseFunction*)ret;
        }
        // If the factory was unable to create the object, then create it here.
        return new vtkLabelMapPiecewiseFunction;
};
