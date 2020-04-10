#ifndef __LOG_PARSER_H__
#define __LOG_PARSER_H__

#include <sys/stat.h>

#include <glib.h>
#include <glib/gprintf.h>

enum {
  LOG_PATH,
  LOG_FMT,
  LOG_LEVEL,
  LOG_MAX_BYTES,
  LOG_BACKUP_COUNT,
  LOG_INFO_NUMS
};

extern int      log_conf_load           (GKeyFile *key_file, char *conf_file, gboolean reload_flag);
extern gboolean log_conf_get_value      (GKeyFile *key_file, char *section, char *key, gchar **value);
extern gboolean log_conf_get_settings   (GKeyFile *key_file, gchar **info);
extern int      log_conf_get_level_flag (char *log_level);
extern gboolean log_write               (GKeyFile *key_file, char *message, char *file_name, int line_number);

#endif
