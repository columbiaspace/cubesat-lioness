// ======================================================================
// \title  StarTracker.hpp
// \author kevinyang
// \brief  hpp file for StarTracker component implementation class
// ======================================================================

#ifndef LionessSw_StarTracker_HPP
#define LionessSw_StarTracker_HPP

#include "LionessSw/Components/StarTracker/StarTrackerComponentAc.hpp"

namespace LionessSw {

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

}  // namespace LionessSw

#endif
