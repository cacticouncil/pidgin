//==============================================================================
//
// Filename: qtnullclient.cpp
//
// File purpose: Definitions for intermediary function(s) that receives user
// selection for plugin protocol and login credentials and uses functions in
// qtlibpurple.cpp to establish connection to libpurple plugin protocol.
// 
//==============================================================================

#include "qtlibpurple.h"
#include <defines.h>
#include <purple.h>
#include <iostream>
#include <string>


// Arguments:
//     pluginNum: Menu option number selected by User in Qt UI window.
//     strusername: username entered by User in Qt UI window.
//     strPassword: password entered by User in Qt UI window.
// Caution: Unexpected behavior was previously encountered with credentials
// passed as const char* after conversion from QString using QString::toUtf8()
// method. Consequently, an alternative method of passing strings after
// using QString::toStdString cpnversion method was used instead.
void connectPurplePluginProtocol(int pluginNum, std::string strUsername,
    std::string strPassword)
{
    const char *prpl = NULL;
    char name[128];

    GList *iter;
    GList *names = NULL;
    PurpleAccount *account;
    PurpleSavedStatus *status;

    // pidgin comments: libpurple's built-in DNS resolution forks processes to
    // perform blocking lookups without blocking the main process.  It does not
    // handle SIGCHLD itself, so if the UI does not you quickly get an army
    // of zombie subprocesses marching around.
    #ifndef _WIN32
        signal(SIGCHLD, SIG_IGN);
    #endif
   
    init_libpurple();
    printf("libpurple initialized.\n");
    
    iter = purple_plugins_get_protocols();
    for (int i = 0; iter; iter = iter->next)
    {
        PurplePlugin *plugin = static_cast<PurplePlugin*>(iter->data);
        PurplePluginInfo *info = plugin->info;
        if (info && info->name) {
            names = g_list_append(names, info->id);
        }
    }
    
    prpl = static_cast<const char*>(g_list_nth_data(names, pluginNum));
   
    const char* username = strUsername.c_str();
    const char* password = strPassword.c_str();

    account = purple_account_new(username, prpl);
    purple_account_set_password(account, password);
    purple_account_set_enabled(account, UI_ID, TRUE);
    status = purple_savedstatus_new(NULL, PURPLE_STATUS_AVAILABLE);
    purple_savedstatus_activate(status);
    
    connect_to_signals_for_demonstration_purposes_only();

    return;
}