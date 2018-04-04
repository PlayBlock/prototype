#
#P2PUnitTest: P2PTestFindingCommonBlockHeader   BlockHeaderMuch       
#
rm -rf "data0"
./eth --config "config/config-general.json" -d "data0"  -t 1  --sociable -v 4 --p2p-unit-test P2PTestFindingCommonBlockHeader  --p2p-sub-test BlockHeaderMuch   --p2pfiller-path "filled-blockchain/ChainASyncB.json" --chainname "A"
