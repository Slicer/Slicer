#ifndef __qSlicerModuleSelectorWidget_p_h
#define __qSlicerModuleSelectorWidget_p_h

#include "ui_qSlicerModuleSelectorWidget.h"
#include "qSlicerModuleSelectorWidget.h"

/// qCTK includes
#include <qCTKPimpl.h>

/// QT includes
#include <QObject>

//---------------------------------------------------------------------------
class qSlicerModuleSelectorWidgetPrivate: public QObject,
                                          public qCTKPrivate<qSlicerModuleSelectorWidget>,
                                          public Ui_qSlicerModuleSelectorWidget
{
  Q_OBJECT
public:
  QCTK_DECLARE_PUBLIC(qSlicerModuleSelectorWidget);
  
  explicit qSlicerModuleSelectorWidgetPrivate(){}
  virtual ~qSlicerModuleSelectorWidgetPrivate(){}
    
  void setupUi(QWidget* widget);

  void addModules(const QStringList& moduleNames);
  void removeModule(const QString& name);

public slots:
  void onHistoryButtonClicked();
  void onPreviousButtonClicked();
  void onNextButtonClicked();

  void onComboBoxActivated(const QString& title);
};

#endif
