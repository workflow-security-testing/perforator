GO_PROGRAM()

SUBSCRIBER(g:perforator)

SRCS(
    launcher.go
    ssh.go
)

GO_EMBED_PATTERN(wrapper.tmpl.sh)

END()
