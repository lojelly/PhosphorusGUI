#pragma once

#include <stdio.h>

/**
  Represents the type of a log statement.
*/
typedef enum
{
	/**
	  Used for normal log messages.
	*/
	VL_INFO,
	/**
	  Used to signal that an operation
	  was successful.
	*/
	VL_SUCCESS,
	/**
	  Used to debug a value or expression.
	*/
	VL_DEBUG,
	/**
	  Used to send a warning message.
	*/
	VL_WARNING,
	/**
	  Used to send an error message.
	*/
	VL_ERROR
} vl_type;

/**
  Represents a color using values of 0 to 255.
*/
typedef struct vl_color
{
	/**
	  The red value.
	*/
	unsigned char r;
	/**
	  The green value.
	*/
	unsigned char g;
	/**
	  The blue value.
	*/
	unsigned char b;
} vl_color;

/**
  White.
*/
#define VL_WHITE (vl_color) { 255, 255, 255 }
/**
  Light blue.
*/
#define VL_LIGHT_BLUE (vl_color) { 0, 125, 215 }
/**
  Blue.
*/
#define VL_BLUE (vl_color) { 0, 255, 255 }
/**
  Light green.
*/
#define VL_LIGHT_GREEN (vl_color) { 0, 175, 0 }
/**
  Green.
*/
#define VL_GREEN (vl_color) { 0, 255, 0 }
/**
  Light orange.
*/
#define VL_LIGHT_ORANGE (vl_color) { 200, 125, 0 }
/**
  Orange.
*/
#define VL_ORANGE (vl_color) { 255, 150, 0 }
/**
  Light yellow.
*/
#define VL_LIGHT_YELLOW (vl_color) { 200, 200, 0 }
/**
  Yellow.
*/
#define VL_YELLOW (vl_color) { 255, 255, 0 }
/**
  Light red.
*/
#define VL_LIGHT_RED (vl_color) { 200, 0, 0 }
/**
  Red.
*/
#define VL_RED (vl_color) { 255, 0, 0 }

/**
  Represents the color scheme of VibrantLogs.

  @see VL_DEFAULT_COLORS
*/
typedef struct vl_color_scheme
{
	/**
	  The color of the time (if printed).

	  @see vl_config.
	  @see vl_use_config(vl_config).
	*/
	vl_color time_color;
	/**
	  The color of the date (if printed).

	  @see vl_config.
	  @see vl_use_config(vl_config).
	*/
	vl_color date_color;
	/**
	  The color of the prefix of info messages.
	*/
	vl_color info_prefix_color;
	/**
	  The main color of info messages.
	*/
	vl_color info_color;
	/**
	  The color of the prefix of successful operation messages.
	*/
	vl_color success_prefix_color;
	/**
	  The main color of successful operation messages.
	*/
	vl_color success_color;
	/**
	  The color of the prefix of debug messages.
	*/
	vl_color debug_prefix_color;
	/**
	  The main color of debug messages.
	*/
	vl_color debug_color;
	/**
	  The color of the prefix of warning messages.
	*/
	vl_color warning_prefix_color;
	/**
	  The main color of warning messages.
	*/
	vl_color warning_color;
	/**
	  The color of the prefix of error messages.
	*/
	vl_color error_prefix_color;
	/**
	  The main color of error messages.
	*/
	vl_color error_color;
} vl_color_scheme;

/**
  Obtains the default color scheme of VibrantLogs.
*/
#define VL_DEFAULT_COLORS (vl_color_scheme) { \
		.time_color = VL_WHITE, \
		.date_color = VL_WHITE, \
		.info_prefix_color = VL_LIGHT_BLUE, \
		.info_color = VL_BLUE, \
		.success_prefix_color = VL_LIGHT_GREEN, \
		.success_color = VL_GREEN, \
		.debug_prefix_color = VL_LIGHT_ORANGE, \
		.debug_color = VL_ORANGE, \
		.warning_prefix_color = VL_LIGHT_YELLOW, \
		.warning_color = VL_YELLOW, \
		.error_prefix_color = VL_LIGHT_RED, \
		.error_color = VL_RED \
	};

/**
  Represents a custom config for VibrantLogs.

  When VibrantLogs logs messages, you can set
  how VibrantLogs prints the time and date.
*/
typedef struct vl_config
{
	/**
	  The color scheme of VibrantLogs.

	  By default, it is set to VL_DEFAULT_COLORS.
	*/
	vl_color_scheme colors;
	/**
	  The output destination of VibrantLogs
	  messages.

	  The default destination is stdout, but
	  you can set this to another file, and VibrantLogs
	  will write to that file instead.
	*/
	FILE *output_destination;
	/**
	  The log level of VibrantLogs.

	  This acts as a filter for which log messages are
	  allowed through. For example, if the log level
	  is VL_ERROR, only error messages are printed,
	  but if the log level is VL_WARNING, then both
	  warning and error messages are shown. The VL_DEBUG
	  and VL_SUCCESS log types are exceptions to this rule.
	*/
	vl_type log_level;
	/**
	  Indicates whether or not VibrantLogs will
	  print the current time. This is true by default.
	*/
	bool print_time;
	/**
	  Indicates whether or not VibrantLogs will
	  print the current date. This includes
	  the day, month, and year. This is false
	  by default.
	*/
	bool print_date;
	/**
	  Indicates whether or not VibrantLogs will
	  print the current date's month as a number
	  value, or the month's name. This is false by
	  default.
	*/
	bool print_month_name;
} vl_config;

#ifdef _WIN32
	#ifdef VL_EXPORTS
		#define VL_API __declspec(dllexport)
	#else
		#define VL_API __declspec(dllimport)
	#endif
#else
	#define VL_API
#endif

/**
  Initializes the VibrantLogs library.

  @note This must be called for VibrantLogs
  to work properly.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_init(void);

/**
  Sets the config of VibrantLogs.

  @see vl_config.
*/
VL_API void vl_use_config(vl_config cfg);
/**
  Obtains the current config of VibrantLogs.
*/
VL_API vl_config *vl_curr_config(void);

/**
  Prints a message.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next
  line when printing messages.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_log(vl_type log_type, const char *fmt, ...);
/**
  Continues printing characters to the last message printed.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_print(vl_type log_type, const char *fmt, ...);
/**
  Prints a delayed message.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_delay_log(vl_type log_type, double seconds, const char *fmt, ...);
/**
  Prints a delayed message.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_delay_print(vl_type log_type, double seconds, const char *fmt, ...);

typedef struct timey_timestamp timey_timestamp;
typedef struct timey_datetime timey_datetime;

/**
  Schedules a message using a timestamp.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_schedule_log_ts(vl_type log_type, struct timey_timestamp *ts, const char *fmt, ...);
/**
  Schedules a message using a timestamp.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_schedule_print_ts(vl_type log_type, struct timey_timestamp *ts, const char *fmt, ...);
/**
  Schedules a message using a datetime.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_schedule_log_dt(vl_type log_type, struct timey_datetime *dt, const char *fmt, ...);
/**
  Schedules a message using a datetime.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_schedule_print_dt(vl_type log_type, timey_datetime *dt, const char *fmt, ...);

/**
  Updates the VibrantLogs library.

  This function is used to update delayed logs.
  It must be called in the program's event loop.
  The function also needs the delta time value
  of the program. Delta time is the time between
  the last frame and the current frame.
*/
VL_API void vl_update(double delta_time);
