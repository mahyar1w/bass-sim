#include "test_support.h"

#include "persistence/config_store.h"

#include <filesystem>
#include <fstream>

void test_config() {
  const auto path = std::filesystem::temp_directory_path() / "bass-sim-test-config.json";
  std::filesystem::remove(path);
  const auto missing = bass::load_config(path);
  CHECK(missing.compatible);
  CHECK(missing.value.capture_endpoint_id.empty());
  std::string error;
  bass::UserConfig config;
  config.capture_endpoint_id = "guitar";
  config.playback_endpoint_id = "speakers";
  CHECK(bass::save_config(path, config, error));
  auto loaded = bass::load_config(path);
  CHECK(loaded.compatible);
  CHECK(loaded.value.capture_endpoint_id == "guitar");
  CHECK(loaded.value.playback_endpoint_id == "speakers");
  {
    std::ofstream output(path, std::ios::trunc);
    output << R"({"schema_version":1,"capture_endpoint_id":"missing","playback_endpoint_id":"","requested_sample_rate_hz":12345,"requested_buffer_frames":999})";
  }
  loaded = bass::load_config(path);
  CHECK(loaded.compatible);
  CHECK(loaded.value.capture_endpoint_id == "missing"); // A hint, not a selected session ID.
  CHECK(loaded.value.requested_sample_rate_hz == 48000);
  CHECK(loaded.value.requested_buffer_frames == 128);
  CHECK(!loaded.warnings.empty());
  {
    std::ofstream output(path, std::ios::trunc);
    output << "broken JSON";
  }
  CHECK(!bass::load_config(path).compatible);
  CHECK(!bass::save_config(path, config, error));
  {
    std::ifstream input(path);
    std::string content((std::istreambuf_iterator<char>(input)), {});
    CHECK(content == "broken JSON");
  }
  const auto unwritable = path / "child.json";
  CHECK(!bass::save_config(unwritable, config, error));
  std::filesystem::remove(path);
}
