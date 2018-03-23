#
#P2PUnitTest: P2PTestInvalidNetworkIDStatus
#
rm -rf "data0"
./eth --config "config/config-general.json" -d "data0" -m on -C  -t 1   --sociable -v 4 --p2p-unit-test P2PTestIrrHashDifferent --p2pfiller-path "filled-blockchain/IrrHashDifferent.json"  --chainname "A"
