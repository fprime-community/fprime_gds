
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/GdsComCcsdsConfig/")

register_fprime_module(
    EXCLUDE_FROM_ALL
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/ComCcsds.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/SubtopologyTopologyDefs.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/PingEntries.hpp"
    DEPENDS
        GdsBridge_Subtopologies_ComCcsds_GdsComCcsdsConfig
    INTERFACE
)
