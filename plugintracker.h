/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __PLUGINTRACKER_H__
#define __PLUGINTRACKER_H__

// Plugin handler core
//
// Handles scanning the config file, stocking the vector of plugins with root
// and nonroot plugins, getting the plugin info from the shared object, and
// calling the various plugin handler hooks.
//
// This is fairly platform specific -- posix systems should all be able to use
// the dlopen/dlsym/dlclose system, but it will have to be turned into 
// native code on win32 if we ever port there
//
// PLUGIN AUTHORS:  This is the Kismet internal tracking system that handles
// plugins.  All you need to worry about is filling in the plugin_usrdata struct
// and returning the proper info for register/unregister.
//
// Plugins which operate as root WILL NOT be able to perform root actions after
// the privdrop has occured.  Currently there are no hooks to modularize the 
// server/rootpid system, this may change if needed in the future.  Open any
// files and sockets you need before the drop.

#include "config.h"

#include <stdio.h>
#include <time.h>
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <string>

#include "globalregistry.h"

// Generic plugin function hook
typedef int (*plugin_hook)(GlobalRegistry *);

// This struct is passed to a plugin to be filled in with all their info
typedef struct {
	// Basic info about the plugin
	string pl_name;
	string pl_description;
	string pl_version;

	// Call back to initialized.  A 0 return means "try again later in the build
	// chain", a negative means "failure" and a positive means success.
	plugin_hook plugin_register;

	// Callback to be removed
	plugin_hook plugin_unregister;
	
	// Callback for Kismet to notify a root plugin that the privdrop has occured,
	// if there are any plugin-related priv controls
	plugin_hook plugin_suiddrop;

} plugin_usrdata;

// Plugin information fetch function
typedef int (*plugin_infocall)(plugin_usrdata *);

// Plugin management class
class PluginTracker {
public:
	typedef struct plugin_meta {
		plugin_meta() {
			dlfileptr = NULL;
			activate = 0;
			root = 0;
			infosym = NULL;
		};

		// Path to plugin
		string filename; 

		// Pointer to dlopened file
		void *dlfileptr;
		// Plugin has successfully activated?
		int activate;
		// Plugin is activated as root?
		int root;

		// Data from the plugin
		plugin_usrdata usrdata;

		// Info symbol
		plugin_infocall infosym;
	};

	static void Usage(char *name);

	PluginTracker();
	PluginTracker(GlobalRegistry *in_globalreg);
	~PluginTracker();

	// Parse the config file and load root plugins into the vector
	int ScanRootPlugins();

	// Scan the plugins directories and load all the found plugins for userspace
	int ScanUserPlugins();

	// Activate the vector of plugins (called repeatedly during startup)
	int ActivatePlugins();

	// Kick that we privdropped
	int PrivdropPlugins();

	// Shut down the plugins and close the shared files
	int ShutdownPlugins();

protected:
	GlobalRegistry *globalreg;
	int plugins_active;

	vector<plugin_meta *> plugin_vec;
};

#endif

