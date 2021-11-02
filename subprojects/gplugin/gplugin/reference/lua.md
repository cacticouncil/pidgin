Title: Lua Plugins
Slug: lua

## Lua Plugins

> You **MUST** have the Lua loader plugin installed and working as well as the
> gobject-introspection package for GPlugin instance to use Lua plugins.

### Example Lua Plugin

Like all plugins in GPlugin, Lua plugins must also implement the
`gplugin_query`, `gplugin_load`, and `gplugin_unload` functions.

The following is a basic Lua plugin.

```lua
local lgi = require "lgi"
local GPlugin = lgi.GPlugin

function gplugin_query()
    return GPlugin.PluginInfo {
        id = "gplugin-lua/basic-plugin",
        abi_version = 0x01020304,
        name = "basic plugin",
        category = "test",
        version = "0.0.10",
        license_id = "license-id",
        summary = "basic lua plugin",
        description = "description of the basic lua plugin",
        authors = { "Gary Kramlich &lt;grim@reaperworld.com&gt;" },
        website = "https://keep.imfreedom.org/gplugin/gplugin/"
    }
end

function gplugin_load(plugin)
    return true
end

function gplugin_unload(plugin)
    return true
end
```

