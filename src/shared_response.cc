#include "shared_response.h"

#include <iostream>

#include "boost/format.hpp"
#include "boost/tokenizer.hpp"
#include "boost/lexical_cast.hpp"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

using namespace maidsafe;
using namespace maidsafe::routing;

namespace top {

namespace storage {

void SharedResponse::CheckAndPrintResult() {
    if (responded_nodes_.empty())
        return;

    std::cout << "Received response from following nodes :" << std::endl;
    for (const auto& responsed_node : responded_nodes_) {
        std::cout << "\t" << maidsafe::HexSubstr(responsed_node.string()) << std::endl;
        EXPECT_TRUE(std::find(closest_nodes_.begin(), closest_nodes_.end(), responsed_node) !=
            closest_nodes_.end());
    }
    std::cout << "Average time taken for receiving msg:"
        << (average_response_time_.total_milliseconds() / responded_nodes_.size())
        << " milliseconds" << std::endl;
}

void SharedResponse::PrintRoutingTable(std::string response) {
    if (std::string::npos != response.find("request_routing_table")) {
        std::string response_node_list_msg(
            response.substr(response.find("---") + 3, response.size() - (response.find("---") + 3)));
        std::vector<NodeId> node_list(maidsafe::routing::DeserializeNodeIdList(response_node_list_msg));
        std::cout << "RECEIVED ROUTING TABLE::::" << std::endl;
        for (const auto& node_id : node_list)
            std::cout << "\t" << maidsafe::HexSubstr(node_id.string()) << std::endl;
    }
}

bool SharedResponse::CollectResponse(std::string response, bool print_performance) {
    std::lock_guard<std::mutex> lock(mutex_);
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
    std::string response_id(response.substr(response.find("+++") + 3, 64));
    //   std::cout << "Response with size of " << response.size()
    //             << " bytes received in " << now - msg_send_time_ << " seconds" << std::endl;
    std::cout << "a message from " << NodeId(response_id);
    auto duration((now - msg_send_time_).total_milliseconds());
    if (duration < Parameters::default_response_timeout.count()) {
        if (responded_nodes_.find(NodeId(response_id)) != std::end(responded_nodes_)) {
            std::cout << "Wrong message from " << NodeId(response_id);
            return false;
        }
        responded_nodes_.insert(NodeId(response_id));
        average_response_time_ += (now - msg_send_time_);
        if (print_performance) {
            double rate(static_cast<double>(response.size() * 2) / duration);
            std::cout << "Direct message sent to " << DebugId(NodeId(response_id))
                << " completed in " << duration << " milliseconds, has throughput rate " << rate
                << " kBytes/s when data_size is " << response.size() << " Bytes" << std::endl;
        }
    }
    else {
        std::cout << "timed out ( " << duration / 1000 << " s) to " << DebugId(NodeId(response_id))
            << " when data_size is " << response.size() << std::endl;
    }
    return true;
}

void SharedResponse::PrintGroupPerformance(int data_size) {
    if (responded_nodes_.size() < routing::Parameters::group_size) {
        std::cout << "Only received " << responded_nodes_.size() << " responses for a group msg of "
            << data_size << " Bytes" << std::endl;
        return;
    }
    auto duration(average_response_time_.total_milliseconds() / responded_nodes_.size());
    double rate((static_cast<double>(data_size * 2) / duration));
    std::cout << " completed in " << duration << " milliseconds, has throughput rate " << rate
        << " kBytes/s when data_size is " << data_size << " Bytes" << std::endl;
}

}  //  namespace storage

}  //  namespace top
