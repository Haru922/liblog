#include "liblog.h"

int log_conf_load (GKeyFile *key_file,
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
log_conf_get_value (GKeyFile *key_file,
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
    {
      g_warning ("No Value for the Section:[%s]\nKey:%s.\n", section, key);
    }
    return FALSE;
  }

  return TRUE;
}

int
log_conf_get_level_flag (char *log_level)
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
log_conf_get_settings (GKeyFile *key_file,
                       gchar **log_conf)
{
  if (log_conf_get_value (key_file, "LOG", "PATH", &log_conf[LOG_PATH])
      && log_conf_get_value (key_file, "LOG", "FMT", &log_conf[LOG_FMT])
      && log_conf_get_value (key_file, "LOG", "LEVEL", &log_conf[LOG_LEVEL])
      && log_conf_get_value (key_file, "LOG", "MAX_BYTES", &log_conf[LOG_MAX_BYTES])
      && log_conf_get_value (key_file, "LOG", "BACKUP_COUNT", &log_conf[LOG_BACKUP_COUNT]))
  {
    return TRUE;
  }
  return FALSE;
}

gboolean
log_write (GKeyFile *key_file,
           char *message,
           char *file_name,
           int line_number)
{
  int i;
  gchar *log_conf[LOG_INFO_NUMS];
  GDateTime *local_time;
  gchar *log_time_prefix;
  gchar *output_file_identifier;
  gchar *output_file_name;
  gchar *output_file;
  char *file_extension = "log";
  FILE *output_fp;
  gboolean ret_val;
  GDir *dir;
  GError *error = NULL;
  const char *dir_file;
  struct stat file_info;
  gchar **file_separator;
  int log_file_cnt = 0;
  gchar *log_file;
  gchar *first_log_file = NULL;
  time_t mtime = G_MAXINT32;

  if (!log_conf_get_settings (key_file, log_conf))
  {
    return FALSE;
  }

  local_time = g_date_time_new_now_local ();
  log_time_prefix = g_date_time_format (local_time, "%a %b %d %H:%M:%S %Y"); 
  output_file_identifier = g_date_time_format (local_time, "%F");
  output_file_name = g_strconcat (output_file_identifier, ".", file_extension, NULL);
  output_file = g_strconcat (log_conf[LOG_PATH], output_file_name, NULL);
  g_date_time_unref (local_time);

  dir = g_dir_open (log_conf[LOG_PATH], 0, &error);
  if (error != NULL)
  {
    g_error ("Error finding key in key file: %s\n", error->message);
    g_error_free (error);
  }
  while ((dir_file = g_dir_read_name (dir)) != NULL)
  {
    file_separator = g_strsplit (dir_file, ".", -1);
    if (!g_strcmp0 (file_separator[1], file_extension))
    {
      log_file_cnt++;
      log_file = g_strconcat (log_conf[LOG_PATH], dir_file, NULL);
      stat (log_file, &file_info);
      if (mtime > file_info.st_mtime)
      {
        mtime = file_info.st_mtime;
        if (first_log_file != NULL)
        {
          g_free (first_log_file);
        }
        first_log_file = g_strdup (log_file);
      }
      g_free (log_file);
    }
    g_strfreev (file_separator);
  }

  g_dir_rewind (dir);

  while ((dir_file = g_dir_read_name (dir)) != NULL)
  {
    if (!g_strcmp0 (output_file_name, dir_file))
    {
      stat (output_file, &file_info);
      if (atoi (log_conf[LOG_MAX_BYTES]) <= file_info.st_size)
      {
        output_file_identifier = g_strconcat (output_file_identifier, "+", NULL);
        g_free (output_file_name);
        g_free (output_file);
        output_file_name = g_strconcat (output_file_identifier, ".", file_extension, NULL);
        output_file = g_strconcat (log_conf[LOG_PATH], output_file_name, NULL);
        g_dir_rewind (dir);
      }
    }
  }
  g_dir_close (dir);
  g_free (output_file_identifier);
  g_free (output_file_name);
  
  output_fp = fopen (output_file, "a");
  if (output_fp == NULL)
  {
    g_fprintf (stderr, "Cannot open / create log file:%s\n", output_file);
    ret_val = FALSE;
  }
  else
  {
    g_fprintf (output_fp, log_conf[LOG_FMT], log_time_prefix, log_conf[LOG_LEVEL], file_name, line_number, message); 
    fclose (output_fp);
    ret_val = TRUE;
  }
  g_free (log_time_prefix);
  g_free (output_file);

  if (atoi (log_conf[LOG_BACKUP_COUNT]) == log_file_cnt)
  {
    if (!g_remove (first_log_file))
    {
      g_error ("Cannot Remove File: %s\n", first_log_file);
    }
  }
  g_free (first_log_file);

  for (i = 0; i < LOG_INFO_NUMS; i++)
  {
    g_free (log_conf[i]);
  }

  return ret_val;
}
