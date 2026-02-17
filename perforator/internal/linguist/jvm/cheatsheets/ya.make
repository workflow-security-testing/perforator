RECURSE(tool)

GO_LIBRARY()
RESOURCE(
    perforator/internal/linguist/jvm/cheatsheets/jdk25.txtpb jvm-cheatsheets/normal/jdk25.txtpb
    perforator/internal/linguist/jvm/cheatsheets/jdk25-min.txtpb jvm-cheatsheets/min/jdk25.txtpb
)
END()

