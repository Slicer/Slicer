
#ifndef __vtkLevelSetMeshEvolver_h
#define __vtkLevelSetMeshEvolver_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkSmartPointer.h"
#include "MeshOps.h"

#include <list>

class vtkLevelSetMeshEvolver : public vtkPolyDataAlgorithm
{
public:
  vtkTypeRevisionMacro( vtkLevelSetMeshEvolver, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Default <TODO>
  static vtkLevelSetMeshEvolver *New();

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
  vtkPolyData *GetSource();


  MeshData* GetMeshData() { return myMeshData; }
  void SetMeshData(MeshData* data) { myMeshData = data; }

  void SetActiveContourInit( vtkIntArray* initContourVertIdx )
        { this->activeContourInit->DeepCopy( initContourVertIdx ); }
  vtkIntArray* GetActiveContourInit( ) 
        { return this->activeContourInit;} 
  vtkIntArray* GetActiveContourFinal( ) 
        { return this->activeContourFinal;} // return list of vertex indices of the Final updated 'dense curve'
  
  void SetLists(   const std::list<int>& C,
                   const std::list<int>& Lp1, const std::list<int>& Ln1, const std::list<int>& Lp2,
                   const std::list<int>& Ln2, const std::vector<int>& map_  ) {
          L_z = C; L_n1 = Ln1; L_p1 = Lp1; L_n2 = Ln2; L_p2 = Lp2;
          map = map_;
        }

protected:
  vtkLevelSetMeshEvolver();
  ~vtkLevelSetMeshEvolver() {};

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

private:
  vtkSmartPointer<vtkIntArray> activeContourInit;
  vtkSmartPointer<vtkIntArray> activeContourFinal;
  vtkLevelSetMeshEvolver(const vtkLevelSetMeshEvolver&);  // Not implemented.
  void operator=(const vtkLevelSetMeshEvolver&);  // Not implemented.

  MeshData* myMeshData;
  std::list<int> L_z;
  std::list<int> L_n1;
  std::list<int> L_p1;
  std::list<int> L_n2;
  std::list<int> L_p2;
  std::vector<int> map;
};

#endif
