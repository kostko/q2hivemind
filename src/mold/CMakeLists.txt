
set(mold_src
message.cpp
server.cpp
client.cpp
)

PROTOBUF_GENERATE_CPP(mold_proto_src mold_proto_hdr
bus.proto
control.proto
)

add_library(mold STATIC ${mold_src} ${mold_proto_src})

