#include <QtGlobal>

#include "core/PlatformServices.h"

#if defined(Q_OS_WIN)
#include "platform/windows/WindowsPlatform.h"
#else
#include "platform/generic/GenericUnixPlatform.h"
#endif

namespace pacn {

std::shared_ptr<IPlatformServices> createPlatformServices() {
#if defined(Q_OS_WIN)
    return std::make_shared<WindowsPlatform>();
#else
    return std::make_shared<GenericUnixPlatform>();
#endif
}

}  // namespace pacn
