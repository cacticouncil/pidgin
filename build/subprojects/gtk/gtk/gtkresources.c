#include <gio/gio.h>

#if defined (__ELF__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6))
# define SECTION __attribute__ ((section (".gresource.gtk"), aligned (8)))
#else
# define SECTION
#endif

extern const SECTION union { const guint8 data[1501588]; const double alignment; void * const ptr;}  _gtk_resource_data;

static GStaticResource static_resource = { _gtk_resource_data.data, sizeof (_gtk_resource_data.data), NULL, NULL, NULL };

G_MODULE_EXPORT
GResource *_gtk_get_resource (void);
GResource *_gtk_get_resource (void)
{
  return g_static_resource_get_resource (&static_resource);
}

G_MODULE_EXPORT
void _gtk_unregister_resource (void);
void _gtk_unregister_resource (void)
{
  g_static_resource_fini (&static_resource);
}

G_MODULE_EXPORT
void _gtk_register_resource (void);
void _gtk_register_resource (void)
{
  g_static_resource_init (&static_resource);
}
