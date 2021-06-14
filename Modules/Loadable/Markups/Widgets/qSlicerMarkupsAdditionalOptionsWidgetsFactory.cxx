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
  Oslo University Hospital), based on qSlicerSegmentEditorEffectFactory.cxx by
  Csaba Pinter (Perklab, Queen's Univeristy), and was supported by The Research
  Council of Norway through the ALive project (grant nr. 311393).

  ==============================================================================*/

#include "qSlicerMarkupsAdditionalOptionsWidgetsFactory.h"

// Qt includes
#include <QDebug>

//----------------------------------------------------------------------------
qSlicerMarkupsAdditionalOptionsWidgetsFactory *qSlicerMarkupsAdditionalOptionsWidgetsFactory::Instance = nullptr;

//----------------------------------------------------------------------------
/// \ingroup SlicerRt_QtModules_Segmentations
class qSlicerMarkupsAdditionalOptionsWidgetsFactoryCleanup
{
public:
  inline void use()   {   }

  ~qSlicerMarkupsAdditionalOptionsWidgetsFactoryCleanup()
  {
    if (qSlicerMarkupsAdditionalOptionsWidgetsFactory::Instance)
      {
      qSlicerMarkupsAdditionalOptionsWidgetsFactory::cleanup();
      }
  }
};

//-----------------------------------------------------------------------------
static qSlicerMarkupsAdditionalOptionsWidgetsFactoryCleanup qSlicerMarkupsAdditionalOptionsWidgetsFactoryCleanupGlobal;

//-----------------------------------------------------------------------------
qSlicerMarkupsAdditionalOptionsWidgetsFactory* qSlicerMarkupsAdditionalOptionsWidgetsFactory::instance()
{
  if(!qSlicerMarkupsAdditionalOptionsWidgetsFactory::Instance)
    {
    qSlicerMarkupsAdditionalOptionsWidgetsFactoryCleanupGlobal.use();
    qSlicerMarkupsAdditionalOptionsWidgetsFactory::Instance = new qSlicerMarkupsAdditionalOptionsWidgetsFactory();
    }
  // Return the instance
  return qSlicerMarkupsAdditionalOptionsWidgetsFactory::Instance;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsAdditionalOptionsWidgetsFactory::cleanup()
{
  if (qSlicerMarkupsAdditionalOptionsWidgetsFactory::Instance)
    {
    delete qSlicerMarkupsAdditionalOptionsWidgetsFactory::Instance;
    qSlicerMarkupsAdditionalOptionsWidgetsFactory::Instance = nullptr;
    }
}

//-----------------------------------------------------------------------------
qSlicerMarkupsAdditionalOptionsWidgetsFactory::qSlicerMarkupsAdditionalOptionsWidgetsFactory(QObject* parent)
  : QObject(parent)
{
  this->AdditionalOptionsWidgets.clear();
}

//-----------------------------------------------------------------------------
qSlicerMarkupsAdditionalOptionsWidgetsFactory::~qSlicerMarkupsAdditionalOptionsWidgetsFactory()
{
  // NOTE: The factory cannot claim a strong ownership over the additional
  // widgets. These widgets will normally be added as part of a GUI and will
  // normally deleted by the Qt parent-child mechanisms. Here we check whether
  // the widget was deleted already, if not we reparent to null (important to
  // break the Qt parent-child mechanisms) and then delete them.
  foreach(auto widget, this->AdditionalOptionsWidgets)
    {
    if (widget)
      {
      delete widget;
      }
    }

  this->AdditionalOptionsWidgets.clear();
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsAdditionalOptionsWidgetsFactory::registerAdditionalOptionsWidget(qSlicerMarkupsAdditionalOptionsWidget* widget)
{

  // Check for nullptr
  if (!widget)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid additional options widget";
    return false;
    }

  // Check for emtpy type name
  if (widget->getAdditionalOptionsWidgetTypeName().isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ": Additional options widget has no tpye name.";
    return false;
    }

  // Check for already registered widget
  foreach(auto &registeredWidget, this->AdditionalOptionsWidgets)
    {
    if (registeredWidget->getAdditionalOptionsWidgetTypeName() ==
        widget->getAdditionalOptionsWidgetTypeName())
      {
      qWarning() << Q_FUNC_INFO << ": Additional options widget type "
                 << QString(widget->getAdditionalOptionsWidgetTypeName())
                 << " already registered.";
      return false;
      }
    }

  // Register the additional options widget
  this->AdditionalOptionsWidgets.append(widget);

  // Emit registered signal
  emit additionalOptionsWidgetRegistered();

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsAdditionalOptionsWidgetsFactory::unregisterAdditionalOptionsWidget(qSlicerMarkupsAdditionalOptionsWidget* widget)
{
  // Take ownership
  QScopedPointer<qSlicerMarkupsAdditionalOptionsWidget> widgetPtr(widget);

  // Check for nullptr
  if (!widgetPtr.data())
    {
    qCritical() << Q_FUNC_INFO << ": Invalid additional options widget";
    return false;
    }

  // Check for emtpy type name
  if (widgetPtr.data()->getAdditionalOptionsWidgetTypeName().isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ": Additional options widget has no tpye name.";
    return false;
    }

  // Check for already registered widget
  foreach(auto &registeredWidget, this->AdditionalOptionsWidgets)
    {
    if (registeredWidget->getAdditionalOptionsWidgetTypeName() ==
        widgetPtr.data()->getAdditionalOptionsWidgetTypeName())
      {
      this->AdditionalOptionsWidgets.removeAll(registeredWidget);

      registeredWidget->setParent(nullptr);
      delete registeredWidget;

      // Emit registered signal
      emit additionalOptionsWidgetUnregistered();

      return true;
      }
    }

  return true;
}
