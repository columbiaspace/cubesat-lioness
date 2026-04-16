// ======================================================================
// \title  StarTracker.hpp
// \author kevinyang
// \brief  hpp file for StarTracker component implementation class
// ======================================================================

#ifndef Components_StarTracker_HPP
#define Components_StarTracker_HPP

#include "LionessSw/Components/StarTracker/StarTrackerComponentAc.hpp"

namespace Components {

class StarTracker final : public StarTrackerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct StarTracker object
    StarTracker(const char* const compName  //!< The component name
    );

    //! Destroy StarTracker object
    ~StarTracker();
};

}  // namespace Components

#endif
