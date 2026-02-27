###########################################################
#
# FPRIME_GDS mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the FPRIME_GDS configuration
set(FPRIME_GDS_MISSION_CONFIG_FILE_LIST
  fprime_gds_fcncode_values.h
  fprime_gds_interface_cfg_values.h
  fprime_gds_mission_cfg.h
  fprime_gds_perfids.h
  fprime_gds_msg.h
  fprime_gds_msgdefs.h
  fprime_gds_msgstruct.h
  fprime_gds_tbl.h
  fprime_gds_tbldefs.h
  fprime_gds_tblstruct.h
  fprime_gds_topicid_values.h
)

generate_configfile_set(${FPRIME_GDS_MISSION_CONFIG_FILE_LIST})

