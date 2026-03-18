#include "GdsComCcsdsSubtopologyConfig.hpp"

namespace FprimeGds {
namespace Allocation {
// This instance can be changed to use a different allocator in the ComCcsds Subtopology
Fw::MallocAllocator mallocatorInstance;
Fw::MemAllocator& memAllocator = mallocatorInstance;
}  // namespace Allocation
}  // namespace ComCcsds
