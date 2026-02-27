###########################################################
#
# FPRIME_GDS platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the FPRIME_GDS configuration
set(FPRIME_GDS_PLATFORM_CONFIG_FILE_LIST
  fprime_gds_internal_cfg_values.h
  fprime_gds_platform_cfg.h
  fprime_gds_perfids.h
  fprime_gds_msgids.h
  fprime_gds_msgid_values.h
)

generate_configfile_set(${FPRIME_GDS_PLATFORM_CONFIG_FILE_LIST})

