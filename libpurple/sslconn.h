/**
 * @file sslconn.h SSL API
 * @ingroup core
 */

/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */
#ifndef _PURPLE_SSLCONN_H_
#define _PURPLE_SSLCONN_H_

/** Possible SSL errors. */
typedef enum
{
	PURPLE_SSL_HANDSHAKE_FAILED = 1,
	PURPLE_SSL_CONNECT_FAILED = 2,
	PURPLE_SSL_CERTIFICATE_INVALID = 3
} PurpleSslErrorType;

#include "certificate.h"
#include "proxy.h"

#define PURPLE_SSL_DEFAULT_PORT 443

/** @copydoc _PurpleSslConnection */
typedef struct _PurpleSslConnection PurpleSslConnection;

typedef void (*PurpleSslInputFunction)(gpointer, PurpleSslConnection *,
									 PurpleInputCondition);
typedef void (*PurpleSslErrorFunction)(PurpleSslConnection *, PurpleSslErrorType,
									 gpointer);

struct _PurpleSslConnection
{
	/** Hostname to which the SSL connection will be made */
	char *host;
	/** Port to connect to */
	int port;
	/** Data to pass to PurpleSslConnection::connect_cb() */
	void *connect_cb_data;
	/** Callback triggered once the SSL handshake is complete */
	PurpleSslInputFunction connect_cb;
	/** Callback triggered if there is an error during connection */
	PurpleSslErrorFunction error_cb;
	/** Data passed to PurpleSslConnection::recv_cb() */
	void *recv_cb_data;
	/** User-defined callback executed when the SSL connection receives data */
	PurpleSslInputFunction recv_cb;

	/** File descriptor used to refer to the socket */
	int fd;
	/** Glib event source ID; used to refer to the received data callback
	 * in the glib eventloop */
	guint inpa;
	/** Data related to the underlying TCP connection */
	PurpleProxyConnectData *connect_data;

	/** Internal connection data managed by the SSL backend (GnuTLS/LibNSS/whatever) */
	void *private_data;

	/** Verifier to use in authenticating the peer */
	PurpleCertificateVerifier *verifier;
};

/**
 * SSL implementation operations structure.
 *
 * Every SSL implementation must provide all of these and register it via purple_ssl_set_ops()
 * These should not be called directly! Instead, use the purple_ssl_* functions.
 */
typedef struct
{
	/** Initializes the SSL system provided.
	 *  Returns: @a TRUE if initialization succeeded
	 *  @see purple_ssl_init
	 */
	gboolean (*init)(void);
	/** Unloads the SSL system. Inverse of PurpleSslOps::init.
	 *  @see purple_ssl_uninit
	 */
	void (*uninit)(void);
	/** Sets up the SSL connection for a #PurpleSslConnection once
	 *  the TCP connection has been established
	 *  @see purple_ssl_connect
	 */
	void (*connectfunc)(PurpleSslConnection *gsc);
	/** Destroys the internal data of the SSL connection provided.
	 *  Freeing gsc itself is left to purple_ssl_close()
	 *  @see purple_ssl_close
	 */
	void (*close)(PurpleSslConnection *gsc);
	/** Reads data from a connection (like POSIX read())
	 * @gsc:   Connection context
	 * @data:  Pointer to buffer to drop data into
	 * @len:   Maximum number of bytes to read
	 * Returns:      Number of bytes actually written into @a data (which may be
	 *              less than @a len), or <0 on error
	 * @see purple_ssl_read
	*/
	size_t (*read)(PurpleSslConnection *gsc, void *data, size_t len);
	/** Writes data to a connection (like POSIX send())
	* @gsc:    Connection context
	* @data:   Data buffer to send data from
	* @len:    Number of bytes to send from buffer
	* Returns:       The number of bytes written to @a data (may be less than
	*               @a len) or <0 on error
	* @see purple_ssl_write
	*/
	size_t (*write)(PurpleSslConnection *gsc, const void *data, size_t len);
	/** Obtains the certificate chain provided by the peer
	 *
	 * @gsc:   Connection context
	 * Returns:      A newly allocated list containing the certificates
	 *              the peer provided.
	 * @see PurpleCertificate
	 * @todo        Decide whether the ordering of certificates in this
	 *              list can be guaranteed.
	 */
	GList * (* get_peer_certificates)(PurpleSslConnection * gsc);

	/*< private >*/
	void (*_purple_reserved1)(void);
	void (*_purple_reserved2)(void);
	void (*_purple_reserved3)(void);
	void (*_purple_reserved4)(void);
} PurpleSslOps;

G_BEGIN_DECLS

/**************************************************************************/
/** @name SSL API                                                         */
/**************************************************************************/
/*@{*/

/**
 * Returns whether or not SSL is currently supported.
 *
 * Returns: @a TRUE if SSL is supported, or @a FALSE otherwise.
 */
gboolean purple_ssl_is_supported(void);

/**
 * Returns a human-readable string for an SSL error.
 *
 * @error:      Error code
 * Returns: Human-readable error explanation
 */
const gchar * purple_ssl_strerror(PurpleSslErrorType error);

/**
 * Makes a SSL connection to the specified host and port.  The caller
 * should keep track of the returned value and use it to cancel the
 * connection, if needed.
 *
 * @account:    The account making the connection.
 * @host:       The destination host.
 * @port:       The destination port.
 * @func:       The SSL input handler function.
 * @error_func: The SSL error handler function.  This function
 *                   should <strong>NOT</strong> call purple_ssl_close().  In
 *                   the event of an error the #PurpleSslConnection will be
 *                   destroyed for you.
 * @data:       User-defined data.
 *
 * Returns: The SSL connection handle.
 */
PurpleSslConnection *purple_ssl_connect(PurpleAccount *account, const char *host,
									int port, PurpleSslInputFunction func,
									PurpleSslErrorFunction error_func,
									void *data);

/**
 * Makes a SSL connection to the specified host and port, using the separate
 * name to verify with the certificate.  The caller should keep track of the
 * returned value and use it to cancel the connection, if needed.
 *
 * @account:    The account making the connection.
 * @host:       The destination host.
 * @port:       The destination port.
 * @func:       The SSL input handler function.
 * @error_func: The SSL error handler function.  This function
 *                   should <strong>NOT</strong> call purple_ssl_close().  In
 *                   the event of an error the #PurpleSslConnection will be
 *                   destroyed for you.
 * @ssl_host:   The hostname of the other peer (to verify the CN)
 * @data:       User-defined data.
 *
 * Returns: The SSL connection handle.
 */
PurpleSslConnection *purple_ssl_connect_with_ssl_cn(PurpleAccount *account, const char *host,
									int port, PurpleSslInputFunction func,
									PurpleSslErrorFunction error_func,
									const char *ssl_host,
									void *data);

/**
 * Makes a SSL connection using an already open file descriptor.
 *
 * @account:    The account making the connection.
 * @fd:         The file descriptor.
 * @func:       The SSL input handler function.
 * @error_func: The SSL error handler function.
 * @host:       The hostname of the other peer (to verify the CN)
 * @data:       User-defined data.
 *
 * Returns: The SSL connection handle.
 */
PurpleSslConnection *purple_ssl_connect_with_host_fd(PurpleAccount *account, int fd,
                                           PurpleSslInputFunction func,
                                           PurpleSslErrorFunction error_func,
                                           const char *host,
                                           void *data);

/**
 * Adds an input watcher for the specified SSL connection.
 * Once the SSL handshake is complete, use this to watch for actual data across it.
 *
 * @gsc:   The SSL connection handle.
 * @func:  The callback function.
 * @data:  User-defined data.
 */
void purple_ssl_input_add(PurpleSslConnection *gsc, PurpleSslInputFunction func,
						void *data);

/**
 * Removes an input watcher, added with purple_ssl_input_add().
 *
 * If there is no input watcher set, does nothing.
 *
 * @gsc: The SSL connection handle.
 */
void
purple_ssl_input_remove(PurpleSslConnection *gsc);

/**
 * Closes a SSL connection.
 *
 * @gsc: The SSL connection to close.
 */
void purple_ssl_close(PurpleSslConnection *gsc);

/**
 * Reads data from an SSL connection.
 *
 * @gsc:    The SSL connection handle.
 * @buffer: The destination buffer.
 * @len:    The maximum number of bytes to read.
 *
 * Returns: The number of bytes read.
 */
size_t purple_ssl_read(PurpleSslConnection *gsc, void *buffer, size_t len);

/**
 * Writes data to an SSL connection.
 *
 * @gsc:    The SSL connection handle.
 * @buffer: The buffer to write.
 * @len:    The length of the data to write.
 *
 * Returns: The number of bytes written.
 */
size_t purple_ssl_write(PurpleSslConnection *gsc, const void *buffer, size_t len);

/**
 * Obtains the peer's presented certificates
 *
 * @gsc:    The SSL connection handle
 *
 * Returns: The peer certificate chain, in the order of certificate, issuer,
 *         issuer's issuer, etc. @a NULL if no certificates have been provided,
 */
GList * purple_ssl_get_peer_certificates(PurpleSslConnection *gsc);

/*@}*/

/**************************************************************************/
/** @name Subsystem API                                                   */
/**************************************************************************/
/*@{*/

/**
 * Sets the current SSL operations structure.
 *
 * @ops: The SSL operations structure to assign.
 */
void purple_ssl_set_ops(PurpleSslOps *ops);

/**
 * Returns the current SSL operations structure.
 *
 * Returns: The SSL operations structure.
 */
PurpleSslOps *purple_ssl_get_ops(void);

/**
 * Initializes the SSL subsystem.
 */
void purple_ssl_init(void);

/**
 * Uninitializes the SSL subsystem.
 */
void purple_ssl_uninit(void);

/*@}*/

G_END_DECLS

#endif /* _PURPLE_SSLCONN_H_ */
