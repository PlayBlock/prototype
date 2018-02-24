#include "DPwTestsHelper.h"
#include "Inline/Timing.h"
#include <boost/range/algorithm/find.hpp>
using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::test;

namespace dev {
	namespace test {

		Address newAddress(const Account& account)
		{
			return right160(sha3(rlpList(Address(account.address), u256(account.nonce))));
		}

		inline std::string loadFile(const char* filename)
		{
			Timing::Timer timer;
			std::ifstream stream(filename, std::ios::binary | std::ios::ate);
			if (!stream.is_open())
			{
				std::cerr << "Failed to open " << filename << ": " << std::strerror(errno) << std::endl;
				return std::string();
			}
			std::string data;
			data.resize((unsigned int)stream.tellg());
			stream.seekg(0);
			stream.read(const_cast<char*>(data.data()), data.size());
			stream.close();
			Timing::logRatePerSecond("loaded file", timer, data.size() / 1024.0 / 1024.0, "MB");
			return data;
		}

		std::string loadData(std::string file)
		{
			string filename = dev::test::getFolder(__FILE__) + "/WAVMTest/" + file;
			std::string wasmBytes = loadFile(filename.c_str());
			BOOST_REQUIRE(wasmBytes.size() > 0);
			return toHex(wasmBytes);
		}

		char char_to_symbol(char c) {
			if (c >= 'a' && c <= 'z')
				return (c - 'a') + 1;
			if (c >= '1' && c <= '5')
				return (c - '1') + 27;
			return 0;
		}

		uint64_t string_to_name(const char* str) {

			uint32_t len = 0;
			while (str[len]) ++len;

			uint64_t value = 0;

			for (uint32_t i = 0; i <= 12; ++i) {
				uint64_t c = 0;
				if (i < len && i <= 12) c = char_to_symbol(str[i]);

				if (i < 12) {
					c &= 0x1f;
					c <<= 64 - 5 * (i + 1);
				}
				else {
					c &= 0x0f;
				}

				value |= c;
			}

			return value;
		}

		string name2hex(const string& func_name)
		{
			uint64_t func_int = string_to_name(func_name.c_str());
			bytes func_bytes(sizeof(func_int));
			memcpy(func_bytes.data(), &func_int, sizeof(func_int));
			return toHex(func_bytes);
		}

		void addressToU256(const Address & _address, u256 & _u256)
		{
			h256 _h256 = h256();
			memcpy(&_h256, &_address, sizeof(Address));
			_u256 = (u256)_h256;
		}


		string code_initIO = "0061736d01000000011d0660037f7f7f0060027f7f017f60017f0060027f7f0060000060017e0002b0010c03656e76086164645f75323536000003656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e760873657473746f7265000303656e760c7365747532353676616c7565000303656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030d0c000000000001010102020405040401700000050301000107d3010d066d656d6f727902000f5f5a706c524b35435432353653315f000c0f5f5a6d69524b35435432353653315f000d0f5f5a6d6c524b35435432353653315f000e0f5f5a6476524b35435432353653315f000f0f5f5a726d524b35435432353653315f00100f5f5a6c74524b35435432353653315f00110f5f5a6774524b35435432353653315f00120f5f5a6571524b35435432353653315f0013125f5a3966756e6374696f6e313550617261310014125f5a3966756e6374696f6e32355061726132001504696e69740016056170706c7900170ab2010c0a0020012002200010000b0a0020012002200010090b0a0020012002200010060b0a0020012002200010010b0a0020012002200010050b0b002000200110044100470b0b002000200110034100470b0b002000200110024100470b06002000100a0b0e0020002802002000280204100b0b3c01017f4100410028020441c0006b2200360204418a34100a200041206a41101008200041201008200041206a200010074100200041c0006a3602040b02000b0b18030041040b04304000000041100b0236000041200b023700";
		string code_initIO_while = "0061736d01000000011d0660037f7f7f0060027f7f017f60017f0060027f7f0060000060017e0002b0010c03656e76086164645f75323536000003656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e760873657473746f7265000303656e760c7365747532353676616c7565000303656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030d0c000000000001010102020405040401700000050301000107d3010d066d656d6f727902000f5f5a706c524b35435432353653315f000c0f5f5a6d69524b35435432353653315f000d0f5f5a6d6c524b35435432353653315f000e0f5f5a6476524b35435432353653315f000f0f5f5a726d524b35435432353653315f00100f5f5a6c74524b35435432353653315f00110f5f5a6774524b35435432353653315f00120f5f5a6571524b35435432353653315f0013125f5a3966756e6374696f6e313550617261310014125f5a3966756e6374696f6e32355061726132001504696e69740016056170706c7900170aac010c0a0020012002200010000b0a0020012002200010090b0a0020012002200010060b0a0020012002200010010b0a0020012002200010050b0b002000200110044100470b0b002000200110034100470b0b002000200110024100470b06002000100a0b0e0020002802002000280204100b0b3601017f4100410028020441c0006b2200360204418a34100a200041206a411010082000412010080340200041206a200010070c000b0b02000b0b18030041040b04304000000041100b0236000041200b023700";
		string no_init = "0061736d01000000011a0560037f7f7f0060027f7f017f60017f0060027f7f0060017e0002b1010c03656e760a5741564d417373657274000303656e76086164645f75323536000003656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e760b726561644d657373616765000103656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030c0b0000000000010101020204040401700000050301000107cc010c066d656d6f727902000f5f5a706c524b35435432353653315f000c0f5f5a6d69524b35435432353653315f000d0f5f5a6d6c524b35435432353653315f000e0f5f5a6476524b35435432353653315f000f0f5f5a726d524b35435432353653315f00100f5f5a6c74524b35435432353653315f00110f5f5a6774524b35435432353653315f00120f5f5a6571524b35435432353653315f0013125f5a3966756e6374696f6e313550617261310014125f5a3966756e6374696f6e323550617261320015056170706c7900160aaa040b0a0020012002200010010b0a0020012002200010090b0a0020012002200010070b0a0020012002200010020b0a0020012002200010060b0b002000200110054100470b0b002000200110044100470b0b002000200110034100470b06002000100a0b0e0020002802002000280204100b0bb60303027f047e017f4100410028020441106b220736020442002104423b2103411021024200210503400240024002400240024020044209560d0020022c00002201419f7f6a41ff017141194b0d01200141a0016a21010c020b420021062004420b580d020c030b200141ea016a41002001414f6a41ff01714105491b21010b2001ad42388642388721060b2006421f83200342ffffffff0f838621060b200241016a2102200442017c2104200620058421052003427b7c2203427a520d000b024020052000520d00200741086a4104100841034b412010002007280208100a0b42002104423b210341c00021024200210503400240024002400240024020044209560d0020022c00002201419f7f6a41ff017141194b0d01200141a0016a21010c020b420021062004420b580d020c030b200141ea016a41002001414f6a41ff01714105491b21010b2001ad42388642388721060b2006421f83200342ffffffff0f838621060b200241016a2102200442017c2104200620058421052003427b7c2203427a520d000b024020052000520d0020074108100841074b4120100020072802002007280204100b0b4100200741106a3602040b0b4e040041040b04504000000041100b0b746573745f66756e6331000041200b1e6d6573736167652073686f72746572207468616e206578706563746564000041c0000b0b746573745f66756e633200";
		string div_0_u256 = "0061736d01000000011d0660037f7f7f0060027f7f017f60017f0060027f7f0060000060017e0002a1010b03656e76086164645f75323536000003656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e760c7365747532353676616c7565000303656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030d0c000000000001010102020405040401700000050301000107d3010d066d656d6f727902000f5f5a706c524b35435432353653315f000b0f5f5a6d69524b35435432353653315f000c0f5f5a6d6c524b35435432353653315f000d0f5f5a6476524b35435432353653315f000e0f5f5a726d524b35435432353653315f000f0f5f5a6c74524b35435432353653315f00100f5f5a6774524b35435432353653315f00110f5f5a6571524b35435432353653315f0012125f5a3966756e6374696f6e313550617261310013125f5a3966756e6374696f6e32355061726132001404696e69740015056170706c7900160ab7010c0a0020012002200010000b0a0020012002200010080b0a0020012002200010060b0a0020012002200010010b0a0020012002200010050b0b002000200110044100470b0b002000200110034100470b0b002000200110024100470b0600200010090b0e0020002802002000280204100a0b02000b4101017f4100410028020441e0006b2201360204200141c0006a41101007200141206a41201007200141c0006a200141206a200110014100200141e0006a3602040b0b1c030041040b04304000000041100b063131313131000041200b023000";
		string div_0_i32 = "0061736d01000000011d0660037f7f7f0060027f7f017f60017f0060027f7f0060000060017e00028e010a03656e76086164645f75323536000003656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030d0c000000000001010102020405040401700000050301000107d3010d066d656d6f727902000f5f5a706c524b35435432353653315f000a0f5f5a6d69524b35435432353653315f000b0f5f5a6d6c524b35435432353653315f000c0f5f5a6476524b35435432353653315f000d0f5f5a726d524b35435432353653315f000e0f5f5a6c74524b35435432353653315f000f0f5f5a6774524b35435432353653315f00100f5f5a6571524b35435432353653315f0011125f5a3966756e6374696f6e313550617261310012125f5a3966756e6374696f6e32355061726132001304696e69740014056170706c7900150ab2040c3601017f4100410028020441106b22033602042003200136020c20032002360208200328020c2002200010004100200341106a3602040b3601017f4100410028020441106b22033602042003200136020c20032002360208200328020c2002200010074100200341106a3602040b3601017f4100410028020441106b22033602042003200136020c20032002360208200328020c2002200010064100200341106a3602040b3601017f4100410028020441106b22033602042003200136020c20032002360208200328020c2002200010014100200341106a3602040b3601017f4100410028020441106b22033602042003200136020c20032002360208200328020c2002200010054100200341106a3602040b3b01017f4100410028020441106b22023602042002200036020c20022001360208200228020c2001100421014100200241106a36020420014100470b3b01017f4100410028020441106b22023602042002200036020c20022001360208200228020c2001100321014100200241106a36020420014100470b3b01017f4100410028020441106b22023602042002200036020c20022001360208200228020c2001100221014100200241106a36020420014100470b2801017f4100410028020441106b220136020420012000360208200010084100200141106a3602040b0e002000280200200028020410090b02000b2e01017f410028020441206b2201200037031820014101360214200141003602102001200128021441006d36020c0b0b0a010041040b0410400000";
		string invalid_u256_print = "0061736d01000000011d0660037f7f7f0060027f7f017f60017f0060027f7f0060000060017e00029e010b03656e76086164645f75323536000003656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e76097072696e7475323536000203656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030d0c000000000001010102020405040401700000050301000107d3010d066d656d6f727902000f5f5a706c524b35435432353653315f000b0f5f5a6d69524b35435432353653315f000c0f5f5a6d6c524b35435432353653315f000d0f5f5a6476524b35435432353653315f000e0f5f5a726d524b35435432353653315f000f0f5f5a6c74524b35435432353653315f00100f5f5a6774524b35435432353653315f00110f5f5a6571524b35435432353653315f0012125f5a3966756e6374696f6e313550617261310013125f5a3966756e6374696f6e32355061726132001404696e69740015056170706c7900160a97010c0a0020012002200010000b0a0020012002200010080b0a0020012002200010060b0a0020012002200010010b0a0020012002200010050b0b002000200110044100470b0b002000200110034100470b0b002000200110024100470b0600200010090b0e0020002802002000280204100a0b02000b2101017f4100410028020441206b2201360204200110074100200141206a3602040b0b0a010041040b0410400000";

		string test_func1_string = "00c01eae1a4067a1";
		string test_func2_string = "00001fae1a4067a1";
		string test_func3_string = "00401fae1a4067a1";
		string test_func4_string = "00801fae1a4067a1";
		string test_func5_string = "00c01fae1a4067a1";
		
		string para1_string = "15030000";
		string para2_string = "672b0000ce560000";

		BOOST_FIXTURE_TEST_SUITE(ContractTestsSuite, TestOutputHelperFixture)
		BOOST_AUTO_TEST_CASE(ctCreateContract)
		{
			cout << "ctCreateContract" << endl;
			std::string currencyHex = loadData("currency.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0xea60";
			string gasPrice = "0x04a817c800";
			string value = "0x0";
			string data = "";

			//创建空合约
			client.sendTransaction(gasLimit, gasPrice, "", value, data, account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost = u256(1000000000000000000) - balance;
			string s_cost = cost.str();
			cout << "s_cost: " << s_cost << endl;

			//普通创建合约
			gasLimit = "0x1f71b2";
			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			cout << "s_address: " << s_address << endl;
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + currencyHex, account);  //Create contract.
			client.produce_blocks();

			bytes contractCode = client.code(Address(s_address));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(currencyHex) == 0);  //Check contract code.
			u256 balance3 = client.balance(Address(account.address));
			u256 cost_create = balance - balance3;
			string s_create = cost_create.str();
			cout << "s_create: " << s_create << endl;

			//gasPrice翻倍，创建相同的合约
			gasPrice = "0x09502f9000";  //Duplicate gasPrice.
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + currencyHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance4 = client.balance(Address(account.address));
			u256 cost_create_2 = balance3 - balance4;
			string s_create_2 = cost_create_2.str();
			cout << "s_create_2: " << s_create_2 << endl;
			BOOST_REQUIRE(cost_create_2 == (cost_create * 2));  //Check cost.

			//恢复gasPrice，创建合约时，给合约转账
			gasPrice = "0x04a817c800";  //Revert gasPrice.
			value = "0x04";  //Set endowment.
			Address m_newAddress_2 = newAddress(account);  //Contract Address.
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + currencyHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance5 = client.balance(m_newAddress_2);
			cout << "contract endowment: " << balance5.str() << endl;
			BOOST_REQUIRE(balance5 == u256(4));  //Check endowment.

			//创建合约时“to”字段有值，表示普通账户，该地址没有code，“value”字段有值。
			//创建失败，解析为合约调用，“value”的值会转给“to”地址。
			Address m_newAddress_3 = newAddress(account);  //Contract Address.
			client.sendTransaction(gasLimit, gasPrice, account2.address, value, "0x" + currencyHex, account);  //Create contract. (Error)
			client.produce_blocks();

			u256 balance6 = client.balance(m_newAddress_3);
			cout << "contract endowment: " << balance6.str() << endl;
			BOOST_REQUIRE(balance6 == u256());  //Check endowment.

			u256 balance7 = client.balance(Address(account2.address));
			cout << "contract endowment: " << balance7.str() << endl;
			BOOST_REQUIRE(balance7 - u256(1000000000000000000) == u256(4));  //Check endowment.

			bytes contractCode_2 = client.code(m_newAddress_3);  //Get contract address.
			BOOST_REQUIRE(contractCode_2.size() == 0);
			bytes contractCode_3 = client.code(Address(account2.address));
			BOOST_REQUIRE(contractCode_3.size() == 0);

			//创建合约时“to”字段有值，表示普通账户，该地址没有code，“value”字段为零。
			//创建失败，解析为合约调用。
			value = "0x00";
			Address m_newAddress_4 = newAddress(account);  //Contract Address.
			client.sendTransaction(gasLimit, gasPrice, account3.address, value, "0x" + currencyHex, account);  //Create contract. (Error)
			client.produce_blocks();

			u256 balance8 = client.balance(m_newAddress_4);
			cout << "contract endowment: " << balance8.str() << endl;
			BOOST_REQUIRE(balance8 == u256());  //Check endowment.
			u256 balance9 = client.balance(Address(account3.address));
			cout << "contract endowment: " << balance9.str() << endl;
			BOOST_REQUIRE(balance9 == u256(1000000000000000000));  //Check endowment.
			bytes contractCode_4 = client.code(m_newAddress_4);  //Get contract address.
			BOOST_REQUIRE(contractCode_4.size() == 0);
			bytes contractCode_5 = client.code(Address(account3.address));
			BOOST_REQUIRE(contractCode_5.size() == 0);

		}

		BOOST_AUTO_TEST_CASE(ctCodeSize)
		{
			cout << "ctCodeSize" << endl;
			std::string currencyHex = loadData("currency.wasm");

			std::string currency_2Hex = loadData("currency_2.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//创建短合约
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + currencyHex, account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost = u256(1000000000000000000) - balance;
			string s_cost = cost.str();
			cout << "s_cost: " << s_cost << endl;

			//创建长合约
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + currency_2Hex, account2);
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account2.address));
			BOOST_REQUIRE(balance2 < u256(1000000000000000000));
			u256 cost2 = u256(1000000000000000000) - balance2;
			string s_cost2 = cost2.str();
			cout << "s_cost2: " << s_cost2 << endl;

			BOOST_REQUIRE(cost < cost2);

			//调整交易的gasLimit，模拟发布合约代码过长，超过块最大gasLimit限制
			gasLimit = "0x1f71b20000";
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + currency_2Hex, account2);
			client.produce_blocks();

			u256 balance3 = client.balance(Address(account2.address));
			u256 cost3 = balance2 - balance3;
			string s_cost3 = cost3.str();
			cout << "s_cost3: " << s_cost3 << endl;

			BOOST_REQUIRE(cost3  == u256());
		}

		BOOST_AUTO_TEST_CASE(ctLackBalance)
		{
			cout << "ctLackBalance" << endl;
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//验证io可用
			Address m_newAddress = newAddress(account);
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + code_initIO, account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost = u256(1000000000000000000) - balance;
			cout << "cost.str(): " << cost.str() << endl;

			u256 res1 = client.storage(m_newAddress, u256(6));  //Get contract storage.
			BOOST_REQUIRE(res1 == u256(7));
			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(code_initIO) == 0);  //Check contract code.

			//运行init时，gas不足。合约发布失败，扣除所有gas。
			Address m_newAddress2 = newAddress(account);
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + code_initIO_while, account2);
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account2.address));
			u256 cost2 = u256(1000000000000000000) - balance2;
			BOOST_REQUIRE(cost2 == u256(2060722) * u256(20000000000));
			BOOST_REQUIRE(cost2 > cost);
			cout << "cost2.str(): " << cost2.str() << endl;

			u256 res2 = client.storage(m_newAddress2, u256(6));  //Get contract storage.
			BOOST_REQUIRE(res2 == u256());
			bytes contractCode_2 = client.code(Address(m_newAddress2));  //Get contract code.
			BOOST_REQUIRE(contractCode_2.size() == 0);

			//增大gasPrice，使地址余额小于gas*gasPrice。合约发布失败，交易没有被执行，不扣除gas。
			gasPrice = "0x04a817c80000000000";
			Address m_newAddress3 = newAddress(account);
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + code_initIO, account3);
			client.produce_blocks();

			u256 balance3 = client.balance(Address(account3.address));
			BOOST_REQUIRE(balance3 == u256(1000000000000000000));

			u256 res3 = client.storage(m_newAddress3, u256(6));  //Get contract storage.
			BOOST_REQUIRE(res3 == u256());
			bytes contractCode_3 = client.code(Address(m_newAddress3));  //Get contract code.
			BOOST_REQUIRE(contractCode_3.size() == 0);

		}

		BOOST_AUTO_TEST_CASE(ctCompile)
		{
			cout << "ctCompile" << endl;
			std::string currencyHex = loadData("currency.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//创建非法合约
			Address m_newAddress = newAddress(account);
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + currencyHex.substr(0, 100), account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			u256 cost = u256(1000000000000000000) - balance;
			cout << "cost.str(): " << cost.str() << endl;
			BOOST_REQUIRE(cost == u256(2060722) * u256(20000000000));
			bytes contractCode_2 = client.code(Address(m_newAddress));  //Get contract code.
			BOOST_REQUIRE(contractCode_2.size() == 0);
		}

		BOOST_AUTO_TEST_CASE(ctLackField)
		{
			cout << "ctLackField" << endl;
			std::string currencyHex = loadData("currency.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//创建短合约，缺少“to”字段
			Address m_newAddress = newAddress(account);
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + currencyHex, account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost = u256(1000000000000000000) - balance;
			cout << "cost.str(): " << cost.str() << endl;

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(currencyHex) == 0);

			//创建短合约，缺少“gasLimit”字段
			Address m_newAddress_2 = newAddress(account);
			client.sendTransaction("", gasPrice, "", value, "0x" + currencyHex, account);
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			BOOST_REQUIRE(balance == balance2);

			bytes contractCode_2 = client.code(Address(m_newAddress_2));  //Get contract address.
			BOOST_REQUIRE(contractCode_2.size() == 0);

			////创建短合约，缺少“gasPrice”字段
			//Address m_newAddress_3 = newAddress(account2);
			//TransactionSkeleton ts;
			//ts.type = TransactionType::ContractCreation;
			//ts.nonce = u256(account2.nonce);
			//account2.nonce++;
			//ts.value = u256(0);
			//ts.to = Address(0);
			//ts.gasPrice = u256(0);
			//ts.gas = u256(2060722);
			//ts.data = fromHex(code1);
			//ts.from = Address(account2.address);
			//Secret secret(account2.secret);
			//client.sendTransaction(ts, secret);

			//client.produce_blocks();

			//u256 balance3 = client.balance(Address(account2.address));
			//u256 cost_2 = u256(1000000000000000000) - balance3;
			//string ss = cost_2.str();

			//bytes contractCode_3 = client.code(Address(m_newAddress_3));  //Get contract address.
			//BOOST_REQUIRE(contractCode_3.size() == 0);
		}

		BOOST_AUTO_TEST_CASE(ctInit)
		{
			cout << "ctInit" << endl;
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//创建合约，合约中没有init函数
			Address m_newAddress = newAddress(account);
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + no_init, account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost = u256(1000000000000000000) - balance;
			BOOST_REQUIRE(cost < u256(2060722) * u256(20000000000));
			cout << "cost.str(): " << cost.str() << endl;

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(no_init) == 0);

			//调用刚创建的，无init函数的合约
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string + para1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance - balance2;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
			cout << "cost_call.str(): " << cost_call.str() << endl;

		}

		BOOST_AUTO_TEST_CASE(ctDiv0)
		{
			cout << "ctDiv0" << endl;
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + div_0_u256, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			//u256除0
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call == u256(2060722) * u256(20000000000));


			Address m_newAddress2 = newAddress(account);  //Contract Address.
			string s_address2 = m_newAddress2.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + div_0_i32, account);  //Create contract.
			client.produce_blocks();
			u256 balance3 = client.balance(Address(account.address));

			//i32除0
			client.sendTransaction(gasLimit, gasPrice, s_address2, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance4 = client.balance(Address(account.address));
			u256 cost_call2 = balance3 - balance4;
			BOOST_REQUIRE(cost_call2 == u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctInvalidU256Print)
		{
			cout << "ctInvalidU256Print" << endl;
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + invalid_u256_print, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(invalid_u256_print) == 0);

			//输出错误的u256  accessDestroy2.cpp
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctInfiniteLoop)
		{
			cout << "ctInfiniteLoop" << endl;
			std::string infiniteLoopHex = loadData("infiniteLoop.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + infiniteLoopHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(infiniteLoopHex) == 0);

			//无限循环调用注册函数  infiniteLoop.cpp
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call == u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctInfiniteLoop2)
		{
			cout << "ctInfiniteLoop2" << endl;
			std::string infiniteLoop2Hex = loadData("infiniteLoop2.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//无限循环调用"1+1"  infiniteLoop2.cpp
			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + infiniteLoop2Hex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));
			BOOST_REQUIRE(u256(1000000000000000000) - balance1 == u256(2060722) * u256(20000000000));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			BOOST_REQUIRE(contractCode.size() == 0);
		}

		BOOST_AUTO_TEST_CASE(ctOverflow)
		{
			cout << "ctOverflow" << endl;
			std::string overflowHex = loadData("overflow.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + overflowHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(overflowHex) == 0);

			//u256加减法越界  overflow.cpp
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctOverflow2)
		{
			cout << "ctOverflow2" << endl;
			std::string overflow2Hex = loadData("overflow2.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + overflow2Hex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(overflow2Hex) == 0);

			//数组下标越界  overflow2.cpp
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctAssert)
		{
			cout << "ctAssert" << endl;
			std::string assertHex = loadData("assert.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + assertHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(assertHex) == 0);

			//测试脚本中调用注册函数WAVMAssert  assert.cpp
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call == u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctThrow)
		{
			cout << "ctThrow" << endl;
			std::string throwHex = loadData("throw.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + throwHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			BOOST_REQUIRE(contractCode.size() == 0);
			//测试脚本使用throw  throw.cpp
		}

		BOOST_AUTO_TEST_CASE(ctOverflow3)
		{
			cout << "ctOverflow3" << endl;
			std::string overflow3Hex = loadData("overflow3.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + overflow3Hex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(overflow3Hex) == 0);

			//脚本中定义u256为6个int，应该为8个int  overflow3.cpp
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctCast)
		{
			cout << "ctCast" << endl;
			std::string castHex = loadData("cast.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + castHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(castHex) == 0);

			//强制类型转换  cast.cpp
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctInvalidDeletedPoint)
		{
			cout << "ctInvalidDeletedPoint" << endl;
			std::string accessDestroyHex = loadData("accessDestroy.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + accessDestroyHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(accessDestroyHex) == 0);

			//访问调用函数返回的局部变量  accessDestroy.cpp
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctNewDelete)
		{
			cout << "ctNewDelete" << endl;
			std::string newDelete_2Hex = loadData("newDelete_2.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + newDelete_2Hex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(newDelete_2Hex) == 0);

			//new、delete函数，调用到相应的类构造与析构函数  newDelete_2.cpp
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctUnknowInstruction)
		{
			cout << "ctUnknowInstruction" << endl;
			std::string unknowInstructionHex = loadData("unknowInstruction.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//调用未知函数 unknowInstruction.cpp
			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + unknowInstructionHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			BOOST_REQUIRE(contractCode.size() == 0);
			BOOST_REQUIRE(u256(1000000000000000000) - balance1 == u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctMemoryRef)
		{
			cout << "ctMemoryRef" << endl;
			std::string memoryRefHex = loadData("memoryRef.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + memoryRefHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			//memoryRef传入负值 memoryRef.cpp
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call == u256(2060722) * u256(20000000000));
			cout << "cost_call: " << cost_call << endl;
		}

		BOOST_AUTO_TEST_CASE(ctTransferBalance)
		{
			cout << "ctTransferBalance" << endl;
			std::string transferBalanceHex = loadData("transferBalance.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + transferBalanceHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			//合约内部转账，余额不足
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			
			BOOST_REQUIRE(cost_call == u256(2060722) * u256(20000000000));

			u256 balance3 = client.balance(Address("0x0000000000000000000000000000000000011111"));
			BOOST_REQUIRE(balance3 == u256(0));

			//向合约地址转账，data有值
			client.sendTransaction(gasLimit, gasPrice, s_address, "99999", test_func3_string, account);
			client.produce_blocks();
			u256 balance4 = client.balance(Address(s_address));
			cout << "balance4.str(): " << balance4.str() << endl;
			BOOST_REQUIRE(balance4 == u256(99999));

			//合约内部转账，余额足够
			u256 balance5 = client.balance(Address(account.address));
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance6 = client.balance(Address(account.address));
			u256 cost_call2 = balance5 - balance6;
			BOOST_REQUIRE(cost_call2 < u256(2060722) * u256(20000000000));

			u256 balance7 = client.balance(Address("0x0000000000000000000000000000000000011111"));
			u256 balance8 = client.balance(Address(s_address));
			BOOST_REQUIRE_EQUAL(balance7, u256(123));
			BOOST_REQUIRE_EQUAL(balance8, u256(99876));

			//向合约地址转账，data为空
			client.sendTransaction(gasLimit, gasPrice, s_address, "123", "", account);
			client.produce_blocks();

			u256 balance9 = client.balance(Address(account.address));
			u256 cost_call3 = balance6 - balance9;
			BOOST_REQUIRE(cost_call3 < u256(2060722) * u256(20000000000));

			u256 balance10 = client.balance(Address(s_address));
			cout << "balance10.str(): " << balance10.str() << endl;
			BOOST_REQUIRE_EQUAL(balance10 , u256(99999));

			//向合约地址转账，data的值错误
			client.sendTransaction(gasLimit, gasPrice, s_address, "11111", test_func3_string.substr(0, 14), account);
			client.produce_blocks();

			u256 balance11 = client.balance(Address(account.address));
			u256 cost_call4 = balance9 - balance11;
			BOOST_REQUIRE_EQUAL(cost_call4 , u256(2060722) * u256(20000000000));

			u256 balance12 = client.balance(Address(s_address));
			cout << "balance12.str(): " << balance12.str() << endl;
			BOOST_REQUIRE_EQUAL(balance12, u256(99999));
		}

		BOOST_AUTO_TEST_CASE(ctTransferBalance2)
		{
			cout << "ctTransferBalance2" << endl;
			std::string currencyHex = loadData("currency.wasm");
			std::string transferBalanceHex = loadData("transferBalance.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + transferBalanceHex, account);  //Create contract.
			client.produce_blocks();

			Address m_newAddress_2 = newAddress(account);  //Contract Address2.
			string s_address_2 = m_newAddress_2.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + currencyHex, account);
			client.produce_blocks();

			//合约内部转账，余额不足
			u256 balance1 = client.balance(Address(account.address));
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func2_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			BOOST_REQUIRE(cost_call == u256(2060722) * u256(20000000000));

			u256 balance3 = client.balance(m_newAddress_2);
			BOOST_REQUIRE(balance3 == u256(0));

			//向合约地址转账
			client.sendTransaction(gasLimit, gasPrice, s_address, "99999", test_func3_string, account);
			client.produce_blocks();
			u256 balance4 = client.balance(Address(s_address));
			cout << "balance4.str(): " << balance4.str() << endl;
			BOOST_REQUIRE(balance4 == u256(99999));

			//合约内部转账，余额足够
			u256 balance5 = client.balance(Address(account.address));
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func2_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance6 = client.balance(Address(account.address));
			u256 cost_call2 = balance5 - balance6;
			BOOST_REQUIRE(cost_call2 < u256(2060722) * u256(20000000000));

			u256 balance7 = client.balance(m_newAddress_2);
			cout << "balance7.str(): " << balance7.str() << endl;
			BOOST_REQUIRE(balance7 == u256(456));
		}

		BOOST_AUTO_TEST_CASE(ctLackApply)
		{
			cout << "ctLackApply" << endl;
			std::string lackApplyHex = loadData("lackApply.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//部署缺少apply函数的合约
			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + lackApplyHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));
			u256 cost_call2 = u256(1000000000000000000) - balance1;
			BOOST_REQUIRE(cost_call2 < u256(2060722) * u256(20000000000));

			bytes contractCode = client.code(Address(s_address));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(lackApplyHex) == 0);  //Check contract code.

			
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call3 = balance1 - balance2;
			BOOST_REQUIRE(cost_call3 == u256(2060722) * u256(20000000000));
		}

		BOOST_AUTO_TEST_CASE(ctGas)
		{
			cout << "ctGas" << endl;
			std::string gasHex = loadData("gas.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			cout << "s_address: " << s_address << endl;
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + gasHex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));

			//调用合约方法1 加
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call = balance1 - balance2;
			cout << "cost_call.str(): " << cost_call.str() << endl;
			BOOST_REQUIRE(cost_call == u256(452460000000000));

			//调用合约方法2 减
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func2_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance3 = client.balance(Address(account.address));
			u256 cost_call_2 = balance2 - balance3;
			cout << "cost_call_2.str(): " << cost_call_2.str() << endl;
			BOOST_REQUIRE(cost_call_2 == u256(451180000000000));

			
			//调用合约方法3 IO
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func3_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance4 = client.balance(Address(account.address));
			u256 cost_call_3 = balance3 - balance4;
			cout << "cost_call_3.str(): " << cost_call_3.str() << endl;
			BOOST_REQUIRE(cost_call_3 == u256(957580000000000));
		}

		BOOST_AUTO_TEST_CASE(ctInitGas)
		{
			cout << "ctInitGas" << endl;
			std::string initGas_1Hex = loadData("initGas_1.wasm");

			std::string initGas_2Hex = loadData("initGas_2.wasm");

			std::string initGas_3Hex = loadData("initGas_3.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//创建合约1，init执行了两次加法，apply什么也没做
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + initGas_1Hex, account);  //Create contract.
			client.produce_blocks();
			u256 balance1 = client.balance(Address(account.address));
			u256 cost_call = u256(1000000000000000000) - balance1;

			//创建合约1，init执行了一次加法，apply什么也没做
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + initGas_2Hex, account);  //Create contract.
			client.produce_blocks();
			u256 balance2 = client.balance(Address(account.address));
			u256 cost_call2 = balance1 - balance2;

			//创建合约1，init什么也没做，apply执行了一次加法
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + initGas_3Hex, account);  //Create contract.
			client.produce_blocks();
			u256 balance3 = client.balance(Address(account.address));
			u256 cost_call3 = balance2 - balance3;

			BOOST_REQUIRE(cost_call > cost_call2);
			BOOST_REQUIRE(cost_call2 > cost_call3);
		}

		BOOST_AUTO_TEST_CASE(ctCallContract)
		{
			cout << "ctCallContract" << endl;
			std::string currencyHex = loadData("currency.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			cout << "s_address: " << s_address << endl;
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + currencyHex, account);  //Create contract.
			client.produce_blocks();

			u256 balance3 = client.balance(Address(account.address));
			bytes contractCode = client.code(Address(s_address));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(currencyHex) == 0);  //Check contract code.

			//调用合约方法1
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string + para1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance4 = client.balance(Address(account.address));
			u256 cost_call = balance3 - balance4;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
			string s_call = cost_call.str();
			cout << "s_call: " << s_call << endl;

			//调用合约方法2
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func2_string + para2_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance5 = client.balance(Address(account.address));
			u256 cost_call_2 = balance4 - balance5;
			BOOST_REQUIRE(cost_call_2 < u256(2060722) * u256(20000000000));
			string s_call_2 = cost_call_2.str();
			cout << "s_call_2: " << s_call_2 << endl;

			//调用函数散列值太短
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func2_string.substr(0, 14), account);  //Call contract code. (Error)
			client.produce_blocks();

			u256 balance6 = client.balance(Address(account.address));
			u256 cost_call_3 = balance5 - balance6;
			BOOST_REQUIRE(cost_call_3 == u256(2060722) * u256(20000000000));

			//调用函数参数不对
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func2_string + para1_string, account);  //Call contract code. (Error)
			client.produce_blocks();

			u256 balance7 = client.balance(Address(account.address));
			u256 cost_call_4 = balance6 - balance7;
			BOOST_REQUIRE(cost_call_4 == u256(2060722) * u256(20000000000));

		}

		BOOST_AUTO_TEST_CASE(ctTestSampleTransaction)
		{
			cout << "ctTestSampleTransaction" << endl;
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x05";

			client.sendTransaction(gasLimit, gasPrice, account2.address, value, "0x" , account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			u256 balance2 = client.balance(Address(account2.address));
			BOOST_REQUIRE(balance2 - u256(1000000000000000000)  == u256(5));

			u256 cost = u256(1000000000000000000) - balance - u256(5);
			u256 gasSample = cost / u256(20000000000);
			cout << "cost: " << cost << endl;
			cout << "gasSample: " << gasSample << endl;
		}

		BOOST_AUTO_TEST_CASE(ctDefault)
		{
			WASM_CORE::destoryInstance();
			std::string defaultHex = loadData("default.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + defaultHex, account);  //Create contract.
			client.produce_blocks();

			u256 balance3 = client.balance(Address(account.address));
			bytes contractCode = client.code(m_newAddress);  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(defaultHex) == 0);  //Check contract code.

			//调用default方法
			client.sendTransaction(gasLimit, gasPrice, s_address, "0x3", "", account);  //Call contract code.
			client.produce_blocks();

			u256 balance4 = client.balance(Address(account.address));
			u256 cost_call = balance3 - balance4;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
			u256 res1 = client.storage(m_newAddress, u256(1));  //Get contract storage.
			BOOST_REQUIRE_EQUAL(res1 , u256(24));
			BOOST_REQUIRE_EQUAL(client.balance(m_newAddress), u256(3));


			//调用合约方法apply
			client.sendTransaction(gasLimit, gasPrice, s_address, "0x4", test_func1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance5 = client.balance(Address(account.address));
			u256 cost_call_2 = balance4 - balance5;
			BOOST_REQUIRE(cost_call_2 < u256(2060722) * u256(20000000000));
			u256 res2 = client.storage(m_newAddress, u256(1));  //Get contract storage.
			BOOST_REQUIRE_EQUAL(res2, u256(8));
			BOOST_REQUIRE_EQUAL(client.balance(m_newAddress), u256(7));

		}

		BOOST_AUTO_TEST_CASE(ctGame)
		{
			WASM_CORE::destoryInstance();
			std::string gameHex = loadData("game.wasm");

			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + gameHex, account);  //Create contract.
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			bytes contractCode = client.code(Address(s_address));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(gameHex) == 0);  //Check contract code.
			u256 storage = client.storage(m_newAddress, u256(0));  //Get contract storage.
			BOOST_REQUIRE(storage != u256(0));

			//调用合约方法buy
			client.sendTransaction(gasLimit, gasPrice, s_address, "0x64", name2hex("buy") + account3.address.substr(2), account2);  //转账，系统抽成
			client.produce_blocks();

			u256 balance1 = client.balance(Address(account.address));
			u256 balance2 = client.balance(Address(account2.address));
			u256 balance3 = client.balance(Address(account3.address));
			u256 cost_1 = balance1 - balance;
			u256 cost_2 = u256(1000000000000000000) - balance2;
			u256 cost_3 = balance3 - u256(1000000000000000000);
			BOOST_REQUIRE_EQUAL(cost_1, u256(10));
			BOOST_REQUIRE_EQUAL(cost_3, u256(90));
			cout << "cost_2: " << cost_2 << endl;
			cout << "cost_2 - u256(100): " << cost_2 - u256(100) << endl;

			//合约创建者 调用合约方法add
			u256 balance_1 = client.balance(Address(account.address));
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("add") + account3.address.substr(2) + "0232", account);  //添加装备
			client.produce_blocks();
			u256 balance_2 = client.balance(Address(account.address));
			cout << "add_0232: " << balance_1 - balance_2 << endl;
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("add") + account3.address.substr(2) + "0540", account);
			client.produce_blocks();
			balance_1 = client.balance(Address(account.address));
			cout << "add_0540: " << balance_2 - balance_1 << endl;
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("add") + account3.address.substr(2) + "1fbb", account);
			client.produce_blocks();
			balance_2 = client.balance(Address(account.address));
			cout << "add_1fbb: " << balance_1 - balance_2 << endl;
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("add") + account3.address.substr(2) + "55aa", account);
			client.produce_blocks();
			balance_1 = client.balance(Address(account.address));
			cout << "add_55aa: " << balance_2 - balance_1 << endl;

			u256 address_3_u256;
			addressToU256(Address(account3.address), address_3_u256);
			storage = client.storage(m_newAddress, address_3_u256);
			h256 storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[0], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[1], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[2], 50);
			BOOST_REQUIRE_EQUAL(storage_h256[5], 64);
			BOOST_REQUIRE_EQUAL(storage_h256[31], 187);
			BOOST_REQUIRE_EQUAL(storage_h256[85], 0);

			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("add") + account3.address.substr(2) + "1fbb", account); //添加装备超出限制，无变化
			client.produce_blocks();
			storage = client.storage(m_newAddress, address_3_u256);
			storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[31], 187);

			//合约创建者 调用合约方法sub
			balance_1 = client.balance(Address(account.address));
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("sub") + account3.address.substr(2) + "0210", account);  //正确删除装备，有剩余
			client.produce_blocks();
			balance_2 = client.balance(Address(account.address));
			cout << "sub_0210: " << balance_1 - balance_2 << endl;
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("sub") + account3.address.substr(2) + "0540", account);  //完全删除装备，无剩余
			client.produce_blocks();
			balance_1 = client.balance(Address(account.address));
			cout << "sub_0540: " << balance_2 - balance_1 << endl;
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("sub") + account3.address.substr(2) + "1fbc", account);  //错误删除装备，无变化
			client.produce_blocks();
			balance_2 = client.balance(Address(account.address));
			cout << "sub_0210: " << balance_1 - balance_2 << endl;

			storage = client.storage(m_newAddress, address_3_u256);
			storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[0], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[1], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[2], 34);
			BOOST_REQUIRE_EQUAL(storage_h256[5], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[31], 187);

			//合约创建者 调用合约方法send
			balance_1 = client.balance(Address(account.address));
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("send") + account3.address.substr(2) + account2.address.substr(2) + "0223", account);  //错误转移装备数量，无变化
			client.produce_blocks();
			balance_2 = client.balance(Address(account.address));
			cout << "send_0223: " << balance_1 - balance_2 << endl;
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("send") + account3.address.substr(2) + account2.address.substr(2) + "1f0b", account);  //正确转移装备
			client.produce_blocks();
			balance_1 = client.balance(Address(account.address));
			cout << "send_1f0b: " << balance_2 - balance_1 << endl;
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("send") + account3.address.substr(2) + account2.address.substr(2) + "0123", account);  //错误转移没有的装备，无变化
			client.produce_blocks();
			balance_2 = client.balance(Address(account.address));
			cout << "send_0123: " << balance_1 - balance_2 << endl;

			u256 address_2_u256;
			addressToU256(Address(account2.address), address_2_u256);

			storage = client.storage(m_newAddress, address_3_u256);
			storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[1], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[2], 34);
			BOOST_REQUIRE_EQUAL(storage_h256[31], 176);

			storage = client.storage(m_newAddress, address_2_u256);
			storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[1], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[2], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[31], 11);

			//非合约创建者 调用合约方法add、sub、send 无效 存储无变化
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("add") + account3.address.substr(2) + "0105", account3);
			client.produce_blocks();
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("sub") + account3.address.substr(2) + "0205", account3);
			client.produce_blocks();
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("send") + account2.address.substr(2) + account3.address.substr(2) + "1f01", account3);
			client.produce_blocks();

			storage = client.storage(m_newAddress, address_3_u256);
			storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[1], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[2], 34);
			BOOST_REQUIRE_EQUAL(storage_h256[31], 176);

			storage = client.storage(m_newAddress, address_2_u256);
			storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[1], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[2], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[31], 11);

			//装备转移的from与to为同一个地址
			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("send") + account3.address.substr(2) + account3.address.substr(2) + "0210", account);  //什么都不做
			client.produce_blocks();
			storage = client.storage(m_newAddress, address_3_u256);
			storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[2], 34);

			//合约创建者 给零地址（系统预留）保存装备， 期望什么都不做，因为零地址保存了合约所有者（owner）
			Address address_0 = Address(0);
			u256 address_0_u256;
			addressToU256(address_0, address_0_u256);
			storage = client.storage(m_newAddress, address_0_u256);
			h256 storage_h256_temp1 = (h256)storage;

			client.sendTransaction(gasLimit, gasPrice, s_address, "", name2hex("add") + address_0.hex() + "0520", account);
			client.produce_blocks();

			storage = client.storage(m_newAddress, address_0_u256);
			h256 storage_h256_temp2 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256_temp1, storage_h256_temp2);

			//合约创建者 调用合约方法prize
			string prize_data = name2hex("prize");
			prize_data += account3.address.substr(2);
			h256 temp_h256 = (h256)u256(8267);
			prize_data += temp_h256.hex();
			byte temp_byte = 0;
			for (int i = 0; i < 29; i++)
			{
				prize_data += toHex(string(1, temp_byte));
				prize_data += toHex(string(1, temp_byte));
				temp_byte++;
			}

			u256 balance4 = client.balance(Address(account3.address));
			u256 balance_send_1 = client.balance(Address(account.address));
			client.sendTransaction(gasLimit, gasPrice, s_address, "10000", "", account);  //增加奖池金额
			client.produce_blocks();

			u256 balance_send_2 = client.balance(Address(account.address));
			u256 cost_4 = balance_send_1 - balance_send_2;
			cout << "cost_4: " << cost_4 << endl;

			client.sendTransaction(gasLimit, gasPrice, s_address, "", prize_data, account);  //转盘，发奖+发装备
			client.produce_blocks();

			storage = client.storage(m_newAddress, address_3_u256);
			storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[0], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[1], 1);
			BOOST_REQUIRE_EQUAL(storage_h256[2], 36);
			BOOST_REQUIRE_EQUAL(storage_h256[10], 10);
			BOOST_REQUIRE_EQUAL(storage_h256[30], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[31], 176);

			u256 balance5 = client.balance(Address(account3.address));
			u256 prize_value = balance5 - balance4;
			BOOST_REQUIRE_EQUAL(prize_value, u256(8267));
			BOOST_REQUIRE_EQUAL(client.balance(m_newAddress), u256(1733));

			//合约创建者 调用合约方法prize 2
			prize_data = name2hex("prize");
			prize_data += account3.address.substr(2);
			temp_h256 = (h256)u256(0);
			prize_data += temp_h256.hex();
			for (int i = 0; i < 5; i++)
			{
				prize_data += "0102";
			}
			prize_data += "02ee";  //错误的装备数量
			for (int i = 6; i < 29; i++)
			{
				prize_data += "0000";
			}

			balance_1 = client.balance(Address(account.address));
			client.sendTransaction(gasLimit, gasPrice, s_address, "", prize_data, account);  //转盘，发奖+发装备
			client.produce_blocks();
			balance_2 = client.balance(Address(account.address));
			cout << "prize: " << balance_1 - balance_2 << endl;

			storage = client.storage(m_newAddress, address_3_u256);
			storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[0], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[1], 11);
			BOOST_REQUIRE_EQUAL(storage_h256[2], 36);  //数量超过最大上限，无变化
			u256 balance6 = client.balance(Address(account3.address));
			prize_value = balance5 - balance6;
			BOOST_REQUIRE_EQUAL(prize_value, u256(0));
			BOOST_REQUIRE_EQUAL(client.balance(m_newAddress), u256(1733));

			//非合约创建者 调用合约方法prize 无效 存储无变化
			client.sendTransaction(gasLimit, gasPrice, s_address, "", prize_data, account2);
			client.produce_blocks();

			storage = client.storage(m_newAddress, address_3_u256);
			storage_h256 = (h256)storage;
			BOOST_REQUIRE_EQUAL(storage_h256[0], 0);
			BOOST_REQUIRE_EQUAL(storage_h256[1], 11);
			BOOST_REQUIRE_EQUAL(storage_h256[2], 36);
			u256 balance7 = client.balance(Address(account3.address));
			prize_value = balance6 - balance7;
			BOOST_REQUIRE_EQUAL(prize_value, u256(0));
			BOOST_REQUIRE_EQUAL(client.balance(m_newAddress), u256(1733));
		}

		BOOST_AUTO_TEST_SUITE_END()

	}
}