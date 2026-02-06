GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.39.1-0.20251205192105-907593008619)

SRCS(
    bimport.go
    exportdata.go
    gcimporter.go
    iexport.go
    iimport.go
    predeclared.go
    support.go
    ureader_yes.go
)

END()

RECURSE(
    # gotest # st/YMAKE-102
)
