#ifndef __qSlicerTransformsModule_h
#define __qSlicerTransformsModule_h 

#include "qVTKObject.h"
#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMatrix4x4;
class vtkMRMLNode; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerTransformsModule : public qSlicerAbstractCoreModule
{ 
  Q_OBJECT
  QVTK_OBJECT

public:

  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerTransformsModule(QWidget *parent=0);
  virtual ~qSlicerTransformsModule(); 
  
  virtual void dumpObjectInfo();
  
  qSlicerGetModuleTitleDeclarationMacro();
  
  // Return help/about text
  virtual QString helpText();
  virtual QString aboutText(); 

protected slots:
  void onCoordinateReferenceButtonPressed(int id); 
  void onIdentityButtonPressed(); 
  void onInvertButtonPressed(); 
  void onNodeSelected(vtkMRMLNode* node);
  
  // Description:
  // Triggered upon MRML transform node updates
  void onMRMLTransformNodeModified(void* call_data, vtkObject* caller);

protected:
  // Description:
  // Fill the 'minmax' array with the min/max translation value of the matrix. 
  // Parameter expand allows to specify (using a value between 0 and 1) 
  // which percentage of the found min/max value should be substracted/added 
  // to the min/max value found.
  void extractMinMaxTranslationValue(vtkMatrix4x4 * mat, double minmax[2], float expand); 
  
  // Description:
  // Convenient method to return the coordinate system currently selected
  int coordinateReference();  

private:
  class qInternal;
  qInternal* Internal;
};

#endif
