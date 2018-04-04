#
#P2PUnitTest: P2PTestIdlChainASyncBNew   BlockNumberUpperIrr
#
rm -rf "data0"
./eth --config "config/config-general.json" -d "data0" -m on -C  -t 1  --sociable -v 4 --p2p-unit-test P2PTestIdlChainASyncBNew   --p2p-sub-test   BlockNumberUpperIrr  --p2pfiller-path     "filled-blockchain/ChainASyncB.json" --chainname "A"
