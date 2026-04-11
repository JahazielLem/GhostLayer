/**
 * @file include/style_ccs.h
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
#ifndef GHOSTLAYER_STYLE_CCS_H
#define GHOSTLAYER_STYLE_CCS_H

const char *stylesheet = "treeview {"
        "  border: 1px solid #d1d1d1;"
        "}"
        "treeview row:nth-child(even) {"
        "  background-color: #f9f9f9;"
        "}"
        "treeview row:nth-child(odd) {"
        "  background-color: #ffffff;"
        "}"
        "treeview grid-line {"
        "  color: #e0e0e0;"
        "}";

#endif //GHOSTLAYER_STYLE_CCS_H
