#include "vtkMRMLAnnotationPointDisplayNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationPointDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLAnnotationPointDisplayNode> node1;

  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  TEST_SET_GET_INT_RANGE(node1, GlyphType, -1, 10);

  // min glyph type
  if (node1->GetMinimumGlyphType() != vtkMRMLAnnotationPointDisplayNode::GlyphMin)
    {
    std::cerr << "Error: GetMinimumGlyphType " << node1->GetMinimumGlyphType() << " != vtkMRMLAnnotationPointDisplayNode::GlyphMin " << vtkMRMLAnnotationPointDisplayNode::GlyphMin << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "GlyphMin = " << node1->GetMinimumGlyphType() << ", as string = " << node1->GetGlyphTypeAsString(node1->GetMinimumGlyphType()) << std::endl;
    }

  // max glyph type
  if (node1->GetMaximumGlyphType() != vtkMRMLAnnotationPointDisplayNode::GlyphMax)
    {
    std::cerr << "Error: GetMaximumGlyphType " << node1->GetMaximumGlyphType() << " != vtkMRMLAnnotationPointDisplayNode::GlyphMax " << vtkMRMLAnnotationPointDisplayNode::GlyphMax << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "GlyphMax = " << node1->GetMaximumGlyphType() << ", as string = " << node1->GetGlyphTypeAsString(node1->GetMaximumGlyphType()) << std::endl;
    }
  for (int i = vtkMRMLAnnotationPointDisplayNode::GlyphMin;
       i < vtkMRMLAnnotationPointDisplayNode::GlyphMax;
       i++)
    {
    node1->SetGlyphType(i);
    std::cout << i << " GetGlyphType = " << node1->GetGlyphType() << ", as string = " << node1->GetGlyphTypeAsString() << ", GetGlyphTypeAsString(" << i << ") = " << node1->GetGlyphTypeAsString(i) << std::endl;
    }

  // print out the enums
  std::cout << "Enum GlyphShapes:" << std::endl;
  std::cout << "    GlyphMin = " << vtkMRMLAnnotationPointDisplayNode::GlyphMin << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::GlyphMin) << std::endl;
  std::cout << "    Vertex2D = " << vtkMRMLAnnotationPointDisplayNode::Vertex2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::Vertex2D) << std::endl;
  std::cout << "    Dash2D = " << vtkMRMLAnnotationPointDisplayNode::Dash2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::Dash2D) << std::endl;
  std::cout << "    Cross2D = " << vtkMRMLAnnotationPointDisplayNode::Cross2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::Cross2D) << std::endl;
  std::cout << "    ThickCross2D = " << vtkMRMLAnnotationPointDisplayNode::ThickCross2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::ThickCross2D) << std::endl;
  std::cout << "    Triangle2D = " << vtkMRMLAnnotationPointDisplayNode::Triangle2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::Triangle2D) << std::endl;
  std::cout << "    Square2D = " << vtkMRMLAnnotationPointDisplayNode::Square2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::Square2D) << std::endl;
  std::cout << "    Circle2D = " << vtkMRMLAnnotationPointDisplayNode::Circle2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::Circle2D) << std::endl;
  std::cout << "    Diamond2D = " << vtkMRMLAnnotationPointDisplayNode::Diamond2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::Diamond2D) << std::endl;
  std::cout << "    Arrow2D = " << vtkMRMLAnnotationPointDisplayNode::Arrow2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::Arrow2D) << std::endl;
  std::cout << "    ThickArrow2D = " << vtkMRMLAnnotationPointDisplayNode::ThickArrow2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::ThickArrow2D) << std::endl;
  std::cout << "    HookedArrow2D = " << vtkMRMLAnnotationPointDisplayNode::HookedArrow2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::HookedArrow2D) << std::endl;
  std::cout << "    StarBurst2D = " << vtkMRMLAnnotationPointDisplayNode::StarBurst2D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::StarBurst2D) << std::endl;
  std::cout << "    Sphere3D = " << vtkMRMLAnnotationPointDisplayNode::Sphere3D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::Sphere3D) << std::endl;
  std::cout << "    Diamond3D = " << vtkMRMLAnnotationPointDisplayNode::Diamond3D << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::Diamond3D) << std::endl;
  std::cout << "    GlyphMax = " << vtkMRMLAnnotationPointDisplayNode::GlyphMax << ", as string = " << node1->GetGlyphTypeAsString(vtkMRMLAnnotationPointDisplayNode::GlyphMax) << std::endl;

  // semantic assumptions
  if (node1->GetMinimumGlyphType() != vtkMRMLAnnotationPointDisplayNode::Vertex2D)
    {
    std::cerr << "Error: minimum glyph type " << node1->GetMinimumGlyphType() << " != Vertex2d: " << vtkMRMLAnnotationPointDisplayNode::Vertex2D << std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLAnnotationPointDisplayNode::Vertex2D != 1)
    {
    std::cerr << "ERROR: Vertex2D (" << vtkMRMLAnnotationPointDisplayNode::Vertex2D << ") is not defined as 1! Setting glyph types on the source class won't work!" << std::endl;
    return EXIT_FAILURE;
    }

  if (node1->GetMaximumGlyphType() != vtkMRMLAnnotationPointDisplayNode::Sphere3D)
    {
    std::cerr << "Error: maximum glyph type " << node1->GetMaximumGlyphType() << " != Sphere3D: " << vtkMRMLAnnotationPointDisplayNode::Sphere3D << std::endl;
    return EXIT_FAILURE;
    }

  // spot test int to string mapping
  node1->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::Sphere3D);
  if (strcmp(node1->GetGlyphTypeAsString(), "Sphere3D") != 0)
    {
    std::cerr << "ERROR: set the glyph type to " << vtkMRMLAnnotationPointDisplayNode::Sphere3D << ", but get glyph type as string returned " << node1->GetGlyphTypeAsString() << " instead of Sphere3D" << std::endl;
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

  return EXIT_SUCCESS;
}
