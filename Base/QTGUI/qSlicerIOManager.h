#ifndef __qSlicerIOManager_h
#define __qSlicerIOManager_h

/// SlicerQT includes
#include "qSlicerCoreIOManager.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTGUIExport.h"

/// QT declarations
class QWidget;

class qSlicerIOManagerPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerIOManager : public qSlicerCoreIOManager
{
public:
  typedef qSlicerCoreIOManager Superclass;
  qSlicerIOManager();
  virtual ~qSlicerIOManager();

  /// 
  /// Shows up a dialog to let the user pick a file (any file).
  QString getOpenFileName(QWidget* widget)const;

private:
  QCTK_DECLARE_PRIVATE(qSlicerIOManager);
};

#endif
