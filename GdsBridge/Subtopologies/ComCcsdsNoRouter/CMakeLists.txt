
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/ComCcsdsNoRouterConfig/")

register_fprime_module(
    EXCLUDE_FROM_ALL
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/ComCcsdsNoRouter.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/SubtopologyTopologyDefs.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/PingEntries.hpp"
    DEPENDS
        GdsBridge_Subtopologies_ComCcsdsNoRouter_ComCcsdsNoRouterConfig
    INTERFACE
)
