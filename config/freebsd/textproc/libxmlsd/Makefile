# New ports collection makefile for:    clens
# Date created:         March 5, 2012
# Whom:                 Dale Rahn <drahn@conformal.com>
#
# $FreeBSD$
#

PORTNAME=       libxmlsd
PORTVERSION=    0.7.0
CATEGORIES=     textproc
MASTER_SITES=   https://opensource.conformal.com/snapshots/xmlsd/ \
		LOCAL/bf
DISTNAME=       xmlsd-${PORTVERSION}

MAINTAINER=     drahn@conformal.com
COMMENT=        Xmlsd is an xml validation library

BUILD_DEPENDS=  ${LOCALBASE}/lib/libclens.a:${PORTSDIR}/devel/clens
LIB_DEPENDS+=	expat.6:${PORTSDIR}/textproc/expat2

LICENSE=        ISCL
LICENSE_COMB=   single

.include <bsd.port.mk>
