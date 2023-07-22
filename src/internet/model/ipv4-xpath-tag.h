// Implements Xpath from the paper "Explicit Path Control in Commodity Data 
// Centers: Design and Applications".
//
// XPath is used to explicitly identify an end-to-end path with a path ID
// for a packet.
#ifndef NS3_IPV4_XPATH_TAG
#define NS3_IPV4_XPATH_TAG

#include "ns3/tag.h"

namespace ns3 {

class Ipv4XPathTag : public Tag {
public:
	Ipv4XPathTag();

	static TypeId GetTypeId();

	uint32_t GetPathId();
	void SetPathId(uint32_t pathId);

	virtual TypeId GetInstanceTypeId() const;

	virtual uint32_t GetSerializedSize() const;

	virtual void Serialize(TagBuffer i) const;
	virtual void Deserialize(TagBuffer i);

	virtual void Print(std::ostream& os) const;

private:
	uint32_t m_pathId;
};

}  // namespace ns3

#endif  // NS3_IPV4_XPATH_TAG