
#ifndef __qSlicerExtensionsRestoreWidget_h
#define __qSlicerExtensionsRestoreWidget_h

// CTK includes
#include <ctkErrorLogLevel.h>

// Qt includes
#include <QWidget>
#include <QVariant>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerExtensionsRestoreWidgetPrivate;
class qSlicerExtensionsManagerModel;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsRestoreWidget
  : public QWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructor
  explicit qSlicerExtensionsRestoreWidget(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerExtensionsRestoreWidget() override;

  Q_INVOKABLE qSlicerExtensionsManagerModel* extensionsManagerModel()const;
  Q_INVOKABLE void setExtensionsManagerModel(qSlicerExtensionsManagerModel* model);

  // Actions
  QAction* selectAllAction() const;
  QAction* deselectAllAction() const;
  QAction* installSelectedAction() const;

  // Events
  void changeEvent(QEvent* e) override;
  void showEvent(QShowEvent* event) override;

protected slots :
  void onSelectAllExtensionsTriggered();
  void onDeselectAllExtensionsTriggered();
  void onInstallSelectedExtensionsTriggered();
  void onCheckOnStartupChanged(int state);
  void onSilentInstallOnStartupChanged(int state);
  void onProgressChanged(const QString& extensionName, qint64 received, qint64 total);
  void onInstallationFinished(QString extensionName);
  void onExtensionHistoryGatheredOnStartup(const QVariantMap&);

protected:
  QScopedPointer<qSlicerExtensionsRestoreWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsRestoreWidget);
  Q_DISABLE_COPY(qSlicerExtensionsRestoreWidget);
};

#endif
