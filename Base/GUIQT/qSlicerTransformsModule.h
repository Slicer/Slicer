#ifndef __qSlicerTransformsModule_h
#define __qSlicerTransformsModule_h 

#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMatrix4x4;
class vtkMRMLNode; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerTransformsModule : public qSlicerAbstractCoreModule
{ 
  Q_OBJECT

public:

  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerTransformsModule(QWidget *parent=0);
  virtual ~qSlicerTransformsModule(); 
  
  virtual void printAdditionalInfo();
  
  qSlicerGetModuleTitleMacro("Transforms");
  
  // Return help/acknowledgement text
  virtual QString helpText();
  virtual QString acknowledgementText();

public slots:
  void loadTransform();
  void loadTransform(const QString& fileName);

protected:
  virtual void initializer(); 

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
  void extractMinMaxTranslationValue(vtkMatrix4x4 * mat, double& min, double& max); 
  
  // Description:
  // Convenient method to return the coordinate system currently selected
  int coordinateReference();  

private:
  class qInternal;
  qInternal* Internal;
};

#endif
