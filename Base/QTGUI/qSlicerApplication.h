#ifndef __qSlicerApplication_h
#define __qSlicerApplication_h

// Qt includes
#include <QPalette>

// qCTK includes
#include <qCTKPimpl.h>

// QTCORE includes
#include "qSlicerCoreApplication.h"

// QTGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerWidget;
class qSlicerIOManager; 
class qSlicerApplicationPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplication : public qSlicerCoreApplication
{
  Q_OBJECT

public:

  typedef qSlicerCoreApplication Superclass;
  qSlicerApplication(int &argc, char **argv);
  virtual ~qSlicerApplication();

  // Description:
  // Return a reference to the application singleton
  static qSlicerApplication* application();

  // Description:
  void initialize();

  // Description:
  // Get IO Manager
  qSlicerIOManager* ioManager(); 

  // Description:
  // Set the visibility of the top level widgets.
  // If set to 'False', the state of the widget will be saved.
  // If set to 'True, if possible the saved state will be restored.
  void setTopLevelWidgetsVisible(bool visible);

  // Description:
  // Set the visibility of a given top level widgets
  void setTopLevelWidgetVisible(qSlicerWidget* widget, bool visible);

  // Description:
  // Set/Get default window flags that could be used when displaying top level widgets
  void setDefaultWindowFlags(Qt::WindowFlags defaultWindowFlags);
  Qt::WindowFlags defaultWindowFlags() const;

private:
  QCTK_DECLARE_PRIVATE(qSlicerApplication);
};

/** Apply the Slicer palette to the \c palette
 */
void  Q_SLICER_BASE_QTGUI_EXPORT qSlicerPalette(QPalette& palette);

#endif
