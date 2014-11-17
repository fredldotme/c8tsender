#include "playlist.hpp"
#include "chromecast.hpp"
#include "webserver.hpp"
#include "cast_channel.pb.h"

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("%s <ip-of-chromecast>\n", argv[0]);
		return 1;
	}

	GOOGLE_PROTOBUF_VERIFY_VERSION;
	OpenSSL_add_ssl_algorithms();
 	SSL_load_error_strings();

	Playlist playlist;
	ChromeCast chromecast(argv[1]);
	chromecast.init();
	chromecast.setMediaStatusCallback([&chromecast, &playlist](const std::string& playerState,
			const std::string& idleReason, const std::string& uuid) -> void {
		printf("mediastatus: %s %s %s\n", playerState.c_str(), idleReason.c_str(), uuid.c_str());
		if (playerState == "IDLE") {
			if (idleReason == "FINISHED") {
				try {
					PlaylistItem track = playlist.getNextTrack(uuid);
					std::thread foo([&chromecast, track](){
						chromecast.load("http://" + chromecast.getSocketName() + ":8080/stream/" + track.getUUID(),
							track.getName(), track.getUUID());
					});
					foo.detach();
				} catch (...) {
					// ...
				}
			}
		}
	});
	Webserver http(8080, chromecast, playlist);
	while (1) sleep(3600);
	return 0;
}
