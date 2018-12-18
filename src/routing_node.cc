#include <signal.h>
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/routing/utils.h"

#include "commands.h"
#include "utils.h"
#include "db.h"
#include "log.h"
#include "dict.h"
#include "load_conf.h"
#include "session_manager.h"

namespace bptime = boost::posix_time;
namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace ma = maidsafe::asymm;

struct PortRange {
    PortRange(uint16_t first, uint16_t second) : first(first), second(second) {}
    uint16_t first;
    uint16_t second;
};

namespace {

    // This function is needed to avoid use of po::bool_switch causing MSVC warning C4505:
    // 'boost::program_options::typed_value<bool>::name' : unreferenced local function has been removed.
#ifdef MAIDSAFE_WIN32
    void UseUnreferenced() {
        auto dummy = po::typed_value<bool>(nullptr);
        (void)dummy;
    }
#endif
    void ConflictingOptions(const po::variables_map& variables_map, const char* opt1,
        const char* opt2) {
        if (variables_map.count(opt1) && !variables_map[opt1].defaulted() && variables_map.count(opt2) &&
            !variables_map[opt2].defaulted()) {
            throw std::logic_error(std::string("Conflicting options '") + opt1 + "' and '" + opt2 + "'.");
        }
    }

    // Function used to check that if 'for_what' is specified, then
    // 'required_option' is specified too.
    void OptionDependency(const po::variables_map& variables_map, const char* for_what,
        const char* required_option) {
        if (variables_map.count(for_what) && !variables_map[for_what].defaulted()) {
            if (variables_map.count(required_option) == 0 || variables_map[required_option].defaulted()) {
                throw std::logic_error(std::string("Option '") + for_what + "' requires option '" +
                    required_option + "'.");
            }
        }
    }
}  // unnamed namespace


void check_start_type(int business,std::string area,int role){
    static std::map<int,std::string> business_map = {
        {top::storage::BusinessType::kVpn , "VPN"},
        {top::storage::BusinessType::kStorage , "STORAGE"},
        {top::storage::BusinessType::kCdn , "CDN"} 
    };
    static std::map<int,std::string> role_map = {
        {top::storage::RoleType::kClient , "CLIENT"},
        {top::storage::RoleType::kEdge , "EDGE"},
        {top::storage::RoleType::kServer , "SERVER"} 
    };
    
    std::cout << "node start as:";

    auto bfind = business_map.find(business);
    if(bfind != business_map.end()){
        std::cout << "  business: <" << bfind->second << ">";
    } else{
        std::cout << "  business: <>";
    }

    std::cout << "  area: " << area;

    auto rfind = role_map.find(role);
    if(rfind != role_map.end()){
        std::cout << "  role: <" << rfind->second << ">";
    } else {
        std::cout << "  role: <>";
    }
    std::cout<<std::endl;
}



int main(int argc, char** argv) {
    log4cpp::PropertyConfigurator::configure("./conf/log4cpp.properties");
    maidsafe::log::Logging::Instance().Initialise(argc, argv);
    using namespace top::storage;
    SessionManager::Instance();
    try {
        int identity_index;
        boost::system::error_code error_code;
        po::options_description options_description("Options");
        options_description.add_options()("help,h", "Print options.")(
            "start,s", "Start a node (default as vault)")(
            "show_cmd,g", po::value<int>()->default_value(1),
                "show cmd input line")(
            "client,c", po::bool_switch(), "Start as client (default is vault)")(
            "bootstrap,b", "Start as bootstrap (default is non-bootstrap)")(
            "peer,p", po::value<std::string>()->default_value(""), "Endpoint of bootstrap peer")(
            "identity_index,i", po::value<int>(&identity_index)->default_value(-1),
                "Entry from keys file to use as ID (starts from 0)")(
            "pmids_path", po::value<std::string>()->default_value(fs::path(
                fs::temp_directory_path(error_code) / "pmids_list.dat").string()),
                "Path to pmid file")(
            "business", po::value<int>()->default_value(-1), "business provide this node(None = 0,VPN = 1,STORAGE = 2...)")(
            "area", po::value<std::string>()->default_value(""), "area code represent for country,province,city...")(
            "role", po::value<int>()->default_value(1), "the role this node plays(client = 0,edge = 1,server = 2)")(
            "db_path", po::value<std::string>()->default_value(""), "Path to db file")(
            "local_port,l", po::value<uint16_t>()->default_value(8866), "local listening port")(
            "config", po::value<std::string>()->default_value("./conf/node.conf"), "config file path");

        po::variables_map variables_map;
        //     po::store(po::parse_command_line(argc, argv, options_description),
        //               variables_map);
        po::store(
            po::command_line_parser(argc, argv).options(options_description).allow_unregistered().run(),
            variables_map);
        po::notify(variables_map);

		uint16_t local_port;
		if (variables_map.count("bootstrap")) {
			local_port = variables_map["local_port"].as<uint16_t>();
			LOG(kInfo)<<"local port:"<<local_port;
		}

        if (variables_map.count("help") || (!variables_map.count("start"))) {
            LOG(maidsafe::log::kError) << options_description;
            return 0;
        }

		std::string conf_path(variables_map.at("config").as<std::string>());
        top::storage::Config::Instance(conf_path);

        std::string db_path(variables_map.at("db_path").as<std::string>());
		if(db_path.empty()) {
			//get db path  from config file
			db_path = top::storage::Config::Instance()->GetDbPath();
		}		
        if (!top::storage::Db::Instance()->Init(db_path)) {
            LOG(maidsafe::log::kError) << "ERROR init db file: " << db_path << " failed!";
            return 0;
        }

        int business(variables_map.at("business").as<int>());
        std::string area(variables_map.at("area").as<std::string>());
		if(area.empty()) {
			//get country code from config file
			area = top::storage::Config::Instance()->GetCountryCode();
		}
        int role(variables_map.at("role").as<int>());
        check_start_type(business,area,role);
		top::storage::Dict::Instance()->Hset(top::storage::LOCAL_COUNTRY_DB_KEY,"code",area);


        // Load fob list and local fob
        maidsafe::passport::detail::AnmaidToPmid local_key;

		//modified by Hench, 2018-12-3, non-zero node do not use all_key
        std::vector<maidsafe::passport::detail::AnmaidToPmid> all_keys;
		if (variables_map.count("bootstrap")) {
	        auto pmids_path(maidsafe::GetPathFromProgramOptions("pmids_path", variables_map, false, true));
	        if (fs::exists(pmids_path, error_code)) {
	            try {
	                all_keys = maidsafe::passport::detail::ReadKeyChainList(pmids_path);
	            }
	            catch (const std::exception& e) {
	                LOG(maidsafe::log::kError) << "Error: Failed to read key chain list at path : "
	                    << pmids_path.string() << ". error : "
	                    << e.what();
	                return 0;
	            }
	            if (all_keys.empty()) {
	                LOG(maidsafe::log::kError) << "Error: loaded " << all_keys.size() << " fobs.";
	                return 0;
	            }
	            LOG(maidsafe::log::kError) << "Loaded " << all_keys.size() << " fobs.";
	            if (static_cast<uint32_t>(identity_index) >= all_keys.size() || identity_index < 0) {
	                LOG(maidsafe::log::kError) << "ERROR : index exceeds fob pool -- pool has " << all_keys.size()
	                    << " fobs, while identity_index is " << identity_index;
	                return 0;
	            } else {
	                local_key = all_keys[identity_index];
	                LOG(maidsafe::log::kError) << "Using identity #" << identity_index << " from keys file"
	                    << " , value is : "
	                    << maidsafe::HexSubstr(local_key.pmid.name().value);
	            }
	        } else {
	            LOG(maidsafe::log::kError) << "ERROR : pmid list file not found at : "
	                << variables_map.at("pmids_path").as<std::string>()
	               ;
	            return 0;
	        }
		}
		else {
			local_key = maidsafe::passport::detail::AnmaidToPmid();
            maidsafe::Identity test_id(Commands::GenRandomNodeId(maidsafe::NodeId::kSize,business,area,role));
            local_key.pmid.UpdateName(test_id);
			all_keys.push_back(local_key);
		}
		LOG(maidsafe::log::kInfo) << "key info:"<<maidsafe::HexSubstr(local_key.pmid.name().value);

        ConflictingOptions(variables_map, "client", "bootstrap");
        //OptionDependency(variables_map, "start", "identity_index");  //not use identity_index in the edge node
        //OptionDependency(variables_map, "peer", "identity_index");   //not use identity_index in the peer command

        // Ensure correct index range is being used
        bool client_only_node(variables_map["client"].as<bool>());
/*
        if (client_only_node) {
            if (identity_index < static_cast<int>(all_keys.size() / 2)) {
                LOG(maidsafe::log::kError) << "ERROR : Incorrect identity_index used for a client, must between "
                    << all_keys.size() / 2 << " and " << all_keys.size() - 1;
                return 0;
            }
        } else {
            if (identity_index >= static_cast<int>(all_keys.size() / 2)) {
                LOG(maidsafe::log::kError) << "ERROR : Incorrect identity_index used for a vault, must between 0 and "
                    << all_keys.size() / 2 - 1;
                return 0;
            }

        }
*/
        // Initial demo_node
        LOG(maidsafe::log::kError) << "Creating node...";
        top::storage::DemoNodePtr demo_node;
        if (!client_only_node){
			if (variables_map.count("bootstrap")) 
	            demo_node.reset(new top::storage::GenericNode(local_key.pmid, local_port));
			else
	            demo_node.reset(new top::storage::GenericNode(local_key.pmid));				
        }
        else
            demo_node.reset(new top::storage::GenericNode(local_key.maid));
        if (variables_map.count("bootstrap")) {
            if (identity_index >= 2) {
                LOG(maidsafe::log::kError) << "ERROR : trying to use non-bootstrap identity";
                return 0;
            }
            std::cout << "------ Current BootStrap node endpoint info : " << demo_node->endpoint()
                << " ------ " << std::endl;
        }
		else
			identity_index = 2;
					
        if(role == top::storage::RoleType::kEdge){
            top::storage::Dict::Instance()->Hset(top::storage::LOCAL_EDGE_DB_KEY,"ip",top::storage::Config::Instance()->GetIp());
            top::storage::Dict::Instance()->Hset(top::storage::LOCAL_EDGE_DB_KEY,"tcp_port",std::to_string(top::storage::Config::Instance()->GetTcpPort()));
            top::storage::Dict::Instance()->Hset(top::storage::LOCAL_EDGE_DB_KEY,"udp_port",std::to_string(top::storage::Config::Instance()->GetUdpPort()));
        }

        int show_cmd(variables_map["show_cmd"].as<int>());
        Commands* command = Commands::Instance();
        if (!command->Init(demo_node, all_keys, identity_index, show_cmd > 0 ? true : false)) {
            std::cout << "init command failed!" << std::endl;
            return -1;
        }
        std::string peer(variables_map.at("peer").as<std::string>());
        if (!peer.empty()) {
            command->GetPeer(peer);
        }
        command->Run();

        LOG(maidsafe::log::kError) << "Node stopped successfully." ;
    }
    catch (const std::exception& e) {
        LOG(maidsafe::log::kError) << "Error: " << e.what();
        return -1;
    }
    return 0;
}
