#ifndef __vtkPETCTFusionLogic_h
#define __vtkPETCTFusionLogic_h

#include "vtkObject.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkPETCTFusionWin32Header.h"
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

  
  virtual void GetParametersFromDICOMHeader( const char *path);

  virtual void ComputeSUVmax();
  virtual void ComputeSUV();
  virtual void ComputePercentChange();

  double ConvertSUVUnitsToImageUnits(double suvValue);
  double ConvertImageUnitsToSUVUnits(double voxValue);

  virtual void SetAndScaleLUT();

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
};

#endif

