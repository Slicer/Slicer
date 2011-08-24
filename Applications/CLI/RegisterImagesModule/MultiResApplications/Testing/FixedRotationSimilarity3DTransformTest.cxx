
#ifdef _WIN32
#ifndef M_PI
#define M_PI 3.1415926535898
#endif
#endif

#include "itkFixedRotationSimilarity3DTransform.h"

int FixedRotationSimilarity3DTransformTest(int, char * [])
{
  typedef itk::FixedRotationSimilarity3DTransform<double> TransformType;

    {
    std::cout << "Creating identity transform" << std::endl;
    TransformType::Pointer identity = TransformType::New();
    // TransformType::TranslationType trans = identity->GetTranslation();
    // trans[0] = 1;
    // trans[1] = 2;
    // trans[2] = 3;

    // identity->SetScale(2.0);
    // identity->SetTranslation(trans);
    identity->SetIdentity();

    std::cout << "Scale: " << identity->GetScale() << std::endl;
    std::cout << "Translation: " << identity->GetTranslation() << std::endl;

    typedef TransformType::ParametersType ParametersType;
    const ParametersType parameters = identity->GetParameters();
    std::cout << "Parameters" << std::endl;
    std::cout << parameters << std::endl;
    if( parameters[0] != 0 ||
        parameters[1] != 0 ||
        parameters[2] != 0 ||
        parameters[3] != 1 )
      {
      std::cerr << "ERROR: Identity trasform does not have paramteres [0,0,0,1]" << std::endl;
      return EXIT_FAILURE;
      }

    }

    {
    TransformType::Pointer purerotation = TransformType::New();
    purerotation->SetIdentity();

    TransformType::InputPointType center;
    center[0] = 50;
    center[1] = 60;
    center[2] = 70;
    purerotation->SetCenter(center);

    TransformType::AxisType axis;
    axis[0] = 1;
    axis[1] = 1;
    axis[2] = 0;
    axis.Normalize();

    const TransformType::AngleType angle = M_PI / 2; // radians
    purerotation->SetRotation(axis, angle);

    std::cout << "Rotatoin about axis: " << axis << " by pi/2" << std::endl;
    std::cout << purerotation << std::endl;

    typedef TransformType::ParametersType ParametersType;
    const ParametersType parameters = purerotation->GetParameters();
    std::cout << "Parameters" << std::endl;
    std::cout << parameters << std::endl;

    if( parameters[0] != 0 ||
        parameters[1] != 0 ||
        parameters[2] != 0 ||
        parameters[3] != 1 )
      {
      std::cerr << "ERROR: Pure rotation transform does not have paramteres [0,0,0,1]" << std::endl;
      return EXIT_FAILURE;
      }

    }

    {
    TransformType::Pointer rigidtransform = TransformType::New();
    rigidtransform->SetIdentity();

    TransformType::InputPointType center;
    center[0] = 50;
    center[1] = 60;
    center[2] = 70;
    rigidtransform->SetCenter(center);

    TransformType::AxisType axis;
    axis[0] = 1;
    axis[1] = 1;
    axis[2] = 0;
    axis.Normalize();

    const TransformType::AngleType angle = M_PI / 2; // radians
    rigidtransform->SetRotation(axis, angle);

    std::cout << "Rotatoin about axis: " << axis << " by pi/2" << std::endl;

    TransformType::TranslationType trans;
    trans[0] = -20;
    trans[1] = 10;
    trans[2] = .5;

    std::cout << "Translation by " << trans << std::endl;
    rigidtransform->SetTranslation(trans);

    std::cout << rigidtransform << std::endl;

    typedef TransformType::ParametersType ParametersType;
    ParametersType parameters = rigidtransform->GetParameters();
    std::cout << "Parameters" << std::endl;
    std::cout << parameters << std::endl;

    if( parameters[0] != trans[0] ||
        parameters[1] != trans[1] ||
        parameters[2] != trans[2] ||
        parameters[3] != 1 )
      {
      std::cerr << "ERROR: Pure rotation transform does not have paramteres [0,0,0,1]" << std::endl;
      return EXIT_FAILURE;
      }

    // Set parameters and ensure that transformation works correctlyx
    std::cout << "Seeting new parameters and ensuring rotation still exists" << std::endl;
    parameters[0] = .5;
    parameters[1] = -.5;
    parameters[2] = 1.0;
    rigidtransform->SetParameters(parameters);
    std::cout << rigidtransform << std::endl;

    }

    {
    TransformType::Pointer similaritytransform = TransformType::New();
    similaritytransform->SetIdentity();

    TransformType::InputPointType center;
    center[0] = 50;
    center[1] = 60;
    center[2] = 70;
    similaritytransform->SetCenter(center);

    TransformType::AxisType axis;
    axis[0] = 0;
    axis[1] = 0;
    axis[2] = 1;
    axis.Normalize();

    const TransformType::AngleType angle = M_PI / 2; // radians
    similaritytransform->SetRotation(axis, angle);

    std::cout << "Rotatoin about axis: " << axis << " by pi/2" << std::endl;

    TransformType::TranslationType trans;
    trans[0] = -20;
    trans[1] = 10;
    trans[2] = .5;

    std::cout << "Translation by " << trans << std::endl;
    similaritytransform->SetTranslation(trans);

    TransformType::ScaleType scale(1.5);

    std::cout << "Scale by " << scale << std::endl;
    similaritytransform->SetScale(scale);

    std::cout << similaritytransform << std::endl;

    typedef TransformType::ParametersType ParametersType;
    const ParametersType parameters = similaritytransform->GetParameters();
    std::cout << "Parameters" << std::endl;
    std::cout << parameters << std::endl;

    if( parameters[0] != trans[0] ||
        parameters[1] != trans[1] ||
        parameters[2] != trans[2] ||
        parameters[3] != scale )
      {
      std::cerr << "ERROR: Parameters are not translation (" << trans << ") followed by scale (" << scale << ")"
                << std::endl;
      return EXIT_FAILURE;
      }

    std::cout << "Jacobian at center pt" << std::endl;
    TransformType::JacobianType jacobian =
      similaritytransform->GetJacobian(center);
    std::cout << jacobian << std::endl;

    if( jacobian[0][0] != 1.0 ||
        jacobian[1][1] != 1.0 ||
        jacobian[2][2] != 1.0 ||
        jacobian[0][1] != 0.0 ||
        jacobian[0][2] != 0.0 ||
        jacobian[1][0] != 0.0 ||
        jacobian[1][2] != 0.0 ||
        jacobian[2][0] != 0.0 ||
        jacobian[2][1] != 0.0 ||
        jacobian[0][3] != 0.0 ||
        jacobian[2][3] != 0.0 ||
        jacobian[2][3] != 0.0 )
      {
      std::cerr << "Jacobian at center is incorrect" << std::endl;
      return EXIT_FAILURE;
      }

    TransformType::InputPointType centerp1;
    centerp1[0] = center[0];
    centerp1[1] = center[1] + 1.0;
    centerp1[2] = center[2];

    std::cout << "Jacobian at [0,1,0]" << std::endl;
    jacobian = similaritytransform->GetJacobian(centerp1);
    std::cout << jacobian << std::endl;

    if( jacobian[0][0] != 1.0 ||
        jacobian[0][1] != 0.0 ||
        jacobian[0][2] != 0.0 ||
        jacobian[1][0] != 0.0 ||
        jacobian[1][1] != 1.0 ||
        jacobian[1][2] != 0.0 ||
        jacobian[2][0] != 0.0 ||
        jacobian[2][1] != 0.0 ||
        jacobian[2][2] != 1.0 ||
        fabs(jacobian[0][3] + 1.0) > 1.0e-10 ||
        fabs(jacobian[1][3] - 0.0) > 1.0e-10 ||
        fabs(jacobian[2][3] - 0.0) > 1.0e-10 )
      {
      std::cerr << "Jacobian at center_Y + 1 is incorrect" << std::endl;
      return EXIT_FAILURE;
      }

    }

  return EXIT_SUCCESS;
}
