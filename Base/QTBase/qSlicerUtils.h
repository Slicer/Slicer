#ifndef __qSlicerUtils_h
#define __qSlicerUtils_h

#include <QString>

#include "qSlicerBaseQTBaseWin32Header.h"

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerUtils
{
  
public:
  typedef qSlicerUtils Self; 

  // Description:
  // Return true if the string name ends with one of these executable extension:
  // ".bat", ".com", ".sh", ".csh", ".tcsh", ".pl", ".py", ".tcl", ".m", ".exe"
  // Note: The comparison is case insensitive
  static bool isExecutableName(const QString& name); 
  
  
private:
  // Not implemented
  qSlicerUtils(){}
  virtual ~qSlicerUtils(){}

};

#endif
