#ifndef _qSlicerApplication_p_h_
#define _qSlicerApplication_p_h_

// QT includes
#include <QPointer>

// Slicer includes
#include "qSlicerCoreApplication_p.h"
#include "qSlicerLayoutManager.h"
#include "ctkSettingsDialog.h"

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
#include "qSlicerExtensionsManagerDialog.h"
#endif


// QTGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerCommandOptions;
class qSlicerIOManager;
class ctkToolTipTrapper;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplicationPrivate : public qSlicerCoreApplicationPrivate
{
    Q_DECLARE_PUBLIC(qSlicerApplication);
protected:
    qSlicerApplication* const q_ptr;
public:
    typedef qSlicerCoreApplicationPrivate Superclass;

    qSlicerApplicationPrivate(qSlicerApplication& object,
        qSlicerCommandOptions* commandOptions,
        qSlicerIOManager* ioManager);
    ~qSlicerApplicationPrivate() override;

    /// Convenient method regrouping all initialization code
    void init() override;

    /// Initialize application style
    void initStyle();

    QSettings* newSettings() override;

    QPointer<qSlicerLayoutManager> LayoutManager;
    ctkToolTipTrapper* ToolTipTrapper;
    // If MainWindow exists and the dialog is displayed then the MainWindow
    // must be set as parent to ensure correct Z order;
    // but that also transfers the ownership of the object, therefore we use QPointer
    // to keep track if the object is deleted already by the MainWindow.
    QPointer<ctkSettingsDialog> SettingsDialog;
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
    QPointer<qSlicerExtensionsManagerDialog> ExtensionsManagerDialog;
    bool IsExtensionsManagerDialogOpen;
#endif
#ifdef Slicer_USE_QtTesting
    ctkQtTestingUtility* TestingUtility;
#endif
#ifdef Slicer_USE_PYTHONQT
    ctkErrorLogLevel::LogLevel PythonConsoleLogLevel;
#endif
};

#endif