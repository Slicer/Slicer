#ifndef __qSlicerModuleSelectorWidget_p_h
#define __qSlicerModuleSelectorWidget_p_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "ui_qSlicerModuleSelectorWidget.h"
#include "qSlicerModuleSelectorWidget.h"

//---------------------------------------------------------------------------
class qSlicerModuleSelectorWidgetPrivate: public QObject,
                                          public ctkPrivate<qSlicerModuleSelectorWidget>,
                                          public Ui_qSlicerModuleSelectorWidget
{
  Q_OBJECT
public:
  CTK_DECLARE_PUBLIC(qSlicerModuleSelectorWidget);
  
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
