#
#P2PUnitTest: P2PTestIdlChainASyncBNew   NewBlockAddOne 
#
rm -rf "data0"
./eth --config "config/config-general.json" -d "data0"  -t 1  --sociable -v 4 --p2p-unit-test P2PTestIdlChainASyncBNew   --p2p-sub-test  NewBlockAddOne --p2pfiller-path "filled-blockchain/ChainASameChainB.json" --chainname "A"   
