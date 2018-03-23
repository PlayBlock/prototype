#
#P2PUnitTest: P2PTestInvalidNetworkIDStatus
#
rm -rf "data0"
./eth --config "config/config-general.json" -d "data0" -m on -C  -t 1  -a 0x110e3e0a01EcE3a91e04a818F840E9E3D17B3C8f  --sociable -v 4 --p2p-unit-test P2PTestInvalidNetworkIDStatus
