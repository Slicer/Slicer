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

#ifndef qSlicerSingletonViewFactory_h
#define qSlicerSingletonViewFactory_h

// CTK includes
#include "ctkLayoutViewFactory.h"

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class ctkDICOMBrowser;
class qSlicerSingletonViewFactoryPrivate;

/// This class provides an interface for C++ and Python classes to register a singleton view replacement widget.
/// New view widgets can be registered by registering a new qSlicerSingletonViewFactory, and then setting the widget and tag using setWidget(QWidget*) and
/// setTagName(QString).
/// The factory will be responsible for deleting the widget.
/// This factory contains a single pointer to an instance of the widget, so only one view can be created within a given layout.
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSingletonViewFactory : public ctkLayoutViewFactory
{
  Q_OBJECT
public:
  typedef ctkLayoutViewFactory Superclass;
  qSlicerSingletonViewFactory(QObject* parent=nullptr);
  ~qSlicerSingletonViewFactory() override;

  /// Reimplemented to support custom element names
  QStringList supportedElementNames()const override;

  /// Set the singleton widget instance that will be used to create the view
  /// The factory will become responsible for deleting the widget
  Q_INVOKABLE virtual void setWidget(QWidget* widget);
  /// Get the singleton widget instance that will be used to create the view
  Q_INVOKABLE virtual QWidget* widget();

  /// Set the XML tag that identifies the view where the widget should be placed
  Q_INVOKABLE virtual void setTagName(QString tagName);
  /// Get the XML tag that identifies the view where the widget should be placed
  Q_INVOKABLE QString tagName();

public slots:
  virtual void onWidgetDestroyed();

protected:
  QScopedPointer<qSlicerSingletonViewFactoryPrivate> d_ptr;

  /// Reimplemented to instantiate desired singleton widget from the element.
  Q_INVOKABLE QWidget* createViewFromXML(QDomElement layoutElement) override;

private:
  Q_DECLARE_PRIVATE(qSlicerSingletonViewFactory)
};

#endif
