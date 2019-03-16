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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerStylePlugin_h
#define __qSlicerStylePlugin_h

// Qt includes
#include <QStylePlugin>
#include <QtPlugin>

class QStyle;

// Slicer includes
#include "qSlicerBaseQTGUIStylePluginsExport.h"

class Q_SLICER_STYLES_PLUGINS_EXPORT qSlicerStylePlugin : public QStylePlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID QStyleFactoryInterface_iid FILE "SlicerStyle.json")
public:
  /// Superclass typedef
  typedef QStylePlugin Superclass;

  /// Constructors
  qSlicerStylePlugin();
  ~qSlicerStylePlugin() override;

  // QStyle plugin classes to overloaded when creating custom style plugins
  QStyle* create(const QString & key) override;
  virtual QStringList keys() const;
};

#endif
