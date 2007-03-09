
#ifndef IGTPAT2IMGREGISTRATION_H
#define IGTPAT2IMGREGISTRATION_H


#include "vtkIGTWin32Header.h" 
#include "vtkObject.h"

#include "vtkMatrix4x4.h"
#include "vtkPoints.h"
#include "vtkTransform.h"



class VTK_IGT_EXPORT vtkIGTPat2ImgRegistration : public vtkObject
{
public:

    // Constructors/Destructors
    //  Magic lines for vtk and Slicer
    static vtkIGTPat2ImgRegistration *New();
    vtkTypeRevisionMacro(vtkIGTPat2ImgRegistration,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);


    vtkGetObjectMacro(LandmarkTransformMatrix,vtkMatrix4x4);

    vtkGetMacro(NumberOfPoints,int);


    vtkIGTPat2ImgRegistration();

    virtual ~vtkIGTPat2ImgRegistration( );


    // t1, t2, t3: target landmarks 
    // s1, s2, s3: source landmarks 
    void AddPoint(int id, float t1, float t2, float t3, float s1, float s2, float s3);
    int DoRegistration();

    void SetNumberOfPoints(int no);

private:

    vtkPoints *SourceLandmarks;
    vtkPoints *TargetLandmarks;
    vtkMatrix4x4 *LandmarkTransformMatrix;

    int NumberOfPoints;

};


#endif // IGTPAT2IMGREGISTRATION_H
