
#ifndef __vtkInitClosedPath_h
#define __vtkInitClosedPath_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkSmartPointer.h"

class vtkInitClosedPath : public vtkPolyDataAlgorithm
{
public:
  vtkTypeRevisionMacro( vtkInitClosedPath, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Default <TODO>
  static vtkInitClosedPath *New();

  // Description:
  //  Macro for ... ?
  //vtkSetClampMacro( SomeParam,double,0.0,1.0);
  //vtkGetMacro( SomeParam,double);

  // Description:
  // Specify the number of iterations for Laplacian smoothing,
  //vtkSetClampMacro(NumberOfIterations,int,0,VTK_LARGE_INTEGER);
  //vtkGetMacro(NumberOfIterations,int);

  // Description:
  // Specify t..
//  vtkSetMacro(RelaxationFactor,double);
 // vtkGetMacro(RelaxationFactor,double);

  // Description:
  // Turn on/off ....
  //vtkSetMacro(FeatureEdgeSmoothing,int);
  //vtkGetMacro(FeatureEdgeSmoothing,int);
  //vtkBooleanMacro(FeatureEdgeSmoothing,int);

  // Description:
  // Specify the source object ... not needed ??
  void SetSource(vtkPolyData *source);



  void SetInitPointVertexIdx( vtkIntArray* initVertIdx )
        { this->activeContourVertIdx->DeepCopy( initVertIdx ); }
  vtkPolyData *GetSource();
  vtkIntArray* GetActiveContour( ) 
        { return this->activeContourVertIdx;} // return list of vertex indices of the 'dense curve'
  
protected:
  vtkInitClosedPath();
  ~vtkInitClosedPath() {};

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

private:
  vtkSmartPointer<vtkIntArray> activeContourVertIdx; // vertex indices of resulting initial active contour
  vtkInitClosedPath(const vtkInitClosedPath&);  // Not implemented.
  void operator=(const vtkInitClosedPath&);  // Not implemented.
};

#endif
