rm -rf submission
rm xmarty07.zip
mkdir submission
cp src/* submission/

cat <<'EOF' > submission/Makefile
CC=gcc
CFLAGS=-g -std=c99

ifj21: *.c *.h
	$(CC) $(CFLAGS) *.c -o ifj21
EOF

touch submission/dokumentace.pdf

cat <<'EOF' > submission/rozdeleni
xmarty07:33
xstolf00:33
xkoryt04:33
EOF

zip -jrm xmarty07.zip submission/*
