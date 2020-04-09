#include "liblog.h"

int main (int argc, char *argv[]) {
  GKeyFile *key_file = g_key_file_new ();
  gchar *value;

  if (argc != 4)
  {
    g_warning ("Usage: logger conf_file section_name key_name");
    exit (EXIT_FAILURE);
  }

  if (log_read (key_file, argv[1], TRUE) == -1) 
    exit (EXIT_FAILURE);
  else {
    if (log_get_value (key_file, argv[2], argv[3], &value)) {
      g_print ("%s\n", value);
      g_free (value);
    } else {
      g_key_file_free (key_file);
      exit (EXIT_FAILURE);
    }
  }

  if (log_write (key_file, "log_write\n", __FILE__, __LINE__)) {
    g_key_file_free (key_file);
    exit (EXIT_SUCCESS);
  } else {
    g_key_file_free (key_file);
    exit (EXIT_FAILURE);
  }
}
