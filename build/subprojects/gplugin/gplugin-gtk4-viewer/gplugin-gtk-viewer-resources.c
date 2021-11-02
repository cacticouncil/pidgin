#include <gio/gio.h>

#if defined (__ELF__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6))
# define SECTION __attribute__ ((section (".gresource.gplugin_gtk_viewer"), aligned (8)))
#else
# define SECTION
#endif

#ifdef _MSC_VER
static const SECTION union { const guint8 data[705]; const double alignment; void * const ptr;}  gplugin_gtk_viewer_resource_data = { {
  0107, 0126, 0141, 0162, 0151, 0141, 0156, 0164, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
  0030, 0000, 0000, 0000, 0000, 0001, 0000, 0000, 0000, 0000, 0000, 0050, 0010, 0000, 0000, 0000, 
  0000, 0000, 0000, 0000, 0001, 0000, 0000, 0000, 0002, 0000, 0000, 0000, 0003, 0000, 0000, 0000, 
  0005, 0000, 0000, 0000, 0006, 0000, 0000, 0000, 0007, 0000, 0000, 0000, 0010, 0000, 0000, 0000, 
  0320, 0120, 0255, 0377, 0003, 0000, 0000, 0000, 0000, 0001, 0000, 0000, 0010, 0000, 0114, 0000, 
  0010, 0001, 0000, 0000, 0014, 0001, 0000, 0000, 0121, 0232, 0274, 0372, 0000, 0000, 0000, 0000, 
  0014, 0001, 0000, 0000, 0007, 0000, 0114, 0000, 0024, 0001, 0000, 0000, 0030, 0001, 0000, 0000, 
  0122, 0134, 0116, 0150, 0004, 0000, 0000, 0000, 0030, 0001, 0000, 0000, 0012, 0000, 0114, 0000, 
  0044, 0001, 0000, 0000, 0050, 0001, 0000, 0000, 0353, 0273, 0202, 0010, 0007, 0000, 0000, 0000, 
  0050, 0001, 0000, 0000, 0010, 0000, 0114, 0000, 0060, 0001, 0000, 0000, 0064, 0001, 0000, 0000, 
  0113, 0120, 0220, 0013, 0005, 0000, 0000, 0000, 0064, 0001, 0000, 0000, 0004, 0000, 0114, 0000, 
  0070, 0001, 0000, 0000, 0074, 0001, 0000, 0000, 0324, 0265, 0002, 0000, 0377, 0377, 0377, 0377, 
  0074, 0001, 0000, 0000, 0001, 0000, 0114, 0000, 0100, 0001, 0000, 0000, 0104, 0001, 0000, 0000, 
  0065, 0113, 0237, 0202, 0001, 0000, 0000, 0000, 0104, 0001, 0000, 0000, 0011, 0000, 0166, 0000, 
  0120, 0001, 0000, 0000, 0267, 0002, 0000, 0000, 0006, 0074, 0270, 0052, 0002, 0000, 0000, 0000, 
  0267, 0002, 0000, 0000, 0005, 0000, 0114, 0000, 0274, 0002, 0000, 0000, 0300, 0002, 0000, 0000, 
  0147, 0160, 0154, 0165, 0147, 0151, 0156, 0057, 0001, 0000, 0000, 0000, 0166, 0151, 0145, 0167, 
  0145, 0162, 0057, 0000, 0006, 0000, 0000, 0000, 0151, 0155, 0146, 0162, 0145, 0145, 0144, 0157, 
  0155, 0057, 0000, 0000, 0007, 0000, 0000, 0000, 0147, 0160, 0154, 0165, 0147, 0151, 0156, 0057, 
  0000, 0000, 0000, 0000, 0157, 0162, 0147, 0057, 0002, 0000, 0000, 0000, 0057, 0000, 0000, 0000, 
  0004, 0000, 0000, 0000, 0167, 0151, 0156, 0144, 0157, 0167, 0056, 0165, 0151, 0000, 0000, 0000, 
  0125, 0003, 0000, 0000, 0001, 0000, 0000, 0000, 0170, 0332, 0215, 0223, 0113, 0123, 0203, 0060, 
  0024, 0205, 0367, 0375, 0025, 0231, 0354, 0133, 0160, 0254, 0216, 0013, 0240, 0243, 0013, 0273, 
  0161, 0034, 0147, 0174, 0055, 0235, 0220, 0334, 0102, 0044, 0115, 0142, 0162, 0151, 0313, 0277, 
  0067, 0026, 0252, 0155, 0311, 0302, 0035, 0160, 0277, 0163, 0016, 0047, 0217, 0154, 0261, 0133, 
  0053, 0262, 0001, 0347, 0245, 0321, 0071, 0275, 0230, 0245, 0224, 0200, 0346, 0106, 0110, 0135, 
  0345, 0364, 0365, 0345, 0176, 0172, 0103, 0027, 0305, 0044, 0223, 0032, 0301, 0255, 0030, 0207, 
  0142, 0102, 0110, 0346, 0340, 0253, 0225, 0016, 0074, 0121, 0262, 0314, 0151, 0205, 0015, 0375, 
  0263, 0230, 0007, 0213, 0044, 0106, 0131, 0325, 0126, 0122, 0117, 0117, 0351, 0164, 0166, 0171, 
  0065, 0340, 0010, 0153, 0253, 0030, 0002, 0341, 0212, 0171, 0237, 0323, 0345, 0323, 0136, 0260, 
  0304, 0346, 0115, 0302, 0026, 0334, 0273, 0324, 0302, 0154, 0051, 0261, 0314, 0201, 0306, 0060, 
  0307, 0346, 0326, 0132, 0045, 0071, 0303, 0140, 0065, 0114, 0177, 0234, 0202, 0227, 0165, 0306, 
  0202, 0303, 0216, 0150, 0266, 0206, 0234, 0242, 0104, 0005, 0224, 0240, 0143, 0332, 0207, 0010, 
  0126, 0252, 0360, 0261, 0003, 0117, 0213, 0041, 0204, 0364, 0021, 0131, 0162, 0020, 0306, 0175, 
  0004, 0254, 0130, 0253, 0160, 0272, 0225, 0002, 0153, 0132, 0134, 0247, 0351, 0177, 0025, 0065, 
  0310, 0252, 0106, 0132, 0314, 0043, 0022, 0136, 0113, 0045, 0372, 0347, 0360, 0146, 0312, 0117, 
  0340, 0370, 0273, 0010, 0330, 0074, 0163, 0147, 0224, 0002, 0161, 0322, 0060, 0226, 0125, 0373, 
  0075, 0131, 0062, 0367, 0141, 0115, 0130, 0227, 0216, 0026, 0032, 0066, 0343, 0126, 0343, 0320, 
  0170, 0360, 0203, 0364, 0170, 0147, 0166, 0364, 0230, 0072, 0050, 0011, 0166, 0066, 0104, 0206, 
  0015, 0343, 0120, 0033, 0045, 0300, 0235, 0141, 0121, 0103, 0126, 0202, 0032, 0161, 0343, 0042, 
  0152, 0317, 0305, 0266, 0353, 0321, 0220, 0376, 0030, 0171, 0262, 0062, 0255, 0026, 0261, 0152, 
  0203, 0147, 0322, 0307, 0237, 0375, 0174, 0062, 0356, 0075, 0002, 0317, 0240, 0123, 0340, 0150, 
  0230, 0045, 0207, 0043, 0033, 0056, 0110, 0162, 0164, 0103, 0276, 0001, 0100, 0131, 0006, 0102, 
  0000, 0050, 0165, 0165, 0141, 0171, 0051, 0153, 0145, 0145, 0160, 0057, 0003, 0000, 0000, 0000
} };
#else /* _MSC_VER */
static const SECTION union { const guint8 data[705]; const double alignment; void * const ptr;}  gplugin_gtk_viewer_resource_data = {
  "\107\126\141\162\151\141\156\164\000\000\000\000\000\000\000\000"
  "\030\000\000\000\000\001\000\000\000\000\000\050\010\000\000\000"
  "\000\000\000\000\001\000\000\000\002\000\000\000\003\000\000\000"
  "\005\000\000\000\006\000\000\000\007\000\000\000\010\000\000\000"
  "\320\120\255\377\003\000\000\000\000\001\000\000\010\000\114\000"
  "\010\001\000\000\014\001\000\000\121\232\274\372\000\000\000\000"
  "\014\001\000\000\007\000\114\000\024\001\000\000\030\001\000\000"
  "\122\134\116\150\004\000\000\000\030\001\000\000\012\000\114\000"
  "\044\001\000\000\050\001\000\000\353\273\202\010\007\000\000\000"
  "\050\001\000\000\010\000\114\000\060\001\000\000\064\001\000\000"
  "\113\120\220\013\005\000\000\000\064\001\000\000\004\000\114\000"
  "\070\001\000\000\074\001\000\000\324\265\002\000\377\377\377\377"
  "\074\001\000\000\001\000\114\000\100\001\000\000\104\001\000\000"
  "\065\113\237\202\001\000\000\000\104\001\000\000\011\000\166\000"
  "\120\001\000\000\267\002\000\000\006\074\270\052\002\000\000\000"
  "\267\002\000\000\005\000\114\000\274\002\000\000\300\002\000\000"
  "\147\160\154\165\147\151\156\057\001\000\000\000\166\151\145\167"
  "\145\162\057\000\006\000\000\000\151\155\146\162\145\145\144\157"
  "\155\057\000\000\007\000\000\000\147\160\154\165\147\151\156\057"
  "\000\000\000\000\157\162\147\057\002\000\000\000\057\000\000\000"
  "\004\000\000\000\167\151\156\144\157\167\056\165\151\000\000\000"
  "\125\003\000\000\001\000\000\000\170\332\215\223\113\123\203\060"
  "\024\205\367\375\025\231\354\133\160\254\216\013\240\243\013\273"
  "\161\034\147\174\055\235\220\334\102\044\115\142\162\151\313\277"
  "\067\026\252\155\311\302\035\160\277\163\016\047\217\154\261\133"
  "\053\262\001\347\245\321\071\275\230\245\224\200\346\106\110\135"
  "\345\364\365\345\176\172\103\027\305\044\223\032\301\255\030\207"
  "\142\102\110\346\340\253\225\016\074\121\262\314\151\205\015\375"
  "\263\230\007\213\044\106\131\325\126\122\117\117\351\164\166\171"
  "\065\340\010\153\253\030\002\341\212\171\237\323\345\323\136\260"
  "\304\346\115\302\026\334\273\324\302\154\051\261\314\201\306\060"
  "\307\346\326\132\045\071\303\140\065\114\177\234\202\227\165\306"
  "\202\303\216\150\266\206\234\242\104\005\224\240\143\332\207\010"
  "\126\252\360\261\003\117\213\041\204\364\021\131\162\020\306\175"
  "\004\254\130\253\160\272\225\002\153\132\134\247\351\177\025\065"
  "\310\252\106\132\314\043\022\136\113\045\372\347\360\146\312\117"
  "\340\370\273\010\330\074\163\147\224\002\161\322\060\226\125\373"
  "\075\131\062\367\141\115\130\227\216\026\032\066\343\126\343\320"
  "\170\360\203\364\170\147\166\364\230\072\050\011\166\066\104\206"
  "\015\343\120\033\045\300\235\141\121\103\126\202\032\161\343\042"
  "\152\317\305\266\353\321\220\376\030\171\262\062\255\026\261\152"
  "\203\147\322\307\237\375\174\062\356\075\002\317\240\123\340\150"
  "\230\045\207\043\033\056\110\162\164\103\276\001\100\131\006\102"
  "\000\050\165\165\141\171\051\153\145\145\160\057\003\000\000\000"
  "" };
#endif /* !_MSC_VER */

static GStaticResource static_resource = { gplugin_gtk_viewer_resource_data.data, sizeof (gplugin_gtk_viewer_resource_data.data) - 1 /* nul terminator */, NULL, NULL, NULL };

G_GNUC_INTERNAL
GResource *gplugin_gtk_viewer_get_resource (void);
GResource *gplugin_gtk_viewer_get_resource (void)
{
  return g_static_resource_get_resource (&static_resource);
}
/*
  If G_HAS_CONSTRUCTORS is true then the compiler support *both* constructors and
  destructors, in a usable way, including e.g. on library unload. If not you're on
  your own.

  Some compilers need #pragma to handle this, which does not work with macros,
  so the way you need to use this is (for constructors):

  #ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
  #pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(my_constructor)
  #endif
  G_DEFINE_CONSTRUCTOR(my_constructor)
  static void my_constructor(void) {
   ...
  }

*/

#ifndef __GTK_DOC_IGNORE__

#if  __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR(_func) static void __attribute__((constructor)) _func (void);
#define G_DEFINE_DESTRUCTOR(_func) static void __attribute__((destructor)) _func (void);

#elif defined (_MSC_VER) && (_MSC_VER >= 1500)
/* Visual studio 2008 and later has _Pragma */

#include <stdlib.h>

#define G_HAS_CONSTRUCTORS 1

/* We do some weird things to avoid the constructors being optimized
 * away on VS2015 if WholeProgramOptimization is enabled. First we
 * make a reference to the array from the wrapper to make sure its
 * references. Then we use a pragma to make sure the wrapper function
 * symbol is always included at the link stage. Also, the symbols
 * need to be extern (but not dllexport), even though they are not
 * really used from another object file.
 */

/* We need to account for differences between the mangling of symbols
 * for x86 and x64/ARM/ARM64 programs, as symbols on x86 are prefixed
 * with an underscore but symbols on x64/ARM/ARM64 are not.
 */
#ifdef _M_IX86
#define G_MSVC_SYMBOL_PREFIX "_"
#else
#define G_MSVC_SYMBOL_PREFIX ""
#endif

#define G_DEFINE_CONSTRUCTOR(_func) G_MSVC_CTOR (_func, G_MSVC_SYMBOL_PREFIX)
#define G_DEFINE_DESTRUCTOR(_func) G_MSVC_DTOR (_func, G_MSVC_SYMBOL_PREFIX)

#define G_MSVC_CTOR(_func,_sym_prefix) \
  static void _func(void); \
  extern int (* _array ## _func)(void);              \
  int _func ## _wrapper(void) { _func(); g_slist_find (NULL,  _array ## _func); return 0; } \
  __pragma(comment(linker,"/include:" _sym_prefix # _func "_wrapper")) \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) int (* _array ## _func)(void) = _func ## _wrapper;

#define G_MSVC_DTOR(_func,_sym_prefix) \
  static void _func(void); \
  extern int (* _array ## _func)(void);              \
  int _func ## _constructor(void) { atexit (_func); g_slist_find (NULL,  _array ## _func); return 0; } \
   __pragma(comment(linker,"/include:" _sym_prefix # _func "_constructor")) \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) int (* _array ## _func)(void) = _func ## _constructor;

#elif defined (_MSC_VER)

#define G_HAS_CONSTRUCTORS 1

/* Pre Visual studio 2008 must use #pragma section */
#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  section(".CRT$XCU",read)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _wrapper(void) { _func(); return 0; } \
  __declspec(allocate(".CRT$XCU")) static int (*p)(void) = _func ## _wrapper;

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  section(".CRT$XCU",read)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _constructor(void) { atexit (_func); return 0; } \
  __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _constructor;

#elif defined(__SUNPRO_C)

/* This is not tested, but i believe it should work, based on:
 * http://opensource.apple.com/source/OpenSSL098/OpenSSL098-35/src/fips/fips_premain.c
 */

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  init(_func)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void);

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  fini(_func)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void);

#else

/* constructors not supported for this compiler */

#endif

#endif /* __GTK_DOC_IGNORE__ */

#ifdef G_HAS_CONSTRUCTORS

#ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(resource_constructor)
#endif
G_DEFINE_CONSTRUCTOR(resource_constructor)
#ifdef G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(resource_destructor)
#endif
G_DEFINE_DESTRUCTOR(resource_destructor)

#else
#warning "Constructor not supported on this compiler, linking in resources will not work"
#endif

static void resource_constructor (void)
{
  g_static_resource_init (&static_resource);
}

static void resource_destructor (void)
{
  g_static_resource_fini (&static_resource);
}
