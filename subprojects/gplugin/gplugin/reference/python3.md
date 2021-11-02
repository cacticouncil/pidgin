Title: Python3 Plugins
Slut: python3

## Python3 Plugins

> You **MUST** have the Python3 loader plugin installed and working as well as
> the gobject-introspection package for GPlugin installed to use Python3
> plugins.

### Example Python Plugin

Like all plugins in GPlugin, Python plugins must also implement the
`gplugin_query`, `gplugin_load`, and `gplugin_unload` functions.

The following is a basic Python plugin.

```python
import gi

gi.require_version("GPlugin", "0.0")
from gi.repository import GPlugin

def gplugin_plugin_query():
    return GPlugin.PluginInfo(
        id="gplugin-python/basic-plugin",
        abi_version=0x01020304,
        name="basic plugin",
        authors=["author1"],
        category="test",
        version="version",
        license_id="license",
        summary="summary",
        website="website",
        description="description",
    )

def gplugin_plugin_load(plugin):
    return True

def gplugin_plugin_unload(plugin):
    return True
```

