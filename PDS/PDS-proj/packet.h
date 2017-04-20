#ifndef PACKET_H
#define PACKET_H

#include <linux/if_ether.h>

#include "types.h"
#include "macaddr.h"
#include "ipaddr.h"

///
/// \brief Trieda Packet
///
class Packet
{
public:
    Packet(MACAddr *src_mac);
    void set_dst_hwa(MACAddr *dst_mac);

protected:
    static const uint ETH_HDR_LEN = 14;
    static const uint ETH_HW_TYPE = 1;
    static const uchar MAC_BCV4[];
    static const uchar MAC_BCV6[];

    MACAddr *m_src_hwa;                    // Zdrojova MAC
    MACAddr *m_dst_hwa;                    // Cielova MAC
    uchar    m_src_hwa_o[MACAddr::OCTETS]; // Oktety zdrojovej MAC
    uchar    m_dst_hwa_o[MACAddr::OCTETS]; // Oktety cielovej MAC
    uint16_t m_eth_prot;                   // Ethernetovy protokol

    ///
    /// \enum EthDest
    /// \brief Rozlisuje ci sa pouzije BC alebo UC MAC adresa
    /// \var EthDest::UC je unicast
    /// \var EthDest::BCv4 je broadcast nad IPv4
    /// \var EthDest::BCv6 je broadcast nad IPv6
    ///
    enum class EthDest
    {
        UC,
        BCv4,
        BCv6
    };

    uchar *eth_header(EthDest dest);
};

#endif // PACKET_H
