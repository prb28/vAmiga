// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RshServer.h"
#include "MsgQueue.h"
#include "RetroShell.h"
#include "StringUtils.h"

void
RshServer::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    RemoteServer::_dump(category, os);
}

void
RshServer::didConnect()
{
    send("vAmiga Remote Server ");
    send(std::to_string(VER_MAJOR) + ".");
    send(std::to_string(VER_MINOR) + ".");
    send(std::to_string(VER_SUBMINOR));
    send(" (" __DATE__ " " __TIME__ ")\n\n");
    send("Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de\n");
    send("Licensed under the GNU General Public License v3\n\n");
    send("Type 'help' for help.\n");
    send("\n");
}

string
RshServer::_receive()
{
    string payload = connection.recv();
    
    // Ask the client to delete the input (will be replicated by RetroShell)
    send("\033[A\33[2K\r");
    
    return payload;
}

void
RshServer::_send(const string &payload)
{
    connection.send(payload);
}

void
RshServer::_process(const string &payload)
{
    retroShell.press(payload);
    retroShell.press('\n');
}
