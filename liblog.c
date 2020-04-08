#include "liblog.h"

int
log_read (GKeyFile *key_file,
          char *conf_file,
          gboolean reload_flag)
{
  GError *error = NULL;
  
  if (reload_flag)
  {
    g_key_file_free (key_file);
  }
  else if (g_key_file_get_start_group (key_file) != NULL)
  {
    return 1;
  }

  if (!g_key_file_load_from_file (key_file, conf_file, G_KEY_FILE_NONE, &error))
  {
    if (!g_error_matches (error, G_FILE_ERROR, G_FILE_ERROR_NOENT)
        || !g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_NOT_FOUND))
    {
      g_error ("Error loading key file: %s\n", error->message);
    }
    g_error_free (error);
    return -1;
  }
  return 0;
}

gboolean
log_get_value (GKeyFile *key_file,
               char *section,
               char *key,
               gchar **value)
{
  GError *error = NULL;

  if (key_file == NULL) 
  {
    g_error ("Cannot read the key file.");
    return FALSE;
  }

  *value = g_key_file_get_string (key_file, section, key, &error);

  if (*value == NULL)
  {
    if (!g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
    {
      g_error ("Error finding section in key file: %s\n", error->message);
      g_error_free (error);
    }
    else if (!g_error_matches (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND))
    {
      g_error ("Error finding key in key file: %s\n", error->message);
      g_error_free (error);
    }
    else
      g_warning ("No Value for the Section:[%s]\nKey:%s.\n", section, key);
    return FALSE;
  }

  return TRUE;
}

int
log_get_level_flag (char *log_level)
{
  if (!g_strcmp0 (log_level, "ERROR"))
    return G_LOG_LEVEL_ERROR;
  else if (!g_strcmp0 (log_level, "CRITICAL"))
    return G_LOG_LEVEL_CRITICAL;
  else if (!g_strcmp0 (log_level, "WARNING"))
    return G_LOG_LEVEL_WARNING;
  else if (!g_strcmp0 (log_level, "MESSAGE"))
    return G_LOG_LEVEL_MESSAGE;
  else if (!g_strcmp0 (log_level, "DEBUG"))
    return G_LOG_LEVEL_DEBUG;
  else 
    return G_LOG_LEVEL_INFO;
}

gboolean
log_get_conf (GKeyFile *key_file,
              gchar **log_conf)
{
  if (log_get_value (key_file, "LOG", "PATH", &log_conf[LOG_PATH])
      && log_get_value (key_file, "LOG", "FMT", &log_conf[LOG_FMT])
      && log_get_value (key_file, "LOG", "LEVEL", &log_conf[LOG_LEVEL])
      && log_get_value (key_file, "LOG", "MAX_BYTES", &log_conf[LOG_MAX_BYTES])
      && log_get_value (key_file, "LOG", "BACKUP_COUNT", &log_conf[LOG_BACKUP_COUNT]))
  {
    return TRUE;
  }
  return FALSE;
}

gboolean
log_write (GKeyFile *key_file, char *message, char *file_name, int line_number)
{
  int i;
  gchar *log_conf[LOG_INFO_NUMS];
  GDateTime *local_time;
  gchar *log_time_flag;
  gchar *output_file_name;
  FILE *output_file;

  if (!log_get_conf (key_file, log_conf))
  {
    return FALSE;
  }

  local_time = g_date_time_new_now_local ();
  log_time_flag = g_date_time_format (local_time, log_conf[LOG_FMT]); 
  output_file_name = g_date_time_format (local_time, "%F");
  output_file_name = g_strconcat (log_conf[LOG_PATH], output_file_name, ".log", NULL);

  output_file = fopen (output_file_name, "a");
  g_fprintf(output_file, "%s[%s][%s:%d] %s", log_time_flag, log_conf[LOG_LEVEL], file_name, line_number, message);
  fclose (output_file);

  g_date_time_unref (local_time);
  for (i = 0; i < LOG_INFO_NUMS; i++)
  {
    g_free (log_conf[i]);
  }
  g_free (output_file_name);
  g_free (log_time_flag);

  return TRUE;
}
