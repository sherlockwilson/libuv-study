#include "asio_client.h"
#include "message.pb.h"
#include "utils.h"
#include "log.h"

using boost::asio::io_service;
using namespace maidsafe;
using namespace maidsafe::routing;

namespace top {
namespace storage {

    CClientASIO::CClientASIO(
        const std::string& hostname,
        unsigned short port,
        boost::asio::io_service& io_service,
        uint32_t local_port,
        const std::string& local_id,
        const std::string& edge_id,
        maidsafe::routing::Network* network,
        int32_t message_id)
        :  socket_(io_service),
        local_port_(local_port),
        local_id_(local_id),
        edge_id_(edge_id),
        network_(network),
        message_id_(message_id),
        closed_(false) {
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::system::error_code ec;
    boost::asio::ip::tcp::resolver::query query(hostname, boost::lexical_cast<std::string, unsigned short>(port));
    boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query, ec);
    boost::asio::ip::tcp::resolver::iterator end;

    // pick the first endpoint     
    if (iter != end && ec == boost::system::error_code()) {
        boost::asio::ip::tcp::endpoint endpoint = *iter;
        socket_.connect(endpoint, ec);
        memset(m_data, 0, sizeof(m_data));
        socket_.async_read_some(boost::asio::buffer(m_data, _RECV_BUF_SIZE_),
            boost::bind(&CClientASIO::OnRecv, this,
                boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        if (ec) {
            std::cerr << "Error: " << ec << std::endl;
            throw ec;
        }
    }
}

CClientASIO::~CClientASIO() {
}

void CClientASIO::PostSend(const std::string & message) {
    socket_.async_write_some(boost::asio::buffer(message.c_str(), message.size()),
        boost::bind(&CClientASIO::OnSend, this,
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

bool CClientASIO::OnRecv(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        //prepare response
        RoutingMessage proto_message;
        protobuf::VpnPutDataResponse vpn_put_data_res;
        vpn_put_data_res.set_data((char *)m_data, bytes_transferred);
        std::cout << "res length: " << bytes_transferred << std::endl;
        vpn_put_data_res.set_local_port(local_port_);

        std::string rdata;
        if (!vpn_put_data_res.SerializeToString(&rdata)) {
            ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        } else {
            proto_message.set_source_id(local_id_);
            proto_message.set_destination_id(edge_id_);
            proto_message.set_routing_message(false);
            proto_message.add_data(rdata);
            proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
            proto_message.set_direct(true);
            proto_message.set_top_type(static_cast<int32_t>(
                TopStorageMessageType::kVpnPutDataResponse));
            proto_message.set_cacheable(static_cast<int32_t>(0));
            proto_message.set_client_node(false);
            proto_message.set_request(false);
            proto_message.set_hops_to_live(Parameters::hops_to_live);
            proto_message.set_ack_id(0);
            proto_message.set_actual_destination_is_relay_id(false);
            proto_message.set_id(message_id_);

            std::cout << "r local_port:" << local_port_ << std::endl;
            std::cout << "r node_.node_id():" << maidsafe::HexSubstr(local_id_) << std::endl;
            std::cout << "r client id:" << maidsafe::HexSubstr(edge_id_) << std::endl;
            std::cout << "r message.id:" << message_id_ << std::endl;

            network_->SendToDirect(proto_message, maidsafe::NodeId(edge_id_), nullptr);
        }

        memset(m_data, 0, sizeof(m_data));
        socket_.async_read_some(boost::asio::buffer(m_data, _RECV_BUF_SIZE_),
            boost::bind(&CClientASIO::OnRecv, this,
                boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    } else {
        std::cout << "fuck close!" << std::endl;
        OnClose(error);
        return false;
    }
    return true;
}

bool CClientASIO::OnSend(const boost::system::error_code& error, size_t bytes_transferred) {
    std::cout << "Send Bytes:" << bytes_transferred << std::endl;
    if (error) {
        OnClose(error);
        return false;
    }
    return true;
}

void CClientASIO::OnClose(const boost::system::error_code& error) {
    if (socket_.is_open()) {
        try {
            socket_.close();
        } catch (...) {
            ERROR("close failed catched error!");
        }
    }

    closed_ = true;
}

}  // top
}  // storage
