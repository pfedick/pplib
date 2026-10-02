/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#ifndef PPLIB_CORE_SHUTDOWN_H
#define PPLIB_CORE_SHUTDOWN_H

namespace pplib
{

/** @defgroup PPLGroupShutdownHandler Shutdown Handler
 *
 * Funktionen zum Installieren und Verwalten eines globalen Shutdown-Handlers.
 *
 *  @{
 */

/** @brief Installiert einen Shutdown-Handler, der auf Systemsignale reagiert.
 *
 * Mit dieser Funktion wird ein globaler Shutdown-Handler installiert, der auf Systemsignale reagiert.
 *
 * @note Muss vor dem Empfang von Systemsignalen installiert werden.
 *
 * @see UninstallShutdownHandler()
 */
void InstallShutdownHandler();

/** @brief Deinstalliert den zuvor installierten Shutdown-Handler.
 *
 * Mit dieser Funktion wird der zuvor installierte globale Shutdown-Handler wieder entfernt.
 * Nach dem Aufruf dieser Funktion reagiert das Programm nicht mehr auf Systemsignale für einen Shutdown.
 *
 * @see InstallShutdownHandler()
 */
void UninstallShutdownHandler();

/** @brief Überprüft, ob ein Shutdown angefordert wurde.
 *
 * @return `true`, wenn ein Shutdown angefordert wurde, sonst `false`.
 */
bool IsShutdownRequested() noexcept;

/** @brief Fordert einen Shutdown an.
 *
 * Mit dieser Funktion kann ein Shutdown softwareseitig angefordert werden.
 *
 * @note Dies löst den zuvor installierten Shutdown-Handler aus.
 *
 * @see InstallShutdownHandler()
 * @see UninstallShutdownHandler()
 */
void RequestShutdown() noexcept;

/** @brief Setzt den Shutdown-Status zurück.
 *
 * Mit dieser Funktion wird der Shutdown-Status zurückgesetzt, sodass das Programm nicht mehr als heruntergefahren betrachtet wird.
 */
void ResetShutdown() noexcept;

/** @brief Wartet auf einen Shutdown.
 *
 * Mit dieser Funktion kann auf das Eintreten eines Shutdown-Signals gewartet werden.
 *
 * @param timeout_ms Maximale Wartezeit in Millisekunden. Standardwert ist 0 (unendliches Warten).
 * @return `true`, wenn ein Shutdown eingetreten ist, sonst `false`.
 */
bool WaitForShutdown(int timeout_ms = 0) noexcept;

/** @} */ // end of PPLGroupShutdownHandler

} // namespace pplib

#endif /* PPLIB_CORE_SHUTDOWN_H */