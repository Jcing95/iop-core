#!/bin/sh

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

IOPD=${IOPD:-$SRCDIR/bitcoind}
IOPCLI=${IOPCLI:-$SRCDIR/bitcoin-cli}
IOPTX=${IOPTX:-$SRCDIR/bitcoin-tx}
IOPQT=${IOPQT:-$SRCDIR/qt/bitcoin-qt}

[ ! -x $IOPD ] && echo "$IOPD not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
BTCVER=($($IOPCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for bitcoind if --version-string is not set,
# but has different outcomes for bitcoin-qt and bitcoin-cli.
echo "[COPYRIGHT]" > footer.h2m
$IOPD --version | sed -n '1!p' >> footer.h2m

for cmd in $IOPD $IOPCLI $IOPTX $IOPQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${BTCVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${BTCVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
