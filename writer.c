#include "liblog.h"

int main (int argc, char *argv[]) {
  GKeyFile *key_file = g_key_file_new ();

  if (argc != 2) {
    fprintf (stderr, "Usage: ./writer conf_file\n");
    exit (EXIT_FAILURE);
  }

  log_conf_load (key_file, argv[1], TRUE); 

  log_write (key_file, "Log Message from writer.\n", __FILE__, __LINE__);

  g_key_file_free (key_file);
  exit (EXIT_SUCCESS);
}
