#ifndef __qSlicerIOOptions_h
#define __qSlicerIOOptions_h

/// QtCore includes
#include "qSlicerIO.h"
#include "qSlicerBaseQTCoreExport.h"

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerIOOptions
{
public:
  /// Contructor initialize empty properties
  explicit qSlicerIOOptions();
  virtual ~qSlicerIOOptions();

  const qSlicerIO::IOProperties& properties()const;
protected:
  qSlicerIO::IOProperties Properties;
};

#endif
