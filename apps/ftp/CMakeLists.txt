project(CFE_FTP C)

set(APP_SRC_FILES
  fsw/src/ftp_task.c
  fsw/src/ftp_cmds.c
  fsw/src/ftp_utils.c
  fsw/src/ftp_dispatch.c
)

# Create the app module
add_cfe_app(ftp ${APP_SRC_FILES})

target_include_directories(ftp PUBLIC fsw/inc)
# Include the public API from sample_lib to demonstrate how
# to call library-provided functions

# Add table
# add_cfe_tables(sample_app fsw/tables/sample_app_tbl.c)
