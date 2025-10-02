GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.36.0)

SRCS(
    emptyvfs.go
    fs.go
    namespace.go
    os.go
    vfs.go
)

END()

RECURSE(
    gatefs
    httpfs
    mapfs
    zipfs
)
