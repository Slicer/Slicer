/*==============================================================================

Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
Queen's University, Kingston, ON, Canada. All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
and was supported through CANARIE's Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

// QtGUI includes
#include "qSlicerSingletonViewFactory.h"

#include <QDebug>
#include <QMap>
#include <QSharedPointer>
#include <QWidget>

//-----------------------------------------------------------------------------
class qSlicerSingletonViewFactoryPrivate
{
  Q_DECLARE_PUBLIC(qSlicerSingletonViewFactory);
protected:
  qSlicerSingletonViewFactory* q_ptr;
public:
  qSlicerSingletonViewFactoryPrivate(qSlicerSingletonViewFactory& object);
  virtual ~qSlicerSingletonViewFactoryPrivate();

  virtual void init();

  // Widget returned from createViewFromXML
  QWidget* Widget;
  // Tag used for widget in createViewFromXML
  QString TagName;
  // Internal QWidget used to hold the widget until it is taken over by the layout manager
  QSharedPointer<QWidget> InternalWidget;
};

//-----------------------------------------------------------------------------
// qSlicerSingletonViewFactoryPrivate methods

qSlicerSingletonViewFactoryPrivate
::qSlicerSingletonViewFactoryPrivate(qSlicerSingletonViewFactory& object)
  : q_ptr(&object)
  , Widget(nullptr)
  , InternalWidget(new QWidget())
{
}

//-----------------------------------------------------------------------------
qSlicerSingletonViewFactoryPrivate::~qSlicerSingletonViewFactoryPrivate()
{
  Q_Q(qSlicerSingletonViewFactory);
  q->setWidget(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerSingletonViewFactoryPrivate::init()
{
}

//-----------------------------------------------------------------------------
// qSlicerSingletonViewFactory methods

//-----------------------------------------------------------------------------
qSlicerSingletonViewFactory::qSlicerSingletonViewFactory(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSingletonViewFactoryPrivate(*this))
{
  Q_D(qSlicerSingletonViewFactory);
  d->init();
  this->setUseCachedViews(false);
}

//-----------------------------------------------------------------------------
qSlicerSingletonViewFactory::~qSlicerSingletonViewFactory() = default;

//-----------------------------------------------------------------------------
QWidget* qSlicerSingletonViewFactory::widget()
{
  Q_D(qSlicerSingletonViewFactory);
  return d->Widget;
}

//-----------------------------------------------------------------------------
void qSlicerSingletonViewFactory::setWidget(QWidget* widget)
{
  Q_D(qSlicerSingletonViewFactory);
  if (d->Widget == widget)
    {
    return;
    }

  if (d->Widget)
    {
    QObject::disconnect(d->Widget, &QWidget::destroyed, this, &qSlicerSingletonViewFactory::onWidgetDestroyed);
    }

  d->Widget = widget;
  if (d->Widget)
    {
    d->Widget->setParent(d->InternalWidget.data()); // Hold the widget in the internal widget until the layout manager can take it
    QObject::connect(d->Widget, &QWidget::destroyed, this, &qSlicerSingletonViewFactory::onWidgetDestroyed);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSingletonViewFactory::onWidgetDestroyed()
{
  this->setWidget(nullptr);
}

//-----------------------------------------------------------------------------
QString qSlicerSingletonViewFactory::tagName()
{
  Q_D(qSlicerSingletonViewFactory);
  return d->TagName;
}

//-----------------------------------------------------------------------------
void qSlicerSingletonViewFactory::setTagName(QString tagName)
{
  Q_D(qSlicerSingletonViewFactory);
  d->TagName = tagName;
}

//-----------------------------------------------------------------------------
QStringList qSlicerSingletonViewFactory::supportedElementNames() const
{
  Q_D(const qSlicerSingletonViewFactory);
  return QStringList() << d->TagName;
}

//---------------------------------------------------------------------------
QWidget* qSlicerSingletonViewFactory::createViewFromXML(QDomElement layoutElement)
{
  Q_UNUSED(layoutElement);
  Q_D(qSlicerSingletonViewFactory);
  if (d->Widget && d->Widget->isVisible())
    {
    qCritical() << "qSlicerSingletonViewFactory::createViewFromXML - Widget for view \"" << d->TagName << "\" is already in use within the current layout!";
    }

  return this->widget();
}
