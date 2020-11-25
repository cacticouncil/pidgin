#ifndef PURPLE_ZEPHYR_INTERNAL_H
#define PURPLE_ZEPHYR_INTERNAL_H

#include <sysdep.h>

#ifdef LIBZEPHYR_EXT
#include <zephyr/zephyr.h>
#else
#include <zephyr_internal.h>
#endif

#ifndef WIN32
#include <netdb.h>
#endif



#ifdef WIN32

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 512
#endif

#define ETIMEDOUT WSAETIMEDOUT
#define EADDRINUSE WSAEADDRINUSE
#else /* !WIN32 */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 4096
#endif

#endif

#ifdef ZEPHYR_USES_HESIOD
#include <hesiod.h>
#endif

#ifndef ZEPHYR_USES_KERBEROS
#define REALM_SZ	MAXHOSTNAMELEN
#define INST_SZ		0		/* no instances w/o Kerberos */
#define ANAME_SZ	9		/* size of a username + null */
#define CLOCK_SKEW	300		/* max time to cache packet ids */
#endif

#define SERVER_SVC_FALLBACK	htons((unsigned short) 2103)
#define HM_SVC_FALLBACK		htons((unsigned short) 2104)
#define HM_SRV_SVC_FALLBACK	htons((unsigned short) 2105)

#define ZAUTH_UNSET		(-3) /* Internal to client library. */
#define Z_MAXFRAGS		500	/* Max number of packet fragments */
#define Z_MAXNOTICESIZE		400000	/* Max size of incoming notice */
#define Z_MAXQUEUESIZE		1500000	/* Max size of input queue notices */
#define Z_FRAGFUDGE		13	/* Room to for multinotice field */
#define Z_NOTICETIMELIMIT	30	/* Time to wait for fragments */
#define Z_INITFILTERSIZE	30	/* Starting size of uid filter */

typedef struct {
	gint first;
	gint last;
} Z_Hole;

typedef struct {
    ZNotice_Kind_t	kind;
	gint64 time;
	gint packet_len;
	gchar *packet;
	gboolean complete;
    struct sockaddr_in	from;
	GSList *holelist; /* element-type: Z_Hole* */
    ZUnique_Id_t	uid;
    int			auth;
	gint header_len;
	gchar *header;
	gint msg_len;
	gchar *msg;
} Z_InputQ;

extern GQueue Z_input_queue;

extern ZLocations_t *__locate_list;
extern int __locate_num;
extern int __locate_next;

extern ZSubscription_t *__subscriptions_list;
extern int __subscriptions_num;
extern int __subscriptions_next;

extern int __Zephyr_port;		/* Port number */
extern struct in_addr __My_addr;

typedef Code_t (*Z_SendProc)(ZNotice_t *, char *, int, int);

Z_InputQ *Z_GetFirstComplete(void);
Z_InputQ *Z_GetNextComplete(Z_InputQ *);
Code_t Z_XmitFragment(ZNotice_t *, char *, int, int);
void Z_RemQueue(Z_InputQ *);
Code_t Z_AddNoticeToEntry(Z_InputQ *, ZNotice_t *, int);
Code_t Z_FormatAuthHeader(ZNotice_t *, char *, int, int *, Z_AuthProc);
Code_t Z_FormatHeader(ZNotice_t *, char *, int, int *, Z_AuthProc);
Code_t Z_FormatRawHeader(ZNotice_t *, char *, gsize, int *, char **, char **);
Code_t Z_ReadEnqueue(void);
Code_t Z_ReadWait(void);
Code_t Z_SendLocation(char *, char *, Z_AuthProc, char *);
Code_t Z_SendFragmentedNotice(ZNotice_t *notice, int len, Z_AuthProc cert_func,
                              Z_SendProc send_func);
Code_t Z_WaitForComplete(void);
Code_t Z_WaitForNotice(ZNotice_t *notice, int (*pred)(ZNotice_t *, void *),
                       void *arg, int timeout);

#endif /* PURPLE_ZEPHYR_INTERNAL_H */
