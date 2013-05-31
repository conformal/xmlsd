# $OpenBSD: Makefile,v 1.5 2011/06/20 15:31:57 dhill Exp $

COMMENT=	XML validation library

DISTNAME=	xmlsd-0.10.1
EPOCH=		0
PKGNAME=	lib${DISTNAME}
CATEGORIES=    	textproc devel
SHARED_LIBS=	xmlsd	3.2

HOMEPAGE=	http://opensource.conformal.com/wiki/Xmlsd
MASTER_SITES=	http://opensource.conformal.com/snapshots/xmlsd/

MAINTAINER=	Conformal Systems LLC <info@conformal.com>

WANTLIB= 	expat

NO_TEST=	Yes

# ISC
PERMIT_PACKAGE_CDROM=	Yes

do-configure:
	printf "major=${LIBxmlsd_VERSION:R}\nminor=${LIBxmlsd_VERSION:E}\n" \
            > ${WRKSRC}/shlib_version

.include <bsd.port.mk>
