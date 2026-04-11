#ifndef COMCCSDSNOROUTERSUBTOPOLOGY_DEFS_HPP
#define COMCCSDSNOROUTERSUBTOPOLOGY_DEFS_HPP

#include <Fw/Types/MallocAllocator.hpp>
#include <Svc/BufferManager/BufferManager.hpp>
#include <Svc/FrameAccumulator/FrameDetector/CcsdsTcFrameDetector.hpp>
#include "ComCcsdsNoRouterConfig/ComCcsdsNoRouterSubtopologyConfig.hpp"
#include "GdsBridge/Subtopologies/ComCcsdsNoRouter/ComCcsdsNoRouterConfig/FppConstantsAc.hpp"
#include "GdsBridge/Subtopologies/ComCcsdsNoRouter/Ports_ComBufferQueueEnumAc.hpp"
#include "GdsBridge/Subtopologies/ComCcsdsNoRouter/Ports_ComPacketQueueEnumAc.hpp"

namespace ComCcsdsNoRouter {
struct SubtopologyState {
    // Empty - no external state needed for ComCcsdsNoRouter subtopology
};

struct TopologyState {
    SubtopologyState comCcsdsNoRouter;
};
}  // namespace ComCcsdsNoRouter

#endif
