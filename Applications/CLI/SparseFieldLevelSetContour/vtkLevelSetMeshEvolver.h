
#ifndef __vtkLevelSetMeshEvolver_h
#define __vtkLevelSetMeshEvolver_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkSmartPointer.h"
#include "MeshOps.h"

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
  
  void SetLists(   const vector<int>& C, 
                   const vector<int>& Lp1, const vector<int>& Ln1, const vector<int>& Lp2,
                   const vector<int>& Ln2, const vector<int>& map_  ) {
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
  vector<int> L_z;
  vector<int> L_n1;
  vector<int> L_p1;
  vector<int> L_n2;
  vector<int> L_p2;
  vector<int> map;
};

#endif
