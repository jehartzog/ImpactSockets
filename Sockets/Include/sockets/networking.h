/**
 * Created by TekuConcept on July 7, 2018
 */

#ifndef _IMPACT_NETWORKING_H_
#define _IMPACT_NETWORKING_H_

#include <string>
#include <vector>

namespace impact {
namespace networking {
	typedef enum class interface_type {
		// !- CROSS-PLATFORM TYPES    -!
		OTHER,
		ETHERNET,
		WIFI,
		FIREWIRE,
		PPP,
		ATM
	} InterfaceType;


	typedef struct netinterface {
		unsigned int               flags;
		std::string                name;
		std::string                address;
		std::string                netmask;
		std::string                broadcast;
		interface_type             type;
		std::vector<unsigned char> mac;
		bool                       ipv4; // TODO: use union for ipv4,ipv6,other
		netinterface();
	} NetworkInterface;


	std::vector<struct netinterface> find_network_interfaces()
		/* throw(std::runtime_error) */;
}}

#undef MAC_ADDRESS_LENGTH
#endif