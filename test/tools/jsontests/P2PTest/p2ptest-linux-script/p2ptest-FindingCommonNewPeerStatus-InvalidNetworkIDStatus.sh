#
#P2PUnitTest: P2PTestFindingCommonNewPeerStatus   InvalidNetworkIDStatus    
#
rm -rf "data0"
./eth --config "config/config-general.json" -d "data0"  -t 1  --sociable -v 4 --p2p-unit-test P2PTestFindingCommonNewPeerStatus  --p2p-sub-test  InvalidNetworkIDStatus  --p2pfiller-path "filled-blockchain/ChainASyncB.json" --chainname "A"
