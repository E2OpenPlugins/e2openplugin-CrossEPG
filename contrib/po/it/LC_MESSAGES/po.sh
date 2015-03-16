BINDIR="/home/malo/OEDev/Openpli/openpli4/openpli-oe-core/build/tmp/sysroots/x86_64-linux/usr/bin"

mv -f CrossEPG.mo CrossEPG.mo.old
$BINDIR/msgfmt -o CrossEPG.mo CrossEPG.po

