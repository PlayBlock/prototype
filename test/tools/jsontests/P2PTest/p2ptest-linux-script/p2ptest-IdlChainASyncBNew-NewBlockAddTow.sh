#
#P2PUnitTest: P2PTestIdlChainASyncBNew   NewBlockAddTow
#
rm -rf "data0"
./eth --config "config/config-general.json" -d "data0" -m on -C  -t 1  --sociable -v 4 --p2p-unit-test P2PTestIdlChainASyncBNew   --p2p-sub-test   NewBlockAddTow  --p2pfiller-path "filled-blockchain/ChainASyncB.json" --chainname "A"
