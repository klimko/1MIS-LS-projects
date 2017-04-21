// Projekt: PDS - L2 MitM
// Autor:   Daniel Klimaj; xklima22@stud.fit.vutbr.cz

#include "macaddr.h"

MACAddr::MACAddr(ifreq *ifr)
{
    for (uint i=0; i < OCTETS; ++i)
        m_mac[i] = ifr->ifr_hwaddr.sa_data[i];
}

MACAddr::MACAddr(UchrVect oct)
{
    if(oct.size() == OCTETS)
    {
        for(uint i=0; i<OCTETS; ++i)
            m_mac[i] = oct[i];
    }
    else
    {
        for(uint i=0; i<OCTETS; ++i)
            m_mac[i] = 0x00;
    }
}

MACAddr::MACAddr(std::string mac)
{
    for(uint i=0; i<OCTETS; ++i)
        m_mac[i] = 0x00;

    if(mac.find_first_of(':') != std::string::npos)
    {
        StrVect octs = split_addr(mac, ':');
        if(octs.size() == OCTETS)
        {
            for(uint i=0; i<OCTETS; ++i)
                m_mac[i] = literal_to_uchr(octs[i]);
        }
    }
    else if(mac.find_first_of('.') != std::string::npos)
    {
        StrVect quads = split_addr(mac, '.');
        if(quads.size() == QUADS)
        {
            m_mac[0] = literal_to_uchr(quads[0].substr(0, 2));
            m_mac[1] = literal_to_uchr(quads[0].substr(2, 2));
            m_mac[2] = literal_to_uchr(quads[1].substr(0, 2));
            m_mac[3] = literal_to_uchr(quads[1].substr(2, 2));
            m_mac[4] = literal_to_uchr(quads[2].substr(0, 2));
            m_mac[5] = literal_to_uchr(quads[2].substr(2, 2));
        }
    }
}

std::string MACAddr::to_string() const
{
    char buffer[20];
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
        m_mac[0],m_mac[1],m_mac[2],m_mac[3],m_mac[4],m_mac[5]);
    return std::string(buffer);
}

uchar MACAddr::octet(uint idx)
{
    if(idx < OCTETS)
        return m_mac[idx];
    return (uchar) 0;
}

bool MACAddr::eq(MACAddr *other)
{
    for(uint i=0; i<OCTETS; ++i)
    {
        if(m_mac[i] != other->octet(i))
            return false;
    }
    return true;
}

bool MACAddr::empty() const
{
    for(uint i=0; i<OCTETS; ++i)
    {
        if(m_mac[i] != 0x00)
            return false;
    }
    return true;
}

std::string MACAddr::to_xml(std::string mac)
{
    MACAddr *tmp = new MACAddr(mac);
    std::string xml_mac = "";
    xml_mac += str_bytes8(tmp->octet(0));
    xml_mac += str_bytes8(tmp->octet(1));
    xml_mac += ".";
    xml_mac += str_bytes8(tmp->octet(2));
    xml_mac += str_bytes8(tmp->octet(3));
    xml_mac += ".";
    xml_mac += str_bytes8(tmp->octet(4));
    xml_mac += str_bytes8(tmp->octet(5));
    delete tmp;
    return xml_mac;
}
