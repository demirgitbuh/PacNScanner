#include "platform/fake/FakePlatform.h"

// FakePlatform is header-only (inline). This translation unit exists so the
// target has a compiled object and the header is checked during the build.
namespace pacn {
namespace {
[[maybe_unused]] constexpr int kFakePlatformTranslationUnit = 0;
}
}  // namespace pacn
