#
#P2PUnitTest: P2PTestFindingCommonBlockHeader   BlockHeaderNumberEqualIrr
#

rm -rf "data0"
./eth --config "config/config-general.json" -d "data0"  -t 1  --sociable -v 4 --p2p-unit-test P2PTestFindingCommonBlockHeader  --p2p-sub-test BlockHeaderNumberEqualIrr  --p2pfiller-path "filled-blockchain/IrrHashDifferent.json" --chainname "B"
