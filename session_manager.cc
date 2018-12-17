#include "session_manager.h"

namespace top {

namespace storage {

SessionManager::SessionManager() : map_mutex_(), client_map_(), io_service_(), work_(io_service_), asio_thread_() {
    asio_thread_.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_)));
}

SessionManager::~SessionManager() {
    asio_thread_.reset();
}

SessionManager* SessionManager::Instance() {
    static SessionManager ins;
    return &ins;
}

ClientAsioPtr SessionManager::NewSession(
        const std::string& ip,
        unsigned short port,
        uint32_t session_port,
        const std::string& local_id,
        const std::string& edge_id,
        maidsafe::routing::Network* network,
        int32_t message_id) {
    boost::mutex::scoped_lock lock(map_mutex_);
    auto iter = client_map_.find(session_port);
    if (iter != client_map_.end()) {
        if (iter->second->IsClosed()) {
            client_map_.erase(iter);
        } else {
            return iter->second;
        }
    }

    ClientAsioPtr client_ptr;
    try {
        client_ptr.reset(new CClientASIO(
            ip, port, io_service_, session_port, local_id, edge_id, network, message_id));
    } catch (std::exception& e) {
        std::cout << "catched error:" << e.what() << std::endl;
        return nullptr;
    }
    client_map_.insert(std::make_pair(session_port, client_ptr));
    return client_ptr;
}

ClientAsioPtr SessionManager::GetClient(uint32_t session_port, int32_t msg_id) {
    boost::mutex::scoped_lock lock(map_mutex_);
    auto iter = client_map_.find(session_port);
    if (iter != client_map_.end()) {
        if (iter->second->IsClosed()) {
            client_map_.erase(iter);
            return nullptr;
        }
        iter->second->SetMessageId(msg_id);
        return iter->second;
    }

    return nullptr;
}

}  // top

}  // storage
