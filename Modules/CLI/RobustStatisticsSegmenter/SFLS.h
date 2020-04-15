#ifndef SFLS_h_
#define SFLS_h_

// std
#include <list>

// itk
#include "vnl/vnl_vector_fixed.h"

class CSFLS
{
public:
  using Self = CSFLS;

  using NodeType = vnl_vector_fixed<int, 3>;
  using CSFLSLayer = std::list<NodeType>;

  // typedef boost::shared_ptr< Self > Pointer;

  // ctor
  CSFLS()
  {
    //       m_lz.reset(new CSFLSLayer);
    //       m_ln1.reset(new CSFLSLayer);
    //       m_ln2.reset(new CSFLSLayer);
    //       m_lp1.reset(new CSFLSLayer);
    //       m_lp2.reset(new CSFLSLayer);
  }

  //   static Pointer New()
  //   {
  //     return Pointer(new CSFLS);
  //   }

  CSFLSLayer m_lz;
  CSFLSLayer m_ln1;
  CSFLSLayer m_ln2;
  CSFLSLayer m_lp1;
  CSFLSLayer m_lp2;
};

#endif
