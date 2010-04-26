#ifndef __vtkPETCTFusionLogic_h
#define __vtkPETCTFusionLogic_h

#include "vtkObject.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkPETCTFusionWin32Header.h"
#include "vtkPETCTFusionPlots.h"
#include "vtkMRMLPETCTFusionNode.h"


#include <string>
#include <vector>

class VTK_PETCTFUSION_EXPORT vtkPETCTFusionLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkPETCTFusionLogic *New();
  vtkTypeMacro(vtkPETCTFusionLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro (PETCTFusionNode, vtkMRMLPETCTFusionNode);
  vtkSetObjectMacro (PETCTFusionNode, vtkMRMLPETCTFusionNode);
  vtkGetMacro (NumberOfVOIs, int );
  vtkGetObjectMacro ( Plots, vtkPETCTFusionPlots );

  // Description:
  // These methods are used to turn observers on/off when module is entered/exited.
  virtual void Enter ( );
  virtual void Exit ( ) ;

  // Description:
  //Get/Set MRML node storing parameter values
  void SetAndObservePETCTFusionNode(vtkMRMLPETCTFusionNode *n) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->PETCTFusionNode, n);
    }

  virtual vtkIntArray* NewObservableEvents();

  //--- takes an integer label id and a double[3]
//  virtual double *GetColorForLabel ( int label );

  virtual int GetParametersFromDICOMHeader( const char *path);

  virtual void ComputeSUVmax();
  virtual void ComputeSUV();
  virtual void ComputePercentChange();

  double ConvertSUVUnitsToImageUnits(double suvValue);
  double ConvertImageUnitsToSUVUnits(double voxValue);
  double ConvertTimeToSeconds(const char *time);
  double DecayCorrection ( double inval );
  double UndoDecayCorrection ( double inval );

  virtual void SetAndScaleLUT();
  void ClearStudyDate();
  void ShowLongitudinalPlot();
  void ClearLongitudinalPlot();

  // Description:
  // Provides radioactivity (radiation activity) unit conversion:
  virtual double ConvertRadioactivityUnits( double count, const char *fromunits, const char *tounits);
  virtual double ConvertWeightUnits( double count, const char *fromunits, const char *tounits);
    

 protected:
  vtkPETCTFusionLogic();
  ~vtkPETCTFusionLogic();
  vtkPETCTFusionLogic(const vtkPETCTFusionLogic&);
  void operator=(const vtkPETCTFusionLogic&);
  vtkMRMLPETCTFusionNode* PETCTFusionNode;
  bool Visited;
  bool Raised;
  int NumberOfVOIs;
  int Year;
  int Month;
  int Day;
  vtkPETCTFusionPlots *Plots;
  
  
};

#endif

