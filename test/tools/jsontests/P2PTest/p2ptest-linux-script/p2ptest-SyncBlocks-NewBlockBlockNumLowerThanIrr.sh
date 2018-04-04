#
#P2PUnitTest: P2PTestSyncBlocks   NewBlockBlockNumLowerThanIrr 
#
rm -rf "data0"
./eth --config "config/config-general.json" -d "data0" -m on -C  -t 1  --sociable -v 4 --p2p-unit-test  P2PTestSyncBlocks  --p2p-sub-test NewBlockBlockNumLowerThanIrr  --p2pfiller-path "filled-blockchain/ChainASyncB.json"  --chainname "A"
