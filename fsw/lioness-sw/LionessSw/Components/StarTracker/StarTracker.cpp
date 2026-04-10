// ======================================================================
// \title  StarTracker.cpp
// \author kevinyang
// \brief  cpp file for StarTracker component implementation class
// ======================================================================
#include <vector>
#include <unordered_set>
#include <cmath>

#include "LionessSw/Components/StarTracker/StarTracker.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

StarTracker ::StarTracker(const char* const compName) : StarTrackerComponentBase(compName) {}

StarTracker ::~StarTracker() {}

}  // namespace Components
