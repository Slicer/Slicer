/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMarkupsDisplayNode.h"

int vtkMRMLMarkupsDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLMarkupsDisplayNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  TEST_SET_GET_DOUBLE_RANGE(node1, TextScale, 0.0, 100.0);

  TEST_SET_GET_INT_RANGE(node1, GlyphType, -1, 10);

  for (int i = vtkMRMLMarkupsDisplayNode::GetMinimumGlyphType();
       i <= vtkMRMLMarkupsDisplayNode::GetMaximumGlyphType();
       i++)
    {
    node1->SetGlyphType(i);
    std::cout << i << " GetGlyphType = " << node1->GetGlyphType()
              << ", as string = " << node1->GetGlyphTypeAsString()
              << ", GetGlyphTypeAsString(" << i << ") = "
              << node1->GetGlyphTypeAsString(i) << std::endl;
    }

  // print out the enums
  std::cout << "Enum GlyphShapes:" << std::endl;
  std::cout << "    Vertex2D = " << vtkMRMLMarkupsDisplayNode::Vertex2D
            << ", as string = "
            << node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::Vertex2D)
            << std::endl;
  std::cout << "    Dash2D = " << vtkMRMLMarkupsDisplayNode::Dash2D
            << ", as string = "
            << node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::Dash2D)
            << std::endl;
  std::cout << "    Cross2D = " << vtkMRMLMarkupsDisplayNode::Cross2D
            << ", as string = "
            << node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::Cross2D)
            << std::endl;
  std::cout << "    CrossDot2D = " << vtkMRMLMarkupsDisplayNode::CrossDot2D
            << ", as string = "
            << node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::CrossDot2D)
            << std::endl;
  std::cout << "    ThickCross2D = " << vtkMRMLMarkupsDisplayNode::ThickCross2D
            << ", as string = " <<
            node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::ThickCross2D)
            << std::endl;
  std::cout << "    Triangle2D = " << vtkMRMLMarkupsDisplayNode::Triangle2D
            << ", as string = "
            << node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::Triangle2D)
            << std::endl;
  std::cout << "    Square2D = " << vtkMRMLMarkupsDisplayNode::Square2D
            << ", as string = "
            << node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::Square2D)
            << std::endl;
  std::cout << "    Circle2D = " << vtkMRMLMarkupsDisplayNode::Circle2D
            << ", as string = "
            << node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::Circle2D)
            << std::endl;
  std::cout << "    Diamond2D = " << vtkMRMLMarkupsDisplayNode::Diamond2D
            << ", as string = "
            << node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::Diamond2D)
            << std::endl;
  std::cout << "    Arrow2D = " << vtkMRMLMarkupsDisplayNode::Arrow2D
            << ", as string = "
            << node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::Arrow2D)
            << std::endl;
  std::cout << "    ThickArrow2D = " << vtkMRMLMarkupsDisplayNode::ThickArrow2D
            << ", as string = " <<
            node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::ThickArrow2D)
            << std::endl;
  std::cout << "    HookedArrow2D = " << vtkMRMLMarkupsDisplayNode::HookedArrow2D
            << ", as string = " <<
            node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::HookedArrow2D)
            << std::endl;
  std::cout << "    StarBurst2D = " << vtkMRMLMarkupsDisplayNode::StarBurst2D
            << ", as string = " <<
            node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::StarBurst2D)
            << std::endl;
  std::cout << "    Sphere3D = " << vtkMRMLMarkupsDisplayNode::Sphere3D
            << ", as string = "
            << node1->GetGlyphTypeAsString(vtkMRMLMarkupsDisplayNode::Sphere3D)
            << std::endl;

  // spot test int to string mapping
  node1->SetGlyphType(vtkMRMLMarkupsDisplayNode::Sphere3D);
  if (strcmp(node1->GetGlyphTypeAsString(), "Sphere3D") != 0)
    {
    std::cerr << "ERROR: set the glyph type to "
              << vtkMRMLMarkupsDisplayNode::Sphere3D
              << ", but get glyph type as string returned "
              << node1->GetGlyphTypeAsString() << " instead of Sphere3D"
              << std::endl;
    return EXIT_FAILURE;
    }

  // test GlyphTypeIs3D
  node1->SetGlyphTypeFromString("Triangle2D");
  if (node1->GlyphTypeIs3D() == 1)
    {
    std::cerr << "ERROR: triangle 2d not recognised as a 2d glyph" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetGlyphTypeFromString("Sphere3D");
  if (node1->GlyphTypeIs3D() != 1)
    {
    std::cerr << "ERROR: sphere 3d not recognised as a 3d glyph" << std::endl;
    return EXIT_FAILURE;
    }

  TEST_SET_GET_DOUBLE_RANGE(node1, GlyphScale, -1.0, 25.6);

  TEST_SET_GET_BOOLEAN(node1, SliceProjection);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node1, SliceProjectionColor, 0.0, 1.0);

  node1->SetSliceProjectionOpacity(0.0);
  node1->SetSliceProjectionOpacity(0.5);
  if (node1->GetSliceProjectionOpacity() != 0.5)
    {
    std::cerr << "Failed to set projected opacity to 0.5" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetSliceProjectionOpacity(1.0);


  node1->SliceProjectionUseFiducialColorOn();
  if (node1->GetSliceProjectionUseFiducialColor() != true)
    {
    std::cerr << "Failed to turn use fiducial color on with slice projections"
              << ", slice projection = " << node1->GetSliceProjection()
              << std::endl;
    return EXIT_FAILURE;
    }
  node1->SliceProjectionUseFiducialColorOff();

  node1->SliceProjectionOutlinedBehindSlicePlaneOn();
  if (node1->GetSliceProjectionOutlinedBehindSlicePlane() != true)
    {
    std::cerr << "Failed to turn use outline behind slice plane on"
              << ", slice projection = " << node1->GetSliceProjection()
              << std::endl;
    return EXIT_FAILURE;
    }
  node1->SliceProjectionOutlinedBehindSlicePlaneOff();

  return EXIT_SUCCESS;
}
