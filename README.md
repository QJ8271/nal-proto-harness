# nal-proto-harness

Structure-aware LibFuzzer + `libprotobuf-mutator` harness for stateful H.264/HEVC Annex-B NAL unit parsing (`FFmpeg` `libavcodec`).

## Architecture
- **Schema (`proto/h264_nal_stream.proto`)**: Anchors valid SPS (`0x67`) and PPS (`0x68`) initialization packets to bypass shallow header rejection (`ff_h264_decode_seq_parameter_set`), focusing mutation energy on slice headers and RBSP payloads.
- **Harness (`src/harness.cc`)**: Serializes mutated protobuf messages into Annex-B byte streams (`0x00000001` start codes + Exp-Golomb bit-packing) and feeds `avcodec_send_packet` under ASan/UBSan.

## Build (WIP)
```bash
mkdir build && cd build
cmake -DCMAKE_CXX_COMPILER=clang++ -DENABLE_ASAN=ON ..
ninja
