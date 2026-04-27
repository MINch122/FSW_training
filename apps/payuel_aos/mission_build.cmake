###########################################################
#
# PAYUEL_AOS mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PAYUEL_AOS configuration
set(PAYUEL_AOS_MISSION_CONFIG_FILE_LIST
  payuel_aos_fcncodes.h
  payuel_aos_interface_cfg.h
  payuel_aos_mission_cfg.h
  payuel_aos_perfids.h
  payuel_aos_msg.h
  payuel_aos_msgdefs.h
  payuel_aos_msgstruct.h
  payuel_aos_tbl.h
  payuel_aos_tbldefs.h
  payuel_aos_tblstruct.h
  payuel_aos_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(PAYUEL_AOS_CFGFILE_SRC_payuel_aos_interface_cfg "payuel_aos_eds_designparameters.h")
  set(PAYUEL_AOS_CFGFILE_SRC_payuel_aos_tbldefs       "payuel_aos_eds_typedefs.h")
  set(PAYUEL_AOS_CFGFILE_SRC_payuel_aos_tblstruct     "payuel_aos_eds_typedefs.h")
  set(PAYUEL_AOS_CFGFILE_SRC_payuel_aos_msgdefs       "payuel_aos_eds_typedefs.h")
  set(PAYUEL_AOS_CFGFILE_SRC_payuel_aos_msgstruct     "payuel_aos_eds_typedefs.h")
  set(PAYUEL_AOS_CFGFILE_SRC_payuel_aos_fcncodes      "payuel_aos_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(PAYUEL_AOS_CFGFILE ${PAYUEL_AOS_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAYUEL_AOS_CFGFILE}" NAME_WE)
  if (DEFINED PAYUEL_AOS_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAYUEL_AOS_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAYUEL_AOS_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAYUEL_AOS_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
