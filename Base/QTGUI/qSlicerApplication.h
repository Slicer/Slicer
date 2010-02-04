#ifndef __qSlicerApplication_h
#define __qSlicerApplication_h

/// Qt includes
#include <QPalette>

/// qCTK includes
#include <qCTKPimpl.h>

/// QTCORE includes
#include "qSlicerCoreApplication.h"

/// QTGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerWidget;
class qSlicerIOManager;
class qSlicerCommandOptions; 
class qSlicerApplicationPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplication : public qSlicerCoreApplication
{
  Q_OBJECT

public:

  typedef qSlicerCoreApplication Superclass;
  qSlicerApplication(int &argc, char **argv);
  virtual ~qSlicerApplication();

  /// 
  /// Return a reference to the application singleton
  static qSlicerApplication* application();

  /// 
  void initialize(bool& exitWhenDone);

  ///
  /// Get commandOptions
  qSlicerCommandOptions* commandOptions();

  /// 
  /// Get IO Manager
  qSlicerIOManager* ioManager(); 

  /// 
  /// Set the visibility of the top level widgets.
  /// If set to 'False', the state of the widget will be saved.
  /// If set to 'True, if possible the saved state will be restored.
  void setTopLevelWidgetsVisible(bool visible);

  /// 
  /// Set the visibility of a given top level widgets
  void setTopLevelWidgetVisible(qSlicerWidget* widget, bool visible);

  /// 
  /// Set/Get default window flags that could be used when displaying top level widgets
  void setDefaultWindowFlags(Qt::WindowFlags defaultWindowFlags);
  Qt::WindowFlags defaultWindowFlags() const;

  ///
  /// TODO
  /// See http://doc.trolltech.com/4.6/qapplication.html#commitData
  /// and http://doc.trolltech.com/4.6/qsessionmanager.html#allowsInteraction
  //virtual void commitData(QSessionManager & manager);

private:
  QCTK_DECLARE_PRIVATE(qSlicerApplication);
};

/// Apply the Slicer palette to the \c palette
/// Note also that the palette parameter is passed by reference and will be
/// updated using the native paletter and applying Slicer specific properties.
void  Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplyPalette(QPalette& palette);

#endif
