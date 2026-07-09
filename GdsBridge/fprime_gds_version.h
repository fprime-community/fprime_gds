/************************************************************************
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
 *  The FPrime App header file containing version information
 */

#ifndef FPRIME_GDS_VERSION_H
#define FPRIME_GDS_VERSION_H

/* Development Build Macro Definitions */

#define FPRIME_GDS_BUILD_NUMBER    0
#define FPRIME_GDS_BUILD_BASELINE  "v0.1.0" /*!< Development Build: git tag that is the base for the current development */
#define FPRIME_GDS_BUILD_DEV_CYCLE "v0.1.0" /**< @brief Development: Release name for current development cycle */
#define FPRIME_GDS_BUILD_CODENAME  "Hare" /**< @brief: Development: Code name for the current build */

/*
 * Version Macros, see \ref cfsversions for definitions.
 */
#define FPRIME_GDS_MAJOR_VERSION 0  /*!< @brief Major version number. */
#define FPRIME_GDS_MINOR_VERSION 1  /*!< @brief Minor version number. */
#define FPRIME_GDS_REVISION      0  /*!< @brief Revision version number. Value of 0 indicates a development version.*/

/**
 * @brief Last official release.
 */
#define FPRIME_GDS_LAST_OFFICIAL "v0.0.0"

/*!
 * @brief Mission revision.
 *
 * Reserved for mission use to denote patches/customizations as needed.
 * Values 1-254 are reserved for mission use to denote patches/customizations as needed. NOTE: Reserving 0 and 0xFF for
 * cFS open-source development use (pending resolution of nasa/cFS#440)
 */
#define FPRIME_GDS_MISSION_REV 0x0

#define FPRIME_GDS_STR_HELPER(x) #x /*!< @brief Helper function to concatenate strings from integer macros */
#define FPRIME_GDS_STR(x) \
    FPRIME_GDS_STR_HELPER(x) /*!< @brief Helper function to concatenate strings from integer macros */

/*! @brief Development Build Version Number.
 * @details Baseline git tag + Number of commits since baseline. @n
 * See @ref cfsversions for format differences between development and release versions.
 */
#define FPRIME_GDS_VERSION FPRIME_GDS_BUILD_BASELINE "+dev" FPRIME_GDS_STR(FPRIME_GDS_BUILD_NUMBER)

/**
 * @brief Max Version String length.
 * 
 * Maximum length that an OSAL version string can be.
 * 
 */
#define FPRIME_GDS_CFG_MAX_VERSION_STR_LEN 256

/*
 * Older cFE versions (e.g. draco) do not provide CFE_Config_GetVersionString;
 * provide an equivalent local implementation when building against them.
 */
#include "cfe_version.h"
#if CFE_MAJOR_VERSION < 7
#include <stdio.h>
static inline void CFE_Config_GetVersionString(char *Buf, size_t Size, const char *Component,
                                               const char *SrcVersion, const char *CodeName,
                                               const char *LastOffcRel)
{
    (void)LastOffcRel;
    snprintf(Buf, Size, "%s %s (%s)", Component, SrcVersion, CodeName);
}
#endif

#endif /* FPRIME_GDS_VERSION_H */
