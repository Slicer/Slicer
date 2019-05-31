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

  This file was originally developed by Andras Lasso, PerkLab, Queen's University.

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLRulerDisplayableManager.h"

// MRML includes
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkAxisActor2D.h>
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <sstream>

// Constants
static const int RENDERER_LAYER = 1; // layer ID where the orientation marker will be displayed
static const int RULER_BASE_FONT_SIZE = 14; // thin: font size = base; thick: font size is 2x
static const double RULER_MINIMUM_LENGTH = 0.05; // minimum ruler length (as ratio of window width)
static const double RULER_MAXIMUM_LENGTH = 0.7; // maximum ruler length (as ratio of window width)
static const double RULER_LINE_MARGIN = 0.015; // vertical distance of line from edge of view (as ratio of window height)
static const double RULER_TICK_BASE_LENGTH = 0.015; // thin: major tick size = base, minor tick size = base/2; thick: length is 2x (as ratio of window height)
static const double RULER_TEXT_MARGIN = 0.015; // horizontal distaace of ruler text from ruler line (as ratio of window height)

//---------------------------------------------------------------------------
class vtkRulerRendererUpdateObserver : public vtkCommand
{
public:
  static vtkRulerRendererUpdateObserver *New()
    {
    return new vtkRulerRendererUpdateObserver;
    }
  vtkRulerRendererUpdateObserver()
    {
    this->DisplayableManager = nullptr;
    }
  void Execute(vtkObject* vtkNotUsed(wdg), unsigned long vtkNotUsed(event), void* vtkNotUsed(calldata)) override
    {
    if (this->DisplayableManager)
      {
      this->DisplayableManager->UpdateFromRenderer();
      }
  }
  vtkWeakPointer<vtkMRMLRulerDisplayableManager> DisplayableManager;
};

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLRulerDisplayableManager );

//---------------------------------------------------------------------------
class vtkMRMLRulerDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLRulerDisplayableManager * external);
  ~vtkInternal();

  void SetupMarkerRenderer();
  void AddRendererUpdateObserver(vtkRenderer* renderer);
  void RemoveRendererUpdateObserver();

  void SetupRuler();

  void UpdateRuler();
  void ShowActors(bool show);

  vtkSmartPointer<vtkRenderer> MarkerRenderer;
  vtkSmartPointer<vtkAxisActor2D> RulerLineActor;
  // Ruler line actor includes text labels, but the total length cannot be consistently positioned
  // to the right side of the line (the Title is almost usable, but the distance from the line is varying
  // and vertical alignment is not perfect)
  vtkSmartPointer<vtkTextActor> RulerTextActor;

  vtkSmartPointer<vtkRulerRendererUpdateObserver> RendererUpdateObserver;
  int RendererUpdateObservationId;
  vtkWeakPointer<vtkRenderer> ObservedRenderer;

  bool ActorsAddedToRenderer;

  vtkMRMLRulerDisplayableManager* External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLRulerDisplayableManager::vtkInternal::vtkInternal(vtkMRMLRulerDisplayableManager * external)
{
  this->External = external;
  this->RendererUpdateObserver = vtkSmartPointer<vtkRulerRendererUpdateObserver>::New();
  this->RendererUpdateObserver->DisplayableManager = this->External;
  this->RendererUpdateObservationId = 0;
  this->ActorsAddedToRenderer = false;
  this->MarkerRenderer = vtkSmartPointer<vtkRenderer>::New();
  // Prevent erasing Z-buffer (important for quick picking and markup label visibility assessment)
  this->MarkerRenderer->EraseOff();
  this->RulerLineActor = vtkSmartPointer<vtkAxisActor2D>::New();
  this->RulerTextActor = vtkSmartPointer<vtkTextActor>::New();
}

//---------------------------------------------------------------------------
vtkMRMLRulerDisplayableManager::vtkInternal::~vtkInternal()
{
  RemoveRendererUpdateObserver();
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::vtkInternal::ShowActors(bool show)
{
  if (this->ActorsAddedToRenderer == show)
    {
    // no change
    return;
    }
  if (show)
    {
    this->MarkerRenderer->AddViewProp(this->RulerTextActor);
    this->MarkerRenderer->AddViewProp(this->RulerLineActor);
    }
  else
    {
    this->MarkerRenderer->RemoveViewProp(this->RulerTextActor);
    this->MarkerRenderer->RemoveViewProp(this->RulerLineActor);
    }
  this->ActorsAddedToRenderer = show;
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::vtkInternal::AddRendererUpdateObserver(vtkRenderer* renderer)
{
  RemoveRendererUpdateObserver();
  if (renderer)
    {
    this->ObservedRenderer = renderer;
    this->RendererUpdateObservationId = renderer->AddObserver(vtkCommand::StartEvent, this->RendererUpdateObserver);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::vtkInternal::RemoveRendererUpdateObserver()
{
  if (this->ObservedRenderer)
    {
    this->ObservedRenderer->RemoveObserver(this->RendererUpdateObservationId);
    this->RendererUpdateObservationId = 0;
    this->ObservedRenderer = nullptr;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::vtkInternal::SetupMarkerRenderer()
{
  vtkRenderer* renderer = this->External->GetRenderer();
  if (renderer==nullptr)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLRulerDisplayableManager::vtkInternal::SetupMarkerRenderer() failed: renderer is invalid");
    return;
    }

  this->MarkerRenderer->InteractiveOff();

  vtkRenderWindow* renderWindow = renderer->GetRenderWindow();
  if (renderWindow->GetNumberOfLayers() < RENDERER_LAYER+1)
    {
    renderWindow->SetNumberOfLayers( RENDERER_LAYER+1 );
    }
  this->MarkerRenderer->SetLayer(RENDERER_LAYER);
  renderWindow->AddRenderer(this->MarkerRenderer);
  // Parallel projection is needed to prevent actors from warping/tilting
  // when they are near the edge of the window.
  vtkCamera* camera = this->MarkerRenderer->GetActiveCamera();
  if (camera)
    {
    camera->ParallelProjectionOn();
    }

  // In 3D viewers we need to follow the renderer and update the orientation marker accordingly
  vtkMRMLViewNode* threeDViewNode = vtkMRMLViewNode::SafeDownCast(this->External->GetMRMLDisplayableNode());
  if (threeDViewNode)
    {
    this->AddRendererUpdateObserver(renderer);
    }

}


//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::vtkInternal::SetupRuler()
{
  this->RulerLineActor->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
  this->RulerLineActor->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
  this->RulerLineActor->LabelVisibilityOff();
  this->RulerLineActor->RulerModeOff(); // to allow specifying the number of labels (instead of distance between labels)
  this->RulerLineActor->AdjustLabelsOff(); // to allow specifying exact number of labels

  this->RulerLineActor->PickableOff();
  this->RulerLineActor->DragableOff();

  vtkTextProperty* textProperty = this->RulerTextActor->GetTextProperty();
  textProperty->SetFontSize(RULER_BASE_FONT_SIZE);
  textProperty->SetFontFamilyToArial();
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::vtkInternal::UpdateRuler()
{
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(this->External->GetMRMLDisplayableNode());
  if (!viewNode || !viewNode->GetRulerEnabled())
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLRulerDisplayableManager::UpdateMarkerOrientation() failed: view node is invalid");
    this->ShowActors(false);
    return;
    }

  if (this->External->RulerScalePresets.empty())
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLRulerDisplayableManager::UpdateMarkerOrientation() failed: no ruler scale presets are defined");
    this->ShowActors(false);
    return;
    }

  int type = viewNode->GetRulerType();
  if (type==vtkMRMLAbstractViewNode::RulerTypeNone)
    {
    // ruler not visible, no updates are needed
    this->ShowActors(false);
    return;
    }

  int viewWidthPixel = 0;
  int viewHeightPixel = 0;
  double scalingFactorPixelPerMm = 0;

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(viewNode);
  vtkMRMLViewNode* threeDViewNode = vtkMRMLViewNode::SafeDownCast(viewNode);
  if (sliceNode)
    {
    viewWidthPixel = sliceNode->GetDimensions()[0];
    viewHeightPixel = sliceNode->GetDimensions()[1];

    vtkNew<vtkMatrix4x4> rasToXY;
    vtkMatrix4x4::Invert(sliceNode->GetXYToRAS(), rasToXY.GetPointer());

    scalingFactorPixelPerMm = sqrt(
      rasToXY->GetElement(0,0)*rasToXY->GetElement(0,0) +
      rasToXY->GetElement(0,1)*rasToXY->GetElement(0,1) +
      rasToXY->GetElement(0,2)*rasToXY->GetElement(0,2));

    }
  else if (threeDViewNode && this->ObservedRenderer)
    {
    vtkCamera *cam = this->ObservedRenderer->GetActiveCamera();
    if (cam && cam->GetParallelProjection())
      {
      // Viewport: xmin, ymin, xmax, ymax; range: 0.0-1.0; origin is bottom left
      // Determine the available renderer size in pixels
      double minX = 0;
      double minY = 0;
      this->MarkerRenderer->NormalizedDisplayToDisplay(minX, minY);
      double maxX = 1;
      double maxY = 1;
      this->MarkerRenderer->NormalizedDisplayToDisplay(maxX, maxY);
      int rendererSizeInPixels[2] = {static_cast<int>(maxX-minX), static_cast<int>(maxY-minY)};

      viewWidthPixel = rendererSizeInPixels[0];
      viewHeightPixel = rendererSizeInPixels[1];

      // Parallel scale: height of the viewport in world-coordinate distances.
      // Larger numbers produce smaller images.
      scalingFactorPixelPerMm = double(rendererSizeInPixels[1])/cam->GetParallelScale()/2.0;
      }
    }
  else
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLRulerDisplayableManager::UpdateMarkerOrientation() failed: displayable node is invalid");
    this->ShowActors(false);
    return;
    }

  double rulerPreferredLengthPixel = double(viewWidthPixel)/4.0;

  // Find the value in rulerAllowedLengthsMm that is closest to rulerPreferredLength
  double rulerPreferredLength = rulerPreferredLengthPixel / scalingFactorPixelPerMm;
  std::vector<RulerScalePreset>::iterator bestMatchScalePreset = this->External->RulerScalePresets.begin();
  for (std::vector<RulerScalePreset>::iterator it = bestMatchScalePreset+1; it != this->External->RulerScalePresets.end(); ++it)
    {
    if (fabs(it->Length-rulerPreferredLength)<fabs(bestMatchScalePreset->Length-rulerPreferredLength))
      {
      bestMatchScalePreset = it;
      }
    else
      {
      // list is ordered, therefore if the difference has not got smaller
      // then it will not, so we can stop searching
      break;
      }
    }

  double actualRulerLengthPixel = double(bestMatchScalePreset->Length)*scalingFactorPixelPerMm;
  if (actualRulerLengthPixel < RULER_MINIMUM_LENGTH*viewWidthPixel || actualRulerLengthPixel > RULER_MAXIMUM_LENGTH*viewWidthPixel || viewWidthPixel==0)
    {
    // ruler is too small or too big to display or view type is invalid
    this->ShowActors(false);
    return;
    }

  int rulerLineMarginPixel = int(RULER_LINE_MARGIN*viewHeightPixel);
  int rulerTickBaseLengthPixel = int(RULER_TICK_BASE_LENGTH*viewHeightPixel);
  int rulerTextMarginPixel = int(RULER_TEXT_MARGIN*viewHeightPixel);

  // Ruler line
  double pointOrigin[3] = {double(viewWidthPixel)/2.0, static_cast<double>(rulerLineMarginPixel), 0.0};
  this->RulerLineActor->SetPoint2(pointOrigin[0]-double(bestMatchScalePreset->Length)*scalingFactorPixelPerMm/2.0, rulerLineMarginPixel);
  this->RulerLineActor->SetPoint1(pointOrigin[0]+double(bestMatchScalePreset->Length)*scalingFactorPixelPerMm/2.0, rulerLineMarginPixel);
  this->RulerLineActor->SetNumberOfLabels(bestMatchScalePreset->NumberOfMajorDivisions+1);
  this->RulerLineActor->SetNumberOfMinorTicks(bestMatchScalePreset->NumberOfMinorDivisions-1);

  // Ruler text
  std::stringstream labelStr;
  labelStr << bestMatchScalePreset->DisplayedScale*bestMatchScalePreset->Length << " " << bestMatchScalePreset->DisplayedUnitName;
  this->RulerTextActor->SetInput(labelStr.str().c_str());
  this->RulerTextActor->SetDisplayPosition(int((viewWidthPixel+bestMatchScalePreset->Length*scalingFactorPixelPerMm)/2)+rulerTextMarginPixel,rulerLineMarginPixel);

  vtkProperty2D* lineProperty = this->RulerLineActor->GetProperty();
  vtkTextProperty* textProperty = this->RulerTextActor->GetTextProperty();
  switch (type)
  {
  case vtkMRMLAbstractViewNode::RulerTypeThin:
    this->RulerLineActor->SetTickLength(rulerTickBaseLengthPixel);
    this->RulerLineActor->SetMinorTickLength(rulerTickBaseLengthPixel/2.0);
    lineProperty->SetLineWidth(1);
    textProperty->SetFontSize(RULER_BASE_FONT_SIZE);
    textProperty->SetBold(false);
    break;
  case vtkMRMLAbstractViewNode::RulerTypeThick:
    this->RulerLineActor->SetTickLength(rulerTickBaseLengthPixel*2);
    this->RulerLineActor->SetMinorTickLength(rulerTickBaseLengthPixel);
    lineProperty->SetLineWidth(3);
    textProperty->SetFontSize(RULER_BASE_FONT_SIZE*2);
    textProperty->SetBold(true);
    break;
  default:
    break;
  }

  int color = viewNode->GetRulerColor();
  switch (color)
  {
  case vtkMRMLAbstractViewNode::RulerColorWhite:
    lineProperty->SetColor(1,1,1);
    textProperty->SetColor(1,1,1);
    break;
  case vtkMRMLAbstractViewNode::RulerColorBlack:
    lineProperty->SetColor(0,0,0);
    textProperty->SetColor(0,0,0);
    break;
  case vtkMRMLAbstractViewNode::RulerColorYellow:
    lineProperty->SetColor(1,1,0);
    textProperty->SetColor(1,1,0);
    break;
  default:
    break;
  }

  this->ShowActors(true);
}

//---------------------------------------------------------------------------
// vtkMRMLRulerDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLRulerDisplayableManager::vtkMRMLRulerDisplayableManager()
{
  this->Internal = new vtkInternal(this);

  // Presets can be modified using AddRulerScalePreset(), so any application
  // that requires a different behavior can customize it by accessing the
  // displayable manager. It might be better to store this preset information
  // in the scene (add members to the view node or unit node, or add a new
  // ruler display node).

  //                        length, major, minor, unit, scale
  this->AddRulerScalePreset(  1e-2,   2,     5,    "um", 1e3 );
  this->AddRulerScalePreset(2.5e-2,   5,     5,    "um", 1e3 );
  this->AddRulerScalePreset(  5e-2,   5,     0,    "um", 1e3 );
  this->AddRulerScalePreset(  1e-1,   2,     5,    "um", 1e3 );
  this->AddRulerScalePreset(2.5e-1,   5,     5,    "um", 1e3 );
  this->AddRulerScalePreset(  5e-1,   5,     0,    "mm", 1e0 );
  this->AddRulerScalePreset(  1e0,    2,     5,    "mm", 1e0 );
  this->AddRulerScalePreset(2.5e0,    5,     5,    "mm", 1e0 );
  this->AddRulerScalePreset(  5e0,    5,     0,    "mm", 1e0 );
  this->AddRulerScalePreset(  1e1,    2,     5,    "mm", 1e0 );
  this->AddRulerScalePreset(2.5e1,    5,     5,    "mm", 1e0 );
  this->AddRulerScalePreset(  5e1,    5,     0,    "cm", 1e-1 );
  this->AddRulerScalePreset(  1e2,    2,     5,    "cm", 1e-1 );
  this->AddRulerScalePreset(2.5e2,    5,     5,    "cm", 1e-1 );
  this->AddRulerScalePreset(  5e2,    5,     0,    "cm", 1e-1 );
  this->AddRulerScalePreset(  1e3,    2,     5,     "m", 1e-3 );
  this->AddRulerScalePreset(2.5e3,    5,     5,     "m", 1e-3 );
  this->AddRulerScalePreset(  5e3,    5,     0,     "m", 1e-3 );
  this->AddRulerScalePreset(  1e4,    2,     5,     "m", 1e-3 );
  this->AddRulerScalePreset(2.5e4,    5,     5,     "m", 1e-3 );
  this->AddRulerScalePreset(  5e4,    5,     0,     "m", 1e-3 );
}

//---------------------------------------------------------------------------
vtkMRMLRulerDisplayableManager::~vtkMRMLRulerDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::Create()
{
  this->Internal->SetupMarkerRenderer();
  this->Internal->SetupRuler();
  this->Superclass::Create();
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::UpdateFromViewNode()
{
  // View node is changed, which may mean that either the marker type (visibility), size, or orientation is changed
  this->Internal->UpdateRuler();
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* vtkNotUsed(caller))
{
  // view node is changed
  this->UpdateFromViewNode();
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::UpdateFromRenderer()
{
  // Rendering is performed, so let's re-render the marker with up-to-date orientation
  this->Internal->UpdateRuler();
}

//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::AddRulerScalePreset(double length, int numberOfMajorDivisions, int numberOfMinorDivisions,
  const std::string& displayedUnitName, double displayedScale)
{
  // insert into this->RulerScalePresets list, ordered by Length
  std::vector<RulerScalePreset>::iterator it = this->RulerScalePresets.begin();
  for (; it != this->RulerScalePresets.end(); ++it)
    {
    if (it->Length == length)
      {
      // found an exact match, update it
      it->NumberOfMajorDivisions = numberOfMajorDivisions;
      it->NumberOfMinorDivisions = numberOfMinorDivisions;
      it->DisplayedUnitName = displayedUnitName;
      it->DisplayedScale = displayedScale;
      return;
      }
    if (it->Length>length)
      {
      // this element's Length is larger, insert the new element before this
      break;
      }
    }
  RulerScalePreset preset;
  preset.Length = length;
  preset.NumberOfMajorDivisions = numberOfMajorDivisions;
  preset.NumberOfMinorDivisions = numberOfMinorDivisions;
  preset.DisplayedUnitName = displayedUnitName;
  preset.DisplayedScale = displayedScale;
  this->RulerScalePresets.insert(it, preset);
}


//---------------------------------------------------------------------------
void vtkMRMLRulerDisplayableManager::RemoveAllRulerScalePresets()
{
  this->RulerScalePresets.clear();
}
