
#ifndef IGTDATASTREAM_H
#define IGTDATASTREAM_H

#include <string>
#include <vector>

#include "vtkIGTWin32Header.h" 
#include "vtkObject.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"


#define IGT_MATRIX_STREAM 0
#define IGT_IMAGE_STREAM 1

class VTK_IGT_EXPORT vtkIGTDataStream : public vtkObject
{
public:

    // Constructors/Destructors
    //  Magic lines for vtk and Slicer
    static vtkIGTDataStream *New();
    vtkTypeRevisionMacro(vtkIGTDataStream,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);


    vtkIGTDataStream ();
    ~vtkIGTDataStream ();

    vtkSetMacro(Speed,int);
    vtkSetMacro(MultiFactor,float);
    vtkSetMacro(Tracking,int);

    vtkSetObjectMacro(RegMatrix,vtkMatrix4x4);
    vtkGetObjectMacro(RegMatrix,vtkMatrix4x4);

    vtkGetObjectMacro(LocatorMatrix,vtkMatrix4x4);
    vtkGetObjectMacro(LocatorNormalTransform,vtkTransform);


    virtual void StopPulling() {};
    virtual void PullRealTime() {};
    virtual void SetLocatorTransforms();
    virtual void ProcessTimerEvents();

protected:

    int Speed;
    int Tracking;
    float MultiFactor;

    vtkMatrix4x4 *LocatorMatrix;
    vtkMatrix4x4 *RegMatrix;
    vtkTransform *LocatorNormalTransform;

    void QuaternionToXYZ(float *orientation, float *normal, float *transnormal); 
    void ApplyTransform(float *position, float *norm, float *transnorm);
 };

#endif // IGTDATASTREAM_H
