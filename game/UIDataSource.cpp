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
			// We still need to send a response, otherwise the game will hang
			SendResponse(request_id, 1, (unsigned char*)"\0", WSLit("text/plain"));
			return;
		}
		AssetComponent* component = pRes->GetAssetComponent();
		if (component == nullptr) {
			R_Message(PRIORITY_WARNING, "UI: couldn't find resource %s\n", pathBuf);
			delete pRes;
			SendResponse(request_id, 1, (unsigned char*)"\0", WSLit("text/plain"));
			return;
		}
		if (component->meta.componentType != Asset_Data) {
			R_Message(PRIORITY_WARNING, "UI: resource (%s) is not raw\n", pathBuf);
			delete pRes;
			SendResponse(request_id, 1, (unsigned char*)"\0", WSLit("text/plain"));
			return;
		}
		ComponentData* data = component->data.dataComponent;
		SendResponse(request_id, component->meta.decompressedSize, (unsigned char*)data->data, WSLit(data->head.mime));
	}
}