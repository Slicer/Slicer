
#ifdef _WIN32
#ifndef M_PI
#define M_PI 3.1415926535898
#endif
#endif

#include "itkEulerSimilarity3DTransform.h"

namespace
{
  bool almost(double x, double y, double eps)
  {
    return fabs(x - y) <= eps;
  }
}

int EulerSimilarity3DTransformTest(int, char* [])
{
  typedef itk::EulerSimilarity3DTransform<double> TransformType;

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
    if(parameters[0] != 0 ||
       parameters[1] != 0 ||
       parameters[2] != 0 ||
       parameters[3] != 0 ||
       parameters[4] != 0 ||
       parameters[5] != 0 ||
       parameters[6] != 1)
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

    const TransformType::AngleType angle = M_PI/4; // radians
    purerotation->SetRotation(angle, 0, 0);
  
    std::cout << purerotation << std::endl;

    typedef TransformType::ParametersType ParametersType;
    const ParametersType parameters = purerotation->GetParameters();
    std::cout << "Parameters" << std::endl;
    std::cout << parameters << std::endl;

    if(parameters[0] != angle ||
       parameters[1] != 0 ||
       parameters[2] != 0 ||
       parameters[3] != 0 ||
       parameters[4] != 0 ||
       parameters[5] != 0 ||
       parameters[6] != 1)
      {
      std::cerr << "ERROR: Pure rotation transform does not have paramteres [pi/2,0,0,0,0,0,1]" << std::endl;
      return EXIT_FAILURE;
      }

    }

    // Non axis-aligned rotation
    {
    TransformType::Pointer purerotation = TransformType::New();
    purerotation->SetIdentity();  

    TransformType::InputPointType center;
    center[0] = 50;
    center[1] = 60;
    center[2] = 70;
    purerotation->SetCenter(center);

    const TransformType::AngleType angle = M_PI/4; // radians
    purerotation->SetRotation(angle, angle, 0);
  
    std::cout << purerotation << std::endl;

    typedef TransformType::ParametersType ParametersType;
    const ParametersType parameters = purerotation->GetParameters();
    std::cout << "Parameters" << std::endl;
    std::cout << parameters << std::endl;

    if(parameters[0] != angle ||
       parameters[1] != angle ||
       parameters[2] != 0 ||
       parameters[3] != 0 ||
       parameters[4] != 0 ||
       parameters[5] != 0 ||
       parameters[6] != 1)
      {
      std::cerr << "ERROR: Pure rotation transform does not have paramteres [pi/2,0,0,0,0,0,1]" << std::endl;
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

    const TransformType::AngleType angle = M_PI/4; // radians
    rigidtransform->SetRotation(angle, 0, 0);
  
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

    if(parameters[0] != angle ||
       parameters[1] != 0 ||
       parameters[2] != 0 ||
       parameters[3] != trans[0] ||
       parameters[4] != trans[1] ||
       parameters[5] != trans[2] ||
       parameters[6] != 1)
      {
      std::cerr << "ERROR: Pure rotation transform does not have paramteres [pi/2,0,0,-20,10,,5,1]" << std::endl;
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

    const TransformType::AngleType angle = M_PI/4; // radians
    similaritytransform->SetRotation(0, 0, angle);
  
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

    if(parameters[0] != 0 ||
       parameters[1] != 0 ||
       parameters[2] != angle ||
       parameters[3] != trans[0] ||
       parameters[4] != trans[1] ||
       parameters[5] != trans[2] ||
       parameters[6] != scale)
      {
      std::cerr << "ERROR: Parameters are not translation (" << trans << ") followed by scale (" << scale << ")" << std::endl;
      return EXIT_FAILURE;
      }

    std::cout << "Jacobian at center pt" << std::endl;
    TransformType::JacobianType jacobian =
      similaritytransform->GetJacobian(center);
    std::cout << jacobian << std::endl;

      {
      double tjacobian [TransformType::SpaceDimension][TransformType::ParametersDimension] = 
        { {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0},
          {0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0},
          {0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0} };

      for(unsigned int i = 0; 
          i < TransformType::SpaceDimension;
          ++i)
        {
        for(unsigned int j = 0; 
            j < TransformType::ParametersDimension;
            ++j)
          {
          if(!almost(tjacobian[i][j], jacobian[i][j], 1.0e-10))
            {
            std::cerr << "Jacobian does not match theoretical jacobian" << std::endl;

            std::cerr << "Jacobian[" << i << "][" << j <<
              "]: " <<  jacobian[i][j] << std::endl;

            std::cerr << "TJacobian: " << 
              tjacobian[i][j] << std::endl;

            return EXIT_FAILURE;
            }
          }
        }
      }

    TransformType::InputPointType centerp1;
    centerp1[0] = center[0];
    centerp1[1] = center[1] + 1.0;
    centerp1[2] = center[2];

    std::cout << "Jacobian at [0,1,0]" << std::endl;
    jacobian = similaritytransform->GetJacobian(centerp1);
    std::cout << jacobian << std::endl;
    
      {
      double tjacobian [TransformType::SpaceDimension][TransformType::ParametersDimension] = 
        { {0.0, 0.0, -scale/sqrt(2.0), 1.0, 0.0, 0.0, -1.0/sqrt(2.0)},
          {0.0, 0.0, -scale/sqrt(2.0), 0.0, 1.0, 0.0, 1.0/sqrt(2.0)},
          {scale, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0} };

      for(unsigned int i = 0; 
          i < TransformType::SpaceDimension;
          ++i)
        {
        for(unsigned int j = 0; 
            j < TransformType::ParametersDimension;
            ++j)
          {
          if(!almost(tjacobian[i][j], jacobian[i][j], 1.0e-10))
            {
            std::cerr << "Jacobian does not match theoretical jacobian" << std::endl;

            std::cerr << "Jacobian[" << i << "][" << j <<
              "]: " <<  jacobian[i][j] << std::endl;

            std::cerr << "TJacobian: " << 
              tjacobian[i][j] << std::endl;

            return EXIT_FAILURE;
            }
          }
        }
      }
    }

    {
    TransformType::Pointer transform = TransformType::New();
    TransformType::ParametersType p(7);
    p.Fill(0.0);
    p[0] = M_PI/8;
    p[1] = M_PI/10;
    p[2] = -M_PI/6;
    p[3] = 4.0;
    p[4] = 5.0;
    p[5] = 6.0;
    p[6] = 0.8;

    transform->SetParameters(p);
    transform->SetParameters(p);
    transform->SetParameters(p);

    double correctmat[3][3] = 
      {{.7062135564636849, .3695518130045147, .06851182505224822},
       {-.2984926652832313, .6400825161530124, -.3757612290901516},
       {-.2283955810409355, .3061467458920719, .7029293197332636}};

    TransformType::ParametersType p2 = transform->GetParameters();
    std::cout << "Checking get parameters same as set parameters" << std::endl;
    for(unsigned int i = 0; i < 7; ++i)
      {
      if(!almost(p[i],p2[i], 1.0e-10))
        {
        std::cerr << "Stored parameters not the same as those set" << std::endl;
        return EXIT_FAILURE;
        }
      }

    TransformType::MatrixType matrix = transform->GetMatrix();
    std::cout << "Checkig matrix from parameters" << std::endl;
    std::cout << matrix << std::endl;
    for(unsigned int i = 0; i < 3; ++i)
      {
      for(unsigned int j = 0; j < 3; ++j)
        {
        if(!almost(matrix[i][j], correctmat[i][j], 1.e-10))
          {
            std::cerr << "Transform matrix does not match theoretical matrix" << std::endl;

            std::cerr << "Matrix[" << i << "][" << j <<
              "]: " <<  matrix[i][j] << std::endl;

            std::cerr << "TMatrix: " << 
              correctmat[i][j] << std::endl;

            return EXIT_FAILURE;
          
          }
        }
      }
    
    }

  return EXIT_SUCCESS;
}
