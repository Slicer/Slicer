// WFFactory Class
// To be used for encapsulating different interfaces and probably later different widgets

#ifndef WFFACTORY_H_
#define WFFACTORY_H_

//#include <map>
//#include <memory>
//#include <exception>
//#include <vector>
#include <iostream>

//#include <loki/Factory.h>

//using namespace Loki;
namespace WFEngine
{
 
 namespace nmWFFactory
 {

  class WFFactory
  {
  public:
   static WFFactory *GetInstance();
   static void SetInstance(WFFactory *wfFactory);
 
   void registerTypes();
  
//   template<class AbstractProduct, class Product>
//   bool registerType(int id, AbstractProduct absProd, Product prod)
//   {
//    typedef SingletonHolder 
//    <
//        Factory< AbstractProduct, int >,
//        CreateUsingNew,
//        Loki::LongevityLifetime::DieAsSmallObjectChild
//       >
//    aFactory;
     
//   WFClientInterface wfCI;
     
//    factInst = aFactory::Instance();//.Register( 1, Product );
//    factInst.Register(1, Product::New);
    
//    factInst.CreateObject(1);
//   }
//   typedef std::map<int, Loki::Factory> factory_map;
 
  protected:
   WFFactory();
   ~WFFactory();
 
  private:
   static WFFactory *m_wfFactory;
   
//   template<class AbstractProduct, class Product> Loki::Factory<AbstractProduct, int, Loki::NullType, Loki::DefaultFactoryError> factInst;
 
  };
 }
}//namespace

#endif /*WFFACTORY_H_*/
