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
  Oslo University Hospital), based on qSlicerMarkupsAdditionalOptionsWidgetsFactory.h by
  Csaba Pinter (Perklab, Queen's University), and was supported by The Research
  Council of Norway through the ALive project (grant nr. 311393).

  ==============================================================================*/

#ifndef __qslicermarkupsfactory_h_
#define __qslicermarkupsfactory_h_

// Markups widgets includes
#include "qSlicerMarkupsAdditionalOptionsWidget.h"
#include "qSlicerMarkupsModuleWidgetsExport.h"

// Qt includes
#include <QObject>
#include <QList>
#include <QPointer>

class qSlicerMarkupsAdditionalOptionsWidget;
class qSlicerMarkupsAdditionalOptionsWidgetsFactoryCleanup;

/// \ingroup SlicerRt_QtModules_Segmentations
/// \class qSlicerMarkupsAdditionalOptionsWidgetsFactory
/// \brief Singleton class managing segment editor effect plugins
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT qSlicerMarkupsAdditionalOptionsWidgetsFactory : public QObject
{
  Q_OBJECT

public:
  /// Instance getter for the singleton class
  /// \return Instance object
  Q_INVOKABLE static qSlicerMarkupsAdditionalOptionsWidgetsFactory* instance();

public:
  /// Registers an additional options widget.
  Q_INVOKABLE bool registerAdditionalOptionsWidget(qSlicerMarkupsAdditionalOptionsWidget* widget);

  /// Unregisters an additional options widget.
  Q_INVOKABLE bool unregisterAdditionalOptionsWidget(qSlicerMarkupsAdditionalOptionsWidget* widget);

  /// Returns the list of additional options widgets registered
  Q_INVOKABLE const QList<QPointer<qSlicerMarkupsAdditionalOptionsWidget>>& additionalOptionsWidgets()
  { return this->AdditionalOptionsWidgets; }

signals:
  void additionalOptionsWidgetRegistered();
  void additionalOptionsWidgetUnregistered();

protected:
  QList<QPointer<qSlicerMarkupsAdditionalOptionsWidget>> AdditionalOptionsWidgets;

private:
  /// Allows cleanup of the singleton at application exit
  static void cleanup();

private:
  qSlicerMarkupsAdditionalOptionsWidgetsFactory(QObject* parent=nullptr);
  ~qSlicerMarkupsAdditionalOptionsWidgetsFactory() override;

  Q_DISABLE_COPY(qSlicerMarkupsAdditionalOptionsWidgetsFactory);
  friend class qSlicerMarkupsAdditionalOptionsWidgetsFactoryCleanup;
  friend class PythonQtWrapper_qSlicerMarkupsAdditionalOptionsWidgetsFactory; // Allow Python wrapping without enabling direct instantiation

private:
  /// Instance of the singleton
  static qSlicerMarkupsAdditionalOptionsWidgetsFactory* Instance;
};

#endif // __qslicermarkupsfactory_h_
