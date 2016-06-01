#include "tr_local.h"

renderImports_s *trap = nullptr;


void Initialize() {
}

void Shutdown() {

}

void Restart() {

}

static renderExports_s renderExport;
extern "C" {
	renderExports_s* GetRefAPI(renderImports_s* import) {
		trap = import;

		renderExport.Initialize = Initialize;
		renderExport.Shutdown = Shutdown;
		renderExport.Restart = Restart;

		return &renderExport;
	}
}