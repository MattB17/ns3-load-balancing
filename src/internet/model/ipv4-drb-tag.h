#ifndef NS3_IPV4_DRB_TAG
#define NS3_IPV4_DRB_TAG

#include "ns3/tag.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

class Ipv4DrbTag : public Tag {
public:
	Ipv4DrbTag();

	void SetOriginalDstAddr(Ipv4Address addr);
	Ipv4Address GetOriginalDstAddr() const;

	static TypeId GetTypeId();
	virtual TypeId GetInstanceTypeId() const;

	virtual uint32_t GetSerializedSize() const;

	virtual void Serialize(TagBuffer i) const;
	virtual void Deserialize(TagBuffer i);

	virtual void Print(std::ostream& os) const;

private:
	Ipv4Address m_addr;
};

}  // namespace ns3

#endif  // NS3_IPV4_DRB_TAG