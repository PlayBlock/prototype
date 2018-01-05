#pragma once

#include <array>

const std::array<const unsigned, 8> tierStepGas = { { 0, 2, 3, 5, 8, 10, 20, 0 } };

const unsigned basicGas = 2;
const unsigned u256op1 = 3;
const unsigned u256op2 = 5;

const unsigned expGas = 10;
const unsigned expByteGas = 50;
const unsigned sha3Gas = 30;
const unsigned sha3WordGas = 6;
const unsigned sloadGas = 200;
const unsigned sstoreSetGas = 20000;
const unsigned sstoreResetGas = 5000;
const unsigned sstoreRefundGas = 15000;
const unsigned jumpdestGas = 1;
const unsigned logGas = 375;
const unsigned logDataGas = 8;
const unsigned logTopicGas = 375;
const unsigned createGas = 32000;
const unsigned callGas = 700;
const unsigned callStipend = 2300;
const unsigned callValueTransferGas = 9000;
const unsigned callNewAccountGas = 25000;
const unsigned suicideRefundGas = 24000;
const unsigned memoryGas = 3;
const unsigned quadCoeffDiv = 512;
const unsigned createDataGas = 200;
const unsigned txGas = 21000;
const unsigned txCreateGas = 53000;
const unsigned txDataZeroGas = 4;
const unsigned txDataNonZeroGas = 68;
const unsigned copyGas = 3;

const unsigned extcodesizeGas = 700;
const unsigned extcodecopyGas = 700;
const unsigned balanceGas = 400;
const unsigned suicideGas = 5000;
const unsigned blockhashGas = 800;
const unsigned maxCodeSize = 0x6000;