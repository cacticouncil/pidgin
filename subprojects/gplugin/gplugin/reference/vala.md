Title: Vala Plugin Example
Slug: vala

## Vala Plugins

> You **MUST** have the Vala bindings installed on your system for this to
> work. They are built by the default GPlugin build.

### Example Vala Plugin

Due to the way `GPlugin.PluginInfo` info works, you must subclass it and set
your values in the new constructor. It is recommended that you define this
class in a namespace to avoid collisions with other plugins.

Like all plugins in GPlugin, Vala plugins must also implement the
`gplugin_query`, `gplugin_load`, and `gplugin_unload` functions. These
functions must be in the global namespace.

The following is a basic Vala plugin.

```vala
using GPlugin;

namespace BasicPlugin {

public class Info : GPlugin.PluginInfo {
	public Info() {
		string[] authors = {"author1"};

		Object(
			id: "gplugin/vala-basic-plugin",
			abi_version: 0x01020304,
			name: "basic plugin",
			authors: authors,
			category: "test",
			version: "version",
			license_id: "license",
			summary: "summary",
			website: "website",
			description: "description"
		);
	}
}

}

public GPlugin.PluginInfo gplugin_query(out Error error) {
	error = null;

	return new BasicPlugin.Info();
}

public bool gplugin_load(GPlugin.Plugin plugin, out Error error) {
	error = null;

	return true;
}

public bool gplugin_unload(GPlugin.Plugin plugin, out Error error) {
	error = null;

	return true;
}
```

