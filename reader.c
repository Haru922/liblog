#include "liblog.h"

int main (int argc, char *argv[]) {
  GKeyFile *key_file = g_key_file_new ();
  gchar *value;

  if (argc != 4)
  {
    g_warning ("Usage: ./reader conf_file section_name key_name");
    exit (EXIT_FAILURE);
  }

  log_conf_load (key_file, argv[1], TRUE); 
  log_conf_get_value (key_file, argv[2], argv[3], &value);

  g_print ("%s\n", value);
  g_free (value);

  g_key_file_free (key_file);
  exit (EXIT_SUCCESS);
}
