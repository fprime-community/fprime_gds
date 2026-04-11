module ComCcsdsNoRouterConfig {
    #Base ID for the ComCcsdsNoRouter Subtopology, all components are offsets from this base ID
    constant BASE_ID = 0x02000000
    
    module QueueSizes {
        constant aggregator  = 10
    }
    
    module StackSizes {
        constant aggregator = 64 * 1024
    }

    module Priorities {
        constant aggregator = 30
    }

    # Buffer management constants
    module BuffMgr {
        constant frameAccumulatorSize  = 2048     
        constant commsBuffSize         = 2048      
        constant commsFileBuffSize     = 3000      
        constant commsBuffCount        = 20        
        constant commsFileBuffCount    = 30       
        constant commsBuffMgrId        = 200      
    }
}
