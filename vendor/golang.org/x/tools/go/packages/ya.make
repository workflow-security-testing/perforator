GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.39.1-0.20251205192105-907593008619)

SRCS(
    doc.go
    external.go
    golist.go
    golist_overlay.go
    loadmode_string.go
    packages.go
    visit.go
)

END()

RECURSE(
    gopackages
    internal
)
