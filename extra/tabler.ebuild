inherit eutils

DESCRIPTION="A utility to create text art tables from delimited input."
HOMEPAGE="http://sourceforge.net/projects/tabler/"
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.gz"

LICENSE="GPL"
SLOT="0"
KEYWORDS="~alpha amd64 ~hppa ~mips ppc ~ppc64 ~sparc x86"
IUSE=""

src_unpack() {
	unpack ${A}
}

src_compile() {
	econf || die "econf failed"
	emake || die "emake failed"
}

src_install() {
	make DESTDIR="${D}" install || die "make install failed"
	dodoc AUTHORS ChangeLog README
}
