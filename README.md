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
#### 2. `proto/h264_nal_stream.proto`
```protobuf
syntax = "proto3";
package fuzzer.h264;

message SeqParameterSet {
  uint32 profile_idc = 1;          // 66 (Baseline), 77 (Main), 100 (High)
  uint32 level_idc = 2;            // e.g., 31, 41, 51
  uint32 seq_parameter_set_id = 3;
  uint32 log2_max_frame_num_minus4 = 4;
  uint32 pic_order_cnt_type = 5;
  uint32 max_num_ref_frames = 6;
  uint32 pic_width_in_mbs_minus1 = 7;
  uint32 pic_height_in_map_units_minus1 = 8;
}

message PicParameterSet {
  uint32 pic_parameter_set_id = 1;
  uint32 seq_parameter_set_id = 2;
  bool entropy_coding_mode_flag = 3;
  uint32 num_ref_idx_l0_default_active_minus1 = 4;
  int32 pic_init_qp_minus26 = 5;
}

message SliceNalUnit {
  uint32 nal_ref_idc = 1;
  uint32 nal_unit_type = 2;        // 1 (Non-IDR), 5 (IDR), 6 (SEI)
  uint32 first_mb_in_slice = 3;
  uint32 slice_type = 4;
  uint32 frame_num = 5;
  bytes rbsp_payload = 6;          // Mutated macroblock / CAVLC / CABAC data
}

message AnnexBStream {
  SeqParameterSet sps = 1;         // Anchored valid SPS
  PicParameterSet pps = 2;         // Anchored valid PPS
  repeated SliceNalUnit slices = 3;
}
