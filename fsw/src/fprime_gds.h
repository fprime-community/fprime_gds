/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *
 * Main header file for the Sample application
 */

#ifndef FPRIME_GDS_H
#define FPRIME_GDS_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "fprime_gds_mission_cfg.h"
#include "fprime_gds_platform_cfg.h"

#include "fprime_gds_perfids.h"
#include "fprime_gds_msgids.h"
#include "fprime_gds_msg.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct
{
    /*
    ** Command interface counters...
    */
    uint8 CmdCounter;
    uint8 ErrCounter;

    /*
    ** Housekeeping telemetry packet...
    */
    FPRIME_GDS_HkTlm_t HkTlm;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;

    CFE_TBL_Handle_t TblHandles[FPRIME_GDS_PLATFORM_NUMBER_OF_TABLES];
} FPRIME_GDS_Data_t;

/*
** Global data structure
*/
extern FPRIME_GDS_Data_t FPRIME_GDS_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (FPRIME_GDS_Main), these
**       functions are not called from any other source module.
*/
void         FPRIME_GDS_Main(void);
CFE_Status_t FPRIME_GDS_Init(void);

#endif /* FPRIME_GDS_H */
