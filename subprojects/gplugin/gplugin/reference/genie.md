Title: Genie Plugin Example
Slug: genie

## Genie Plugins

> You **MUST** have the Vala bindings installed on your system for this to
> work. They are built by the default GPlugin build.

### Example Genie Plugin

Due to the way `GPlugin.PluginInfo` info works, you must subclass it and set
your values in the new constructor. It is recommended that you define this
class in a namespace to avoid collisions with other plugins.

Like all plugins in GPlugin, Genie plugins must also implement the
`gplugin_query`, `gplugin_load`, and `gplugin_unload` functions. These
functions must be in the global namespace.

The following is a basic Genie plugin.

```genie
uses GPlugin

namespace BasicPlugin
	class Info : GPlugin.PluginInfo
		construct()
			authors : array of string = {"author1"}

			Object(
				id: "gplugin/genie-basic-plugin",
				abi_version: 0x01020304,
				name: "basic plugin",
				authors: authors,
				category: "test",
				version: "version",
				license_id: "license",
				summary: "summary",
				website: "website",
				description: "description"
			)

def gplugin_query(out error : Error) : GPlugin.PluginInfo
	error = null

	return new BasicPlugin.Info()

def gplugin_load(plugin : GPlugin.Plugin, out error : Error) : bool
	error = null

	return true

def gplugin_unload(plugin : GPlugin.Plugin, out error : Error) : bool
	error = null

	return true
```

