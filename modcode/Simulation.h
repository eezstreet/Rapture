#pragma once

// The simulation layer runs on both the client and the server.

namespace Simulation {
	void Initialize(const char* szSaveGamePath);
	void Shutdown();
}