#include <iostream>
#include <cstdlib>
#ifdef _WIN32
#ifndef M_PI
#define M_PI 3.1415926535898
#endif
#endif
#include "itkDecomposedAffine3DTransform.h"

namespace
{
  bool almost(double x, double y, double eps)
  {
    return fabs(x - y) <= eps;
  }
}

int DecomposedAffine3DTransformTest(int, char* [])
{
  typedef itk::DecomposedAffine3DTransform<double> TransformType;

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
       parameters[6] != 1 ||
       parameters[7] != 1 ||
       parameters[8] != 1 ||
       parameters[9] != 0 ||
       parameters[10] != 0 ||
       parameters[11] != 0)
      {
      std::cerr << "ERROR: Identity trasform does not have paramteres [0,0,0,0,0,0,1,1,1,0,0,0]" << std::endl;
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
    purerotation->SetRotation(angle, -angle, angle/2);
  
    std::cout << purerotation << std::endl;

    typedef TransformType::ParametersType ParametersType;
    const ParametersType parameters = purerotation->GetParameters();
    std::cout << "Parameters" << std::endl;
    std::cout << parameters << std::endl;

    if(parameters[0] != angle ||
       parameters[1] != -angle ||
       parameters[2] != angle/2 ||
       parameters[3] != 0 ||
       parameters[4] != 0 ||
       parameters[5] != 0 ||
       parameters[6] != 1 ||
       parameters[7] != 1 ||
       parameters[8] != 1 ||
       parameters[9] != 0 ||
       parameters[10] != 0 ||
       parameters[11] != 0)
      {
      std::cerr << "ERROR: Pure rotation transform does not have paramteres [pi/4,-pi/4,pi/8,0,0,0,1,1,1,0,0,0]" << std::endl;
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
       parameters[6] != 1 ||
       parameters[7] != 1 || 
       parameters[8] != 1 ||
       parameters[9] != 0 ||
       parameters[10] != 0 ||
       parameters[11] != 0)

      {
      std::cerr << "ERROR: Pure rotation transform does not have paramteres [pi/2,0,0,-20,10,,5,1]" << std::endl;
      return EXIT_FAILURE;
      }

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
    similaritytransform->SetRotation(2*angle, -3*angle, angle);
  
    TransformType::TranslationType trans;
    trans[0] = -20;
    trans[1] = 10;
    trans[2] = .5;

    std::cout << "Translation by " << trans << std::endl;
    similaritytransform->SetTranslation(trans);

    TransformType::ScaleVectorType scales;
    scales[0] = 1.5;
    scales[1] = -.8;
    scales[2] = 1.2;

    std::cout << "Scale by " << scales << std::endl;
    similaritytransform->SetScale(scales);

    std::cout << similaritytransform << std::endl;

    typedef TransformType::ParametersType ParametersType;
    const ParametersType parameters = similaritytransform->GetParameters();
    std::cout << "Parameters" << std::endl;
    std::cout << parameters << std::endl;

    if(parameters[0] != 2*angle ||
       parameters[1] != -3*angle ||
       parameters[2] != angle ||
       parameters[3] != trans[0] ||
       parameters[4] != trans[1] ||
       parameters[5] != trans[2] ||
       parameters[6] != scales[0] ||
       parameters[7] != scales[1] ||
       parameters[8] != scales[2] ||
       parameters[9] != 0 ||
       parameters[10] != 0 ||
       parameters[11] != 0)
      {
      std::cerr << "ERROR: Parameters are wrong for scale free transform" << std::endl;
      return EXIT_FAILURE;
      }

    std::cout << "Jacobian at center pt" << std::endl;
    TransformType::JacobianType jacobian =
      similaritytransform->GetJacobian(center);
    std::cout << jacobian << std::endl;

      {
      double tjacobian [TransformType::SpaceDimension][TransformType::ParametersDimension] = 
        { {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
          {0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
          {0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};

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

//            return EXIT_FAILURE;
            }
          }
        }
      }
    }

    {
    TransformType::Pointer transform = TransformType::New();

    TransformType::InputPointType center;
    center[0] = 50;
    center[1] = 60;
    center[2] = 70;
    transform->SetCenter(center);

    TransformType::ParametersType p(TransformType::ParametersDimension);
    p.Fill(0.0);
    p[0] = M_PI/8;
    p[1] = M_PI/10;
    p[2] = -M_PI/6;
    p[3] = 4.0;
    p[4] = 5.0;
    p[5] = 6.0;
    p[6] = 0.8;
    p[7] = 0.9;
    p[8] = 1.1;
    p[9] = 0.05;
    p[10] = -0.1;
    p[11] = 0.075;

    transform->SetParameters(p);

    // For documentation of the seemingly magic numbers here
    // See doc/decomposedaffine.wxm which can be viewed
    // in wxMaxima
    double correctmat[3][3] = 
      {{.7062135564636849,.4510564674532633,.05476333802272874},
       {-.2984926652832313,.7051681974079773,-0.432815461170225},
       {-.2283955810409355,0.332995310076534,1.015198504421974}};

    TransformType::ParametersType p2 = transform->GetParameters();
    std::cout << "Checking get parameters same as set parameters" << std::endl;
    for(unsigned int i = 0; i < TransformType::ParametersDimension; ++i)
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

    // Check the jacobian about a point not on the center
    TransformType::InputPointType point;
    point[0] = center[0] + 5;
    point[1] = center[1] - 3;
    point[2] = center[2] + 20;

    std::cout << "Jacobian at [5, -3, 20]" << std::endl;
    TransformType::JacobianType jacobian =
      transform->GetJacobian(point);
    std::cout << jacobian << std::endl;
    
    // For documentation of the seemingly magic numbers here
    // See doc/decomposedaffine.wxm which can be viewed
    // in wxMaxima
    double trotblock[3][3] = 
      {{-9.081503126502604,19.22561410135243,12.26427714204459},
       {-15.72962482419812,-7.137628792381929,3.27316514041321},
       {-8.98459299385682,-8.284227039865527,0.0}};
    
    // For documentation of the seemingly magic numbers here
    // See doc/decomposedaffine.wxm which can be viewed
    // in wxMaxima
    double tscaleblock[3][3] = 
      {{2.515885794901877,-0.692909649383465,1.712795626306205},
       {-1.063380120071511,-1.200154717786898,-9.39403072725379},
       {-.8136592574583325,-.5740251485476346,17.57323299333159}};

    // For documentation of the seemingly magic numbers here
    // See doc/decomposedaffine.wxm which can be viewed
    // in wxMaxima
    double tskewblock[3][3] = 
      {{-2.118640669391055,14.1242711292737,8.31491579260158},
       {.8954779958496939,-5.969853305664625,14.40185661344278},
       {.6851867431228065,-4.567911620818709,6.888301782571616}};
      
    for(unsigned int i = 0;
        i < 3;
        ++i)
      {
      for(unsigned int j = 0;
          j < 3;
          ++j)
        {
        if(!almost(jacobian[i][j], trotblock[i][j], 1.0e-10) ||
           !almost(jacobian[i][j+3], static_cast<int>(i == j), 1.0e-10) ||
           !almost(jacobian[i][j+6], tscaleblock[i][j], 1.0e-10) ||
           !almost(jacobian[i][j+9], tskewblock[i][j], 1.0e-10))
          {
          std::cerr << "Error in jacobian at [" << i << 
            "," << j << "]" << std::endl;
          return EXIT_FAILURE;
          }
        }
      }
      
    }

  return EXIT_SUCCESS;
}
