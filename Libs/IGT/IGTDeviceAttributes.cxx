#include "igtauroratracker.h"

// Constructors/Destructors
//  

IGTAuroraTracker::IGTAuroraTracker ( ) {
}

IGTAuroraTracker::~IGTAuroraTracker ( ) { }

//  
// Methods
//  


// Accessor methods
//  


// Other methods
//  



#include "igtdeviceattributes.h"

// Constructors/Destructors
//  

IGTDeviceAttributes::IGTDeviceAttributes ( ) {
initAttributes();
}

IGTDeviceAttributes::~IGTDeviceAttributes ( ) { }

//  
// Methods
//  


// Accessor methods
//  


// Public static attribute accessor methods
//  


// Public attribute accessor methods
//  


// Protected static attribute accessor methods
//  


// Protected attribute accessor methods
//  


// Private static attribute accessor methods
//  


// Private attribute accessor methods
//  


/**
 * Set the value of m_status
 * @param new_var the new value of m_status
 */
void IGTDeviceAttributes::setStatus ( int new_var ) {
  m_status = new_var;
}

/**
 * Get the value of m_status
 * @return the value of m_status
 */
int IGTDeviceAttributes::getStatus ( ) {
  return m_status;
}

// Other methods
//  

void IGTDeviceAttributes::initAttributes ( ) {
}

