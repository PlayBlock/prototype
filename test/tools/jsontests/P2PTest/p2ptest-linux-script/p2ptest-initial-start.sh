#
#P2PUnitTest: initial start test
#
rm -rf "data0"
./eth --config "config/config-initial-start-test.json" -d "data0" -m on -C  -t 1  -a 0x110e3e0a01EcE3a91e04a818F840E9E3D17B3C8f  --sociable -v 4 --p2p-unit-test P2PTestNoProduceStart --p2pfiller-path filled-blockchain/P2PUnitTest.json --chainname "A" 
