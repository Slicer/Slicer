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

#ifndef __qMRMLWidgetsPlugin_h
#define __qMRMLWidgetsPlugin_h

#include "qMRMLWidgetsConfigure.h" // For MRML_WIDGETS_HAVE_QT5, MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT

// Qt includes
#ifdef MRML_WIDGETS_HAVE_QT5
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>
#else
#include <QDesignerCustomWidgetCollectionInterface>
#endif

// MRMLWidgets includes
#include "qMRMLCheckableNodeComboBoxPlugin.h"
#include "qMRMLClipNodeWidgetPlugin.h"
#include "qMRMLCollapsibleButtonPlugin.h"
#include "qMRMLColorListViewPlugin.h"
#include "qMRMLColorTableComboBoxPlugin.h"
#include "qMRMLColorTableViewPlugin.h"
#include "qMRMLCoordinatesWidgetPlugin.h"
#include "qMRMLDisplayNodeViewComboBoxPlugin.h"
#include "qMRMLDisplayNodeWidgetPlugin.h"
#include "qMRMLEventBrokerWidgetPlugin.h"
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#include "qMRMLExpandingWebViewPlugin.h"
#endif
#include "qMRMLLabelComboBoxPlugin.h"
#include "qMRMLLayoutWidgetPlugin.h"
#include "qMRMLLinearTransformSliderPlugin.h"
#include "qMRMLListWidgetPlugin.h"
#include "qMRMLMatrixWidgetPlugin.h"
#include "qMRMLModelInfoWidgetPlugin.h"
#include "qMRMLNavigationViewPlugin.h"
#include "qMRMLNodeAttributeTableViewPlugin.h"
#include "qMRMLNodeAttributeTableWidgetPlugin.h"
#include "qMRMLNodeComboBoxPlugin.h"
#include "qMRMLPlotWidgetPlugin.h"
#include "qMRMLPlotViewControllerWidgetPlugin.h"
#include "qMRMLRangeWidgetPlugin.h"
#include "qMRMLROIWidgetPlugin.h"
#include "qMRMLScalarInvariantComboBoxPlugin.h"
#include "qMRMLSliceControllerWidgetPlugin.h"
#include "qMRMLSliceInformationWidgetPlugin.h"
#include "qMRMLSliceWidgetPlugin.h"
#include "qMRMLSliderWidgetPlugin.h"
#include "qMRMLSpinBoxPlugin.h"
#include "qMRMLThreeDViewInformationWidgetPlugin.h"
#include "qMRMLThreeDViewPlugin.h"
#include "qMRMLTransformSlidersPlugin.h"
#include "qMRMLTreeViewPlugin.h"
#include "qMRMLVolumeInfoWidgetPlugin.h"
#include "qMRMLVolumeThresholdWidgetPlugin.h"
#include "qMRMLWidgetPlugin.h"
#include "qMRMLWindowLevelWidgetPlugin.h"
#include "qMRMLSceneFactoryWidgetPlugin.h"
#include "qMRMLTableViewPlugin.h"

// \class Group the plugins in one library
class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
#ifdef MRML_WIDGETS_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
#endif
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const override
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qMRMLCheckableNodeComboBoxPlugin
            << new qMRMLClipNodeWidgetPlugin
            << new qMRMLCollapsibleButtonPlugin
            << new qMRMLColorListViewPlugin
            << new qMRMLColorTableComboBoxPlugin
            << new qMRMLColorTableViewPlugin
            << new qMRMLColorTableViewPlugin
            << new qMRMLCoordinatesWidgetPlugin
            << new qMRMLDisplayNodeViewComboBoxPlugin
            << new qMRMLDisplayNodeWidgetPlugin
            << new qMRMLEventBrokerWidgetPlugin
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
            << new qMRMLExpandingWebViewPlugin
#endif
            << new qMRMLLabelComboBoxPlugin
            << new qMRMLLayoutWidgetPlugin
            << new qMRMLLinearTransformSliderPlugin
            << new qMRMLListWidgetPlugin
            << new qMRMLMatrixWidgetPlugin
            << new qMRMLModelInfoWidgetPlugin
            << new qMRMLNavigationViewPlugin
            << new qMRMLNodeAttributeTableViewPlugin
            << new qMRMLNodeAttributeTableWidgetPlugin
            << new qMRMLNodeComboBoxPlugin
            << new qMRMLPlotWidgetPlugin
            << new qMRMLPlotViewControllerWidgetPlugin
            << new qMRMLRangeWidgetPlugin
            << new qMRMLROIWidgetPlugin
            << new qMRMLScalarInvariantComboBoxPlugin
            << new qMRMLSceneFactoryWidgetPlugin
            << new qMRMLSliceControllerWidgetPlugin
            << new qMRMLSliceInformationWidgetPlugin
            << new qMRMLSliceWidgetPlugin
            << new qMRMLSliderWidgetPlugin
            << new qMRMLSpinBoxPlugin
            << new qMRMLTableViewPlugin
            << new qMRMLThreeDViewInformationWidgetPlugin
            << new qMRMLThreeDViewPlugin
            << new qMRMLTransformSlidersPlugin
            << new qMRMLTreeViewPlugin
            << new qMRMLVolumeInfoWidgetPlugin
            << new qMRMLVolumeThresholdWidgetPlugin
            << new qMRMLWidgetPlugin
            << new qMRMLWindowLevelWidgetPlugin;
    return plugins;
    }
};

#endif
