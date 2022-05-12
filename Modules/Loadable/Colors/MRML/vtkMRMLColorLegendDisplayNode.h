/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLColorLegendDisplayNode_h
#define __vtkMRMLColorLegendDisplayNode_h

// MRML includes
#include <vtkMRMLDisplayNode.h>
#include "vtkSlicerColorsModuleMRMLExport.h"

class vtkMRMLColorNode;
class vtkTextProperty;

/// \brief Class describing how to display a color legend.
///
/// This display node must be added as a display node to a displayable node
/// and a display node of that same data node must be set using SetAndObservePrimaryDisplayNode.
///
/// The color legend will be displayed for the colormap and scalar range defined in the primary display node.
/// Scalar range and color node that are set in the vtkMRMLColorLegendDisplayNode are ignored.
///
class VTK_SLICER_COLORS_MODULE_MRML_EXPORT vtkMRMLColorLegendDisplayNode : public vtkMRMLDisplayNode
{
public:
  /// color legend orientation preset on a view node
  enum OrientationType : int { Horizontal = 0, Vertical, Orientation_Last };

  static vtkMRMLColorLegendDisplayNode *New();
  vtkTypeMacro(vtkMRMLColorLegendDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "ColorLegendDisplay"; }

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLColorLegendDisplayNode);

  //@{
  /// Get/Set color legend orientation
  /// Possible values: vtkMRMLColorLegendDisplayNode::Horizontal, vtkMRMLColorLegendDisplayNode::Vectical
  vtkGetMacro(Orientation, OrientationType);
  vtkSetMacro(Orientation, OrientationType);
  //@}

  //@{
  /// Get/Set color legend position of lower left point.
  /// Values are fraction of window size.
  vtkGetVector2Macro(Position, double);
  vtkSetVector2Macro(Position, double);
  //@}

  //@{
  /// Color legend size as fraction of window size.
  /// Instead of storing width and height, the two values correspond to
  /// size of short side and long side, as this allows quick orientation switching
  /// without the need to adjust the size.
  /// For vertical orientation: width, height.
  /// For horizontal orientation: height, width.
  vtkGetVector2Macro(Size, double);
  vtkSetVector2Macro(Size, double);
  //@}

  //@{
  /// Get/Set color legend title
  vtkGetMacro(TitleText, std::string);
  vtkSetMacro(TitleText, std::string);
  //@}

  //@{
  /// Get/set color legend labels format string according to printf
  /// function specification (https://www.cplusplus.com/reference/cstdio/printf/).
  ///
  /// Examples:
  /// - display with 1 fractional digits: %.1f
  /// - display integer: %.0f
  /// - display with 4 significant digits: %.4g
  /// - string label annotation: %s
  vtkGetMacro(LabelFormat, std::string);
  vtkSetMacro(LabelFormat, std::string);
  //@}

  /// Default label format for numerical values.
  /// Recommended when UseColorNamesForLabels is false.
  /// \sa SetLabelFormat
  virtual std::string GetDefaultNumericLabelFormat();

  /// Default label format for text label values.
  /// Recommended when UseColorNamesForLabels is true.
  /// \sa SetLabelFormat
  virtual std::string GetDefaultTextLabelFormat();

  //@{
  /// Get/set maximum number of colors in color legend
  vtkGetMacro(MaxNumberOfColors, int);
  vtkSetMacro(MaxNumberOfColors, int);
  //@}

  //@{
  /// Get/set number of displayed labels in color legend
  vtkGetMacro(NumberOfLabels, int);
  vtkSetMacro(NumberOfLabels, int);
  //@}

  //@{
  /// Get/set use color names for labels flag
  vtkGetMacro(UseColorNamesForLabels, bool);
  vtkSetMacro(UseColorNamesForLabels, bool);
  //@}

  //@{
  /// Text property object that controls the display properties of title.
  /// The text object property controls background color/opacity, frame size/color, font, etc.
  /// This function always returns a valid vtkTextProperty pointer.
  vtkGetObjectMacro(TitleTextProperty, vtkTextProperty);
  //@}

  //@{
  /// Text property object that controls the display properties of labels.
  /// The text object property controls background color/opacity, frame size/color, font, etc.
  /// This function always returns a valid vtkTextProperty pointer.
  vtkGetObjectMacro(LabelTextProperty, vtkTextProperty);
  //@}

  //@{
  /// Get/set primary display node, i.e., the display node that specifies the color table and scalar range
  /// that this color legend display node should display.
  /// This node already has an indirect reference to the displayable node, but a displayable node may have
  /// multiple display nodes, that is why this reference to a specific display node is necessary.
  vtkMRMLDisplayNode* GetPrimaryDisplayNode();
  void SetAndObservePrimaryDisplayNode(vtkMRMLDisplayNode* node);
  //@}

  /// Handles events registered in the observer manager
  void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

protected:
  vtkMRMLColorLegendDisplayNode();
  ~vtkMRMLColorLegendDisplayNode() override;
  vtkMRMLColorLegendDisplayNode(const vtkMRMLColorLegendDisplayNode&);
  void operator=(const vtkMRMLColorLegendDisplayNode&);

  static const char* GetOrientationAsString(int id);
  static int GetOrientationFromString(const char* name);
  void SetOrientation(int id);

  //@{
  /// Get/set the text style from/as a string.
  /// String format follows html-style conventions.
  void SetTitleTextPropertyFromString(std::string textPropertyString);
  void SetLabelTextPropertyFromString(std::string textPropertyString);
  std::string GetTitleTextPropertyAsString();
  std::string GetLabelTextPropertyAsString();
  //@}

private:
  OrientationType Orientation{ vtkMRMLColorLegendDisplayNode::Vertical }; // Vertical or Horizontal
  double Position[2]{ 0.95, 0.5 }; // color legend position within view
  double Size[2]{ 0.15, 0.5 }; // color legend width within view
  std::string TitleText; // color legend title
  vtkTextProperty* TitleTextProperty{ nullptr };
  vtkTextProperty* LabelTextProperty{ nullptr };
  // 7 so that the number appears aligned to the right even if it is large (>1000)
  // .1 to show one fractional digit
  // f to make the behavior more predictable for users (behavior of g is somewhat complex)
  std::string LabelFormat;
  int MaxNumberOfColors{ 256 };
  int NumberOfLabels{ 5 };
  bool UseColorNamesForLabels{ false };
  bool CenterLabels{ false };
};

#endif
