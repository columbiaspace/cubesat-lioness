// ======================================================================
// \title  startrackerHandler.hpp
// \author hufamily
// \brief  hpp file for startrackerHandler component implementation class
// ======================================================================

#ifndef LionessSw_startrackerHandler_HPP
#define LionessSw_startrackerHandler_HPP

#include "LionessSw/Components/startrackerHandler/startrackerHandlerComponentAc.hpp"

namespace LionessSw {

class startrackerHandler final : public startrackerHandlerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct startrackerHandler object
    startrackerHandler(const char* const compName  //!< The component name
    );

    //! Destroy startrackerHandler object
    ~startrackerHandler();
};

}  // namespace LionessSw

#endif
