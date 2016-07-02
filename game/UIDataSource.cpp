#include "ui_local.h"

using namespace UI;
using namespace Awesomium;

namespace UI {
	UIDataSource::UIDataSource() {
		// do nothing?
	}

	UIDataSource::~UIDataSource() {
		// do nothing?
	}

	void UIDataSource::OnRequest(int request_id, const WebString& path) {
		char pathBuf[128];
		path.ToUTF8(pathBuf, sizeof(pathBuf));
		Resource* pRes = Resource::ResourceSyncURI(pathBuf);
		if (pRes == nullptr) {
			R_Message(PRIORITY_WARNING, "UI: couldn't find resource %s\n", pathBuf);
			delete pRes;
			return;
		}
		AssetComponent* component = pRes->GetAssetComponent();
		if (component == nullptr) {
			R_Message(PRIORITY_WARNING, "UI: couldn't find resource %s\n", pathBuf);
			delete pRes;
			return;
		}
		if (component->meta.componentType != Asset_Data) {
			R_Message(PRIORITY_WARNING, "UI: resource (%s) is not raw\n", pathBuf);
			delete pRes;
			return;
		}
		ComponentData* data = component->data.dataComponent;
		SendResponse(request_id, component->meta.decompressedSize, (unsigned char*)data->data, WSLit(data->head.mime));
	}
}