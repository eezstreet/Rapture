#include "sys_local.h"

namespace Network {
	namespace Packets {
		// Serverside handling
		namespace Server {
			void Packet_PingDeserialize(Packet& packet, int clientNum) {
				// Sort of semi-hack, it just queues up a PONG packet
				Network::Server::QueuePacket(PACKET_PONG, clientNum, nullptr);
				R_Message(PRIORITY_MESSAGE, "server ping from client %i\n");
			}

			void Packet_DropSerialize(Packet& packet, int clientNum, void* extraData) {
				// TODO: add a reason
				Network::Server::DropClient(clientNum);
			}

			void Packet_DropDeserialize(Packet& packet, int clientNum) {
				Network::Server::DropClient(clientNum);
			}

			void Packet_ClientAttemptDeserialize(Packet& packet, int clientNum) {
				// FIXME: should not be receiving these!!
			}

			void Packet_ClientAcceptSerialize(Packet& packet, int clientNum, void* extraData) {
				// FIXME: should not be sending these!!
			}

			void Packet_ClientDeniedSerialize(Packet& packet, int clientNum, void* extraData) {
				// FIXME: should not be sending these!!
			}
		}

		// Clientside handling
		namespace Client {
			void Packet_PingDeserialize(Packet& packet, int clientNum) {
				// Sort of semi-hack, it just queues up a PONG packet
				Network::Client::QueuePacket(PACKET_PONG, nullptr);
			}

			void Packet_DropSerialize(Packet& packet, int clientNum, void* extraData) {
				// TODO: give reason
			}

			void Packet_DropDeserialize(Packet& packet, int clientNum) {
				// TODO: give reason
			}

			void Packet_ClientAttemptSerialize(Packet& packet, int clientNum, void* extraData) {

			}

			void Packet_ClientAcceptDeserialize(Packet& packet, int clientNum) {

			}

			void Packet_ClientDeniedDeserialize(Packet& packet, int clientNum) {

			}
		}
	}
}