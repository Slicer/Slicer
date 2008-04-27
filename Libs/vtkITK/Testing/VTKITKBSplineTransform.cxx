#include "vtkITKBSplineTransform.h"

#include "itkBSplineDeformableTransform.h"

#include "vtkSmartPointer.h"

int main( int, char** )
{
  // we want to transform a 300x400x300 image, with grid points 100
  // pixels apart. So, we need 4x5x4 "interior" grid points. Since the
  // spline order is 3, we need 1 exterior node on the "left" and 2
  // exterior nodes on the "right".  So, we need a spline grid of
  // 7x8x7. We want to transform the whole image, so the origin should
  // be set to spline grid location (0,0).


  //  B--o--o--o--o--o--o   A = image(0,0)       A = spline_grid(1,1)
  //  |  |  |  |  |  |  |   B = image(-100,-100) B = spline_grid(0,0)
  //  o--A--+--+--+--o--o
  //  |  |  |  |  |  |  |
  //  o--+--+--+--+--o--o   origin should be set to (-100,-100)
  //  |  |  |  |  |  |  |
  //  o--+--+--+--+--o--o
  //  |  |  |  |  |  |  |
  //  o--o--o--o--o--o--o
  //  |  |  |  |  |  |  |
  //  o--o--o--o--o--o--o

  {
    std::cout << "ITK:\n";

    typedef itk::BSplineDeformableTransform<double,3,3> itkBSplineType;

    itkBSplineType::RegionType region;
    itkBSplineType::OriginType origin;
    itkBSplineType::SpacingType spacing;

    assert( origin.Size() == 3 );
    origin[0] = -100;
    origin[1] = -100;
    origin[2] = -100;

    assert( spacing.Size() == 3 );
    spacing[0] = 100;
    spacing[1] = 100;
    spacing[2] = 100;

    itkBSplineType::RegionType::SizeType sz;
    assert( sz.Size() == 3 );
    sz[0] = 7;
    sz[1] = 8;
    sz[2] = 7;
    region.SetSize( sz );

    itkBSplineType::RegionType::IndexType idx;
    assert( idx.Size() == 3 );
    idx[0] = idx[1] = idx[2] = 0;
    region.SetIndex( idx );

    itkBSplineType::Pointer bspline = itkBSplineType::New();
    bspline->SetGridRegion( region );
    bspline->SetGridOrigin( origin );
    bspline->SetGridSpacing( spacing );

    //  The coefficients are passed to the B-spline in the form of
    //  an array where the first set of elements are the first
    //  component of the displacements for all the nodes, and the
    //  second set of elemets is formed by the second component of the
    //  displacements for all the nodes.
    const unsigned int numberOfParameters = bspline->GetNumberOfParameters();
    typedef itkBSplineType::ParametersType ParametersType;
    ParametersType parameters( numberOfParameters );

    const unsigned int numberOfNodes = numberOfParameters / 3;
    assert( numberOfNodes == 7 * 8 * 7 );

    std::cout << " number of parameters = " << numberOfParameters << std::endl;

    parameters.Fill( 0.0 );
    // set the parameter at image(100,200,0)=>node(2,3,1)
    // the displacement of x at (100,200,0)
    parameters.SetElement( 2 + 3*7 + 1*7*8, 3 );

    bspline->SetParameters( parameters );

    itkBSplineType::InputPointType inputPoint;
    inputPoint[0] = 100; inputPoint[1] = 200; inputPoint[2] = 0;
    itkBSplineType::OutputPointType outputPoint = 
      bspline->TransformPoint( inputPoint );
    std::cout << " InputPoint: " << inputPoint << std::endl;
    std::cout << " outputPoint: " << outputPoint << std::endl;
  }


  {
    std::cout << "vtkITK:\n";

    typedef vtkSmartPointer<vtkITKBSplineTransform> BSplineSPtr;
    BSplineSPtr vtkBSpline = vtkITKBSplineTransform::New();

    double origin[3] = { -100.0, -100.0, -100.0 };
    double spacing[3] = { 100.0, 100.0, 100.0 };
    unsigned gridsize[3] = { 7, 8, 7 };
    vtkBSpline->SetSplineOrder( 3 );
    vtkBSpline->SetGridOrigin( origin );
    vtkBSpline->SetGridSpacing( spacing );
    vtkBSpline->SetGridSize( gridsize );

    vtkDoubleArray* parameters = vtkDoubleArray::New();
    parameters->SetNumberOfValues( vtkBSpline->GetNumberOfParameters() );

    std::cout << " number of parameters = " << vtkBSpline->GetNumberOfParameters() << std::endl;

    // See comments above about which parameters to set
    parameters->FillComponent( 0, 0.0 );
    parameters->SetValue( 2 + 3*7 + 1*7*8, 3 );

    vtkBSpline->SetParameters( *parameters );

    double inputPoint[3] = { 100, 200, 0 };
    double outputPoint[3] = { -1, -1, -1 };
    vtkBSpline->TransformPoint( inputPoint, outputPoint );
    std::cout << " inputPoint: " << inputPoint[0] << " "
              << inputPoint[1] << " "
              << inputPoint[2] << std::endl;

    std::cout << " outputPoint: " << outputPoint[0] << " "
              << outputPoint[1] << " "
              << outputPoint[2] << std::endl;

    double inversePoint[3] = { -1, -1, -1 };
    vtkBSpline->Inverse();
    vtkBSpline->TransformPoint( outputPoint, inversePoint );

    std::cout << " inversePoint: " << inversePoint[0] << " "
              << inversePoint[1] << " "
              << inversePoint[2] << std::endl;

    parameters->Delete();
    vtkBSpline->Delete();
  }

}
