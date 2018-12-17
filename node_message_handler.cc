#include "node_message_handler.h"

#include <map>

#include "maidsafe/routing/network.h"

#include "message.pb.h"
#include "db.h"
#include "log.h"
#include "routing_network.h"
#include "utils.h"
#include "dict.h"
#include "session_manager.h"

using namespace maidsafe;
using namespace maidsafe::routing;
namespace args = std::placeholders;

namespace top {

namespace storage {

TopStorageMessageHandler::TopStorageMessageHandler(GenericNode& node): node_(node) {};
TopStorageMessageHandler::~TopStorageMessageHandler() {}

void TopStorageMessageHandler::HandleMessage(RoutingMessage& message) {
    switch (static_cast<TopStorageMessageType>(message.top_type())) {
    case TopStorageMessageType::kGroupPutData: {
        if (message.source_id() == routing_table_->kNodeId().string()) {
            LOG(kInfo) << "source is equal to this node id";
            NodeHandleMessage(message);
            break;
        }

        LOG(kInfo) << "new group data in.";
        if (message.request()) {
            HandleGroupPutDataRequest(message);
        } else {
            HandleCallbackResponse(message);
        }
        break;
    }
    case TopStorageMessageType::kGroupGetData:
        break;
    case TopStorageMessageType::kGroupChooseRequest:
        LOG(kInfo) << "kGroupChooseRequest message id: " << message.id();
        HandleChooseGroupRequest(message);
        break;
    case TopStorageMessageType::kGroupChooseResponse:
        LOG(kInfo) << "kGroupChooseResponse message id: " << message.id();
        HandleCallbackResponse(message);
        break;
    case TopStorageMessageType::kChunkChooseRequest:
        LOG(kInfo) << "kChunkChooseRequest message id: " << message.id();
        HandleChunkChoose(message);
        break;
    case TopStorageMessageType::kChunkChooseResponse:
        LOG(kInfo) << "kChunkChooseResponse message id: " << message.id();
        HandleCallbackResponse(message);
        break;
    case TopStorageMessageType::kGroupChooseSlave:
        LOG(kInfo) << "kGroupChooseSlave message id: " << message.id();
        HandleChooseGroupSlave(message);
        break;
    case TopStorageMessageType::kFindVpnNodesRequest:
        LOG(kInfo) << "kFindVpnNodesRequest message id: " << message.id();
        HandleFindNodesRequest(message);
        break;
    case TopStorageMessageType::kFindVpnNodesResponse:
        LOG(kInfo) << "kFindVpnNodesResponse message id: " << message.id();
        HandleFindNodesResponse(message);
        break;
    case TopStorageMessageType::kVpnServerInfoRequest:
        LOG(kInfo) << "kVpnServerInfoRequest message id: " << message.id();
        HandleVpnServerInfoRequest(message);
        break;
    case TopStorageMessageType::kVpnServerInfoResponse:
        LOG(kInfo) << "kVpnServerInfoResponse message id: " << message.id();
        HandleVpnServerInfoResponse(message);
        break;
    case TopStorageMessageType::kVpnPutDataRequest:
        LOG(kInfo) << "kVpnPutDataRequest message id: " << message.id();
        HandleVpnPutDataRequest(message);
        break;
    case TopStorageMessageType::kVpnPutDataResponse:
        LOG(kInfo) << "kVpnPutDataResponse message id: " << message.id();
        HandleVpnPutDataResponse(message);
        break;
    case TopStorageMessageType::kBitvpnDataRequest:
        LOG(kInfo) << "kBitvpnDataRequest message id: " << message.id();
        HandleBitvpnDataRequest(message);
        break;
    case TopStorageMessageType::kBitvpnDataResponse:
        LOG(kInfo) << "kBitvpnDataRequest message id: " << message.id();
        HandleBitvpnDataResponse(message);
        break;
    case TopStorageMessageType::kFindEdgeNodesRequest:
        LOG(kInfo) << "kFindEdgeNodesRequest message id: " << message.id();
        HandleFindEdgeNodesRequest(message);
        break;
    case TopStorageMessageType::kFindEdgeNodesResponse:
        LOG(kInfo) << "kFindEdgeNodesResponse message id: " << message.id();
        HandleFindEdgeNodesResponse(message);
        break;
    case TopStorageMessageType::kEdgeInfoRequest:
        LOG(kInfo) << "kEdgeInfoRequest message id: " << message.id();
        HandleEdgeInfoRequest(message);
        break;
    case TopStorageMessageType::kEdgeInfoResponse:
        LOG(kInfo) << "kEdgeInfoResponse message id: " << message.id();
        HandleEdgeInfoResponse(message);
        break;
    default:
        MessageHandler::HandleMessage(message);
        break;
    }
}

void TopStorageMessageHandler::HandleBitvpnDataRequest(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        ERROR("ChooseChunkNodesRequest has no data.");
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kTopError),
                   "there is no more nodes for chunks",
                   message);
    }

    protobuf::BitvpnDataRequest bitvpn_req;
    if (!bitvpn_req.ParseFromString(message.data(0))) {
        ERROR("HandleVpnServerInfoRequest ParseFromString failed.");
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kTopError),
                   "there is no more nodes for chunks",
                   message);
    }

    NodeId destination_node_id(message.destination_id());
    if (routing_table_->kNodeId() == destination_node_id ||
            routing_table_->IsThisNodeClosestTo(destination_node_id)) {
        return SendBitvpnResponse(message);
    } else {
        return network_->SendToClosestNode(message);
    }
}

void TopStorageMessageHandler::HandleBitvpnDataResponse(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        std::cout << "bitvpn data is empty." << std::endl;
        return;
    }

    protobuf::BitvpnDataResponse res;
    if (!res.ParseFromString(message.data(0))) {
        std::cout << "HandleBitvpnDataResponse ParseFromString failed." << std::endl;
    }

    std::cout << "HandleBitvpnDataResponse ok: " << res.msg() << std::endl;
    if (GlobalVpnCallback) {
        GlobalVpnCallback(0, res.msg());
    }
}

void TopStorageMessageHandler::HandleVpnPutDataRequest(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        ERROR("HandleVpnPutDataRequest has no data.");
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kTopError),
                   "there is no more nodes for vpn",
                   message);
    }

    protobuf::VpnPutDataRequest vpn_put_data_req;
    if (!vpn_put_data_req.ParseFromString(message.data(0))) {
        ERROR("HandleVpnPutDataRequest ParseFromString failed.");
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kTopError),
                   "there is no more nodes for vpn ",
                   message);
    }

    // this is dest
    NodeId destination_node_id(message.destination_id());
    if (routing_table_->kNodeId() == destination_node_id ||
            routing_table_->IsThisNodeClosestTo(destination_node_id)) {

        std::string  data = vpn_put_data_req.data();
        uint32_t   local_port = vpn_put_data_req.local_port();
        uint32_t  vpn_port = vpn_put_data_req.vpn_port();
        std::string  vpn_ip = vpn_put_data_req.vpn_ip();

        ClientAsioPtr client = SessionManager::Instance()->GetClient(local_port, message.id());
        if (!client) {
            client = SessionManager::Instance()->NewSession(
                         vpn_ip,
                         vpn_port,
                         local_port,
                         node_.node_id().string(),
                         message.source_id(),
                         network_,
                         message.id());
        }

        if (!client) {
            ERROR("client is null session error.");
            return;
        }

        client->PostSend(data);
    } else {
        return network_->SendToClosestNode(message);
    }
}

void TopStorageMessageHandler::HandleForwardResponse(RoutingMessage& message, uint32_t local_port, std::string res) {
    //prepare response
    RoutingMessage proto_message;

    protobuf::VpnPutDataResponse vpn_put_data_res;
    vpn_put_data_res.set_data(res);
    vpn_put_data_res.set_local_port(local_port);


    std::string rdata;
    if (!vpn_put_data_res.SerializeToString(&rdata)) {
        ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        return;
    }

    proto_message.set_source_id(node_.node_id().string());
    proto_message.set_destination_id(message.source_id());
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
    proto_message.set_id(message.id());

    return network_->SendToDirect(proto_message, maidsafe::NodeId(message.source_id()), nullptr);
}


void TopStorageMessageHandler::HandleVpnPutDataResponse(RoutingMessage& res_message) {
    if (res_message.data_size() > 0) {
        protobuf::VpnPutDataResponse response;
        if (response.ParseFromString(res_message.data(0))) {
            //TODO call to callback
            std::cout << "get ss-server response. size = " << response.data().size() << std::endl;
            //send to shadowsocks
        } else {
            std::cout << "vpn put data message parse failed!" << std::endl;
        }
    } else {
        std::cout << "message not valid!" << std::endl;
    }
    //HandleCallbackResponse(message);
}


void TopStorageMessageHandler::HandleVpnServerInfoRequest(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        ERROR("ChooseChunkNodesRequest has no data.");
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kTopError),
                   "there is no more nodes for chunks",
                   message);
    }

    protobuf::VpnServerInfoRequest vpn_server_req;
    if (!vpn_server_req.ParseFromString(message.data(0))) {
        ERROR("HandleVpnServerInfoRequest ParseFromString failed.");
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kTopError),
                   "there is no more nodes for chunks",
                   message);
    }

    NodeId destination_node_id(message.destination_id());
    if (routing_table_->kNodeId() == destination_node_id ||
            routing_table_->IsThisNodeClosestTo(destination_node_id)) {
        return SendVpnServerInfoResponse(message);
    } else {
        return network_->SendToClosestNode(message);
    }
}

void TopStorageMessageHandler::HandleVpnServerInfoResponse(RoutingMessage& message) {
    HandleCallbackResponse(message);
}

void TopStorageMessageHandler::SendBitvpnResponse(const RoutingMessage& message) {
    RoutingMessage proto_message;
    protobuf::BitvpnDataResponse bitvpn_res;
    bitvpn_res.set_msg("bitvpn res");
    std::string data;
    if (!bitvpn_res.SerializeToString(&data)) {
        ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        return;
    }

    proto_message.set_source_id(node_.node_id().string());
    proto_message.set_destination_id(message.source_id());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(
                                   TopStorageMessageType::kBitvpnDataResponse));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(false);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(message.id());

    network_->SendToDirect(proto_message, maidsafe::NodeId(message.source_id()), nullptr);
}


void TopStorageMessageHandler::SendVpnServerInfoResponse(const RoutingMessage& message) {
    RoutingMessage proto_message;
    protobuf::VpnServerInfoResponse vpn_server_res;
    HandleGetVpnInfo(&vpn_server_res);

    std::string data;
    if (!vpn_server_res.SerializeToString(&data)) {
        ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        return;
    }

    proto_message.set_source_id(node_.node_id().string());
    proto_message.set_destination_id(message.source_id());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(
                                   TopStorageMessageType::kVpnServerInfoResponse));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(false);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(message.id());

    network_->SendToDirect(proto_message, maidsafe::NodeId(message.source_id()), nullptr);
}

void TopStorageMessageHandler::HandleGetVpnInfo(protobuf::VpnServerInfoResponse* response) {
    std::string vpn_ip;
    if (!Dict::Instance()->Hget(LOCAL_VPN_SERVER_DB_KEY, "server_ip", &vpn_ip)) {
        LOG(log4cpp::Priority::ERROR) << "get data from db failed!";
        return;
    }
    response->set_ip(vpn_ip);

    std::string vpn_port;
    if (!Dict::Instance()->Hget(LOCAL_VPN_SERVER_DB_KEY, "server_port", &vpn_port)) {
        LOG(log4cpp::Priority::ERROR) << "get data from db failed!";
        return;
    }

    try {
        response->set_port(std::stoi(vpn_port));
    } catch (std::exception& e) {
        LOG(log4cpp::Priority::ERROR) << "get vpn server port failed: " << e.what();
        return;
    }

    std::string password;
    if (!Dict::Instance()->Hget(LOCAL_VPN_SERVER_DB_KEY, "password", &password)) {
        LOG(log4cpp::Priority::ERROR) << "get data from db failed!";
        return;
    }
    response->set_passwd(password);

    std::string enc_method;
    if (!Dict::Instance()->Hget(LOCAL_VPN_SERVER_DB_KEY, "enc_method", &enc_method)) {
        LOG(log4cpp::Priority::ERROR) << "get data from db failed!";
        return;
    }
    response->set_method(enc_method);
}

void TopStorageMessageHandler::HandleFindNodesRequest(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        ERROR("ChooseChunkNodesRequest has no data.");
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kTopError),
                   "there is no more nodes for chunks",
                   message);
    }

    protobuf::FindVpnNodesRequest find_vpn_req;
    if (!find_vpn_req.ParseFromString(message.data(0))) {
        ERROR("ChooseChunkNodesRequest ParseFromString failed.");
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kTopError),
                   "there is no more nodes for chunks",
                   message);
    }

    NodeId destination_node_id(message.destination_id());
    if (routing_table_->kNodeId() == destination_node_id ||
            routing_table_->IsThisNodeClosestTo(destination_node_id)) {
        NodeId destination_node_id(message.destination_id());
        std::vector<NodeInfo> chunk_nodes = routing_table_->GetClosestNodes(
                                                destination_node_id,
                                                find_vpn_req.count());
        if (chunk_nodes.size() != find_vpn_req.count()) {
            return SendErrorAckMessage(
                       static_cast<int>(TopStorageMessageType::kNoEnoughNodes),
                       "there is no more nodes for chunks",
                       message);
        }

        return SendFindVpnNodesResponse(chunk_nodes, message);
    } else {
        return network_->SendToClosestNode(message);
    }
}

void TopStorageMessageHandler::SendFindVpnNodesResponse(
    const std::vector<NodeInfo>& chunk_nodes,
    const RoutingMessage& message) {
    RoutingMessage proto_message;
    protobuf::FindVpnNodesResponse find_vpn_res;
    for (uint32_t i = 0; i < chunk_nodes.size(); ++i) {
        find_vpn_res.add_nodes(chunk_nodes[i].id.string());
    }

    std::string data;
    if (!find_vpn_res.SerializeToString(&data)) {
        ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        return;
    }

    proto_message.set_source_id(node_.node_id().string());
    proto_message.set_destination_id(message.source_id());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(
                                   TopStorageMessageType::kFindVpnNodesResponse));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(false);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(message.id());

    network_->SendToDirect(proto_message, maidsafe::NodeId(message.source_id()), nullptr);
}

void TopStorageMessageHandler::HandleFindNodesResponse(RoutingMessage& message) {
    HandleCallbackResponse(message);
}


//鏍规嵁闈欐€佽矾鐢辫〃瀵绘壘杞彂鑺傜偣
void TopStorageMessageHandler::HandleFindEdgeNodesRequest(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        LOG(log4cpp::Priority::ERROR) << "FindEdgeNodesRequest has no data.";
        return;
    }

    protobuf::FindEdgeNodesRequest find_edge_req;
    if (!find_edge_req.ParseFromString(message.data(0))) {
        LOG(log4cpp::Priority::ERROR) << "FindEdgeNodesRequest ParseFromString failed.";
        return;
    }

    NodeId destination_node_id(message.destination_id());
    if (routing_table_->kNodeId() == destination_node_id ||
            routing_table_->IsThisNodeClosestTo(destination_node_id)) {
        NodeId destination_node_id(message.destination_id());
        std::vector<NodeInfo> chunk_nodes = routing_table_->GetClosestNodes(
                                                destination_node_id,
                                                find_edge_req.count());

        //filter invalide node
        std::string des_type = destination_node_id.string().substr(0, 9);
		if(routing_table_->kNodeId().string() != message.source_id()) {
			NodeInfo thisNode;
			thisNode.id = routing_table_->kNodeId();
			chunk_nodes.push_back(thisNode);
		}
		LOG(log4cpp::Priority::INFO) <<"destination nodeid type:"<< maidsafe::HexEncode(des_type);
        for(auto it = chunk_nodes.begin(); it != chunk_nodes.end(); ) {
            std::string id_str = (*it).id.string();
			LOG(log4cpp::Priority::INFO) <<"found node may be suitable:"<< maidsafe::HexEncode(id_str);
            if(des_type.compare(id_str.substr(0, 9)) != 0) {
                chunk_nodes.erase(it);
            } else {
                it++;
            }
        }

        if (chunk_nodes.size() <= 0) {
            LOG(log4cpp::Priority::ERROR) << "there is no more nodes for find edge.";
            return;
        }

        return SendFindEdgeNodesResponse(chunk_nodes, message);
    } else {
        return network_->SendToClosestNode(message);
    }
}


void TopStorageMessageHandler::SendFindEdgeNodesResponse(
    const std::vector<NodeInfo>& chunk_nodes,
    const RoutingMessage& message) {
    RoutingMessage proto_message;
    protobuf::FindEdgeNodesResponse find_edge_res;
    for (uint32_t i = 0; i < chunk_nodes.size(); ++i) {
        find_edge_res.add_nodes(chunk_nodes[i].id.string());
    }

    std::string data;
    if (!find_edge_res.SerializeToString(&data)) {
        LOG(log4cpp::Priority::ERROR) << "FindEdgeNodesResponse SerializeToString failed!";
        return;
    }

    proto_message.set_source_id(node_.node_id().string());
    proto_message.set_destination_id(message.source_id());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(
                                   TopStorageMessageType::kFindEdgeNodesResponse));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(false);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(message.id());

    network_->SendToDirect(proto_message, maidsafe::NodeId(message.source_id()), nullptr);
}



void TopStorageMessageHandler::HandleFindEdgeNodesResponse(RoutingMessage& message) {
    HandleCallbackResponse(message);
}


//get edge tcp/udp ip and port
void TopStorageMessageHandler::HandleEdgeInfoRequest(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        LOG(log4cpp::Priority::ERROR) << "EdgeInfoRequest has no data.";
        return;
    }

    protobuf::EdgeInfoRequest edge_info_req;
    if (!edge_info_req.ParseFromString(message.data(0))) {
        LOG(log4cpp::Priority::ERROR) << "HandleEdgeInfoRequest ParseFromString failed.";
        return;
    }

    NodeId destination_node_id(message.destination_id());
    if (routing_table_->kNodeId() == destination_node_id ) {
        return SendEdgeInfoResponse(message);
    } else if( routing_table_->IsThisNodeClosestTo(destination_node_id)) {
        // check the closest node is valide or not
        std::string edge_type = destination_node_id.string().substr(0, 9);
        if(edge_type.compare(routing_table_->kNodeId().string().substr(0, 9)) == 0) {
            return SendEdgeInfoResponse(message);
        } else {
            LOG(log4cpp::Priority::ERROR) << "there is no more nodes for edgeinfo request";
            return;
        }
    } else {
        return network_->SendToClosestNode(message);
    }
}


void TopStorageMessageHandler::SendEdgeInfoResponse(const RoutingMessage& message) {
    RoutingMessage proto_message;
    protobuf::EdgeInfoResponse edge_info_res;
    HandleGetEdgeInfo(&edge_info_res);

    std::string data;
    if (!edge_info_res.SerializeToString(&data)) {
        LOG(log4cpp::Priority::ERROR) << "EdgeInfoRequest SerializeToString failed!";
        return;
    }

    proto_message.set_source_id(node_.node_id().string());
    proto_message.set_destination_id(message.source_id());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(
                                   TopStorageMessageType::kEdgeInfoResponse));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(false);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(message.id());

    network_->SendToDirect(proto_message, maidsafe::NodeId(message.source_id()), nullptr);
}


//read tcp/udp ip:port from local db
void TopStorageMessageHandler::HandleGetEdgeInfo(protobuf::EdgeInfoResponse* response) {
    std::string edge_ip;

    /*
    response->set_ip("127.0.0.1");
    response->set_tcp_port(9999);
    response->set_udp_port(8888);
    */

    if (!Dict::Instance()->Hget(LOCAL_EDGE_DB_KEY, "ip", &edge_ip)) {
        LOG(log4cpp::Priority::ERROR) << "get data from db failed!";
        return;
    }
    response->set_ip(edge_ip);

    std::string tcp_port;
    if (!Dict::Instance()->Hget(LOCAL_EDGE_DB_KEY, "tcp_port", &tcp_port)) {
        LOG(log4cpp::Priority::ERROR) << "get data from db failed!";
        return;
    }

    try {
        response->set_tcp_port(std::stoi(tcp_port));
    } catch (std::exception& e) {
        LOG(log4cpp::Priority::ERROR) << "get edge tcp port failed: " << e.what();
        return;
    }

    std::string udp_port;
    if (!Dict::Instance()->Hget(LOCAL_EDGE_DB_KEY, "udp_port", &udp_port)) {
        LOG(log4cpp::Priority::ERROR) << "get data from db failed!";
        return;
    }

    try {
        response->set_udp_port(std::stoi(udp_port));
    } catch (std::exception& e) {
        LOG(log4cpp::Priority::ERROR) << "get edge udp port failed: " << e.what();
        return;
    }
}



void TopStorageMessageHandler::HandleEdgeInfoResponse(RoutingMessage& message) {
    HandleCallbackResponse(message);
}


void TopStorageMessageHandler::HandleChooseChunkSlave(RoutingMessage& message) {
    if (message.source_id() == message.destination_id() &&
            NodeId(message.destination_id()) == routing_table_->kNodeId()) {

        protobuf::NodeInfo node_info;
        if(!PrepareChunkServer(message, node_info)) {
            return;
        }

        RoutingMessage proto_message;
        if (!CreateChunkChooseResponse(node_info.ip(), node_info.port(), message, proto_message)) {
            return;
        }

        return HandleCallbackResponse(proto_message);
    }

    protobuf::NodeInfo node_info;
    if (PrepareChunkServer(message, node_info)) {
        SendChunkChooseResponse(node_info.ip(), node_info.port(), message);
    }
}

void TopStorageMessageHandler::HandleChooseChunkNodeMaster(
    RoutingMessage& message,
    protobuf::ChooseChunkNodesRequest& choose_chunk_req) {
    // this is chunk closest node
    protobuf::NodeInfo node_info;
    if (PrepareChunkServer(message, node_info)) {
        SendChunkChooseResponse(node_info.ip(), node_info.port(), message);
    }

    int slave_chunk_size = TopParameters::file_chunk_node_num - 1;
    NodeId destination_node_id(message.destination_id());
    std::vector<NodeInfo> chunk_nodes = routing_table_->GetClosestNodes(
                                            destination_node_id,
                                            slave_chunk_size);
    if (chunk_nodes.size() != slave_chunk_size) {
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kNoEnoughNodes),
                   "there is no more nodes for chunks",
                   message);
    }

    choose_chunk_req.set_is_master_send(1);
    std::string data;
    choose_chunk_req.SerializeToString(&data);
    message.clear_data();
    message.add_data(data);
    message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kChunkChooseSlave));
    // prepare chunk server by this node
    // todo: choose valid node(example: disk left enough)
    for (const auto& i : chunk_nodes) {
        // other node prepare and ack to group master
        message.clear_ack_node_ids();
        message.set_ack_id(0);
        message.set_destination_id(i.id.string());
        NodeInfo node;
        if (routing_table_->GetNodeInfo(i.id, node)) {
            network_->SendToDirect(message, node.id, node.connection_id);
        } else {
            network_->SendToClosestNode(message);
        }
    }
}

void TopStorageMessageHandler::HandleChunkChoose(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        ERROR("ChooseChunkNodesRequest has no data.");
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kTopError),
                   "there is no more nodes for chunks",
                   message);
    }

    protobuf::ChooseChunkNodesRequest choose_chunk_req;
    if (!choose_chunk_req.ParseFromString(message.data(0))) {
        ERROR("ChooseChunkNodesRequest ParseFromString failed.");
        return SendErrorAckMessage(
                   static_cast<int>(TopStorageMessageType::kTopError),
                   "there is no more nodes for chunks",
                   message);
    }

    NodeId destination_node_id(message.destination_id());
    if (routing_table_->kNodeId() == destination_node_id ||
            routing_table_->IsThisNodeClosestTo(destination_node_id)) {
        return HandleChooseChunkNodeMaster(message, choose_chunk_req);
    } else {
        return network_->SendToClosestNode(message);
    }
}

bool TopStorageMessageHandler::PrepareChunkServer(
    const RoutingMessage& message,
    protobuf::NodeInfo& node_info) {
    if (message.data_size() <= 0) {
        ERROR("choose chunk message is empty.");
        return false;
    }

    protobuf::ChooseChunkNodesRequest choose_chunk_req;
    if (!choose_chunk_req.ParseFromString(message.data(0))) {
        ERROR("ChooseChunkNodesRequest ParseFromString failed!");
        return false;
    }

    // todo save group and chunk info, check chunk size
    leveldb::Status put_st = Db::Instance()->Put(
                                 create_chunk_key(choose_chunk_req.chunk_id()),
                                 choose_chunk_req.group_id());
    if (!put_st.ok()) {
        ERROR("put chunk info failed![%s]", put_st.ToString().c_str());
        return false;
    }

    node_info.set_ip("choose_ip_for_local_and_public");
    node_info.set_port(1000);
    node_info.set_node_id(routing_table_->kNodeId().string());
    return true;
}

void TopStorageMessageHandler::SendErrorAckMessage(
    int status,
    const std::string& msg,
    const RoutingMessage& message) {
    protobuf::ChooseChunkNodesResponse choose_chunk_res;
    choose_chunk_res.set_msg(msg);
    choose_chunk_res.set_status(status);
    std::string data;
    if (!choose_chunk_res.SerializeToString(&data)) {
        ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        return;
    }

    RoutingMessage proto_message;
    proto_message.set_source_id(node_.node_id().string());
    proto_message.set_destination_id(message.source_id());
    proto_message.set_routing_message(false);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(false);
    proto_message.add_data(data);
    proto_message.set_top_type(static_cast<int32_t>(
                                   TopStorageMessageType::kChunkChooseResponse));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(false);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(message.id());
    network_->SendToDirect(proto_message, maidsafe::NodeId(message.source_id()), nullptr);
}

bool TopStorageMessageHandler::CreateChunkChooseResponse(
    const std::string& ip,
    uint32_t port,
    const RoutingMessage& message,
    RoutingMessage& proto_message) {
    protobuf::ChooseChunkNodesResponse choose_chunk_res;
    choose_chunk_res.set_ip(ip);
    choose_chunk_res.set_port(port);
    choose_chunk_res.set_status(static_cast<int32_t>(TopStorageMessageType::kTopSuccess));
    std::string data;
    if (!choose_chunk_res.SerializeToString(&data)) {
        ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        return false;
    }

    proto_message.set_source_id(node_.node_id().string());
    proto_message.set_destination_id(message.source_id());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(
                                   TopStorageMessageType::kChunkChooseResponse));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(false);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(message.id());
    return true;
}


void TopStorageMessageHandler::SendChunkChooseResponse(
    const std::string& ip,
    uint32_t port,
    const RoutingMessage& message) {
    RoutingMessage proto_message;
    if (!CreateChunkChooseResponse(ip, port, message, proto_message)) {
        return;
    }

    network_->SendToDirect(proto_message, maidsafe::NodeId(message.source_id()), nullptr);
}

bool NodeValidateMessage(const RoutingMessage& message) {
    if (!message.IsInitialized()) {
        LOG(kWarning) << "Uninitialised message dropped.";
        return false;
    }

    if (message.hops_to_live() <= 0) {
        std::string route_history;
        for (const auto& route : message.route_history())
            route_history += HexSubstr(route) + ", ";
        LOG(kError) << "Message has traversed more hops than expected. "
                    << Parameters::max_route_history
                    << " last hops in route history are: " << route_history
                    << " \nMessage source: " << HexSubstr(message.source_id())
                    << ", \nMessage destination: " << HexSubstr(message.destination_id())
                    << ", \nMessage type: " << message.type() << ", \nMessage id: " << message.id();
        return false;
    }

    if (!CheckId(message.destination_id())) {
        LOG(kWarning) << "Stray message dropped, need destination ID for processing."
                      << " id: " << message.id();
        return false;
    }

    if (!(message.has_source_id() || (message.has_relay_id() && message.has_relay_connection_id()))) {
        LOG(kWarning) << "Message should have either src id or relay information.";
        assert(false && "Message should have either src id or relay information.");
        return false;
    }

    if (message.has_source_id() && !CheckId(message.source_id())) {
        LOG(kWarning) << "Invalid source id field.";
        return false;
    }

    if (message.has_relay_id() && !NodeId(message.relay_id()).IsValid()) {
        LOG(kWarning) << "Invalid relay id field.";
        return false;
    }

    if (message.has_relay_connection_id() && !NodeId(message.relay_connection_id()).IsValid()) {
        LOG(kWarning) << "Invalid relay connection id field.";
        return false;
    }

    if (static_cast<MessageType>(message.type()) == MessageType::kConnect)
        if (!message.direct()) {
            LOG(kWarning) << "kConnectRequest type message must be direct.";
            return false;
        }

    if (static_cast<MessageType>(message.type()) == MessageType::kFindNodes &&
            (message.request() == false)) {
        if ((!message.direct())) {
            LOG(kWarning) << "kFindNodesResponse type message must be direct.";
            return false;
        }
    }

    return true;
}

void TopStorageMessageHandler::NodeHandleMessage(RoutingMessage& message) {
    if (!message.source_id().empty() && !IsAck(message) &&
            (message.destination_id() != message.source_id()) &&
            (message.destination_id() == routing_table_->kNodeId().string()) &&
            !network_utils_->firewall_.Add(NodeId(message.source_id()), message.id())) {
        return;
    }

    if (!NodeValidateMessage(message)) {
        LOG(kWarning) << "Validate message failed id: " << message.id();
        BOOST_ASSERT_MSG((message.hops_to_live() > 0),
                         "Message has traversed maximum number of hops allowed");
        return;
    }

    if (!message.routing_message() && !message.client_node() && message.has_source_id()) {
        NodeInfo node_info;
        node_info.id = NodeId(message.source_id());
        if (routing_table_->CheckNode(node_info))
            response_handler_->CheckAndSendConnectRequest(node_info.id);
    }

    message.set_hops_to_live(message.hops_to_live() - 1);

    if (IsValidCacheableGet(message) && HandleCacheLookup(message)) {
        return;
    }

    if (IsValidCacheablePut(message)) {
        StoreCacheCopy(message);
    }

    if (IsGroupMessageRequestToSelfId(message)) {
        return HandleGroupMessageToSelfId(message);
    }

    if (routing_table_->client_mode()) {
        return HandleClientMessage(message);
    }

    if (message.source_id().empty()) {
        return HandleRelayRequest(message);
    }

    if (!NodeId(message.source_id()).IsValid()) {
        LOG(kWarning) << "Stray message dropped, need valid source ID for processing."
                      << " id: " << message.id();
        return;
    }

    if (message.destination_id() == routing_table_->kNodeId().string()) {
        return HandleMessageForThisNode(message);
    }

    if (IsRelayResponseForThisNode(message)) {
        return HandleRoutingMessage(message);
    }

    if (client_routing_table_->Contains(NodeId(message.destination_id())) && IsDirect(message)) {
        return HandleMessageForNonRoutingNodes(message);
    }

    if (routing_table_->IsThisNodeInRange(NodeId(message.destination_id()),
                                          Parameters::closest_nodes_size)) {
        return HandleMessageAsClosestNode(message);
    } else {
        return HandleMessageAsFarNode(message);
    }
}

bool TopStorageMessageHandler::SendChooseChunkListRequest(
    const std::string& group_id,
    const std::string& chunk_id,
    std::mutex& choose_mutex,
    ChunkNodeMap& chunk_node_map) {
    RoutingMessage proto_message;
    if (!CreateChooseChunkReqest(group_id, chunk_id, 0, proto_message)) {
        ERROR("create chunk choose request failed!");
        return false;
    }

    int operation_count = 0;
    std::mutex mutex;
    int expect_respondent = TopParameters::file_chunk_node_num;
    std::condition_variable cond_var;

    auto callable = [&operation_count, &mutex, expect_respondent,
                      &cond_var, chunk_id, &chunk_node_map, &choose_mutex, this](std::string data) {
        RoutingMessage res_message;
        if (res_message.ParseFromString(data)) {
            if (res_message.data_size() > 0) {
                protobuf::ChooseChunkNodesResponse response;
                if (response.ParseFromString(res_message.data(0))) {
                    if (response.status() != static_cast<int>(TopStorageMessageType::kTopSuccess)) {
                        ERROR("get chunk info failed! msg: %s", response.msg().c_str());
                    } else {
                        protobuf::NodeInfo node_info;
                        node_info.set_ip(response.ip());
                        node_info.set_port(response.port());
                        node_info.set_node_id(res_message.source_id());
                        DEBUG("chunk node info: ip: %s, port: %d", response.ip().c_str(), response.port());
                        std::lock_guard<std::mutex> lock(choose_mutex);
                        InsertChunkInfoToMap(chunk_id, node_info, chunk_node_map);
                    }
                } else {
                    ERROR("ChooseChunkNodesResponse ParseFromString failed!");
                }
            } else {
                ERROR("no data, ChooseChunkNodesResponse ParseFromString failed!");
            }
        } else {
            ERROR("RoutingMessage ParseFromString failed!");
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
            ++operation_count;
            if (operation_count == expect_respondent) {
                cond_var.notify_one();
            }
        }
    };

    auto timeout(Parameters::default_response_timeout);
    // wait TopParameters::file_chunk_node_num nodes to response
    timer_->AddTask(
        Parameters::default_response_timeout,
        callable,
        expect_respondent,
        proto_message.id());
    network_->SendToClosestNode(proto_message);
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (operation_count != expect_respondent)
            cond_var.wait(lock);
    }

    DEBUG("choose chunk list success.");
    Parameters::default_response_timeout = timeout;
    return true;
}

bool TopStorageMessageHandler::CreateChooseChunkReqest(
    const std::string& group_id,
    const std::string& chunk_id,
    int master_send,
    RoutingMessage& proto_message) {
    protobuf::ChooseChunkNodesRequest choose_chunk_req;
    choose_chunk_req.set_group_id(group_id);
    choose_chunk_req.set_chunk_id(chunk_id);
    choose_chunk_req.set_chunk_size(4 * 1024 * 1024);
    choose_chunk_req.set_is_master_send(master_send);
    std::string data;
    if (!choose_chunk_req.SerializeToString(&data)) {
        ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        return false;
    }

    proto_message.set_source_id(node_.node_id().string());
    // 设置chunk_id
    proto_message.set_destination_id(chunk_id);
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kChunkChooseRequest));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(true);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(TopParameters::message_id++);
    return true;
}

bool TopStorageMessageHandler::SendChooseChunkList(
    const RoutingMessage& message,
    const protobuf::ChooseGroupRequest& choose_group_req,
    ChunkNodeMap& chunk_node_map) {
    if (choose_group_req.merkle_tree_size() <= 0) {
        ERROR("there is no chunk to choose chunk nodes!");
        return false;
    }

    const protobuf::MerkleTreeLevel& chunk_list = choose_group_req.merkle_tree(
                choose_group_req.merkle_tree_size() - 1);
    if (chunk_list.level_hash_size() <= 0) {
        ERROR("there is no chunk to choose chunk nodes!");
        return false;
    }

    if (chunk_list.level_hash_size() > TopParameters::file_chunk_max_num) {
        ERROR("chunk num is exceeded max[%d] now[%d]",
              TopParameters::file_chunk_max_num, chunk_list.level_hash_size());
        return false;
    }

    std::mutex choose_chunk_nodes_mutex;
    for (int i = 0; i < chunk_list.level_hash_size(); ++i) {
        NodeId destination_node_id(chunk_list.level_hash(i));
        if (routing_table_->kNodeId() == destination_node_id ||
                routing_table_->IsThisNodeClosestTo(destination_node_id)) {
            if (!SelfChooseChunkList(
                        message,
                        chunk_list.level_hash(i),
                        choose_chunk_nodes_mutex,
                        chunk_node_map)) {
                ERROR("self choose chunk list failed!");
                return false;
            }
        } else {
            if (!SendChooseChunkListRequest(
                        message.destination_id(),
                        chunk_list.level_hash(i),
                        choose_chunk_nodes_mutex,
                        chunk_node_map)) {
                ERROR("send to other node choose chunk list failed!");
                return false;
            }
        }

    }

    return true;
}

void TopStorageMessageHandler::InsertChunkInfoToMap(
    const std::string& chunk_id,
    const protobuf::NodeInfo& node_info,
    ChunkNodeMap& chunk_node_map) {
    auto find_res = chunk_node_map.find(chunk_id);
    if (find_res != chunk_node_map.end()) {
        find_res->second.push_back(node_info);
    } else {
        std::vector<protobuf::NodeInfo> tmp_vec;
        tmp_vec.push_back(node_info);
        auto ins_res = chunk_node_map.insert(std::make_pair(chunk_id, tmp_vec));
        if (!ins_res.second) {
            ERROR("insert chunk to chunk node map failed!");
        }
    }
}

bool TopStorageMessageHandler::SelfChooseChunkList(
    const RoutingMessage& src_message,
    const std::string& chunk_id,
    std::mutex& choose_mutex,
    ChunkNodeMap& chunk_node_map) {
    NodeId destination_node_id(chunk_id);
    if (routing_table_->kNodeId() != destination_node_id &&
            !routing_table_->IsThisNodeClosestTo(destination_node_id)) {
        ERROR("this is not chunk master.");
        return false;
    }

    protobuf::NodeInfo node;
    if (!PrepareChunkServer(src_message, node)) {
        ERROR("this node prepare chunk server failed!");
    }

    {
        std::lock_guard<std::mutex> lock(choose_mutex);
        InsertChunkInfoToMap(chunk_id, node, chunk_node_map);
    }

    int operation_count = 0;
    std::mutex mutex;
    int expect_respondent = TopParameters::file_chunk_node_num - 1;
    std::condition_variable cond_var;

    auto callable = [&operation_count, &mutex, expect_respondent,
                      &cond_var, chunk_id, &chunk_node_map, &choose_mutex, this](std::string data) {
        RoutingMessage res_message;
        if (res_message.ParseFromString(data)) {
            if (res_message.data_size() > 0) {
                protobuf::ChooseChunkNodesResponse response;
                if (response.ParseFromString(res_message.data(0))) {
                    if (response.status() != static_cast<int>(TopStorageMessageType::kTopSuccess)) {
                        ERROR("get chunk info failed! msg: %s", response.msg().c_str());
                    } else {
                        protobuf::NodeInfo node_info;
                        node_info.set_ip(response.ip());
                        node_info.set_port(response.port());
                        node_info.set_node_id(res_message.source_id());
                        DEBUG("chunk node info: ip: %s, port: %d", response.ip().c_str(), response.port());
                        std::lock_guard<std::mutex> lock(choose_mutex);
                        InsertChunkInfoToMap(chunk_id, node_info, chunk_node_map);
                    }
                } else {
                    ERROR("ChooseChunkNodesResponse ParseFromString failed!");
                }
            } else {
                ERROR("no data, ChooseChunkNodesResponse ParseFromString failed!");
            }
        } else {
            ERROR("RoutingMessage ParseFromString failed!");
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
            ++operation_count;
            if (operation_count == expect_respondent) {
                cond_var.notify_one();
            }
        }
    };

    std::vector<NodeInfo> chunk_nodes = routing_table_->GetClosestNodes(
                                            destination_node_id,
                                            expect_respondent);
    if (chunk_nodes.size() != expect_respondent) {
        ERROR("there is no more nodes for choose chunk node now: [%d], need[%d]",
              chunk_nodes.size(), expect_respondent);
        return false;
    }

    RoutingMessage message;
    if (!CreateChooseChunkReqest(src_message.destination_id(), chunk_id, 1, message)) {
        ERROR("create chunk choose request failed!");
        return false;
    }

    message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kChunkChooseSlave));
    auto timeout(Parameters::default_response_timeout);
    timer_->AddTask(
        Parameters::default_response_timeout,
        callable,
        expect_respondent,
        message.id());
    // todo: choose valid node(example: disk left enough)
    for (const auto& i : chunk_nodes) {
        // other node prepare and ack to group master
        message.clear_ack_node_ids();
        message.set_ack_id(0);
        message.set_destination_id(i.id.string());
        NodeInfo node;
        if (routing_table_->GetNodeInfo(i.id, node)) {
            network_->SendToDirect(message, node.id, node.connection_id);
        } else {
            network_->SendToClosestNode(message);
        }
    }

    {
        std::unique_lock<std::mutex> lock(mutex);
        if (operation_count != expect_respondent)
            cond_var.wait(lock);
    }

    DEBUG("choose chunk SelfChooseChunkList ok.");
    Parameters::default_response_timeout = timeout;
    return true;
}

bool TopStorageMessageHandler::CreateGroupMetaInfo(
    const RoutingMessage& message,
    const ChunkNodeMap& chunk_node_map,
    protobuf::GroupMetaInfo& group_meta) {
    if (message.data_size() <= 0) {
        ERROR("choose group request data is empty.");
        return false;
    }

    protobuf::ChooseGroupRequest choose_group_req;
    if (!choose_group_req.ParseFromString(message.data(0))) {
        ERROR("ChooseGroupRequest ParseFromString failed!");
        return false;
    }

    if (choose_group_req.merkle_tree_size() <= 0) {
        ERROR("ChooseGroupRequest merkle_tree_size is 0!");
        return false;
    }

    group_meta.set_group_id(message.destination_id());
    group_meta.set_file_size(choose_group_req.size());
    group_meta.set_file_name("");
    group_meta.set_chunk_size(choose_group_req.chunk_size());
    for (int i = 0; i < choose_group_req.merkle_tree_size(); ++i) {
        protobuf::MerkleTreeLevel* level = group_meta.add_merkle_tree();
        *level = choose_group_req.merkle_tree(i);
    }

    const protobuf::MerkleTreeLevel& last_level = choose_group_req.merkle_tree(
                choose_group_req.merkle_tree_size() - 1);
    for (int i = 0; i < last_level.level_hash_size(); ++i) {
        auto find_pos = chunk_node_map.find(last_level.level_hash(i));
        if (find_pos == chunk_node_map.end()) {
            ERROR("chunk id[%s] is not in chunk nodes",
                  maidsafe::HexSubstr(last_level.level_hash(i)).c_str());
            return false;
        }

        protobuf::ChunkInfo* chunk_info = group_meta.add_chunk_info_list();
        chunk_info->set_chunk_id(last_level.level_hash(i));
        for (auto iter = find_pos->second.begin(); iter != find_pos->second.end(); ++iter) {
            protobuf::NodeInfo* node_info = chunk_info->add_chunk_nodes();
            *node_info = *iter;
        }
    }

    return true;
}

bool TopStorageMessageHandler::SaveGroupMeta(
    const std::string& group_id,
    const std::string& group_meta) {
    leveldb::Status put_st = Db::Instance()->Put(
                                 create_group_key(group_id),
                                 group_meta);
    if (!put_st.ok()) {
        ERROR("put chunk info failed![%s]", put_st.ToString().c_str());
        return false;
    }

    return true;
}

void TopStorageMessageHandler::HandleChooseGroupMaster(
    RoutingMessage& message,
    protobuf::ChooseGroupRequest& choose_group_req) {
    NodeId group_id(message.destination_id());
    if (routing_table_->kNodeId() == group_id ||
            routing_table_->IsThisNodeClosestTo(group_id)) {
        // this is group master
        ChunkNodeMap chunk_node_map;
        if (!SendChooseChunkList(
                    message,
                    choose_group_req,
                    chunk_node_map)) {
            ERROR("SendChooseChunkList failed!");
            return;
        }

        DEBUG("choose all chunk list success!");
        protobuf::GroupMetaInfo group_meta;
        if (!CreateGroupMetaInfo(message, chunk_node_map, group_meta)) {
            ERROR("create group meta info failed!");
            return;
        }

        std::string data;
        if (!group_meta.SerializeToString(&data)) {
            ERROR("group_meta SerializeToString failed!");
            return;
        }

        if (!SaveGroupMeta(message.destination_id(), data)) {
            ERROR("save group meta info failed!");
            return;
        }

        const std::string source_id = message.source_id();
        RoutingMessage res_message;
        if (!CreateChooseGroupResponse(message, group_id.string(), data, res_message)) {
            return;
        }

        if (NodeId(source_id) == routing_table_->kNodeId()) {
            HandleCallbackResponse(res_message);
        } else {
            SendChooseGroupResponse(res_message);
        }

        res_message.set_source_id(source_id);
        SendChooseGroupToSlave(group_id, res_message);
    } else {
        network_->SendToClosestNode(message);
    }
}

void TopStorageMessageHandler::SendChooseGroupToSlave(
    const NodeId& group_id,
    RoutingMessage& message) {
    int group_slave_num = TopParameters::file_meta_group_num - 1;
    std::vector<NodeInfo> group_nodes = routing_table_->GetClosestNodes(
                                            group_id,
                                            group_slave_num);
    if (group_nodes.size() != group_slave_num) {
        ERROR("no more node to send group choose info to slave.now[%d] need[%d]",
              group_nodes.size(), group_slave_num);
        return;
    }

    message.clear_ack_node_ids();
    message.set_ack_id(0);
    message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kGroupChooseSlave));
    for (const auto& i : group_nodes) {
        // other node prepare and ack to group master
        message.set_destination_id(i.id.string());
        NodeInfo node;
        if (routing_table_->GetNodeInfo(i.id, node)) {
            network_->SendToDirect(message, node.id, node.connection_id);
        } else {
            network_->SendToClosestNode(message);
        }
    }
}

bool TopStorageMessageHandler::CreateChooseGroupResponse(
    const RoutingMessage& message,
    const std::string& group_id,
    const std::string& group_meta,
    RoutingMessage& proto_message) {
    protobuf::ChooseGroupResponse choose_group_res;
    choose_group_res.set_status(static_cast<int>(TopStorageMessageType::kTopSuccess));
    choose_group_res.set_group_id(group_id);
    choose_group_res.set_group_meta(group_meta);

    std::string data;
    if (!choose_group_res.SerializeToString(&data)) {
        ERROR("SerializeToString failed!");
        return false;
    }

    proto_message.set_source_id(message.source_id());
    proto_message.set_destination_id(message.source_id());
    proto_message.set_routing_message(false);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.add_data(data);
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kGroupChooseResponse));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(false);
    proto_message.set_hops_to_live(Parameters::hops_to_live - 1);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(message.id());
    return true;
}

void TopStorageMessageHandler::SendChooseGroupResponse(RoutingMessage& message) {
    message.set_source_id(routing_table_->kNodeId().string());
    NodeInfo node;
    if (routing_table_->GetNodeInfo(NodeId(message.destination_id()), node)) {
        network_->SendToDirect(message, node.id, node.connection_id);
    } else {
        network_->SendToClosestNode(message);
    }
}

void TopStorageMessageHandler::HandleChooseGroupRequest(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        ERROR("message is error, no data in.");
        return;
    }

    protobuf::ChooseGroupRequest choose_group_req;
    if (!choose_group_req.ParseFromString(message.data(0))) {
        ERROR("GroupPutDataRequest parse from string failed.");
        return;
    }

    return HandleChooseGroupMaster(message, choose_group_req);
}

void TopStorageMessageHandler::HandleGroupPutDataRequest(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        ERROR("message is error, no data in.");
        return;
    }

    protobuf::GroupPutDataRequest group_put_data_req;
    if (!group_put_data_req.ParseFromString(message.data(0))) {
        ERROR("GroupPutDataRequest parse from string failed.");
        return;
    }

    leveldb::Status status = Db::Instance()->Put(group_put_data_req.group_id(), message.data(0));
    if (!status.ok()) {
        ERROR("write group message failed!");
    }

    RoutingMessage proto_message;
    proto_message.set_source_id(routing_table_->kNodeId().string());
    proto_message.set_destination_id(message.source_id());
    proto_message.set_routing_message(false);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kGroupPutData));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(false);
    proto_message.set_hops_to_live(Parameters::hops_to_live - 1);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(message.id());

    protobuf::GroupPutDataResponse response;
    response.set_status(0);
    protobuf::ChunkInfo* chunk_info = response.add_chunk_list();
    chunk_info->set_chunk_id("test_chunk id");
    protobuf::NodeInfo* node = chunk_info->add_chunk_nodes();
    node->set_ip("test_ip");
    node->set_node_id(routing_table_->kNodeId().string());
    node->set_port(10001);
    std::string data;
    if (!response.SerializeToString(&data)) {
        ERROR("SerializeToString failed!");
        return;
    }

    proto_message.add_data(data);
    network_->SendToDirect(proto_message, maidsafe::NodeId(message.source_id()), nullptr);

}

void TopStorageMessageHandler::HandleCallbackResponse(RoutingMessage& message) {
    std::string data;
    if (!message.SerializeToString(&data)) {
        ERROR("SerializeToString failed!");
        timer_->CancelTask(message.id());
        return;
    }

    timer_->AddResponse(message.id(), data);
}

void TopStorageMessageHandler::HandleChooseGroupSlave(RoutingMessage& message) {
    if (message.data_size() <= 0) {
        ERROR("HandleChooseGroupSlave data is empty.");
        return;
    }

    protobuf::ChooseGroupResponse choose_group_res;
    if (!choose_group_res.ParseFromString(message.data(0))) {
        ERROR("ChooseGroupResponse ParseFromString failed!");
        return;
    }

    if (!SaveGroupMeta(choose_group_res.group_id(), choose_group_res.group_meta())) {
        ERROR("HandleChooseGroupSlave SaveGroupMeta failed!");
        return;
    }

    NodeId sorce_id(message.source_id());
    if (sorce_id == routing_table_->kNodeId()) {
        return HandleCallbackResponse(message);
    }

    message.set_destination_id(message.source_id());
    message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kGroupChooseResponse));
    SendChooseGroupResponse(message);
}


}  // namespace storage

}  // namespace top
