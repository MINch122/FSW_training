###########################################################
#
# PAY_SLT mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the SAMPLE_APP configuration
set(PAY_SLT_MISSION_CONFIG_FILE_LIST
  pay_slt_fcncodes.h
  pay_slt_interface_cfg.h
  pay_slt_mission_cfg.h
  pay_slt_perfids.h
  pay_slt_msg.h
  pay_slt_msgdefs.h
  pay_slt_msgstruct.h
  pay_slt_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(PAY_SLT_CFGFILE_SRC_pay_slt_interface_cfg "SLT_IFB_eds_designparameters.h")
  set(PAY_SLT_CFGFILE_SRC_pay_slt_msgdefs       "SLT_IFB_eds_typedefs.h")
  set(PAY_SLT_CFGFILE_SRC_pay_slt_msgstruct     "SLT_IFB_eds_typedefs.h")
  set(PAY_SLT_CFGFILE_SRC_pay_slt_fcncodes      "SLT_IFB_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(PAY_SLT_CFGFILE ${PAY_SLT_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${PAY_SLT_CFGFILE}" NAME_WE)
  if (DEFINED PAY_SLT_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${PAY_SLT_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${PAY_SLT_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${PAY_SLT_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
