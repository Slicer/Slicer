#include "qSlicerApplication.h"

// SlicerQT includes
#include "qSlicerWidget.h"
#include "qSlicerIOManager.h"

// QT includes
#include <QRect>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFontInfo>
#include <QFontDatabase>
#include <QWidget>
#include <QMap>
#include <QDebug>

#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
struct qSlicerApplicationPrivate: public qCTKPrivate<qSlicerApplication>
{
  QCTK_DECLARE_PUBLIC(qSlicerApplication);
  qSlicerApplicationPrivate()
    {
    }

  // Description:
  // Initialize application Palette/Font
  void initPalette();
  void initFont();

  // Description:
  // Load application styleSheet
  void loadStyleSheet();

  QMap<QWidget*,bool>           TopLevelWidgetsSavedVisibilityState;
  Qt::WindowFlags               DefaultWindowFlags;
};

//-----------------------------------------------------------------------------
qSlicerApplication::qSlicerApplication(int &argc, char **argv)
  : Superclass(argc, argv)
{
  QCTK_INIT_PRIVATE(qSlicerApplication);
  QCTK_D(qSlicerApplication);
  d->initFont();
  d->initPalette();
  d->loadStyleSheet();
  
  qSlicerIOManager* ioManager = new qSlicerIOManager;
  Q_ASSERT(ioManager);
  // Note: qSlicerCoreApplication class takes ownership of the ioManager and
  // will be responsible to delete it
  this->setCoreIOManager(ioManager);
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
void qSlicerApplication::initialize()
{
  
  this->Superclass::initialize();
}

//-----------------------------------------------------------------------------
qSlicerIOManager* qSlicerApplication::ioManager()
{
  qSlicerIOManager* ioManager = reinterpret_cast<qSlicerIOManager*>(this->coreIOManager());
  Q_ASSERT(ioManager);
  return ioManager;
}

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerApplication, Qt::WindowFlags, setDefaultWindowFlags, DefaultWindowFlags);
QCTK_GET_CXX(qSlicerApplication, Qt::WindowFlags, defaultWindowFlags, DefaultWindowFlags);

//-----------------------------------------------------------------------------
void qSlicerApplication::setTopLevelWidgetsVisible(bool visible)
{
  QCTK_D(qSlicerApplication);
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
  QCTK_D(qSlicerApplication);
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
// qSlicerApplicationPrivate methods

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::initPalette()
{
  QCTK_P(qSlicerApplication);
  QPalette palette = p->palette();
  /* Old palette that makes a high contrast in Windows.
  p.setColor(QPalette::Window, Qt::white);
  p.setColor(QPalette::Base, Qt::white);
  p.setColor(QPalette::AlternateBase, QColor("#e4e4fe"));
  p.setColor(QPalette::Button, Qt::white);
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

  p->setPalette(palette);
}

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::initFont()
{
  /*
  QCTK_P(qSlicerApplication);
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
}//-----------------------------------------------------------------------------
// Static methods

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::loadStyleSheet()
{
//   QCTK_P(qSlicerApplication);
//   QString styleSheet =
//     "background-color: white;"
//     "alternate-background-color: #e4e4fe;";
//
//   p->setStyleSheet(styleSheet);
}
