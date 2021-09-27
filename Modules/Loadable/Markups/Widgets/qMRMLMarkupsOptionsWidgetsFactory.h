/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital), based on qMRMLMarkupsOptionsWidgetsFactory.h by
  Csaba Pinter (Perklab, Queen's University), and was supported by The Research
  Council of Norway through the ALive project (grant nr. 311393).

  ==============================================================================*/

#ifndef __qslicermarkupsfactory_h_
#define __qslicermarkupsfactory_h_

// Markups widgets includes
#include "qMRMLMarkupsAbstractOptionsWidget.h"
#include "qSlicerMarkupsModuleWidgetsExport.h"

// Qt includes
#include <QObject>
#include <QMap>
#include <QPointer>
#include <QWidget>

// -----------------------------------------------------------------------------------------
class qMRMLMarkupsAbstractOptionsWidget;
class qMRMLMarkupsOptionsWidgetsFactoryCleanup;

/// \ingroup SlicerRt_QtModules_Segmentations
/// \class qMRMLMarkupsOptionsWidgetsFactory
/// \brief Singleton class managing segment editor effect plugins
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT qMRMLMarkupsOptionsWidgetsFactory : public QObject
{
  Q_OBJECT

public:
  /// Instance getter for the singleton class
  /// \return Instance object
  Q_INVOKABLE static qMRMLMarkupsOptionsWidgetsFactory* instance();

public:
  /// Registers an additional options widget.
  /// This factory object takes ownership of the widget (even if it fails to register it
  /// due to for example a widget already existing for that markup type), therefore the
  /// caller must NOT delete the widget.
  Q_INVOKABLE bool registerOptionsWidget(qMRMLMarkupsAbstractOptionsWidget* widget);

  /// Unregisters an additional options widget.
  Q_INVOKABLE bool unregisterOptionsWidget(const QString& className);

  /// Unregisters an additional options widget.
  Q_INVOKABLE bool unregisterOptionsWidget(qMRMLMarkupsAbstractOptionsWidget* widget);

  /// Unregister all widgets
  Q_INVOKABLE void unregisterAll();

  /// Returns the list of additional options widgets registered
  Q_INVOKABLE const QList<QString> registeredOptionsWidgetsClassNames() const
  { return this->RegisteredWidgets.keys(); }

  /// Creates a markups option widget of a given class name. The markups option
  /// widget must have been registered previously (nullptr will return if not).
  Q_INVOKABLE qMRMLMarkupsAbstractOptionsWidget* createWidget(const QString& widgetClassName);

signals:
  void optionsWidgetRegistered();
  void optionsWidgetUnregistered();

protected:
  QMap<QString, QPointer<qMRMLMarkupsAbstractOptionsWidget>> RegisteredWidgets;

private:
  /// Allows cleanup of the singleton at application exit
  static void cleanup();

private:
  qMRMLMarkupsOptionsWidgetsFactory(QObject* parent=nullptr);
  ~qMRMLMarkupsOptionsWidgetsFactory() override;

  Q_DISABLE_COPY(qMRMLMarkupsOptionsWidgetsFactory);
  friend class qMRMLMarkupsOptionsWidgetsFactoryCleanup;
  friend class PythonQtWrapper_qMRMLMarkupsOptionsWidgetsFactory; // Allow Python wrapping without enabling direct instantiation

private:
  /// Instance of the singleton
  static qMRMLMarkupsOptionsWidgetsFactory* Instance;
};

#endif // __qslicermarkupsfactory_h_
