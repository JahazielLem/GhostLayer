/**
 * @file include/pcap_loader.h
 * @brief PROJECT
 *
 * DESCRIPTION
 *
 * @author astrobyte
 * @date 2026-04-11
 * @license GNU General Public License
 * @copyright Copyright (c) 2026 kevin Leon
 * @contact kevinleon.morales@gmail.com
 */
#ifndef GHOSTLAYER_PCAP_LOADER_H
#define GHOSTLAYER_PCAP_LOADER_H

#include "../include/main_gui.h"


void pcap_reader_open_file(const char*filename);
void pcap_reader_dialog(GtkWidget *button, gpointer user_data);
#endif //GHOSTLAYER_PCAP_LOADER_H
