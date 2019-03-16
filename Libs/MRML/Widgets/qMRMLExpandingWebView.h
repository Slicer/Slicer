/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qMRMLExpandingWebView_h
#define __qMRMLExpandingWebView_h

// Qt includes
#include <QWebEngineView>

#include "qMRMLWidgetsExport.h"

class qMRMLExpandingWebViewPrivate;

// MRML includes
class vtkMRMLScene;

/// \brief qMRMLExpandingWebView is the display canvas for some or all of a MRML scene.
///
/// qMRMLExpandingWebView is currently implemented as a subclass of QWebView
class QMRML_WIDGETS_EXPORT qMRMLExpandingWebView :
    public QWebEngineView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWebEngineView Superclass;

  /// Constructors
  explicit qMRMLExpandingWebView(QWidget* parent = nullptr);
  ~qMRMLExpandingWebView() override;

  /// Return a pointer on the current MRML scene
  vtkMRMLScene* mrmlScene() const;

  // Redefine the sizeHint so layouts work properly.
  QSize sizeHint() const override;

public slots:

  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* newScene);

  /// subclasses reimplement this to handle updates in the mrml scene
  virtual void updateWidgetFromMRML();

signals:

  /// When designing custom qMRMLWidget in the designer, you can connect the
  /// mrmlSceneChanged signal directly to the aggregated MRML widgets that
  /// have a setMRMLScene slot.
  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  QScopedPointer<qMRMLExpandingWebViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLExpandingWebView);
  Q_DISABLE_COPY(qMRMLExpandingWebView);
};

#endif
