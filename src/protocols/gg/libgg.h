/* $Id: libgg.h 10052 2004-06-10 02:15:16Z thekingant $ */

/*
 *  (C) Copyright 2001 Wojtek Kaniewski <wojtekka@irc.pl>,
 *                     Robert J. Wo�ny <speedy@ziew.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __GG_LIBGG_H
#define __GG_LIBGG_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(sun) && !defined(INADDR_NONE)
  #define INADDR_NONE 0xffffffff
#endif

#include <sys/types.h>

/*
 * typ zmiennej okre�laj�cej numerek danej osoby.
 */
typedef unsigned long uin_t;

/*
 * struktura opisuj�ca dan� sesj�. tworzona przez gg_login().
 */
struct gg_session {
        int fd;         	/* podgl�dany deskryptor */
        int check;      	/* sprawdzamy zapis czy odczyt */
        int state;      	/* aktualny stan maszynki */
        int error;      	/* kod b��du dla GG_STATE_ERROR */
	int type;		/* rodzaj sesji. == GG_SESSION_GG */
	
        int async;      	/* czy po��czenie jest asynchroniczne */
	int pid;        	/* pid procesu resolvera */
	int port;       	/* port, z kt�rym si� ��czymy */
	int seq;        	/* numer sekwencyjny ostatniej wiadomo�ci */
	int last_pong;  	/* czas otrzymania ostatniego ping/pong */

	unsigned int server_ip;	/* adres serwera */
	unsigned int client_ip;	/* adres klienta */
	int client_port;	/* port, na kt�rym klient s�ucha */

	uin_t uin;		/* numerek klienta */
	char *password;		/* i jego has�o. zwalniane automagicznie */
        
	int initial_status;	/* pocz�tkowy stan klienta */

	char *recv_buf;		/* bufor na otrzymywane pakiety */
	int recv_done;		/* ile ju� wczytano do bufora */
        int recv_left;		/* i ile jeszcze trzeba wczyta� */
	
	char *userlist_reply;	/* fragment odpowiedzi listy kontakt�w */

	int userlist_blocks;	/* na ile kawa�k�w podzielono list� kontakt�w */
};

/*
 * og�lna struktura opisuj�ca stan wszystkich operacji http.
 */
struct gg_http {
        int fd;                 /* podgl�dany deskryptor */
        int check;              /* sprawdzamy zapis czy odczyt */
        int state;              /* aktualny stan maszynki */
        int error;              /* kod b��du dla GG_STATE_ERROR */
 	int type;		/* rodzaj sesji. == GG_SESSION_HTTP */
	
        int async;              /* czy po��czenie asynchroniczne */
	int pid;                /* pid procesu resolvera */
	int port;               /* port, z kt�rym si� ��czymy */

        char *query;            /* bufor zapytania http */
        char *header;           /* bufor nag��wka */
        int header_size;        /* rozmiar wczytanego nag��wka */
        char *body;             /* bufor otrzymanych informacji */
        int body_size;          /* ilo�� informacji */

        void *data;             /* dane danej operacji http */
};

/*
 * og�lna struktura opisuj�ca r�ne sesje. przydatna w klientach.
 */
struct gg_common {
        int fd;                 /* podgl�dany deskryptor */
        int check;              /* sprawdzamy zapis czy odczyt */
        int state;              /* aktualny stan maszynki */
        int error;              /* kod b��du dla GG_STATE_ERROR */
	int type;		/* rodzaj sesji */
};

/*
 * rodzaje sesji.
 */
enum {
	GG_SESSION_GG = 1,	/* po��czenie z serwerem gg */
	GG_SESSION_HTTP,	/* og�lna sesja http */
	GG_SESSION_SEARCH,	/* szukanie */
	GG_SESSION_REGISTER	/* rejestrowanie */
};

/*
 * r�ne stany asynchronicznej maszynki.
 */
enum {
        /* wsp�lne */
        GG_STATE_IDLE = 0,		/* nie powinno wyst�pi�. */
        GG_STATE_RESOLVING,             /* wywo�a� gethostbyname() */
	GG_STATE_CONNECTING,            /* wywo�a� connect() */
	GG_STATE_READING_DATA,		/* czeka na dane http */
	GG_STATE_ERROR,			/* wyst�pi� b��d. kod w x->error */

        /* gg_session */
	GG_STATE_CONNECTING_GG,         /* wywo�a� connect() */
	GG_STATE_READING_KEY,           /* czeka na klucz */
	GG_STATE_READING_REPLY,         /* czeka na odpowied� */
	GG_STATE_CONNECTED,             /* po��czy� si� */

        /* gg_http */
	GG_STATE_READING_HEADER,	/* czeka na nag��wek http */
	GG_STATE_PARSING,               /* przetwarza dane */
	GG_STATE_DONE                  /* sko�czy� */
};

/*
 * dla zachowania kompatybilno�ci wstecz. w wersji 1.0 b�dzie usuni�te. oby.
 */
#define GG_STATE_WRITING_HTTP GG_STATE_READING_DATA
#define GG_STATE_WAITING_FOR_KEY GG_STATE_READING_KEY
#define GG_STATE_SENDING_KEY GG_STATE_READING_REPLY
#define GG_STATE_FINISHED GG_STATE_DONE

/*
 * co proces klienta powinien sprawdza� w deskryptorach?
 */
enum {
	GG_CHECK_NONE = 0,		/* nic. nie powinno wyst�pi� */
	GG_CHECK_WRITE = 1,		/* sprawdzamy mo�liwo�� zapisu */
	GG_CHECK_READ = 2		/* sprawdzamy mo�liwo�� odczytu */
};

struct gg_session *gg_login(uin_t uin, char *password, int async);
void gg_free_session(struct gg_session *sess);
void gg_logoff(struct gg_session *sess);
int gg_write(struct gg_session *sess, const char *buf, int length);
int gg_change_status(struct gg_session *sess, int status);
int gg_send_message(struct gg_session *sess, int msgclass, uin_t recipient, const unsigned char *message);
int gg_send_message_richtext(struct gg_session *sess, int msgclass, uin_t recipient, const unsigned char *message, const unsigned char *format, int formatlen);
int gg_ping(struct gg_session *sess);
int gg_userlist_request(struct gg_session *sess, char type, const char *request);

struct gg_notify_reply {
	uin_t uin;			/* numerek */
	unsigned long status;		/* status danej osoby */
	unsigned long remote_ip;	/* adres ip delikwenta */
	unsigned short remote_port;	/* port, na kt�rym s�ucha klient */
	unsigned long version;		/* == 0x0b */
	unsigned short dunno2;		/* znowu port? */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_NOTIFY_REPLY60 0x0011
	
struct gg_notify_reply60 {
	uint32_t uin;			/* numerek plus flagi w MSB */
	uint8_t status;			/* status danej osoby */
	uint32_t remote_ip;		/* adres ip delikwenta */
	uint16_t remote_port;		/* port, na kt�rym s�ucha klient */
	uint8_t version;		/* wersja klienta */
	uint8_t image_size;		/* maksymalny rozmiar grafiki w KiB */
	uint8_t dunno1;			/* 0x00 */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_STATUS60 0x000f
	
struct gg_status60 {
	uint32_t uin;			/* numerek plus flagi w MSB */
	uint8_t status;			/* status danej osoby */
	uint32_t remote_ip;		/* adres ip delikwenta */
	uint16_t remote_port;		/* port, na kt�rym s�ucha klient */
	uint8_t version;		/* wersja klienta */
	uint8_t image_size;		/* maksymalny rozmiar grafiki w KiB */
	uint8_t dunno1;			/* 0x00 */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;


struct gg_status {
	uin_t uin;			/* numerek */
	unsigned long status;		/* nowy stan */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

enum {
	GG_EVENT_NONE = 0,
	GG_EVENT_MSG,
	GG_EVENT_NOTIFY,
	GG_EVENT_STATUS,
	GG_EVENT_ACK,
	GG_EVENT_CONN_FAILED,
	GG_EVENT_CONN_SUCCESS,
	GG_EVENT_STATUS60,		/* kto� zmieni� stan w GG 6.0 */
	GG_EVENT_NOTIFY60,		/* kto� si� pojawi� w GG 6.0 */
	GG_EVENT_USERLIST,		/* odpowied� listy kontakt�w w GG 6.0 */
};

/*
 * nied�ugo si� tego pozb�d� na rzecz sensownej obs�ugi b��d�w. --w
 */
enum {
	GG_FAILURE_RESOLVING = 1,
	GG_FAILURE_CONNECTING,
	GG_FAILURE_INVALID,
	GG_FAILURE_READING,
	GG_FAILURE_WRITING,
	GG_FAILURE_PASSWORD,
	GG_FAILURE_404
};

/*
 * rodzaje b��d�w, na razie u�ywane przez http. bez rozczulania si� nad
 * powodami. klient powie, �e albo nie znalaz� hosta, albo nie m�g� si�
 * po��czy�, albo nie m�g� wys�a�, albo nie m�g� odebrac. i tyle. jak
 * kto� b�dzie chcia�, to b�dzie m�g� sprawdzi� errno. ale po co?
 */
enum {
	GG_ERROR_RESOLVING = 1,
	GG_ERROR_CONNECTING,
	GG_ERROR_READING,
	GG_ERROR_WRITING
};

/*
 * struktura opisuj�ca rodzaj zdarzenia. wychodzi z gg_watch_fd()
 */
struct gg_event {
        int type;
        union {
                struct {
                        uin_t sender;
			int msgclass;
			time_t time;
                        unsigned char *message;
                } msg;
                struct gg_notify_reply *notify;
		struct {			/* @notify60 informacja o li�cie kontakt�w -- GG_EVENT_NOTIFY60 */
			uin_t uin;		/* numer */
			int status;	/* stan */
			uint32_t remote_ip;	/* adres ip */
			uint16_t remote_port;	/* port */
			int version;	/* wersja klienta */
			int image_size;	/* maksymalny rozmiar grafiki w KiB */
			char *descr;		/* opis stanu */
			time_t time;		/* czas powrotu */
		} *notify60;
                struct gg_status status;
		struct {			/* @status60 zmiana stanu -- GG_EVENT_STATUS60 */
			uin_t uin;		/* numer */
			int status;	/* nowy stan */
			uint32_t remote_ip;	/* adres ip */
			uint16_t remote_port;	/* port */
			int version;	/* wersja klienta */
			int image_size;	/* maksymalny rozmiar grafiki w KiB */
			char *descr;		/* opis stanu */
			time_t time;		/* czas powrotu */
		} status60;
		struct {			/* @userlist odpowied� listy kontakt�w serwera */
			char type;		/* rodzaj odpowiedzi */
			char *reply;		/* tre�� odpowiedzi */
		} userlist;
                struct {
                        uin_t recipient;
                        int status;
                        int seq;
                } ack;
		int failure;
        } event;
};

struct gg_event *gg_watch_fd(struct gg_session *sess);
void gg_free_event(struct gg_event *e);

int gg_notify(struct gg_session *sess, uin_t *userlist, int count);
int gg_add_notify(struct gg_session *sess, uin_t uin);
int gg_remove_notify(struct gg_session *sess, uin_t uin);


/*
 * OBS�UGA HTTP
 */

struct gg_http *gg_http_connect(char *hostname, int port, int async, char *method, char *path, char *header);
int gg_http_watch_fd(struct gg_http *h);
void gg_http_stop(struct gg_http *h);
void gg_free_http(struct gg_http *h);

/* 
 * SZUKANIE U�YTKOWNIK�W
 */

/*
 * struktura opisuj�ca kryteria wyszukiwania. argument gg_search().
 */
struct gg_search_request {
	int active;		/* czy ma szuka� tylko aktywnych? */

	/* mode 0 */
	char *nickname;		/* pseudonim */
	char *first_name;	/* imi� */
	char *last_name;	/* nazwisko */
	char *city;		/* miasto */
	int gender;		/* p�e� */
	int min_birth;		/* urodzony od roku... */
	int max_birth;		/* urodzony do roku... */
	
	/* mode 1 */
	char *email;		/* adres e-mail */

	/* mode 2 */
	char *phone;		/* numer telefonu */
	
	/* mode 3 */
	uin_t uin;		/* numerek */
};

/*
 * struktura opisuj�ca rezultat wyszukiwania. pole gg_http.
 */
struct gg_search {
	int count;				/* ilo�� znalezionych */
	struct gg_search_result *results;	/* tabelka z nimi */
};

/*
 * pojedynczy rezultat wyszukiwania.
 */
struct gg_search_result {
	uin_t uin;		/* numerek */
	char *first_name;	/* imi� */
	char *last_name;	/* nazwisko */
	char *nickname;		/* pseudonim */
	int born;		/* rok urodzenia */
	int gender;		/* p�e� */
	char *city;		/* miasto */
	int active;		/* czy jest aktywny */
};

#define GG_GENDER_NONE 0	/* nie podano lub bez znaczenia */
#define GG_GENDER_FEMALE 1	/* kobieta */
#define GG_GENDER_MALE 2	/* m�czyzna */

struct gg_http *gg_search(struct gg_search_request *r, int async);
int gg_search_watch_fd(struct gg_http *f);
void gg_free_search(struct gg_http *f);

struct gg_search_request *gg_search_request_mode_0(char *nickname, char *first_name, char *last_name, char *city, int gender, int min_birth, int max_birth, int active);
struct gg_search_request *gg_search_request_mode_1(char *email, int active);
struct gg_search_request *gg_search_request_mode_2(char *phone, int active);
struct gg_search_request *gg_search_request_mode_3(uin_t uin, int active);

/*
 * OPERACJE NA KATALOGU PUBLICZNYM
 */

struct gg_pubdir {
	int success;		/* czy si� uda�o */
	uin_t uin;		/* otrzymany numerek. 0 je�li b��d */
};

struct gg_http *gg_register(char *email, char *password, int async);
void gg_free_register(struct gg_http *f);

int gg_pubdir_watch_fd(struct gg_http *f);
#define gg_register_watch_fd gg_pubdir_watch_fd

/*
 * je�li chcemy sobie podebugowa�, wystarczy ustawi� `gg_debug_level'.
 * niestety w miar� przybywania wpis�w `gg_debug(...)' nie chcia�o mi
 * si� ustawia� odpowiednich leveli, wi�c wi�kszo�� sz�a do _MISC.
 */

extern int gg_debug_level;

#define GG_DEBUG_NET 1
#define GG_DEBUG_TRAFFIC 2
#define GG_DEBUG_DUMP 4
#define GG_DEBUG_FUNCTION 8
#define GG_DEBUG_MISC 16

void gg_debug(int level, char *format, ...);

/*
 * Pare ma�ych zmiennych do obs�ugi "http proxy"
 *   
 */
 
extern int gg_http_use_proxy;
extern char *gg_http_proxy_host;
extern int gg_http_proxy_port;

/*
 * -------------------------------------------------------------------------
 * poni�ej znajduj� si� wewn�trzne sprawy biblioteki. zwyk�y klient nie
 * powinien ich w og�le rusza�, bo i nie ma po co. wszystko mo�na za�atwi�
 * procedurami wy�szego poziomu, kt�rych definicje znajduj� si� na pocz�tku
 * tego pliku.
 * -------------------------------------------------------------------------
 */

int gg_resolve(int *fd, int *pid, char *hostname);
void gg_debug(int level, char *format, ...);
char *gg_alloc_sprintf(char *format, ...);
char *gg_get_line(char **ptr);
int gg_connect(void *addr, int port, int async);
void gg_read_line(int sock, char *buf, int length);
void gg_chomp(char *line);
char *gg_urlencode(const char *str);
int gg_http_hash(const unsigned char *email, const unsigned char *password);

#define GG_APPMSG_HOST "appmsg.gadugadu.pl"
#define GG_APPMSG_PORT 80
#define GG_PUBDIR_HOST "pubdir.gadugadu.pl"
#define GG_PUBDIR_PORT 80
#define GG_REGISTER_HOST "register.gadugadu.pl"
#define GG_REGISTER_PORT 80
#define GG_DEFAULT_PORT 8074
#define GG_HTTPS_PORT 443
#define GG_HTTP_USERAGENT "Mozilla/4.0 (compatible MSIE 5.0; Windows 98; I)"
#define GG_HAS_AUDIO_MASK 0x40000000

struct gg_header {
	unsigned long type;		/* typ pakietu */
	unsigned long length;		/* d�ugo�� reszty pakietu */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_WELCOME 0x0001

struct gg_welcome {
	unsigned long key;		/* klucz szyfrowania has�a */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;
	
#define GG_LOGIN 0x000c

struct gg_login {
	uin_t uin;			/* tw�j numerek */
	unsigned long hash;		/* hash has�a */
	unsigned long status;		/* status na dzie� dobry */
	unsigned long version;		/* == 0x20 */
	unsigned long local_ip;		/* m�j adres ip */
	unsigned short local_port;	/* port, na kt�rym s�ucham */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_LOGIN60 0x0015

struct gg_login60 {
	uint32_t uin;			/* m�j numerek */
	uint32_t hash;			/* hash has�a */
	uint32_t status;		/* status na dzie� dobry */
	uint32_t version;		/* moja wersja klienta */
	uint8_t dunno1;			/* 0x00 */
	uint32_t local_ip;		/* m�j adres ip */
	uint16_t local_port;		/* port, na kt�rym s�ucham */
	uint32_t external_ip;		/* zewn�trzny adres ip */
	uint16_t external_port;		/* zewn�trzny port */
	uint8_t image_size;		/* maksymalny rozmiar grafiki w KiB */
	uint8_t dunno2;			/* 0xbe */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_LOGIN_OK 0x0003

#define GG_LOGIN_FAILED 0x0009

#define GG_NEW_STATUS 0x0002

#define GG_STATUS_NOT_AVAIL 0x0001		/* niedost�pny */
#define GG_STATUS_NOT_AVAIL_DESCR 0x0015	/* niedost�pny z opisem (4.8) */
#define GG_STATUS_AVAIL 0x0002			/* dost�pny */
#define GG_STATUS_AVAIL_DESCR 0x0004		/* dost�pny z opisem (4.9) */
#define GG_STATUS_BUSY 0x0003			/* zaj�ty */
#define GG_STATUS_BUSY_DESCR 0x0005		/* zaj�ty z opisem (4.8) */
#define GG_STATUS_INVISIBLE 0x0014		/* niewidoczny (4.6) */
#define GG_STATUS_INVISIBLE_DESCR 0x0016	/* niewidoczny z opisem (4.9) */
#define GG_STATUS_BLOCKED 0x0006		/* zablokowany */

#define GG_STATUS_FRIENDS_MASK 0x8000		/* tylko dla znajomych (4.6) */

/* GG_S() stan bez uwzgl�dnienia trybu tylko dla znajomych */
#define GG_S(x) ((x) & ~GG_STATUS_FRIENDS_MASK)

/* GG_S_D() stan opisowy */
#define GG_S_D(x) (GG_S(x) == GG_STATUS_NOT_AVAIL_DESCR || GG_S(x) == GG_STATUS_AVAIL_DESCR || GG_S(x) == GG_STATUS_BUSY_DESCR || GG_S(x) == GG_STATUS_INVISIBLE_DESCR)

struct gg_new_status {
	unsigned long status;			/* na jaki zmieni�? */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_NOTIFY 0x0010
	
struct gg_notify {
	uin_t uin;		/* numerek danej osoby */
	char dunno1;		/* == 3 */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;
	
#define GG_NOTIFY_REPLY 0x000c	/* tak, to samo co GG_LOGIN */
	
/* struct gg_notify_reply zadeklarowane wy�ej */

#define GG_ADD_NOTIFY 0x000d
#define GG_REMOVE_NOTIFY 0x000e
	
struct gg_add_remove {
	uin_t uin;		/* numerek */
	char dunno1;		/* == 3 */
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_STATUS 0x0002

/* struct gg_status zadeklarowane wcze�niej */
	
#define GG_SEND_MSG 0x000b

#define GG_CLASS_QUEUED 0x0001
#define GG_CLASS_OFFLINE GG_CLASS_QUEUED
#define GG_CLASS_MSG 0x0004
#define GG_CLASS_CHAT 0x0008

struct gg_send_msg {
	unsigned long recipient;
	unsigned long seq;
	unsigned long msgclass;
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_SEND_MSG_ACK 0x0005

#define GG_ACK_DELIVERED 0x0002
#define GG_ACK_QUEUED 0x0003
	
struct gg_send_msg_ack {
	unsigned long status;
	unsigned long recipient;
	unsigned long seq;
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_RECV_MSG 0x000a
	
struct gg_recv_msg {
	unsigned long sender;
	unsigned long seq;
	unsigned long time;
	unsigned long msgclass;
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_PING 0x0008
	
#define GG_PONG 0x0007

#define GG_USERLIST_REQUEST 0x0016

#define GG_USERLIST_PUT 0x00
#define GG_USERLIST_PUT_MORE 0x01
#define GG_USERLIST_GET 0x02

struct gg_userlist_request {
	uint8_t type;
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

#define GG_USERLIST_REPLY 0x0010

#define GG_USERLIST_PUT_REPLY 0x00
#define GG_USERLIST_PUT_MORE_REPLY 0x02
#define GG_USERLIST_GET_REPLY 0x06
#define GG_USERLIST_GET_MORE_REPLY 0x04

struct gg_userlist_reply {
	uint8_t type;
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;

/* listy */

struct list {
	void *data;
	struct list *next;
};

typedef struct list * list_t;


#ifdef __cplusplus
}
#endif

#endif /* __GG_LIBGG_H */

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: notnil
 * End:
 *
 * vim: shiftwidth=8:
 */
