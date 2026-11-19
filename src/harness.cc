#include <cstdint>
#include <vector>
#include "h264_nal_stream.pb.h"
#include "src/libfuzzer/libfuzzer_macro.h"

namespace {

void WriteStartCode(std::vector<uint8_t>* out) {
  out->insert(out->end(), {0x00, 0x00, 0x00, 0x01});
}

void PackSpsToAnnexB(const fuzzer::h264::SeqParameterSet& sps, std::vector<uint8_t>* out) {
  WriteStartCode(out);
  out->push_back(0x67); // NAL_SPS
  out->push_back(static_cast<uint8_t>(sps.profile_idc() ? sps.profile_idc() : 100));
  out->push_back(0x00);
  out->push_back(static_cast<uint8_t>(sps.level_idc() ? sps.level_idc() : 41));
}

void PackPpsToAnnexB(const fuzzer::h264::PicParameterSet& pps, std::vector<uint8_t>* out) {
  WriteStartCode(out);
  out->push_back(0x68); // NAL_PPS
  out->push_back(static_cast<uint8_t>(pps.entropy_coding_mode_flag() ? 0xCE : 0x8E));
}

void PackSliceToAnnexB(const fuzzer::h264::SliceNalUnit& slice, std::vector<uint8_t>* out) {
  WriteStartCode(out);
  uint8_t nal_header = static_cast<uint8_t>(((slice.nal_ref_idc() & 0x3) << 5) | (slice.nal_unit_type() & 0x1F));
  out->push_back(nal_header ? nal_header : 0x65);
  const auto& payload = slice.rbsp_payload();
  out->insert(out->end(), payload.begin(), payload.end());
}

extern "C" void FeedToFFmpegDecoder(const uint8_t* data, size_t size);

}  // namespace

DEFINE_PROTO_FUZZER(const fuzzer::h264::AnnexBStream& stream) {
  std::vector<uint8_t> annexb_buf;
  annexb_buf.reserve(4096);
  PackSpsToAnnexB(stream.sps(), &annexb_buf);
  PackPpsToAnnexB(stream.pps(), &annexb_buf);
  for (const auto& slice : stream.slices()) {
    PackSliceToAnnexB(slice, &annexb_buf);
  }
  FeedToFFmpegDecoder(annexb_buf.data(), annexb_buf.size());
}
