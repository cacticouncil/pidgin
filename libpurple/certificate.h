/**
 * @file certificate.h Public-Key Certificate API
 * @ingroup core
 */

/*
 *
 * purple
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _PURPLE_CERTIFICATE_H
#define _PURPLE_CERTIFICATE_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _Certificate Certificate;
typedef struct _CertificateScheme CertificateScheme;

/** A certificate instance
 *
 *  An opaque data structure representing a single certificate under some
 *  CertificateScheme
 */
struct _Certificate
{
	/** Scheme this certificate is under */
	CertificateScheme * scheme;
	/** Opaque pointer to internal data */
	gpointer data;
};

/** A certificate type
 *
 *  A CertificateScheme must implement all of the fields in the structure,
 *  and register it using TODO:purple_register_certscheme()
 *
 *  There may be only ONE CertificateScheme provided for each certificate
 *  type, as specified by the "name" field.
 */
struct _CertificateScheme
{
	/** Name of the certificate type
	 *  ex: "x509", "pgp", etc.
	 *  This must be globally unique - you may not register more than one
	 *  CertificateScheme of the same name at a time.
	 */
	gchar * name;

	/** User-friendly name for this type
	 *  ex: N_("X.509 Certificates")
	 */
	gchar * fullname;

	/** Imports a certificate from a file
	 *
	 *  @param filename   File to import the certificate from
	 *  @return           Pointer to the newly allocated Certificate struct
	 *                    or NULL on failure.
	 */
	Certificate * (* import_certificate)(gchar * filename);

	/** Destroys and frees a Certificate structure
	 *
	 *  Destroys a Certificate's internal data structures and calls
	 *  free(crt)
	 *
	 *  @param crt  Certificate instance to be destroyed. It WILL NOT be
	 *              destroyed if it is not of the correct
	 *              CertificateScheme. Can be NULL
	 */
	void (* destroy_certificate)(Certificate * crt);
	
	/* TODO: Fill out this structure */
};


/*****************************************************************************/
/** @name PurpleCertificate Subsystem API                                    */
/*****************************************************************************/
/*@{*/

/* TODO: ADD STUFF HERE */

/*@}*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PURPLE_CERTIFICATE_H */
