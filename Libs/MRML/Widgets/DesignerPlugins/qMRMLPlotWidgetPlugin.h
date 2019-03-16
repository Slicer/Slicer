/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

#ifndef __qMRMLPlotWidgetPlugin_h
#define __qMRMLPlotWidgetPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLPlotWidgetPlugin :
  public QObject,
  public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLPlotWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
//   QIcon    icon() const;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
