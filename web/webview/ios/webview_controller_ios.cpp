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

ErrCode WebviewControllerIOS::LoadData(const std::string& data, const std::string& mimeType,
    const std::string& encoding, const std::string& baseUrl, const std::string& historyUrl)
{
    loadDataOC(webId_, data, mimeType, encoding, baseUrl, historyUrl);
    return NO_ERROR;
}

std::string WebviewControllerIOS::GetUrl()
{
    return getUrlOC(webId_);
}

bool WebviewControllerIOS::AccessBackward()
{
    return accessBackwardOC(webId_);
}

bool WebviewControllerIOS::AccessForward()
{
    return accessForwardOC(webId_);
}

ErrCode WebviewControllerIOS::Backward()
{
    backwardOC(webId_);
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::Forward()
{
    forwardOC(webId_);
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::Refresh()
{
    refreshOC(webId_);
    return NO_ERROR;
}

void WebviewControllerIOS::EvaluateJavaScript(const std::string& script)
{
    EvaluateJavaScriptOC(webId_, script, WebviewController::OnReceiveValue);
}

bool WebviewControllerIOS::IsInit()
{
    return webId_ != -1;
}
} // namespace OHOS::Plugin
