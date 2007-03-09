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



#include "igttrackerstream.h"

// Constructors/Destructors
//  

IGTTrackerStream::IGTTrackerStream ( ) {
initAttributes();
}

IGTTrackerStream::~IGTTrackerStream ( ) { }

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
 * Set the value of m_TrackerAttributesBuffer
 * @param new_var the new value of m_TrackerAttributesBuffer
 */
void IGTTrackerStream::setTrackerAttributesBuffer ( IGTTrackerAttributes new_var ) {
  m_TrackerAttributesBuffer = new_var;
}

/**
 * Get the value of m_TrackerAttributesBuffer
 * @return the value of m_TrackerAttributesBuffer
 */
IGTTrackerAttributes IGTTrackerStream::getTrackerAttributesBuffer ( ) {
  return m_TrackerAttributesBuffer;
}

// Other methods
//  


/**
 */
void IGTTrackerStream::perform_patient_to_image_matrix ( ) {

}


/**
 */
void IGTTrackerStream::register_tracker_to_data_buffer ( ) {

}

void IGTTrackerStream::initAttributes ( ) {
}

