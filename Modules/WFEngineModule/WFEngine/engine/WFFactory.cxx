// WFFactory Class
// To be used for encapsulating different interfaces and probably later different widgets

//#include <iostream>

//#include "WFSocketHandler.h"
//#include <WFClientInterface.h>
//#include "WFOptionsInterface.h"
#include "WFFactory.h"

using namespace WFEngine::nmWFFactory;
//using namespace WFEngine::nmWFClientInterface;
//using namespace WFEngine::nmWFSocketHandler;


WFFactory *WFFactory::m_wfFactory = NULL;

WFFactory::WFFactory()
{
 
}

WFFactory::~WFFactory()
{
 
}

WFFactory *WFFactory::GetInstance()
{
 if (WFFactory::m_wfFactory == NULL) m_wfFactory = new WFFactory();
 return m_wfFactory;
}

void WFFactory::SetInstance(WFFactory *wfFactory)
{
 m_wfFactory = wfFactory;
}

//void WFFactory::registerTypes()
//{
////Factory<AbstractProduct, int>, CreateUsingNew,
////Loki::LongevityLifetime::DieAsSmallObjectChild aFactory;
//
// typedef SingletonHolder 
// <
//     Factory< WFSocketHandler, int >,
//     CreateUsingNew,
//     Loki::LongevityLifetime::DieAsSmallObjectChild
//    >
// aFactory;
//  
////WFClientInterface wfCI;
//  
// Loki::Factory<WFSocketHandler, int, Loki::NullType, Loki::DefaultFactoryError> factInst = aFactory::Instance();//.Register( 1, Product );
// factInst.Register(1, WFClientInterface::New);
// 
// factInst.CreateObject(1);
////cout<<ok1<<endl;
////aFactory::
//}

//template <class AbstractProduct, class Product>
//void WFFactory<AbstractProduct, Product>::registerType(AbstractProduct absProd, Product prod)
//{
// 
//}
