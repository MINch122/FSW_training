###########################################################
#
# MEOW mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the MEOW configuration
set(MEOW_MISSION_CONFIG_FILE_LIST
  meow_fcncode_values.h
  meow_interface_cfg_values.h
  meow_mission_cfg.h
  meow_perfids.h
  meow_msg.h
  meow_msgdefs.h
  meow_msgstruct.h
  meow_topicid_values.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(MEOW_CFGFILE_SRC_meow_interface_cfg_values "meow_eds_designparameters.h")
  set(MEOW_CFGFILE_SRC_meow_msgdefs              "meow_eds_typedefs.h")
  set(MEOW_CFGFILE_SRC_meow_msgstruct            "meow_eds_typedefs.h")
  set(MEOW_CFGFILE_SRC_meow_fcncode_values       "meow_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(MEOW_CFGFILE ${MEOW_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${MEOW_CFGFILE}" NAME_WE)
  if (DEFINED MEOW_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${MEOW_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${MEOW_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${MEOW_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
