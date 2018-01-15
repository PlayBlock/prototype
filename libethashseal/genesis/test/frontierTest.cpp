/*
		This file is part of cpp-ethereum.

		cpp-ethereum is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version.

		cpp-ethereum is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../../GenesisInfo.h"

static std::string const c_genesisInfoFrontierTest =
R"E(
{
	"sealEngine": "Ethash",
	"params":{
		"accountStartNonce": "0x00", 
		"networkID" : "0x88",
		"chainID": "0x88",
		"maximumExtraDataSize": "0x20",
		"tieBreakingGas": false,
		"minGasLimit": "0x1388",
		"maxGasLimit": "7fffffffffffffff",
		"gasLimitBoundDivisor": "0x0400",
		"minimumDifficulty": "0x020000",
		"difficultyBoundDivisor": "0x0800",
		"durationLimit": "0x0d",
		"blockReward": "0x4563918244F40000",
		"enableStaleProduction" : "1", 
		"allowFutureBlocks" : true,
        "producerAccounts" : [
			"0x0070015693bb96335dd8c7025dded3a2da735db1",
			"0x2ecffd9dc5561bbdf23929a250bb99c758d64dbb",
			"0x5c30B1688Bf9b515F268deBF6e286c36DD1E1AF3"
		]
    },
	"privateKeys": {
		"0x0070015693bb96335dd8c7025dded3a2da735db1": "329cde16d721501c7f1d16d620644d34fe12f3d68e6fc9d7fd238a984e5dc289",
		"0x7A62B4Df0Df87742E3d4a0099B39C30939A7D74c": "542f36083237819e222b4f8e682c555186182bb4978caafe9089de09f05d16ec",
		"0x5c30B1688Bf9b515F268deBF6e286c36DD1E1AF3": "7980c9db65ea3bb9b1ac11400968f149c037314ba47397c894750162225f523d",
		"0xfc9f39081b56b1612acbbf0bdfb350b53fae11d7": "0b4c6b0cd7d29ea49bd66363a357ba950b3e873a3f32948b61ecbabbdfa29246",
        "0x2ecffd9dc5561bbdf23929a250bb99c758d64dbb": "36b7699e4db7d74b22293421d54e20c41d35f18fc3a7908de309c59229dff73b",
        "0xd4ad13effbb688e77930431a24fd296b71d6c1c9": "aa6ca438af6cb11ac5b97908d61bbe4b33dd2886783f534ada4fe4af925bc964",
        "0x53650a167586d1411c3d9a9a29fa9685efcac6ef": "252577856ad124b6a8a233c6849ea3da60910c57302b014eb19829456adc429b",
        "0x0f5d4d1cb57aeab539b5976850fdf983d9bcdbf1": "a949983290e28f80a85814dd7f12cdf00f932e9fd4f9724b35a8bd6ceb70984d",
        "0xd401f0f9f95fc9aed246a3ef441e4903848ef554": "a5b425ab38fa36e5aa1fc58d77d0bf6cb40b2c8a9b7820d56a757a1924203a42",
        "0x0874ebb72289617e84220a0b8463b23071c36c37": "04532ad03f5f5cf111fc1b686e418122c07c59572e1a3bdd49de0719f0fc019a",
        "0x202d4e20a86aa821ebd4c02c613099052ffea6c9": "008bf42de29c25ab52f496ea64da5afc7abfe15c3abdd9c595d98dc783a32a73",
        "0xa333d00639f2be4aa58cc14aa7230bccc6001343": "5b6fe4b2f93848ad80e3ea37117c3945bd5a82ece2c5ade3c0d220b6f647b2df",
        "0x75813ae7bf1f9f47a8b168a342c258871dbcbc86": "d5c9c6f2435d056b681b5ab44d72c643c3ba3787dd2b68fa94ccd87333af9e94",
        "0x68bf457802894d342c5ae4b6badc735980e090b5": "377e5f5a46596a8a5816d59a9f0480b4ed0746adbad2f94665b816d2c203391c",
        "0xc109036229145360c90ba4cb768cdd7121460d10": "4232b04419cddf7896e224a846504bee3d08497b0e6f43815552ec10dca20e2f",
        "0x32cc7ac533a1291c19f9b1fbf43bb4f117e9f9de": "de2883277ce476de1856bed79afc0005de58bef025ee6637c21748476f832b2d",
        "0x956feb0119392f88937dfe6fc194b5d0fd13fa58": "beef41dac1a5ca18ec4bfbef013d297ce8b31ed91d65d68fc2fe4bfd5cda0deb",
        "0xf5a4f7e8b56e0601106d8152bf4a7ed11603b014": "1d8a250173489c95b324e11742a620172438e0b1aa0d51739e0a58a6ba2c6f36",
        "0xf856de35ef56d6f9f258b8a4de6e11be21cca525": "8abbfe1a7fe89e900a7578c87edf54159f63154eee5bff4df467e9d4506c3946",
        "0x1235cc0207838ce92b8c1d1c72e2a1049d191f4d": "9640bc4d3f3c87805a3b406f2fd9df7bba64c95ff732d94b420eb8009e63c8c9",
        "0x6eaa7ec4508b82fc300b131707485a93fe85bff8": "fabac3ed98f7c3d753e70ebbab53dadf12a2f2ed3db31b1301140fededb61757",
		"0x0c338296B1bEa1e4529D173ea5Ae95508144d9f3": "5c02eb8b326c56e8b68caea90da49fb781c6a998ce5c73806f67c27531938e57"
    },
    "genesis": {
            "nonce": "0x000000000000002a",
            "difficulty": "0x400",
            "mixHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
            "author": "0x0000000000000000000000000000000000000000",
            "timestamp": "0x00",
            "parentHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
            "extraData": "0x",
            "gasLimit": "0x2fefd8",
			"initialProducers" : [
			"0x0070015693bb96335dd8c7025dded3a2da735db1", 
			"0x2ecffd9dc5561bbdf23929a250bb99c758d64dbb",
			"0x5c30B1688Bf9b515F268deBF6e286c36DD1E1AF3",
			"0xfc9f39081b56b1612acbbf0bdfb350b53fae11d7"
			]
    },
    "accounts": {
       	"0000000000000000000000000000000000000001": { "precompiled": { "name": "ecrecover", "linear": { "base": 3000, "word": 0 } } },
		"0000000000000000000000000000000000000002": { "precompiled": { "name": "sha256", "linear": { "base": 60, "word": 12 } } },
		"0000000000000000000000000000000000000003": { "precompiled": { "name": "ripemd160", "linear": { "base": 600, "word": 120 } } },
		"0000000000000000000000000000000000000004": { "precompiled": { "name": "identity", "linear": { "base": 15, "word": 3 } } },
		"0000000000000000000000000000000000000005": { "precompiled": { "name": "modexp", "startingBlock": "0x42ae50" } },
		"0000000000000000000000000000000000000006": { "precompiled": { "name": "alt_bn128_G1_add", "startingBlock": "0x42ae50", "linear": { "base": 500, "word": 0 } } },
		"0000000000000000000000000000000000000007": { "precompiled": { "name": "alt_bn128_G1_mul", "startingBlock": "0x42ae50", "linear": { "base": 40000, "word": 0 } } },
		"0000000000000000000000000000000000000008": { "precompiled": { "name": "alt_bn128_pairing_product", "startingBlock": "0x42ae50" } },
		"3282791d6fd713f1e94f4bfd565eaa78b3a0599d": {
		"balance": "1337000000000000000000"
		},
		"0000000000000000000000000000000000000020": { "wei": "1", "code":"0x00" },
		"0000000000000000000000000000000000000021": { "wei": "1", "code":"0x00" },
		"0000000000000000000000000000000000000022": {"precompiled": { "name": "mortgage", "linear": { "base": 210000, "word": 7 } } },
		"0000000000000000000000000000000000000023": {"precompiled": { "name": "redeem", "linear": { "base": 210000, "word": 8 } } },
		"0000000000000000000000000000000000000024": {"precompiled": { "name": "candidateRegister", "linear": { "base": 2100000, "word": 9 } } },
		"0000000000000000000000000000000000000025": {"precompiled": { "name": "candidateDeregister", "linear": { "base": 210000, "word": 10 } } },
		"0000000000000000000000000000000000000026": {"precompiled": { "name": "vote", "linear": { "base": 210000, "word": 6 } } },
		"0000000000000000000000000000000000000027": {"precompiled": { "name": "removeVote", "linear": { "base": 210000, "word": 6 } } },
		"0000000000000000000000000000000000000028": {"precompiled": { "name": "assign", "linear": { "base": 60, "word": 6 } } },
		"0000000000000000000000000000000000000029": {"precompiled": { "name": "deAssign", "linear": { "base": 60, "word": 6 } } },
		"000000000000000000000000000000000000002a": {"precompiled": { "name": "send", "linear": { "base": 210000, "word": 6 } } },
		"000000000000000000000000000000000000002b": { "wei": "1", "code":"0x00" },
		"000000000000000000000000000000000000002c": {"precompiled": { "name": "powReceive", "linear": { "base": 60, "word": 6 } } }
    }	
	}
   
)E";

