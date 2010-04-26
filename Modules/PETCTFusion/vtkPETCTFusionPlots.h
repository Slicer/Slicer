#ifndef __vtkPETCTFusionPlots_h
#define __vtkPETCTFusionPlots_h

#include "vtkObject.h"
#include "vtkPETCTFusionWin32Header.h"
#include "vtkMRMLDoubleArrayNode.h"

class VTK_PETCTFUSION_EXPORT vtkPETCTFusionPlots : public vtkObject
{

 public:
  static vtkPETCTFusionPlots *New();
  vtkTypeRevisionMacro(vtkPETCTFusionPlots, vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetObjectMacro ( MRMLScene,  vtkMRMLScene );
  vtkGetObjectMacro ( MRMLScene,  vtkMRMLScene );

  vtkIntArray* GetLabelList();
  int GetNumberOfLabels();
  int GetNumberOfTimepoints ( int label );
  vtkMRMLDoubleArrayNode* GetPlotData(int label);
  int OutputDataInCSV(ostream& os, int label);
  int OutputAllDataInCSV(ostream& os );

  void AddSUV( int label, int year, int month, int day, double suvmax, double suvmean );

  // Description:
  // Called each time SUV for a new timepoint is computed.
  int Update();
  void GeneratePlot();
  void GenerateXLabels(int label);
  void GenerateYLabels(int label);
  void ResetPlots();
  void ClearPlots();
  
 protected:
  vtkPETCTFusionPlots();
  virtual ~vtkPETCTFusionPlots();
  //BTX
  typedef std::map<int, vtkMRMLDoubleArrayNode*> VOItoPlotDataMapType;
  //ETX

 private:
  VOItoPlotDataMapType SUVPlotData;  
  vtkMRMLScene* MRMLScene;

};


#endif //__vtkPETCTFusionPlots_h
