GO_PROGRAM()

SRCS(
    launcher.go
    ssh.go
)

GO_EMBED_PATTERN(wrapper.tmpl.sh)

END()
