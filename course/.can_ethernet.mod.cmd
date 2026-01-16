savedcmd_can_ethernet.mod := printf '%s\n'   can_ethernet.o | awk '!x[$$0]++ { print("./"$$0) }' > can_ethernet.mod
