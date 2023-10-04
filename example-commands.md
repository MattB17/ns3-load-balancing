Running fat-tree example:
* `./ns3 run "fat-tree-2-tier --randomSeed=233 --load=0.3 --serverCount=8 --spineToLeafCapacity=10"`

Running letflow example:
* `./ns3 run  "ipv4-letflow-routing-example --verbose=true --tracing=true --numSmallFlows=1"`

Build
* `./ns3 clean`
* `./ns3 configure --build-profile=debug --enable-examples --enable-tests`
* `./ns3 build`

TcpDump
* `tcpdump -nn -tt -r <filename>.pcap`
