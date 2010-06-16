// Qt includes
#include <QCleanlooksStyle>
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QFontDatabase>
#include <QFontInfo>
#include <QMap>
#include <QPalette>
#include <QRect>
#include <QStyle>
#include <QWidget>

// CTK includes
#include <ctkSettings.h>

// SlicerQt includes
#include "qSlicerWidget.h"
#include "qSlicerIOManager.h"
#include "qSlicerCommandOptions.h"
#include "qSlicerApplication.h"
#ifdef Slicer3_USE_PYTHONQT
# include "qSlicerPythonManager.h"
#endif

#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
void qSlicerApplyPalette(QPalette& palette)
{
  /* Old palette that makes a high contrast in Windows.
  palette.setColor(QPalette::Window, Qt::white);
  palette.setColor(QPalette::Base, Qt::white);
  palette.setColor(QPalette::AlternateBase, QColor("#e4e4fe"));
  palette.setColor(QPalette::Button, Qt::white);
  */
  palette.setColor(QPalette::Button, "#fcfcfc");
  palette.setColor(QPalette::Light, "#c8c8c8");
  palette.setColor(QPalette::Midlight, "#e6e6e6");
  palette.setColor(QPalette::Dark, "#aaaaaa");
  palette.setColor(QPalette::Mid, "#c8c8c8");
  palette.setColor(QPalette::Base, Qt::white);
  palette.setColor(QPalette::Window, Qt::white);
  palette.setColor(QPalette::Shadow, "#5a5a5a");
  palette.setColor(QPalette::AlternateBase, QColor("#e4e4fe"));
}



//-----------------------------------------------------------------------------
class qSlicerApplicationPrivate: public ctkPrivate<qSlicerApplication>
{
  public:
  CTK_DECLARE_PUBLIC(qSlicerApplication);
  qSlicerApplicationPrivate();

  ///
  /// Initialize application style
  void init();

  ///
  /// Initialize application style
  void initStyle();

  ///
  /// Initialize application palette
  /// Note: the palette is reset to its default values by initStyle
  void initPalette();

  ///
  /// Initialize application font
  void initFont();

  // Description:
  // Load application styleSheet
  void loadStyleSheet();

  QMap<QWidget*,bool>           TopLevelWidgetsSavedVisibilityState;
  Qt::WindowFlags               DefaultWindowFlags;
};


//-----------------------------------------------------------------------------
// qSlicerApplicationPrivate methods

//-----------------------------------------------------------------------------
qSlicerApplicationPrivate::qSlicerApplicationPrivate()
{

}

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::init()
{
  CTK_P(qSlicerApplication);
  this->initStyle();
  this->initFont();
  this->initPalette();
  this->loadStyleSheet();

  // Note: qSlicerCoreApplication class takes ownership of the ioManager and
  // will be responsible to delete it
  p->setCoreIOManager(new qSlicerIOManager);
  
  #ifdef Slicer3_USE_PYTHONQT
  // Note: qSlicerCoreApplication class takes ownership of the pythonManager and
  // will be responsible to delete it
  p->setCorePythonManager(new qSlicerPythonManager());
  #endif
}

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::initStyle()
{
  CTK_P(qSlicerApplication);
  p->setStyle(new QCleanlooksStyle);
}

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::initPalette()
{
  CTK_P(qSlicerApplication);
  QPalette myPalette = p->palette();
  qSlicerApplyPalette(myPalette);
  p->setPalette(myPalette);
}

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::initFont()
{
  /*
  CTK_P(qSlicerApplication);
  QFont f("Verdana", 9);
  QFontInfo ff(f);
  QFontDatabase database;
  foreach (QString family, database.families())
    {
    cout << family.toStdString() << endl;
    }

  cout << "Family: " << ff.family().toStdString() << endl;
  cout << "Size: " << ff.pointSize() << endl;
  p->setFont(f);
  */
}

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::loadStyleSheet()
{
//   CTK_P(qSlicerApplication);
//   QString styleSheet =
//     "background-color: white;"
//     "alternate-background-color: #e4e4fe;";
//
//   p->setStyleSheet(styleSheet);
}


//-----------------------------------------------------------------------------
// qSlicerApplication methods

//-----------------------------------------------------------------------------
qSlicerApplication::qSlicerApplication(int &_argc, char **_argv):Superclass(_argc, _argv)
{
  CTK_INIT_PRIVATE(qSlicerApplication);
  CTK_D(qSlicerApplication);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerApplication::~qSlicerApplication()
{
}

//-----------------------------------------------------------------------------
qSlicerApplication* qSlicerApplication::application()
{
  qSlicerApplication* app = qobject_cast<qSlicerApplication*>(QApplication::instance());
  return app;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::initialize(bool& exitWhenDone)
{
  // If specific command line option are required for the different Slicer apps
  // (SlicerQT, SlicerBatch, SlicerDaemon, ...).
  // The class qSlicerCommandOptions could be subclassed into, for example,
  // qSlicerGUICommandOptions, qSlicerDaemonCommandOptions, ...
  // Each subclass should be added in their respective Applications/Slicer{Batch, Daemon}
  // directory.
  // The following line should also be moved into the 'Main.cxx' specific to each app.
  // This comment should also be deleted !
  this->setCoreCommandOptions(new qSlicerCommandOptions(this->settings()));

  // Proceed to initialization of the Core
  this->Superclass::initialize(exitWhenDone);
}

//-----------------------------------------------------------------------------
qSlicerCommandOptions* qSlicerApplication::commandOptions()
{
  qSlicerCommandOptions* _commandOptions =
    dynamic_cast<qSlicerCommandOptions*>(this->coreCommandOptions());
  Q_ASSERT(_commandOptions);
  return _commandOptions;
}

//-----------------------------------------------------------------------------
qSlicerIOManager* qSlicerApplication::ioManager()
{
  qSlicerIOManager* _ioManager = dynamic_cast<qSlicerIOManager*>(this->coreIOManager());
  Q_ASSERT(_ioManager);
  return _ioManager;
}

#ifdef Slicer3_USE_PYTHONQT
//-----------------------------------------------------------------------------
qSlicerPythonManager* qSlicerApplication::pythonManager()
{
  qSlicerPythonManager* _pythonManager = 
    qobject_cast<qSlicerPythonManager*>(this->corePythonManager());
  Q_ASSERT(_pythonManager);
  return _pythonManager;
}
#endif

//-----------------------------------------------------------------------------
CTK_SET_CXX(qSlicerApplication, Qt::WindowFlags, setDefaultWindowFlags, DefaultWindowFlags);
CTK_GET_CXX(qSlicerApplication, Qt::WindowFlags, defaultWindowFlags, DefaultWindowFlags);

//-----------------------------------------------------------------------------
void qSlicerApplication::setTopLevelWidgetsVisible(bool visible)
{
  CTK_D(qSlicerApplication);
  foreach(QWidget * widget, this->topLevelWidgets())
    {
    // Store current visibility state
    if (!visible)
      {
      if (!d->TopLevelWidgetsSavedVisibilityState.contains(widget))
        {
        d->TopLevelWidgetsSavedVisibilityState[widget] = widget->isVisible();
        }
      widget->hide();
      }
    else
      {
      QMap<QWidget*,bool>::const_iterator it = d->TopLevelWidgetsSavedVisibilityState.find(widget);

      // If widget state was saved, restore it. Otherwise skip.
      if (it != d->TopLevelWidgetsSavedVisibilityState.end())
        {
        widget->setVisible(it.value());
        }
      }
    }

  // Each time widget are set visible. Internal Map can be cleared.
  if (visible)
    {
    d->TopLevelWidgetsSavedVisibilityState.clear();
    }
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setTopLevelWidgetVisible(qSlicerWidget* widget, bool visible)
{
  if (!widget) { return; }
  Q_ASSERT(!widget->parent());
  CTK_D(qSlicerApplication);
  // When internal Map is empty, it means top widget are visible
  if (d->TopLevelWidgetsSavedVisibilityState.empty())
    {
    widget->setVisible(visible);
    }
  else
    {
    d->TopLevelWidgetsSavedVisibilityState[widget] = visible;
    }
}

//-----------------------------------------------------------------------------
QSettings* qSlicerApplication::newSettings(const QString& organization,
                                           const QString& application)
{
  return new ctkSettings(organization, application, this);
}
