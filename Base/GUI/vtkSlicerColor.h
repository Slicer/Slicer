#ifndef __vtkSlicerColor_h
#define __vtkSlicerColor_h

#include "vtkObject.h"

// Description:
// Color definitios used in Slicer's style.
//
class vtkSlicerColor : public vtkObject
{
 public:
    static vtkSlicerColor* New ( );
    vtkTypeRevisionMacro ( vtkSlicerColor, vtkObject );

    
    // colors
    double White [3];
    double Black [3];
    double MediumGrey [3];
    double LightGrey [3];
    double Cornsilk2 [3];
    double Cornsilk3 [3];
    double Cornsilk4 [3];
    double Seashell2 [3];
    double Seashell3 [3];
    double Seashell4 [3];   
    double MediumGreen [3];
    double DarkGreen [3];
    double MediumRed [3];
    double DarkRed [3];
    double BrightYellow [3];
    double MediumYellow [3];
    double DarkYellow [3];
    double MediumBlue [3];
    double ViewerBlue [3];

    char *GetColorAsHex(double *color);
    
protected:
    
    vtkSlicerColor ( );
    ~vtkSlicerColor ( );

    int SetColor (double *color, double r, double g, double b);

    char HexColor [6];
    
 private:
    vtkSlicerColor ( const vtkSlicerColor&); // Not implemented
    void operator = ( const vtkSlicerColor&); // Not implemented
};

#endif
