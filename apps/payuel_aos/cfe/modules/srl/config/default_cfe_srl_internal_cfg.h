/**
 * @file
 *   CFE Serial Services (CFE_SRL) Application Private Config Definitions
 *
 * This provides default values for configurable items that are internal
 * to this module and do NOT affect the interface(s) of this module.  Changes
 * to items in this file only affect the local module and will be transparent
 * to external entities that are using the public interface(s).
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef CFE_SRL_INTERNAL_CFG_H
#define CFE_SRL_INTERNAL_CFG_H

/**
 * Define SRL Task Priority
 */
#define CFE_PLATFORM_SRL_START_TASK_PRIORITY    71

/**
 * Define SRL Task Stack Size
 */
#define CFE_PLATFORM_SRL_START_TASK_STACK_SIZE  CFE_PLATFORM_ES_DEFAULT_STACK_SIZE

/**
 * PSP iodriver driver module name
 * These module will be varied by underlying OS
 */
#if defined(OS_RTEMS_4_DEPRECATED) || defined(OS_RTEMS_5) || defined(OS_RTEMS_6)
#define CFE_SRL_SERIAL_DRIVER       "rtems_serial"
#else
#define CFE_SRL_SERIAL_DRIVER       "linux_serial"
#endif

#define CFE_SRL_DISCRETE_DRIVER     "linux_gpio"

#endif /* CFE_SRL_INTERNAL_CFG_H */