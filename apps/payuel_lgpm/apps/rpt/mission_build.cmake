###########################################################
#
# RPT mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the RPT configuration
set(RPT_MISSION_CONFIG_FILE_LIST
  rpt_fcncodes.h
  rpt_interface_cfg.h
  rpt_mission_cfg.h
  rpt_perfids.h
  rpt_msg.h
  rpt_msgdefs.h
  rpt_msgstruct.h
  rpt_tbl.h
  rpt_tbldefs.h
  rpt_tblstruct.h
  rpt_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(RPT_CFGFILE ${RPT_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${RPT_CFGFILE}" NAME_WE)
  if (DEFINED RPT_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${RPT_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${RPT_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${RPT_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
