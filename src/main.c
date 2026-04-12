/**
 * @file src/main.c
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-09
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */
#include "main_gui.h"

int main(int argc, char *argv[]) {
  register_dissectors();
  dissector_parser_register();
  return gui_main(argc, argv);
}
