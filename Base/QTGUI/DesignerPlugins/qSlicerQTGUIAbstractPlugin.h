/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerQTGUIAbstractPlugin_h
#define __qSlicerQTGUIAbstractPlugin_h

// QT includes
#include <QDesignerCustomWidgetInterface>
#include <QObject>

// QTGUI includes
#include "qSlicerBaseQTGUIDesignerPluginsExport.h"

class Q_SLICER_DESIGNER_PLUGINS_EXPORT qSlicerQTGUIAbstractPlugin
  : public QObject
  , public QDesignerCustomWidgetInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface);
public:

  qSlicerQTGUIAbstractPlugin(QObject* parent = 0);
  // Don't reimplement this method.
  QString group() const;
  // You can reimplement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;
};

#endif
