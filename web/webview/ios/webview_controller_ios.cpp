#include "webview_controller_ios.h"

#include "AceWebControllerBridge.h"
#include "log.h"
using namespace OHOS::NWebError;

namespace OHOS::Plugin {
WebviewControllerIOS::~WebviewControllerIOS() {}

ErrCode WebviewControllerIOS::LoadUrl(const std::string& url)
{
    loadUrlOC(webId_, url, std::map<std::string, std::string>());
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::LoadUrl(const std::string& url, const std::map<std::string, std::string>& httpHeaders)
{
    loadUrlOC(webId_, url, httpHeaders);
    return NO_ERROR;
}
bool WebviewControllerIOS::IsInit()
{
    return webId_ != -1;
}
} // namespace OHOS::Plugin