#include <iostream>
#include <algorithm>
#include <ctime>
#include <locale>
#include "area_code.h"
#include "csv.h"

namespace top {

namespace storage {


bool ContinentCountry::check() {
    if(this->_continent.size() == this->_continentRegion.size() &&
            this->_continentRegion.size() == this->_country.size() &&
            this->_country.size() == this->_capital.size() &&
            this->_capital.size() == this->_countryCode2.size() &&
            this->_countryCode2.size() == this->_countryCode3.size()) {
        std::cout << "check success, size = " << this->_continent.size() << std::endl;
        return true;
    }
    std::cout << "check fail" << std::endl;
    return false;
}

void ContinentCountry::toupper(std::string &str) {
    std::locale loc;
    std::string after;
    for(size_t i = 0; i < str.size(); ++i) {
        char c = std::toupper(str[i], loc);
        after.append(1, c);
    }
    str.swap(after);
}

std::string ContinentCountry::ttoupper(std::string str) {
    toupper(str);
    return str;
}


void ContinentCountry::Init(std::string filename) {
    _filename = filename;
    io::CSVReader<6, io::trim_chars<' '>, io::double_quote_escape<',', '\"'> > in(filename);
    in.read_header(io::ignore_extra_column, "Continent", "ContinentRegion", "Country",
                   "Capital", "CountryCode(2)", "CountryCode(3)");

    std::string   continent;
    std::string   continentRegion;
    std::string   country;
    std::string   capital;
    std::string   countryCode2;
    std::string   countryCode3;

    while(in.read_row(continent, continentRegion, country, capital, countryCode2, countryCode3)) {
        toupper(continent);
        toupper(continent);
        toupper(continentRegion);
        toupper(country);
        toupper(capital);
        toupper(countryCode2);
        toupper(countryCode3);

        this->_continent.push_back(continent);
        this->_continentRegion.push_back(continentRegion);
        this->_country.push_back(country);
        this->_capital.push_back(capital);
        this->_countryCode2.push_back(countryCode2);
        this->_countryCode3.push_back(countryCode3);
    }
}


std::string ContinentCountry::GetContinent(std::string countryCode) {
    auto ifind = std::find(this->_countryCode2.begin(), this->_countryCode2.end(), ttoupper(countryCode));
    if(ifind != this->_countryCode2.end()) {
        int index = std::distance(this->_countryCode2.begin(), ifind);
        return this->_continent[index];
    } else {
        return "";
    }
}


std::string ContinentCountry::GetContinentRegion(std::string countryCode) {
    auto ifind = std::find(this->_countryCode2.begin(), this->_countryCode2.end(), ttoupper(countryCode));
    //illegal country code
    if(ifind != this->_countryCode2.end()) {
        int index = std::distance(this->_countryCode2.begin(), ifind);
        return this->_continentRegion[index];
    } else {
        return "";
    }
}
//end ContinentCountry



bool DtCloudZonePNone::check() {
    if(this->_id.size() == this->_isBgp.size() &&
            this->_isBgp.size() == this->_isGbc.size() &&
            this->_isGbc.size() == this->_name.size() &&
            this->_name.size() == this->_nextDserverid.size() &&
            this->_nextDserverid.size() == this->_serverCount.size() &&
            this->_serverCount.size() == this->_version.size() &&
            this->_continent.size() == this->_continentRegion.size() &&
            this->_continentRegion.size() == this->_countryCode.size() &&
            this->_countryCode.size() == this->_isp.size() &&
            this->_isp.size() == this->_state.size()) {
        std::cout << "check success, size = " << this->_id.size() << std::endl;
        return true;
    }
    std::cout << "check fail" << std::endl;
    return false;
}

void DtCloudZonePNone::toupper(std::string &str) {
    std::locale loc;
    std::string after;
    for(size_t i = 0; i < str.size(); ++i) {
        char c = std::toupper(str[i], loc);
        after.append(1, c);
    }
    str.swap(after);
}


std::string DtCloudZonePNone::ttoupper(std::string str) {
    toupper(str);
    return str;
}

void DtCloudZonePNone::Init(std::string filename) {
    _filename = filename;
    io::CSVReader<12, io::trim_chars<' '>, io::double_quote_escape<',', '\"'> > in(filename);

    in.read_header(io::ignore_extra_column, "id (N)", "isBGP (N)", "isGBC (N)", "name (S)",
                   "nextDServerId (N)", "serverCount (N)", "version (N)", "continent (S)",
                   "continentRegion (S)", "countryCode (S)", "isp (S)", "state (S)");

    std::string   id;
    std::string   isBgp;
    std::string   isGbc;
    std::string   name;
    std::string   nextDserverid;
    std::string   serverCount;
    std::string   version;
    std::string   continent;
    std::string   continentRegion;
    std::string   countryCode;
    std::string   isp;
    std::string   state;

    while(in.read_row(id, isBgp, isGbc, name, nextDserverid, serverCount,
                      version, continent, continentRegion, countryCode, isp, state)) {
        toupper(id);
        toupper(isBgp);
        toupper(isGbc);
        toupper(name);
        toupper(nextDserverid);
        toupper(serverCount);
        toupper(version);
        toupper(continent);
        toupper(continentRegion);
        toupper(countryCode);
        toupper(isp);
        toupper(state);


        this->_id.push_back(id);
        this->_isBgp.push_back(isBgp);
        this->_isGbc.push_back(isGbc);
        this->_name.push_back(name);
        this->_nextDserverid.push_back(nextDserverid);
        this->_serverCount.push_back(serverCount);
        this->_version.push_back(version);
        this->_continent.push_back(continent);
        this->_continentRegion.push_back(continentRegion);
        this->_countryCode.push_back(countryCode);
        this->_isp.push_back(isp);
        this->_state.push_back(state);

    }
}


void DtCloudZonePNone::strtok_cmd(std::string src, std::vector<std::string> &split) {
    const char delim[2] = " ";
    char * p;
    int i = 0;

    p  = strtok((char*)src.data(), delim);
    while (p != NULL) {
        std::string temp_str(p, strlen(p));
        split.push_back(temp_str);

        p = strtok(NULL, delim);
        i++;
    }
}


int DtCloudZonePNone::getId(std::string continent, std::string continentRegion, std::string countryCode, std::string state) {
    std::string up_st     = ttoupper(state);
    std::string up_cc     = ttoupper(countryCode);
    std::string up_conreg = ttoupper(continentRegion);
    std::string up_con    = ttoupper(continent);

    //level 1,compare state
    if(up_st.size() > 0) {
        auto it = std::find(this->_state.begin(), this->_state.end(), up_st);
        if(it != this->_state.end()) {
            int index = std::distance(this->_state.begin(), it);
            return std::stoi(this->_id[index]);
        }
    }

    //level 2,compare country code
    if(up_cc.size() > 0) {
        bool country_find = false;
        std::vector<int> cf;
        for(size_t i = 0; i < this->_countryCode.size(); ++i) {
            if(up_cc.compare(this->_countryCode[i]) == 0) {
                std::vector<std::string> split;
                strtok_cmd(up_conreg, split);
                for(size_t j = 0; j < split.size(); ++j) {
                    auto sit = this->_name[i].find(split[j]);
                    if(sit != std::string::npos) {
                        return std::stoi(this->_id[i]);
                    }
                }// end for(size_t j= 0...
                country_find  = true;
                //相同的国家码，记录候选id
                cf.push_back(std::stoi(this->_id[i]));
            } // end if(up_cc.compare...
        }//end for(size_t i = 0...
        if(country_find) {
            int random = rand() % cf.size();
            return cf[random];
        }
    } //end if(up_cc.size() > 0...

    //level 3,compare continent and continentRegin
    if(up_conreg.size() > 0) {
        std::vector<int> cf;
        for(size_t i = 0; i < this->_continentRegion.size(); ++i) {
            if(up_conreg.compare(this->_continentRegion[i]) == 0) {
                cf.push_back(std::stoi(this->_id[i]));
            }
        }
        if(cf.size() > 0) {
            int random = rand() % cf.size();
            return cf[random];
        }
    }

    //level 4,compare for continent
    if(up_con.size() > 0) {
        std::vector<int> cf;
        for(size_t i = 0; i < this->_continent.size() ; ++i) {
            if(up_con.compare(this->_continent[i]) == 0) {
                cf.push_back(std::stoi(this->_id[i]));
            }
        }

        if(cf.size() > 0) {
            int random = rand() % cf.size();
            return cf[random];
        }
    }

    //finally,not found
    return -1;
}

std::string DtCloudZonePNone::getCountryCode(int id) {
    auto ifind = std::find(this->_id.begin(), this->_id.end(), std::to_string(id));
    if(ifind != this->_id.end()) {
        int index = std::distance(this->_id.begin(), ifind);
        return this->_countryCode[index];
    }
    return "";
}


//end for DtCloudZonePNone



bool DtRouteTable::check() {
    return true;
}

void DtRouteTable::toupper(std::string &str) {
    std::locale loc;
    std::string after;
    for(size_t i = 0; i < str.size(); ++i) {
        char c = std::toupper(str[i], loc);
        after.append(1, c);
    }
    str.swap(after);
}


std::string DtRouteTable::ttoupper(std::string str) {
    toupper(str);
    return str;
}

void DtRouteTable::Init(std::string filename) {
    srand (time(NULL));

    _filename = filename;
    io::CSVReader<11, io::trim_chars<' '>, io::double_quote_escape<',', '\"'> > in(filename);

    in.read_header(io::ignore_extra_column, "Source Cloud", "Dest Cloud", "Total Speed", "imSpeed1",
                   "imCloud1", "imSpeed2", "imCloud2", "imSpeed3", "imCloud3", "imSpeed4", "PrimaryRoute");


    std::string    sourceCloud;
    std::string    destCloud;
    std::string    totalSpeed;
    std::string    imSpeed1;
    std::string    imCloud1;
    std::string    imSpeed2;
    std::string    imCloud2;
    std::string    imSpeed3;
    std::string    imCloud3;
    std::string    imSpeed4;
    std::string    primaryRoute;

    while(in.read_row(sourceCloud, destCloud, totalSpeed, imSpeed1, imCloud1,
                      imSpeed2, imCloud2, imSpeed3, imCloud3, imSpeed4, primaryRoute)) {

        if(sourceCloud.size() <= 0 || destCloud.size() <= 0) {
            continue;
        }

        toupper(sourceCloud);
        toupper(destCloud);
        toupper(totalSpeed);
        toupper(imSpeed1);
        toupper(imCloud1);
        toupper(imSpeed2);
        toupper(imCloud2);
        toupper(imSpeed3);
        toupper(imCloud3);
        toupper(imSpeed4);
        toupper(primaryRoute);

        //such as 0:5 表示 src 为 0,dst 为 5 的路由
        std::string key = sourceCloud + ":" + destCloud;
        SingleRoute temp;
        PSingleRoute ptemp = std::make_shared<SingleRoute>(temp);
        ptemp->push_back(std::stoi(sourceCloud));

        if(imCloud1.size() > 0) {
            ptemp->push_back(std::stoi(imCloud1));
        }
        if(imCloud2.size() > 0) {
            ptemp->push_back(std::stoi(imCloud2));
        }
        if(imCloud3.size() > 0) {
            ptemp->push_back(std::stoi(imCloud3));
        }

        ptemp->push_back(std::stoi(destCloud));

        auto it = this->_routeMap.find(key);
        if(it != this->_routeMap.end()) {
            //value is vector of vector
            (it->second)->push_back(ptemp);
        } else {
            MultiRoute mr;
            mr.push_back(ptemp);
            PMultiRoute pmr = std::make_shared<MultiRoute>(mr);
            this->_routeMap.insert(std::pair < std::string, PMultiRoute >(key, pmr));
        }

        //逆序src and dst，作为新的路由
        SingleRoute rtemp(ptemp->begin(), ptemp->end());
        PSingleRoute prtemp = std::make_shared<SingleRoute>(rtemp);

        std::reverse(prtemp->begin(), prtemp->end());
        std::string rkey = destCloud + ":" + sourceCloud;

        auto rit = this->_routeMap.find(rkey);
        if(rit != this->_routeMap.end()) {
            //value is vector of vector
            (rit->second)->push_back(prtemp);
        } else {
            MultiRoute mr;
            mr.push_back(prtemp);
            PMultiRoute pmr = std::make_shared<MultiRoute>(mr);
            this->_routeMap.insert(std::pair < std::string, PMultiRoute >(rkey, pmr));
        }

    }

    /*
    for(auto it = this->_imSpeed4.begin(); it != this->_imSpeed4.end() ; it++) {
        if((*it).size() >0){
            std::cout << std::stoi(*it) << std::endl;
        }
    }
    */
}

void DtRouteTable::PrintAllRoute() {
    for(auto it = this->_routeMap.begin(); it != this->_routeMap.end(); ++it) {
        std::cout << "key: " << it->first << std::endl;
        for(auto vit = (it->second)->begin(); vit != (it->second)->end(); ++vit ) {
            for(auto rit = (*vit)->begin(); rit != (*vit)->end(); ++rit) {
                std::cout << *rit;
                if(rit != (*vit)->begin() + (*vit)->size() - 1) {
                    std::cout << "   ->   ";
                }
            }
            std::cout << std::endl << std::endl;
        }
    }
}

void DtRouteTable::FindRoute(int source_cloud, int dest_cloud, std::vector<int> &route) {
    if(source_cloud == dest_cloud) {
        return;
    }
    std::string key = std::to_string(source_cloud) + ":" + std::to_string(dest_cloud);
    auto ifind = this->_routeMap.find(key);
    if(ifind != this->_routeMap.end()) {
        std::cout << "source_cloud: " << source_cloud << " dest_cloud: " << dest_cloud << "  found availale route" << std::endl;
        PMultiRoute pm = ifind->second;
        size_t route_size = pm->size();
        int random = rand() % route_size;
        PSingleRoute ps = pm->at(random);
        for(auto it = ps->begin(); it != ps->end(); ++it) {
            route.push_back(*it);
        }
    }
}

//end DtRouteTable

WorldRoute::WorldRoute(std::string filecc, std::string filedz, std::string filedt) {
    this->fcc.Init(filecc);
    this->fdz.Init(filedz);
    this->fdt.Init(filedt);
}


bool WorldRoute::check() {
    bool chcode = this->fcc.check() ? (this->fdz.check() ? this->fdt.check() : false) : false;
    return chcode;
}

std::string WorldRoute::GetBestRoute(std::string src_country, std::string dst_country, std::string src_state, std::string dst_state) {
    std::string scon =  fcc.GetContinent(src_country);
    std::string scon_region =  fcc.GetContinentRegion(src_country);

    std::string dcon =  fcc.GetContinent(dst_country);
    std::string dcon_region =  fcc.GetContinentRegion(dst_country);
    if(scon.empty() || scon_region.empty() || dcon.empty() || dcon_region.empty()) {
        std::cout << "error: src = " << src_country << " dst = " << dst_country << " country code not exist" << std::endl;
        return "";
    }

    int  sid = fdz.getId(scon, scon_region, src_country, src_state);
    int  did = fdz.getId(dcon, dcon_region, dst_country, dst_state);


    std::vector<int> route;
    fdt.FindRoute(sid, did, route);

    if(route.size() <= 2) {
        std::cout << "can't find edge route,please connect directly" << std::endl;
        return "";
    }
    //TODO  这里只取第一跳的edge
    int first_edge = route[1];
    std::string first_edge_country = fdz.getCountryCode(first_edge);
    return first_edge_country;
}



} //end namespace storage
} // end top
