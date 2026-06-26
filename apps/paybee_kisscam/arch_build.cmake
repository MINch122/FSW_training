###########################################################
#
# paybee_kisscam platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the paybee_kisscam configuration
set(paybee_kisscam_PLATFORM_CONFIG_FILE_LIST
  paybee_kisscam_internal_cfg.h
  paybee_kisscam_platform_cfg.h
  paybee_kisscam_perfids.h
  paybee_kisscam_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(paybee_kisscam_CFGFILE ${paybee_kisscam_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${paybee_kisscam_CFGFILE}" NAME_WE)
  if (DEFINED paybee_kisscam_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${paybee_kisscam_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${paybee_kisscam_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${paybee_kisscam_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
