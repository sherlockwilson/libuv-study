#include "commands.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>

#include "boost/format.hpp"
#include "boost/filesystem.hpp"
#include "boost/tokenizer.hpp"
#include "boost/lexical_cast.hpp"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"
#include "common/line_parser.h"

#include "shared_response.h"
#include "message.pb.h"
#include "log.h"
#include "dict.h"
#include "utils.h"

// extern "C"{
// #include "showdowsocks/shadowsocks.h"
// extern int ss_remote_recv(char* buf, int len,short port);
// }

namespace fs = boost::filesystem;
using namespace maidsafe;
using namespace maidsafe::routing;
using namespace std::chrono;

namespace top {

namespace storage {

Commands::Commands()
    : demo_node_(), all_keys_(), all_ids_(),
      identity_index_(-1), bootstrap_peer_ep_(), data_size_(1024 * 1024),
      data_rate_(1024 * 1024), result_arrived_(false), finish_(false), wait_mutex_(),
      wait_cond_var_(), mark_results_arrived_(), show_cmd_(false),
      vpn_nodes_(), init_mutex_(), inited_(false), routing_nodes_(),boot_peer_("./bootstrap.dat") {

    PRoute_ = std::make_shared<WorldRoute>("./conf/Continent_ContinentRegion_Country.csv",
                                           "./conf/dt_cloud_zone.PN1.csv", "./conf/dt_route_table.csv");
}

Commands* Commands::Instance() {
    static Commands ins;
    return &ins;
}


/*
 * 业务 | 区域 | 角色
 * 4 bytes | 4 bytes | 1 bytes
 */
std::string Commands::GenRandomNodeId(int len, int business, std::string area, int role ) {
    if(len <= 0)
        return "";

    char  s[9];
    int random_len = len - 4 - 4 - 1;

    //业务
    uint32_t b = 0;
    if(business >= 0 && business <= 31) {
        b      = 1 << business & 0xffffffff;
    }
    memcpy(s, &b, 4);

    //区域
    std::string area_(area);
    int more_char = 4 - area_.size();
    for(int i = 0; i < more_char; ++i) {
        area_ += " ";
    }
    memcpy(s + 4, area_.data(), 4);


    //角色
    uint8_t r = 0;
    if(role >= 0 && role <= 31) {
        r      = 1 << role & 0xff;
    }
    memcpy(s + 4 + 4, &r, 1);


    std::string str(s, 4 + 4 + 1);

	LOG(log4cpp::Priority::INFO) << "generate node front half:" << maidsafe::HexEncode(str);
    return str + maidsafe::RandomString(random_len);
}


bool Commands::Init(
    DemoNodePtr demo_node,
    std::vector<maidsafe::passport::detail::AnmaidToPmid> all_keys,
    int identity_index, bool show_cmd) {
    std::unique_lock<std::mutex> lock(init_mutex_);
    if (inited_) {
        ERROR("commands has inited!");
        return false;
    }

    demo_node_ = demo_node;
    all_keys_ = std::move(all_keys);
    identity_index_ = identity_index;
    show_cmd_ = show_cmd;

    // CalculateClosests will only use all_ids_ to calculate expected respondents
    // here it is assumed that the first half of fobs will be used as vault
    // and the latter half part will be used as client, which shall not respond msg
    // i.e. shall not be put into all_ids_
    for (size_t i(0); i < (all_keys_.size() / 2); ++i)
        all_ids_.push_back(NodeId(all_keys_[i].pmid.name().value));

    demo_node->functors_.request_public_key = [this](
                const NodeId & node_id,
    GivePublicKeyFunctor give_public_key) {
        this->Validate(node_id, give_public_key);
    };  // NOLINT
    demo_node->functors_.message_and_caching.message_received = [this](
    const std::string & wrapped_message, const ReplyFunctor & reply_functor) {
        std::string reply_msg(wrapped_message + "+++" + demo_node_->node_id().string());
        if (std::string::npos != wrapped_message.find("request_routing_table"))
            reply_msg = reply_msg + "---" + demo_node_->SerializeRoutingTable();
        reply_functor(reply_msg);
    };
    mark_results_arrived_ = std::bind(&Commands::MarkResultArrived, this);
    inited_ = true;
    return true;
}

void Commands::Validate(const NodeId& node_id, GivePublicKeyFunctor give_public_key) {
    if (node_id == NodeId())
        return;
    auto iter(all_keys_.begin());
    bool find(false);
    while ((iter != all_keys_.end()) && !find) {
        if (iter->pmid.name()->string() == node_id.string())
            find = true;
        else
            ++iter;
    }
    if (iter != all_keys_.end()) {
        give_public_key((*iter).pmid.public_key());
        std::cout << "get public key:" << maidsafe::HexSubstr(node_id.string()) << ": " << maidsafe::HexSubstr(asymm::EncodeKey((*iter).pmid.public_key()).string()) << std::endl;
        std::cout << "get public key:" << maidsafe::Base64Encode(node_id.string()) << ": " << maidsafe::Base64Encode(asymm::EncodeKey((*iter).pmid.public_key()).string()) << std::endl;

    }

}

void Commands::Run() {
    PrintUsage();

    if ((!demo_node_->joined()) && (identity_index_ >= 2)) {  // &&
        //if(!demo_node_->joined()) {
        // (bootstrap_peer_ep_ != boost::asio::ip::udp::endpoint())) {
        // All parameters have been setup via cmdline directly, join the node immediately
        std::cout << "Joining the node ......" << std::endl;
        Join();
    }

    while (!finish_) {
        if (!show_cmd_) {
#ifdef _WIN32
            Sleep(1);
#else
            sleep(1);
#endif // _WIN32

            continue;
        }

        std::cout << std::endl << std::endl << "Enter command > ";
        std::string cmdline;
        std::getline(std::cin, cmdline);
        {
            std::unique_lock<std::mutex> lock(wait_mutex_);
            ProcessCommand(cmdline);
            wait_cond_var_.wait(lock, boost::bind(&Commands::ResultArrived, this));
            result_arrived_ = false;
        }
    }
}


void Commands::SetBootPeerPath(std::string bpath){
    boot_peer_ = bpath;
}


void Commands::GetEndpointFromRoutingTable(){
    std::vector<std::string> rtvec;
    demo_node_->get_routingtable_endpoint(rtvec);
    std::ofstream out(boot_peer_);
    for(auto it = rtvec.begin(); it != rtvec.end(); ++it) {
		std::string peer(*it);
		LOG(log4cpp::Priority::INFO) << "write bootstrap " << peer << " to bootstrap file" ;
        peer += "\n";
        out << peer;
		//GetPeer(peer);
    }
    out.close();
}

void Commands::PrintRoutingTable() {
/*
    auto routing_nodes = demo_node_->ReturnRoutingTable();
    std::cout << "ROUTING TABLE::::" << std::endl;
    for (const auto& routing_node : routing_nodes)
        std::cout << "\t" << maidsafe::HexSubstr(routing_node.string()) << std::endl;
*/

    routing_nodes_ = demo_node_->ReturnRoutingTable();
    routing_nodes_.insert(routing_nodes_.begin(), demo_node_->node_id());
    std::cout << "ROUTING TABLE::::" << std::endl;
    for (uint32_t i = 0; i < routing_nodes_.size(); ++i) {
        std::cout << maidsafe::HexEncode(routing_nodes_[i].string()) << std::endl;
    }

}

void Commands::GetPeer(const std::string& peer) {
    size_t delim = peer.rfind(':');
    try {
        bootstrap_peer_ep_.port(static_cast<uint16_t>(atoi(peer.substr(delim + 1).c_str())));
        bootstrap_peer_ep_.address(boost::asio::ip::address::from_string(peer.substr(0, delim)));
        std::cout << "Going to bootstrap from endpoint " << bootstrap_peer_ep_ << std::endl;
    } catch (...) {
        std::cout << "Could not parse IPv4 peer endpoint from " << peer << std::endl;
    }
    auto bootstrap_file_path(maidsafe::routing::detail::GetOverrideBootstrapFilePath<false>());
    boost::filesystem::remove(bootstrap_file_path);
    try {
        WriteBootstrapContacts(BootstrapContacts{ demo_node_->endpoint(), bootstrap_peer_ep_ },
                               bootstrap_file_path);
    } catch (const std::exception& /*error*/) {} // File updated by peer zerostate node
}

void Commands::ZeroStateJoin() {
    if (demo_node_->joined()) {
        std::cout << "Current node already joined" << std::endl;
        return;
    }
    if (identity_index_ > 1) {
        std::cout << "can't exec ZeroStateJoin as a non-bootstrap node" << std::endl;
        return;
    }



    std::condition_variable cond_var;
    std::mutex mutex;

    std::weak_ptr<GenericNode> weak_node(demo_node_);
    demo_node_->functors_.network_status = [this, &cond_var, weak_node](const int& result) {
        if (std::shared_ptr<GenericNode> node = weak_node.lock()) {
            ASSERT_GE(result, ReturnCode::kSuccess);
            if (result == static_cast<int>(node->expected()) && !node->joined()) {
                node->set_joined(true);
                cond_var.notify_one();
            } else {
                std::cout << "Network Status Changed" << std::endl;
                //this->PrintRoutingTable();
                this->ShowRoutingNodes();
            }
        }
    };


    NodeInfo peer_node_info;
    if (identity_index_ == 0) {
        peer_node_info.id = NodeId(all_keys_[1].pmid.name().value);
        peer_node_info.public_key = all_keys_[1].pmid.public_key();
    } else {
        peer_node_info.id = NodeId(all_keys_[0].pmid.name().value);
        peer_node_info.public_key = all_keys_[0].pmid.public_key();
    }
    peer_node_info.connection_id = peer_node_info.id;

    ReturnCode ret_code(
        static_cast<ReturnCode>(demo_node_->ZeroStateJoin(bootstrap_peer_ep_, peer_node_info)));
    EXPECT_EQ(ReturnCode::kSuccess, ret_code);
    if (ret_code == ReturnCode::kSuccess) {
        demo_node_->set_joined(ret_code == ReturnCode::kSuccess);
    }
}

void Commands::SendMessages(
    int identity_index,
    const maidsafe::routing::DestinationType& destination_type,
    bool is_routing_req,
    maidsafe::routing::protobuf::Message& message,
    maidsafe::routing::ResponseFunctor response_functor) {
    std::chrono::milliseconds msg_sent_time(0);
    CalculateTimeToSleep(msg_sent_time);

    int  messages_count = 1;
    if (messages_count * ((destination_type != DestinationType::kGroup) ? 1 : 4) > 10)
        Parameters::default_response_timeout *=
            (messages_count * ((destination_type != DestinationType::kGroup) ? 1 : 4));

    {
        std::vector<NodeId> closest_nodes;
        NodeId dest_id;
        int expect_respondent = MakeMessage(identity_index, destination_type, closest_nodes, dest_id);
        if (expect_respondent == 0)
            return;
        std::cout << "expect_respondent: " << expect_respondent << " dest_id: " << maidsafe::HexSubstr(dest_id.string()) << std::endl;
        auto start = std::chrono::steady_clock::now();
        SendAMessage(expect_respondent, dest_id, message, response_functor);
    }
}

int Commands::ShadowsocksSend2(char *buf, int length) {
    this->ShadowsocksSend((const char *)buf, (uint32_t)length, 9999);
    return 0;
}


void Commands::ShadowsocksSend(const char *buf, uint32_t length, uint32_t port) {
    std::string buffer(buf, length);


    protobuf::VpnPutDataRequest vpn_put_data_req;
    vpn_put_data_req.set_data(buffer);
    vpn_put_data_req.set_local_port(port);
    vpn_put_data_req.set_vpn_port(10077);
    vpn_put_data_req.set_vpn_ip("10.88.0.100");

    std::cout << "ready to send " << length << " bytes." << std::endl;

    std::string data;
    if (!vpn_put_data_req.SerializeToString(&data)) {
        std::cout << "SerializeToString failed!" << std::endl;
        return;
    }

    RoutingMessage proto_message;
    proto_message.set_source_id(demo_node_->node_id().string());
    proto_message.set_destination_id(GetNodeIdByIndex(1).string());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kVpnPutDataRequest));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(true);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(RandomUint32());
    SendMessages(-1, DestinationType::kDirect, true, proto_message, nullptr);
}

void Commands::SendMessages(int id_index, const DestinationType& destination_type,
                            bool is_routing_req, int messages_count) {
    std::string data, data_to_send;
    //  Check message type
    if (is_routing_req)
        data = "request_routing_table";
    else
        data_to_send = data = RandomAlphaNumericString(data_size_);
    std::chrono::milliseconds msg_sent_time(0);
    CalculateTimeToSleep(msg_sent_time);

    bool infinite(false);
    if (messages_count == -1)
        infinite = true;

    uint32_t message_id(0);
    unsigned int expect_respondent(0);
    std::atomic<int> successful_count(0);
    std::mutex mutex;
    std::condition_variable cond_var;
    unsigned int operation_count(0);
    //   Send messages
    auto timeout(Parameters::default_response_timeout);
    std::cout << "message_count " << messages_count << std::endl;
    if (messages_count * ((destination_type != DestinationType::kGroup) ? 1 : 4) > 10)
        Parameters::default_response_timeout *=
            (messages_count * ((destination_type != DestinationType::kGroup) ? 1 : 4));
    for (int index = 0; index < messages_count || infinite; ++index) {
        std::vector<NodeId> closest_nodes;
        NodeId dest_id;
        expect_respondent = MakeMessage(id_index, destination_type, closest_nodes, dest_id);
        if (expect_respondent == 0)
            return;
        auto start = std::chrono::steady_clock::now();
        data = ">:< " + std::to_string(++message_id) + " <:>" + data;
        SendAMessage(successful_count, operation_count, mutex, cond_var, messages_count,
                     expect_respondent, closest_nodes, dest_id, data);

        data = data_to_send;
    }
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (operation_count != (messages_count * expect_respondent))
            cond_var.wait(lock);
    }
    std::cout << "Succcessfully received messages count::" << successful_count << std::endl;
    std::cout << "Unsucccessfully received messages count::" << (messages_count - successful_count)
              << std::endl;
    Parameters::default_response_timeout = timeout;
}

NodeId Commands::GetNodeIdByIndex(int id_index) {
    int identity_index;
    if (id_index >= 0)
        identity_index = id_index;
    else
        identity_index = RandomUint32() % (all_keys_.size() / 2);

    if ((identity_index >= 0) && (static_cast<uint32_t>(identity_index) >= all_keys_.size())) {
        std::cout << "ERROR : destination index out of range" << std::endl;
        return NodeId();
    }

    if (identity_index >= 0) {
        return NodeId(all_keys_[identity_index].pmid.name().value);
    }

    return NodeId();
}


unsigned int Commands::MakeMessage(int id_index, const DestinationType& destination_type,
                                   std::vector<NodeId>& closest_nodes, NodeId& dest_id) {
    int identity_index;
    if (id_index >= 0)
        identity_index = id_index;
    else
        identity_index = RandomUint32() % (routing_nodes_.size());

    if ((identity_index >= 0) && (static_cast<uint32_t>(identity_index) >= routing_nodes_.size())) {
        std::cout << "ERROR : destination index out of range" << std::endl;
        return 0;
    }
    if (identity_index >= 0) {
        if (destination_type == DestinationType::kGroup)
            dest_id = NodeId(RandomString(NodeId::kSize));
        else
            dest_id = routing_nodes_[identity_index];
    }
    std::cout << "Sending a msg from : " << maidsafe::HexSubstr(demo_node_->node_id().string())
              << " to " << (destination_type != DestinationType::kGroup ? ": " : "group : ")
              << maidsafe::HexSubstr(dest_id.string())
              << " , expect receive response from :" << std::endl;
    unsigned int expected_respodents(destination_type != DestinationType::kGroup ? 1 : 4);
    std::vector<NodeId> closests;
    if (destination_type == DestinationType::kGroup)
        NodeId farthest_closests(CalculateClosests(dest_id, closests, expected_respodents));
    else
        closests.push_back(dest_id);
    for (const auto& node_id : closests)
        std::cout << "\t" << maidsafe::HexSubstr(node_id.string()) << std::endl;
    closest_nodes = closests;
    return expected_respodents;
}

void Commands::CalculateTimeToSleep(std::chrono::milliseconds& msg_sent_time) {
    size_t num_msgs_per_second = data_rate_ / data_size_;
    msg_sent_time = std::chrono::milliseconds(1000 / num_msgs_per_second);
}

void Commands::SendAMessage(
    unsigned int expect_respondent,
    maidsafe::NodeId dest_id,
    maidsafe::routing::protobuf::Message& message,
    maidsafe::routing::ResponseFunctor response_functor) {
    if (expect_respondent == 1) {
        demo_node_->SendDirect(dest_id, message, false, response_functor);
    } else {
        demo_node_->SendGroup(dest_id, message, false, response_functor);
    }
}

void Commands::SendAMessage(std::atomic<int>& successful_count, unsigned int& operation_count,
                            std::mutex& mutex, std::condition_variable& cond_var,
                            int messages_count, unsigned int expect_respondent,
                            std::vector<NodeId> closest_nodes, NodeId dest_id, std::string data) {
    bool group_performance(false);
    if ((expect_respondent > 1) && (closest_nodes.empty()))
        group_performance = true;
    auto data_size(data.size());
    auto shared_response_ptr = std::make_shared<SharedResponse>(closest_nodes, expect_respondent);
    auto callable = [shared_response_ptr, &successful_count, &operation_count, &mutex,
                                          messages_count, expect_respondent, &cond_var, group_performance,
                         data_size, this](std::string response) {
        if (!response.empty()) {
            std::string string(response.substr(0, 20));
            std::cout << "Received message: " << string << "\n";
            if (!shared_response_ptr->CollectResponse(response, !group_performance))
                return;
            if (shared_response_ptr->expected_responses_ == 1)
                shared_response_ptr->PrintRoutingTable(response);
            if (shared_response_ptr->responded_nodes_.size() ==
                    shared_response_ptr->closest_nodes_.size()) {
                shared_response_ptr->CheckAndPrintResult();
                ++successful_count;
            }
        } else {
            std::cout << "Error Response received in "
                      << boost::posix_time::microsec_clock::universal_time() -
                      shared_response_ptr->msg_send_time_ << std::endl;
        }
        {
            std::lock_guard<std::mutex> lock(mutex);
            ++operation_count;
            if (operation_count == (messages_count * expect_respondent)) {
                if (group_performance)
                    shared_response_ptr->PrintGroupPerformance(static_cast<int>(data_size));
                cond_var.notify_one();
            }
        }
    };
    //  Send the msg
    if (expect_respondent == 1) {
        demo_node_->SendDirect(dest_id, data, false, callable);
    } else {
        if (group_performance)
            std::cout << "Group message sent to " << DebugId(dest_id);
        demo_node_->SendGroup(dest_id, data, false, callable);
    }
}

int Commands::Join() {
    if (demo_node_->joined()) {
        std::cout << "Current node already joined" << std::endl;
        return 0;
    }
    std::condition_variable cond_var;
    std::mutex mutex;

    std::weak_ptr<GenericNode> weak_node(demo_node_);
    demo_node_->functors_.network_status = [this, &cond_var, weak_node](const int& result) {
        if (std::shared_ptr<GenericNode> node = weak_node.lock()) {
            ASSERT_GE(result, ReturnCode::kSuccess);
            if (result == static_cast<int>(node->expected()) && !node->joined()) {
                node->set_joined(true);
                cond_var.notify_one();
            } else {
                std::cout << "Network Status Changed" << std::endl;
                //this->PrintRoutingTable();
                this->ShowRoutingNodes();
				this->GetEndpointFromRoutingTable();
            }
        }
    };

    demo_node_->Join();

    if (!demo_node_->joined()) {
        std::unique_lock<std::mutex> lock(mutex);
        auto result = cond_var.wait_for(lock, std::chrono::seconds(20));
        EXPECT_EQ(result, std::cv_status::no_timeout);
        Sleep(std::chrono::milliseconds(600));
    }
    std::cout << "Current Node joined, following is the routing table :" << std::endl;
    PrintRoutingTable();

    if (demo_node_->joined()) {
        return 0;
    }

    return 1;
}

void Commands::PrintUsage() {
    std::cout << "\thelp Print options.\n";
    std::cout << "\tpeer <endpoint> Set BootStrap peer endpoint.\n";
    std::cout << "------ Current BootStrap node endpoint info : " << demo_node_->endpoint() << " ------ " << std::endl;
    std::cout << "\tzerostatejoin ZeroStateJoin.\n";
    std::cout << "\tjoin Normal Join.\n";
    std::cout << "\tprt Print Local Routing Table.\n";
    std::cout << "\tpb  Print endpoint from Local Routing Table.\n";
    std::cout << "\tedgecode <src> <dst>  Get Edge Country Code base src and dst .\n";
    std::cout << "\trrt <dest_index> Request Routing Table from peer node with the specified"
              << " identity-index.\n";
    std::cout << "\tsenddirect <dest_index> <num_msg> Send a msg to a node with specified"
              << "  identity-index. -1 for infinite (Default 1)\n";
    std::cout << "\tsendgroup <dest_index> Send a msg to group (default is Random GroupId,"
              << " dest_index for using existing identity as a group_id)\n";
    std::cout << "\tsendmultiple <num_msg> Send num of msg to randomly picked-up destination."
              << " -1 for infinite (Default 10)\n";
    std::cout << "\tsendgroupmultiple <num_msg> Send num of group msg to randomly "
              << " picked-up destination. -1 for infinite\n";
    std::cout << "\tdatasize <data_size> Set the data_size for the message.\n";
    std::cout << "\tdatarate <data_rate> Set the data_rate for the message.\n";
    std::cout << "\tattype Print the NatType of this node.\n";
    std::cout << "\tperformance Execute performance test from this node.\n";
    std::cout << "\texit Exit application.\n";
}

void Commands::ProcessCommand(const std::string& cmdline) {
    if (cmdline.empty()) {
        demo_node_->PostTaskToAsioService(mark_results_arrived_);
        return;
    }

    std::string cmd;
    Arguments args;
    try {
        boost::char_separator<char> sep(" ");
        boost::tokenizer<boost::char_separator<char>> tok(cmdline, sep);
        for (auto it = tok.begin(); it != tok.end(); ++it) {
            if (it == tok.begin())
                cmd = *it;
            else
                args.push_back(*it);
        }
    } catch (const std::exception& e) {
        LOG(kError) << "Error processing command: " << e.what();
    }

    if (cmd == "help") {
        PrintUsage();
    } else if (cmd == "prt") {
        PrintRoutingTable();
    } else if (cmd == "pb") {
        GetEndpointFromRoutingTable();
    } else if (cmd == "edgecode") {
        if(args.size() == 2) {
            GetEdgeRoute(args[0], args[1]);
        } else {
            GetEdgeRoute("cn", "Us");
        }
    } else if (cmd == "edge") {
        std::vector<top::vpn::VpnEndPoint> ev;
        if(args.size() == 1) {
            GetAllEdgesInfo(args[0], ev);
        } else {
            GetAllEdgesInfo("US", ev);
        }
    } else if (cmd == "rrt") {
        if (args.size() == 1) {
            SendMessages(atoi(args[0].c_str()), DestinationType::kDirect, true, 1);
        } else {
            std::cout << "Error : Try correct option" << std::endl;
        }
    } else if (cmd == "peer") {
        if (args.size() == 1)
            GetPeer(args[0]);
        else
            std::cout << "Error : Try correct option" << std::endl;
    } else if (cmd == "zerostatejoin") {
        ZeroStateJoin();
    } else if (cmd == "join") {
        Join();
    } else if (cmd == "senddirect") {
        if (args.size() == 1) {
            SendMessages(atoi(args[0].c_str()), DestinationType::kDirect, false, 1);
        } else if (args.size() == 2) {
            int count(atoi(args[1].c_str()));
            bool infinite(count < 0);
            if (infinite) {
                std::cout << " Running infinite messaging test. press Ctrl + C to terminate the program"
                          << std::endl;
                SendMessages(atoi(args[0].c_str()), DestinationType::kDirect, false, -1);
            } else {
                SendMessages(atoi(args[0].c_str()), DestinationType::kDirect, false, count);
            }
        }
    } else if (cmd == "sendgroup") {
        if (args.empty())
            SendMessages(-1, DestinationType::kGroup, false, 1);
        else
            SendMessages(atoi(args[0].c_str()), DestinationType::kGroup, false, 1);
    } else if (cmd == "sendgroupmultiple") {
        if (args.size() == 1) {
            SendMessages(-1, DestinationType::kGroup, false, atoi(args[0].c_str()));
        }
    } else if (cmd == "sendmultiple") {
        int num_msg(10);
        if (!args.empty())
            num_msg = atoi(args[0].c_str());
        if (num_msg == -1) {
            std::cout << " Running infinite messaging test. press Ctrl + C to terminate the program"
                      << std::endl;
            SendMessages(-1, DestinationType::kDirect, false, -1);
        } else {
            SendMessages(-1, DestinationType::kDirect, false, num_msg);
        }
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
        std::cout << "Sent " << num_msg << " messages to randomly picked-up targets. Finished in :"
                  << boost::posix_time::microsec_clock::universal_time() - now << std::endl;
    } else if (cmd == "datasize") {
        if (args.size() == 1)
            data_size_ = atoi(args[0].c_str());
        else
            std::cout << "Error : Try correct option" << std::endl;
    } else if (cmd == "datarate") {
        if (args.size() == 1)
            data_rate_ = atoi(args[0].c_str());
        else
            std::cout << "Error : Try correct option" << std::endl;
    } else if (cmd == "nattype") {
        std::cout << "NatType for this node is : " << demo_node_->nat_type() << std::endl;
    } else if (cmd == "performance") {
        PerformanceTest();
    } else if (cmd == "exit") {
        std::cout << "Exiting application...\n";
        finish_ = true;
    } else if (cmd == "put_group") {
        PutGroupData(atoi(args[0].c_str()));
    } else if (cmd == "sslocal") {
        ShadowsocksSend("hello", 5, 9999);
    } else if (cmd == "vpns") {
        GetVpnNodes("0");
    } else if (cmd == "nodes") {
        ShowRoutingNodes();
    } else if (cmd == "convpn") {
        if (vpn_nodes_.size() <= 0) {
            std::cout << "please use cmd: vpns  to choose 4 vpn nodes." << std::endl;
            return;
        }

        if (args.size() < 1) {
            std::cout << "please choose 0 ~ " << vpn_nodes_.size() << " to choose connect to vpn server." << std::endl;
            return;
        }
        ConnectVpn(std::atoi(args[0].c_str()));
    } else if (cmd == "put_chunk") {
        if (args.size() < 4) {
            std::cout << "args error, mast has chunk index, chunk server ip and port" << std::endl;
        } else {
            PutDataToChunkNode(
                atoi(args[0].c_str()),
                args[1].c_str(),
                args[2].c_str(),
                args[3].c_str());
        }
    } else if (cmd == "choose_group") {
        if (args.size() < 2) {
            std::cout << "args error, mast has group index and chunk index" << std::endl;
        } else {
            auto start = system_clock::now();
            ChooseGroup(atoi(args[0].c_str()), atoi(args[1].c_str()));
            auto end = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            std::cout << "choose group chunk_size: "
                      << int(pow(2.0, atof(args[1].c_str())))
                      << " use time: "
                      << (double(duration.count()) * microseconds::period::num / microseconds::period::den)
                      << "s " << std::endl;

        }
    } else {
        std::cout << "Invalid command : " << cmd << std::endl;
        PrintUsage();
    }
    demo_node_->PostTaskToAsioService(mark_results_arrived_);
}

void Commands::ShowRoutingNodes() {
    routing_nodes_ = demo_node_->ReturnRoutingTable();
    routing_nodes_.insert(routing_nodes_.begin(), demo_node_->node_id());
    for (uint32_t i = 0; i < routing_nodes_.size(); ++i) {
        std::cout << maidsafe::HexSubstr(routing_nodes_[i].string()) << std::endl;
    }
}

void Commands::ConnectVpn(int node_idx) {
    if (node_idx < 0 || node_idx > vpn_nodes_.size()) {
        std::cout << "node index unvalid: " << node_idx << std::endl;
        return;
    }

    protobuf::VpnServerInfoRequest vpn_req;
    vpn_req.set_test("test");
    std::string data;
    if (!vpn_req.SerializeToString(&data)) {
        ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        return;
    }

    RoutingMessage proto_message;
    proto_message.set_source_id(demo_node_->node_id().string());
    proto_message.set_destination_id(vpn_nodes_[node_idx]);
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kVpnServerInfoRequest));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(true);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(TopParameters::message_id++);

    int operation_count = 0;
    std::mutex mutex;
    std::condition_variable cond_var;

    auto callable = [&operation_count, &mutex, &cond_var, this](std::string data) {
        RoutingMessage res_message;
        if (res_message.ParseFromString(data)) {
            if (res_message.data_size() > 0) {
                protobuf::VpnServerInfoResponse response;
                if (response.ParseFromString(res_message.data(0))) {
                    std::cout << "commands test callback called!" << std::endl;
                    std::cout << response.ip() << ":" << response.port() << std::endl;
                    std::cout << response.passwd() << ":" << response.method() << std::endl;
                    std::cout << "start shadowsocks client..." << std::endl;
                    int ret;
                    char cmd[512];
                    sprintf(
                        cmd,
                        "sudo sslocal -s %s -p %d -b 127.0.0.1 -l 1080 "
                        "-k \"%s\" -m \"%s\" -d restart",
                        response.ip().c_str(),
                        response.port(),
                        response.passwd().c_str(),
                        response.method().c_str());
                    ret = system(cmd);
                    std::cout << "sslocal: The value returned was: " << ret << std::endl;
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
            cond_var.notify_one();
        }
    };

    auto timeout(Parameters::default_response_timeout);
    SendMessages(-1, DestinationType::kDirect, true, proto_message, callable);
    {
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock);
    }
    Parameters::default_response_timeout = timeout;
}

void Commands::BitvpnSend(const std::string& msg) {
    protobuf::BitvpnDataRequest bitvpn_req;
    bitvpn_req.set_msg(msg);
    std::string data;
    if (!bitvpn_req.SerializeToString(&data)) {
        ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        return;
    }

    RoutingMessage proto_message;
    proto_message.set_source_id(demo_node_->node_id().string());
    proto_message.set_destination_id(GetNodeIdByIndex(0).string());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kBitvpnDataRequest));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(true);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(TopParameters::message_id++);

    SendMessages(-1, DestinationType::kDirect, true, proto_message, nullptr);
    std::cout << "bitvpn message sended!" << std::endl;
}


std::string Commands::GetEdgeRoute(std::string src_country_code, std::string dst_country_code) {
    std::string edge = PRoute_->GetBestRoute(src_country_code, dst_country_code);
    if(!edge.empty()) {
        std::cout << "src_country_code: " << src_country_code
                  << " dst_country_code: " << dst_country_code
                  << " edge: " << edge << std::endl;
    } else {
        std::cout << "no edge available" << std::endl;
    }
    return edge;
}


//给定业务类型和区域，寻找匹配的 edge 节点
void Commands::GetEdgeNodes(const int &business, const std::string &area) {
    // search edge node, role = 1
    NodeId dest_id = NodeId(Commands::GenRandomNodeId(maidsafe::NodeId::kSize, business, area, 1));
    protobuf::FindVpnNodesRequest find_edge_req;
    find_edge_req.set_count(4);
    find_edge_req.set_key(dest_id.string());
    std::string data;
    if (!find_edge_req.SerializeToString(&data)) {
        ERROR("kFindEdgeNodesRequest SerializeToString failed!");
        return;
    }
    RoutingMessage proto_message;
    proto_message.set_source_id(demo_node_->node_id().string());
    proto_message.set_destination_id(dest_id.string());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kFindEdgeNodesRequest));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(true);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(TopParameters::message_id++);

    int operation_count = 0;
    std::mutex mutex;
    std::condition_variable cond_var;

    auto callable = [&operation_count, &mutex,
                      &cond_var, this](std::string data) {
        RoutingMessage res_message;
        if (res_message.ParseFromString(data)) {
            if (res_message.data_size() > 0) {
                protobuf::FindEdgeNodesResponse response;
                if (response.ParseFromString(res_message.data(0))) {
                    std::cout << "get edge nodes: " << response.nodes_size() << std::endl;
                    edge_nodes_.clear();
                    for (int i = 0; i < response.nodes_size(); ++i) {
                        edge_nodes_.push_back(response.nodes(i));
                        std::cout << maidsafe::HexSubstr(response.nodes(i)) << std::endl;
                    }
                } else {
                    ERROR("kFindEdgeNodesResponse ParseFromString failed!");
                }
            } else {
                ERROR("no data, kFindEdgeNodesResponse ParseFromString failed!");
            }
        } else {
            ERROR("RoutingMessage ParseFromString failed!");
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
            cond_var.notify_one();
        }
    };

    auto timeout(Parameters::default_response_timeout);
    SendMessages(-1, DestinationType::kDirect, true, proto_message, callable);
    {
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock);
    }
    Parameters::default_response_timeout = timeout;
}


//get tcp/udp ip:port from edge
void Commands::GetEdgeInfo(size_t node_idx, std::vector<top::vpn::VpnEndPoint > &endpoint_vec) {
    if (edge_nodes_.size() <= 0 || node_idx > edge_nodes_.size() - 1) {
        std::cout << "edge nodes not available or node index illegal" << std::endl;
        return;
    }

    //random generate a node_index
    //size_t node_idx = maidsafe::RandomUint32() % edge_nodes_.size();

    protobuf::EdgeInfoRequest edge_req;
    edge_req.set_test("test");
    std::string data;
    if (!edge_req.SerializeToString(&data)) {
        ERROR("EdgeInfoRequest SerializeToString failed!");
        return;
    }

    RoutingMessage proto_message;
    proto_message.set_source_id(demo_node_->node_id().string());
    proto_message.set_destination_id(edge_nodes_[node_idx]);
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kEdgeInfoRequest));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(true);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(TopParameters::message_id++);

    int operation_count = 0;
    std::mutex mutex;
    std::condition_variable cond_var;

    auto callable = [&operation_count, &mutex, &cond_var, this, &endpoint_vec](std::string data) {
        RoutingMessage res_message;
        if (res_message.ParseFromString(data)) {
            if (res_message.data_size() > 0) {
                protobuf::EdgeInfoResponse response;
                if (response.ParseFromString(res_message.data(0))) {
                    std::cout << "commands getedgeinfo callback called!" << std::endl;

                    top::vpn::VpnEndPoint vet = {top::vpn::VpnProtocal::kTcp, response.ip(), response.tcp_port()};
                    endpoint_vec.push_back(vet);

                    top::vpn::VpnEndPoint veu = {top::vpn::VpnProtocal::kUdp, response.ip(), response.udp_port()};
                    endpoint_vec.push_back(veu);
                } else {
                    ERROR("EdgeInfoResponse ParseFromString failed!");
                }
            } else {
                ERROR("no data, EdgeInfoResponse ParseFromString failed!");
            }
        } else {
            ERROR("RoutingMessage ParseFromString failed!");
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
            cond_var.notify_one();
        }
    };

    auto timeout(Parameters::default_response_timeout);
    SendMessages(-1, DestinationType::kDirect, true, proto_message, callable);
    {
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock);
    }
    Parameters::default_response_timeout = timeout;
}


void Commands::GetAllEdgesInfo(const std::string dst_country_code, std::vector<top::vpn::VpnEndPoint> &endpoint_vec) {
    //TODO  get src_country_code from conf
    //std::string src_country_code("CN");
    std::string src_country_code;
    Dict::Instance()->Hget(top::storage::LOCAL_COUNTRY_DB_KEY,"code",&src_country_code);

    std::string edge = GetEdgeRoute(src_country_code, dst_country_code);
    if(edge.empty()) {
        return;
    }

    GetEdgeNodes(kVpn, edge);
    for(size_t i = 0 ; i < edge_nodes_.size(); ++i) {
        GetEdgeInfo(i, endpoint_vec);
    }

    for(auto it = endpoint_vec.begin(); it != endpoint_vec.end(); ++it) {
        std::cout << "VpnEndPoint.protocal = " << (*it).protocal << std::endl;
        std::cout << "VpnEndPoint.ip       = " << (*it).ip << std::endl;
        std::cout << "VpnEndPoint.port     = " << (*it).port << std::endl;
    }
}



void Commands::GetVpnNodes(const std::string&) {
    NodeId dest_id = NodeId(RandomString(NodeId::kSize));
    protobuf::FindVpnNodesRequest find_vpn_req;
    find_vpn_req.set_count(4);
    find_vpn_req.set_key(dest_id.string());
    std::string data;
    if (!find_vpn_req.SerializeToString(&data)) {
        ERROR("ChooseChunkNodesRequest SerializeToString failed!");
        return;
    }

    RoutingMessage proto_message;
    proto_message.set_source_id(demo_node_->node_id().string());
    proto_message.set_destination_id(dest_id.string());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kFindVpnNodesRequest));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(true);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(TopParameters::message_id++);

    int operation_count = 0;
    std::mutex mutex;
    std::condition_variable cond_var;

    auto callable = [&operation_count, &mutex,
                      &cond_var, this](std::string data) {
        RoutingMessage res_message;
        if (res_message.ParseFromString(data)) {
            if (res_message.data_size() > 0) {
                protobuf::FindVpnNodesResponse response;
                if (response.ParseFromString(res_message.data(0))) {
                    std::cout << "get vpn nodes: " << response.nodes_size() << std::endl;
                    vpn_nodes_.clear();
                    for (int i = 0; i < response.nodes_size(); ++i) {
                        vpn_nodes_.push_back(response.nodes(i));
                        std::cout << maidsafe::HexSubstr(response.nodes(i)) << std::endl;
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
            cond_var.notify_one();
        }
    };

    auto timeout(Parameters::default_response_timeout);
    SendMessages(-1, DestinationType::kDirect, true, proto_message, callable);
    {
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock);
    }
    Parameters::default_response_timeout = timeout;
}

void Commands::MarkResultArrived() {
    {
        std::lock_guard<std::mutex> lock(wait_mutex_);
        result_arrived_ = true;
    }
    wait_cond_var_.notify_one();
}

//callback func, called when data is transfered over.
int utp_send_over_callback(char* data, int status) {
    return 0;
}

void Commands::PutDataToChunkNode(int id_index, const char* ip, const char* port, const char* slavers) {
    protobuf::SaveChunkRequest save_chunk_req;
    save_chunk_req.set_group_id(GetNodeIdByIndex(id_index).string());
    save_chunk_req.set_chunk_id(GetNodeIdByIndex(id_index).string());
    save_chunk_req.set_chunk_data(std::string("hello chunk server"));
    save_chunk_req.set_request_type(0);  // 0 is send to master

    top::common::LineParser line_parser(slavers, ',');
    for (int i = 0; i < line_parser.Count(); ++i) {
        top::common::LineParser ip_port(line_parser[i], ':');
        if (ip_port.Count() != 2) {
            std::cout << "slavers is error: " << slavers << std::endl;
            return;
        }

        protobuf::NodeInfo* node_info = save_chunk_req.add_chunk_nodes();
        node_info->set_ip(ip_port[0]);
        node_info->set_port(atoi(ip_port[1]));
        node_info->set_node_id("node_id");
    }

    std::string data;
    if (!save_chunk_req.SerializeToString(&data)) {
        std::cout << "SerializeToString failed!" << std::endl;
        return;
    }
    std::cout << "send data: " << data.size() << std::endl;
}

void Commands::PutGroupData(int chunk_size) {
    protobuf::GroupPutDataRequest group_put_data_req;
    group_put_data_req.set_group_id("test_group");
    group_put_data_req.set_chunk_size(chunk_size);
    group_put_data_req.set_size(100);
    group_put_data_req.set_node_id("test_node_id");
    for (int i = 0; i < 10; ++i) {
        protobuf::MerkleTreeLevel* merkle_tree_level = group_put_data_req.add_merkle_tree();
        for (int j = 0; j <= i; ++j) {
            merkle_tree_level->add_level_hash(std::string("ip"));
        }
    }

    std::string data;
    if (!group_put_data_req.SerializeToString(&data)) {
        std::cout << "SerializeToString failed!" << std::endl;
        return;
    }

    RoutingMessage proto_message;
    proto_message.set_source_id(demo_node_->node_id().string());
    // 设置group_id
    proto_message.set_destination_id(GetNodeIdByIndex(11).string());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kGroupPutData));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(true);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(RandomUint32());

    int operation_count = 0;
    std::mutex mutex;
    int expect_respondent = Parameters::group_size;
    std::condition_variable cond_var;

    auto callable = [&operation_count, &mutex, expect_respondent, &cond_var, this](std::string data) {
        RoutingMessage res_message;
        if (res_message.ParseFromString(data)) {
            if (res_message.data_size() > 0) {
                protobuf::GroupPutDataResponse response;
                if (response.ParseFromString(res_message.data(0))) {
                    std::cout << "status: " << response.status() << std::endl;
                    std::cout << "chunk size: " << response.chunk_list_size() << std::endl;
                } else {
                    std::cout << "group put data message parse failed!" << std::endl;
                }
            }
        } else {
            std::cout << "routing message parse failed! data: " << data << std::endl;
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
            ++operation_count;
            if (operation_count == expect_respondent) {
                cond_var.notify_one();
            }
        }

        std::cout << operation_count << ":" << expect_respondent << std::endl;
    };

    auto timeout(Parameters::default_response_timeout);
    SendMessages(-1, DestinationType::kGroup, true, proto_message, callable);
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (operation_count != expect_respondent)
            cond_var.wait(lock);
    }

    std::cout << "send over." << std::endl;
    Parameters::default_response_timeout = timeout;

}

void Commands::ChooseGroup(int id_index, int chunk_size) {
    protobuf::ChooseGroupRequest choose_group_req;
    choose_group_req.set_group_id("test_group");
    choose_group_req.set_chunk_size(chunk_size);
    choose_group_req.set_size(11);
    choose_group_req.set_node_id(NodeId(RandomString(NodeId::kSize)).string());
    choose_group_req.set_is_master_send(0);
    for (int i = 0; i < choose_group_req.chunk_size(); ++i) {
        protobuf::MerkleTreeLevel* merkle_tree_level = choose_group_req.add_merkle_tree();
        int hash_size = int(std::pow(2.0, double(i)));
        std::cout << hash_size << std::endl;
        for (int j = 0; j < hash_size; ++j) {
            merkle_tree_level->add_level_hash(NodeId(RandomString(NodeId::kSize)).string());
        }
    }

    std::string data;
    if (!choose_group_req.SerializeToString(&data)) {
        std::cout << "SerializeToString failed!" << std::endl;
        return;
    }

    RoutingMessage proto_message;
    proto_message.set_source_id(demo_node_->node_id().string());
    // 设置group_id
    proto_message.set_destination_id(GetNodeIdByIndex(id_index).string());
    proto_message.set_routing_message(false);
    proto_message.add_data(data);
    proto_message.set_type(static_cast<int32_t>(MessageType::kNodeLevel));
    proto_message.set_direct(true);
    proto_message.set_top_type(static_cast<int32_t>(TopStorageMessageType::kGroupChooseRequest));
    proto_message.set_cacheable(static_cast<int32_t>(0));
    proto_message.set_client_node(false);
    proto_message.set_request(true);
    proto_message.set_hops_to_live(Parameters::hops_to_live);
    proto_message.set_ack_id(0);
    proto_message.set_actual_destination_is_relay_id(false);
    proto_message.set_id(RandomUint32());

    int operation_count = 0;
    std::mutex mutex;
    int expect_respondent = TopParameters::file_meta_group_num;
    std::condition_variable cond_var;

    auto callable = [&operation_count, &mutex, expect_respondent, &cond_var, this](std::string data) {
        RoutingMessage res_message;
        if (res_message.ParseFromString(data)) {
            if (res_message.data_size() > 0) {
                protobuf::ChooseGroupResponse response;
                if (response.ParseFromString(res_message.data(0))) {
                    std::cout << "status: " << response.status() << std::endl;
                } else {
                    std::cout << "group put data message parse failed!" << std::endl;
                }
            }
        } else {
            std::cout << "routing message parse failed!" << std::endl;
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
            ++operation_count;
            if (operation_count == expect_respondent) {
                cond_var.notify_one();
            }
        }

        std::cout << operation_count << ":" << expect_respondent << std::endl;
    };

    std::cout << "cmmand msg id: " << proto_message.id() << std::endl;
    demo_node_->AddTask(callable, 4, proto_message.id());
    auto timeout(Parameters::default_response_timeout);
    SendMessages(id_index, DestinationType::kDirect, true, proto_message, nullptr);
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (operation_count != expect_respondent)
            cond_var.wait(lock);
    }

    std::cout << "send over." << std::endl;
    Parameters::default_response_timeout = timeout;
}

NodeId Commands::CalculateClosests(const NodeId& target_id, std::vector<NodeId>& closests,
                                   unsigned int num_of_closests) {
    if (all_ids_.size() <= num_of_closests) {
        closests = all_ids_;
        return closests[closests.size() - 1];
    }
    std::sort(all_ids_.begin(), all_ids_.end(), [&](const NodeId & lhs, const NodeId & rhs) {
        return NodeId::CloserToTarget(lhs, rhs, target_id);
    });
    closests = std::vector<NodeId>(
                   all_ids_.begin() + boost::lexical_cast<bool>(all_ids_[0] == target_id),
                   all_ids_.begin() + num_of_closests + boost::lexical_cast<bool>(all_ids_[0] == target_id));
    return closests[closests.size() - 1];
}

void Commands::PerformanceTest() {
    std::cout << "*************  Performance Test Sending Direct Message *************" << std::endl;
    RunPerformanceTest(false);
    std::cout << "*************  Performance Test Sending Group Message *************" << std::endl;
    RunPerformanceTest(true);
}

void Commands::RunPerformanceTest(bool is_send_group) {
    data_size_ = 1;
    int iteration(1);
    uint32_t message_id(0);
    unsigned int expect_respondent(is_send_group ? routing::Parameters::group_size : 1);
    std::vector<NodeId> closest_nodes;
    while (data_size_ < ((1024 * 1024) + 1024)) {
        std::string data, data_to_send;
        data_to_send = data = RandomAlphaNumericString(data_size_);

        auto routing_nodes = demo_node_->ReturnRoutingTable();
        for (const auto& routing_node : routing_nodes) {
            std::atomic<int> successful_count(0);
            std::mutex mutex;
            std::condition_variable cond_var;
            unsigned int operation_count(0);
            data = ">:<" + std::to_string(++message_id) + "<:>" + data;
            SendAMessage(successful_count, operation_count, mutex, cond_var, 1,
                         expect_respondent, closest_nodes, routing_node, data);
            data = data_to_send;  // remove the message_id part
            {
                std::unique_lock<std::mutex> lock(mutex);
                // shall setup a timed out here ?
                if (operation_count != expect_respondent)
                    cond_var.wait(lock);
            }
        }
        data_size_ = 1000 * iteration;
        iteration *= 2;
    }
}

}  //  namespace routing

}  //  namespace maidsafe
