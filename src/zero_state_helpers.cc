#include "zero_state_helpers.h"

#include "boost/filesystem/operations.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/routing/bootstrap_file_operations.h"

namespace fs = boost::filesystem;

using namespace maidsafe;
using namespace maidsafe::routing;

namespace top {

namespace storage {

fs::path LocalNetworkBootstrapFile() {
	return fs::path(ThisExecutableDir() / "local_network_bootstrap.dat");
}

void WriteZeroStateBootstrapFile(const boost::asio::ip::udp::endpoint& contact0,
	const boost::asio::ip::udp::endpoint& contact1) {
	fs::path bootstrap_file{ maidsafe::routing::detail::GetOverrideBootstrapFilePath<false>() };
	fs::remove(bootstrap_file);
	WriteBootstrapContacts(BootstrapContacts{ contact0, contact1 }, bootstrap_file);
}

void WriteLocalNetworkBootstrapFile() {
	fs::remove(LocalNetworkBootstrapFile());
	WriteBootstrapContacts(
		BootstrapContacts(1, BootstrapContact(AsioToBoostAsio(GetLocalIp()), kLivePort)),
		LocalNetworkBootstrapFile());
}

}  // namespace storage

}  // namespace top
