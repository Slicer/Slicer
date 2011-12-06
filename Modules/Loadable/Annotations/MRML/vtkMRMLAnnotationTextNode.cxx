
#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationTextNode.h"


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationTextNode);

//-----------------------------------------------------------------------------
vtkMRMLAnnotationTextNode::vtkMRMLAnnotationTextNode()
{
  this->SetCaptionCoordinates(0.05, 0.05);
}

//-----------------------------------------------------------------------------
int vtkMRMLAnnotationTextNode::SetCaptionCoordinates(double newCoord[2])
{
  return this->SetCaptionCoordinates(newCoord[0], newCoord[1]);
}

//-----------------------------------------------------------------------------
int vtkMRMLAnnotationTextNode::SetCaptionCoordinates(double x, double y)
{
  if (x < 0.0 || x > 1.0)
    {
    vtkErrorMacro("SetCaptionCoordinates: x is out of range of normalized viewport coordinates 0-1: " << x);
    return 0;
    }
  if (y < 0.0 || y > 1.0)
    {
    vtkErrorMacro("SetCaptionCoordinates: y is out of range of normalized viewport coordinates 0-1: " << y);
    return 0;
    }
  double newCoord[3];
  newCoord[0] = x;
  newCoord[1] = y;
  newCoord[2] = 0.0;
  return this->SetControlPoint(1,newCoord,1,1);
}


