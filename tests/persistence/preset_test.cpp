#include "test_support.h"

#include "persistence/preset_store.h"

#include <filesystem>
#include <fstream>

void test_presets() {
  const auto path = std::filesystem::temp_directory_path() / "bass-sim-test-presets.json";
  std::filesystem::remove(path);
  std::string error;
  CHECK(!bass::valid_preset_name(""));
  CHECK(!bass::valid_preset_name("   "));
  CHECK(!bass::valid_preset_name(" padded "));
  CHECK(!bass::valid_preset_name(std::string(81, 'x')));
  CHECK(!bass::valid_preset_name(std::string("\xc0\xaf", 2)));
  CHECK(bass::trim_preset_name("  Clean Bass  ") == "Clean Bass");
  CHECK(bass::valid_preset_name("Bäss"));
  CHECK(bass::valid_preset_name(std::string(80, 'a')));
  CHECK(bass::valid_preset_name("Clean Bass"));
  CHECK(!bass::save_presets(path, {{"Duplicate", bass::Parameters{}},
                                    {"Duplicate", bass::Parameters{}}}, error));
  CHECK(bass::save_presets(path, {{"Clean Bass", bass::Parameters{}}}, error));
  const auto result = bass::load_presets(path);
  CHECK(result.compatible);
  CHECK(result.presets.size() == 1);
  CHECK(result.presets.front().name == "Clean Bass");
  {
    std::ofstream output(path, std::ios::trunc);
    output << R"({"schema_version":1,"unused":"ignored","presets":[{"name":"Warm","sound":{"wet_mix":9,"input_gain_db":3,"tone_cutoff_hz":"bad"}},{"name":"Warm","sound":{}}]})";
  }
  const auto invalid_field = bass::load_presets(path);
  CHECK(invalid_field.compatible);
  CHECK(invalid_field.presets.size() == 1);
  CHECK(invalid_field.presets.front().sound.wet_mix == 1.f);
  CHECK(invalid_field.presets.front().sound.input_gain_db == 3.f);
  CHECK(invalid_field.presets.front().sound.tone_cutoff_hz == 2400.f);
  CHECK(!invalid_field.warnings.empty());
  {
    std::ofstream output(path, std::ios::trunc);
    output << R"({"schema_version":1,"presets":[{"name":"Unsafe","sound":{"input_gain_db":1e999}}]})";
  }
  const auto nonfinite = bass::load_presets(path);
  CHECK(!nonfinite.compatible);
  CHECK(!nonfinite.presets.empty());
  CHECK(nonfinite.presets.front().sound.input_gain_db == 0.f);
  {
    std::ofstream output(path, std::ios::trunc);
    output << R"({"schema_version":2,"presets":[]})";
  }
  CHECK(!bass::load_presets(path).compatible);
  CHECK(!bass::save_presets(path, {{"New", bass::Parameters{}}}, error));
  {
    std::ifstream input(path);
    std::string content((std::istreambuf_iterator<char>(input)), {});
    CHECK(content.find("schema_version\":2") != std::string::npos);
  }
  std::filesystem::remove(path);
}
