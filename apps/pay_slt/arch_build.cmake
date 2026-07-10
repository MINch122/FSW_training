###########################################################
#
# PAY_SLT platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PAY_SLT configuration
set(PAY_SLT_PLATFORM_CONFIG_FILE_LIST
  pay_slt_internal_cfg.h
  pay_slt_platform_cfg.h
  pay_slt_perfids.h
  pay_slt_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(PAY_SLT_CFGFILE ${PAY_SLT_PLATFORM_CONFIG_FILE_LIST})
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
