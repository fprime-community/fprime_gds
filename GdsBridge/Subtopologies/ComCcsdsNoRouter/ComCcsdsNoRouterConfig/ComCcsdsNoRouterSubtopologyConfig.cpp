#include "ComCcsdsNoRouterSubtopologyConfig.hpp"

namespace ComCcsdsNoRouter {
namespace Allocation {
// This instance can be changed to use a different allocator in the ComCcsdsNoRouter Subtopology
Fw::MallocAllocator mallocatorInstance;
Fw::MemAllocator& memAllocator = mallocatorInstance;
}  // namespace Allocation
}  // namespace ComCcsdsNoRouter
